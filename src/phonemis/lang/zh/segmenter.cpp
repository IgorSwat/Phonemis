#include "segmenter.h"
#include "constants.h"

#include <phonemis/utils/conversions.h>
#include <phonemis/utils/io.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace phonemis::zh {

using namespace utils;

namespace {

// jieba's stand-in for log(0).
constexpr double kMinLogProb = -3.14e100;

enum State : size_t { B = 0, E = 1, M = 2, S = 3 };
constexpr std::array<char, 4> kStateNames = {'B', 'E', 'M', 'S'};

// States that can precede each state.
constexpr std::array<std::array<State, 2>, 4> kPrevious = {{
  {E, S},  // B
  {B, M},  // E
  {B, M},  // M
  {E, S},  // S
}};

size_t state_index(const std::string& name) {
  auto it = std::ranges::find(kStateNames, name.at(0));
  if (name.size() != 1 || it == kStateNames.end()) {
    throw std::runtime_error("Segmenter: unknown HMM state " + name);
  }
  return it - kStateNames.begin();
}

} // namespace

Segmenter::Segmenter(const tagger::Config& config) {
  if (!config.data_filepath.has_value()) {
    throw std::runtime_error("Segmenter: data_filepath must be provided in the configuration.");
  }

  // The dictionary goes straight into the flat arrays rather than staying in the parsed
  // JSON, which would take several times its final size.
  offsets_.push_back(0);
  std::string section;
  std::string key;
  auto json = io::load_json(
      config.data_filepath.value(),
      [&](int depth, nlohmann::json::parse_event_t event, nlohmann::json& parsed) {
        using Event = nlohmann::json::parse_event_t;
        if (event == Event::key && depth == 1) {
          section = parsed.get<std::string>();
        } else if (section == "words" && depth == 2) {
          // Dropping the key too keeps an empty entry from staying behind.
          if (event == Event::key) {
            key = parsed.get<std::string>();
            return false;
          }
          if (event == Event::value) {
            words_ += conversions::utf8_to_u32(key);
            offsets_.push_back(static_cast<uint32_t>(words_.size()));
            frequencies_.push_back(parsed.get<uint32_t>());
            return false;
          }
        }
        return true;
      });

  // JSON objects come sorted by UTF-8 bytes, which is code point order; check, since the
  // lookups depend on it.
  for (size_t id = 1; id < frequencies_.size(); id++) {
    if (word(id - 1) >= word(id)) {
      throw std::runtime_error("Segmenter: dictionary words must be unique and sorted.");
    }
  }
  log_total_ = std::log(json.at("total").get<double>());

  const auto& hmm = json.at("hmm");
  start_.fill(kMinLogProb);
  for (const auto& [state, prob] : hmm.at("start").items()) {
    start_[state_index(state)] = prob.get<double>();
  }
  for (auto& row : transitions_) {
    row.fill(kMinLogProb);
  }
  for (const auto& [from, row] : hmm.at("trans").items()) {
    for (const auto& [to, prob] : row.items()) {
      transitions_[state_index(from)][state_index(to)] = prob.get<double>();
    }
  }
  for (const auto& [state, row] : hmm.at("emit").items()) {
    auto& emissions = emissions_[state_index(state)];
    for (const auto& [c, prob] : row.items()) {
      emissions[conversions::utf8_to_u32(c).at(0)] = prob.get<double>();
    }
  }
}

std::u32string_view Segmenter::word(size_t id) const {
  return std::u32string_view{words_}.substr(offsets_[id], offsets_[id + 1] - offsets_[id]);
}

Segmenter::Range Segmenter::extend(Range range, size_t length, char32_t c) const {
  // Within the range, the word made of the shared prefix alone sorts first, and the
  // rest are sorted by their next character.
  auto partition_point = [&](auto before) {
    size_t lo = range.first;
    size_t hi = range.last;
    while (lo < hi) {
      size_t mid = lo + (hi - lo) / 2;
      if (before(word(mid))) {
        lo = mid + 1;
      } else {
        hi = mid;
      }
    }
    return lo;
  };
  return {
    partition_point([&](std::u32string_view w) { return w.size() <= length || w[length] < c; }),
    partition_point([&](std::u32string_view w) { return w.size() <= length || w[length] <= c; }),
  };
}

uint32_t Segmenter::frequency(std::u32string_view text) const {
  Range range{0, frequencies_.size()};
  for (size_t i = 0; i < text.size() && range.first < range.last; i++) {
    range = extend(range, i, text[i]);
  }
  if (range.first < range.last && word(range.first).size() == text.size()) {
    return frequencies_[range.first];
  }
  return 0;
}

std::vector<std::u32string_view> Segmenter::segment(std::u32string_view text) const {
  std::vector<std::u32string_view> words;

  // jieba leaves characters past U+9FD5 out of its runs, so each stands alone.
  size_t start = 0;
  for (size_t i = 0; i <= text.size(); i++) {
    if (i == text.size() || !constants::han::is_segmentable(text[i])) {
      if (i > start) {
        segment_known(text.substr(start, i - start), words);
      }
      if (i < text.size()) {
        words.push_back(text.substr(i, 1));
      }
      start = i + 1;
    }
  }

  return words;
}

void Segmenter::segment_known(std::u32string_view text,
                              std::vector<std::u32string_view>& words) const {
  const size_t n = text.size();

  // Every dictionary word starting at each position, as (end, frequency); a lone
  // character stands in when there is none.
  std::vector<std::vector<std::pair<size_t, uint32_t>>> dag(n);
  for (size_t k = 0; k < n; k++) {
    Range range{0, frequencies_.size()};
    for (size_t i = k; i < n; i++) {
      range = extend(range, i - k, text[i]);
      if (range.first == range.last) {
        break;
      }
      if (word(range.first).size() == i - k + 1 && frequencies_[range.first] > 0) {
        dag[k].emplace_back(i, frequencies_[range.first]);
      }
    }
    if (dag[k].empty()) {
      dag[k].emplace_back(k, 0);
    }
  }

  // The most probable path, from the end backwards; ties go to the longer word.
  std::vector<std::pair<double, size_t>> route(n + 1, {0.0, 0});
  for (size_t idx = n; idx-- > 0;) {
    route[idx] = {-std::numeric_limits<double>::infinity(), 0};
    for (const auto& [end, freq] : dag[idx]) {
      std::pair<double, size_t> candidate{
          std::log(static_cast<double>(freq > 0 ? freq : 1)) - log_total_ + route[end + 1].first,
          end};
      route[idx] = std::max(route[idx], candidate);
    }
  }

  // Single characters on the path gather into a buffer. The HMM splits a buffer the
  // dictionary does not know as a whole; a known one stays as single characters.
  size_t buffer_start = 0;
  size_t buffer_length = 0;
  auto flush = [&]() {
    auto buffer = text.substr(buffer_start, buffer_length);
    if (buffer_length == 1) {
      words.push_back(buffer);
    } else if (buffer_length > 1 && frequency(buffer) == 0) {
      segment_unknown(buffer, words);
    } else {
      for (size_t i = 0; i < buffer_length; i++) {
        words.push_back(buffer.substr(i, 1));
      }
    }
    buffer_length = 0;
  };

  for (size_t x = 0; x < n;) {
    size_t y = route[x].second + 1;
    if (y - x == 1) {
      if (buffer_length == 0) {
        buffer_start = x;
      }
      buffer_length++;
    } else {
      flush();
      words.push_back(text.substr(x, y - x));
    }
    x = y;
  }
  flush();
}

void Segmenter::segment_unknown(std::u32string_view text,
                                std::vector<std::u32string_view>& words) const {
  const size_t n = text.size();
  auto emission = [&](size_t state, char32_t c) {
    auto it = emissions_[state].find(c);
    return it != emissions_[state].end() ? it->second : kMinLogProb;
  };

  // Viterbi; on equal probabilities the later state wins, as in jieba.
  std::vector<std::array<double, 4>> prob(n);
  std::vector<std::array<size_t, 4>> previous(n);
  for (size_t y = 0; y < 4; y++) {
    prob[0][y] = start_[y] + emission(y, text[0]);
  }
  for (size_t t = 1; t < n; t++) {
    for (size_t y = 0; y < 4; y++) {
      double em = emission(y, text[t]);
      std::pair<double, size_t> best{-std::numeric_limits<double>::infinity(), 0};
      for (size_t y0 : kPrevious[y]) {
        best = std::max(best, {prob[t - 1][y0] + transitions_[y0][y] + em, y0});
      }
      prob[t][y] = best.first;
      previous[t][y] = best.second;
    }
  }

  std::vector<size_t> states(n);
  states[n - 1] = std::max(std::pair{prob[n - 1][E], size_t{E}},
                           std::pair{prob[n - 1][S], size_t{S}}).second;
  for (size_t t = n - 1; t > 0; t--) {
    states[t - 1] = previous[t][states[t]];
  }

  size_t begin = 0;
  size_t next = 0;
  for (size_t i = 0; i < n; i++) {
    if (states[i] == B) {
      begin = i;
    } else if (states[i] == E) {
      words.push_back(text.substr(begin, i + 1 - begin));
      next = i + 1;
    } else if (states[i] == S) {
      words.push_back(text.substr(i, 1));
      next = i + 1;
    }
  }
  if (next < n) {
    words.push_back(text.substr(next));
  }
}

} // namespace phonemis::zh
