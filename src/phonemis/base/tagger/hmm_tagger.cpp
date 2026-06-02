#include "hmm_tagger.h"
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/io.h>
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace phonemis::tagger {

using namespace utils;

namespace {
// Small constant for unseen observations/transitions in log space.
const double LOG_EPSILON = std::log(1e-7);
}  // namespace

HMMTagger::HMMTagger(const Config& config) {
  if (!config.data_filepath.has_value()) {
    throw std::runtime_error("HMMTagger: data_filepath must be provided in the configuration.");
  }

  nlohmann::json json = utils::io::load_json(config.data_filepath.value());

  // Validate required HMM components
  const std::vector<std::string> required = {"start_prob", "emission", "transition"};
  for (const auto& key : required) {
    if (!json.contains(key) || !json[key].is_object()) {
      throw std::invalid_argument("HMMTagger: JSON missing or invalid object '" + key + "'");
    }
  }

  // Parses a JSON probability into log-space, rejecting values outside [0, 1]
  // (including negatives and NaN, which would make std::log produce NaN and
  // silently corrupt the Viterbi computation). A stored 0 maps to -inf, which
  // is valid in log space (an impossible event).
  auto to_log_prob = [](const nlohmann::json& node, const std::string& context) -> double {
    const double p = node.get<double>();
    if (!(p >= 0.0 && p <= 1.0)) {
      throw std::invalid_argument(
          "HMMTagger: probability for '" + context + "' must be in [0, 1], got " + std::to_string(p));
    }
    return std::log(p);
  };

  // 1. The start-probability keys define the tag set. Assign each tag a dense
  //    integer id; sorting keeps the id assignment (and thus Viterbi
  //    tie-breaking) deterministic across runs.
  for (const auto& [tag, prob] : json["start_prob"].items()) {
    tag_names_.push_back(tag);
  }
  std::ranges::sort(tag_names_);

  const size_t T = tag_names_.size();
  std::unordered_map<Tag, size_t> tag_id;
  tag_id.reserve(T);
  for (size_t id = 0; id < T; ++id) {
    tag_id.emplace(tag_names_[id], id);
  }

  // 2. Start probabilities, indexed by tag id (stored as log probabilities).
  start_probs_.assign(T, LOG_EPSILON);
  for (const auto& [tag, prob] : json["start_prob"].items()) {
    start_probs_[tag_id.at(tag)] = to_log_prob(prob, tag);
  }

  // 3. Emission probabilities: P(Word | Tag), one word map per tag id. Tags
  //    not present in the start set (and therefore not in the model's state
  //    space) are ignored, mirroring the original behavior.
  emission_probs_.resize(T);
  for (const auto& [tag, words] : json["emission"].items()) {
    auto it = tag_id.find(tag);
    if (it == tag_id.end()) continue;
    for (const auto& [word, prob] : words.items()) {
      emission_probs_[it->second][word] = to_log_prob(prob, tag + "/" + word);
    }
  }

  // 4. Transition probabilities: P(Tag_i | Tag_{i-1}) as a flat T x T matrix,
  //    pre-filled with LOG_EPSILON so missing transitions need no runtime check.
  transition_probs_.assign(T * T, LOG_EPSILON);
  for (const auto& [prev_tag, next_tags] : json["transition"].items()) {
    auto prev_it = tag_id.find(prev_tag);
    if (prev_it == tag_id.end()) continue;
    for (const auto& [next_tag, prob] : next_tags.items()) {
      auto next_it = tag_id.find(next_tag);
      if (next_it == tag_id.end()) continue;
      transition_probs_[prev_it->second * T + next_it->second] =
          to_log_prob(prob, prev_tag + "->" + next_tag);
    }
  }
}

void HMMTagger::tag_sentence(std::span<Token> sentence) const {
  if (sentence.empty()) return;

  const size_t T = tag_names_.size();
  if (T == 0) return;  // No model loaded -> nothing to tag.

  const size_t n = sentence.size();

  // The recursion only ever reads the previous column, so dp keeps just two
  // rolling columns. backtrace keeps the full [step][state] grid of tag ids.
  std::vector<double> dp_prev(T), dp_curr(T);
  std::vector<std::vector<int>> backtrace(n, std::vector<int>(T, 0));

  // log P(word | tag id), falling back to LOG_EPSILON for unseen words.
  auto emit_log_prob = [this](size_t tag_id, const std::string& word) {
    const auto& word_map = emission_probs_[tag_id];
    auto it = word_map.find(word);
    return it != word_map.end() ? it->second : LOG_EPSILON;
  };

  // Step 0: Initialization
  std::string first_word = conversions::u32_to_utf8(
    should_lowercase(sentence[0].text) ?
      strings::to_lower(sentence[0].text) :
      sentence[0].text
  );
  for (size_t tag = 0; tag < T; ++tag) {
    dp_prev[tag] = start_probs_[tag] + emit_log_prob(tag, first_word);
  }

  // Steps 1..n-1: Viterbi Recursion
  for (size_t t = 1; t < n; ++t) {
    std::string word = conversions::u32_to_utf8(sentence[t].text);

    for (size_t curr = 0; curr < T; ++curr) {
      const double emit_p = emit_log_prob(curr, word);

      double max_p = -std::numeric_limits<double>::infinity();
      int best_prev = 0;
      for (size_t prev = 0; prev < T; ++prev) {
        const double p = dp_prev[prev] + transition_probs_[prev * T + curr] + emit_p;
        if (p > max_p) {
          max_p = p;
          best_prev = static_cast<int>(prev);
        }
      }
      dp_curr[curr] = max_p;
      backtrace[t][curr] = best_prev;
    }
    std::swap(dp_prev, dp_curr);  // dp_prev now holds the column for step t.
  }

  // Backtracking: pick the most likely final state (dp_prev holds the last
  // column after the final swap), then reverse-walk the path.
  int current = static_cast<int>(std::ranges::max_element(dp_prev) - dp_prev.begin());

  for (size_t t = n - 1; ; --t) {
    sentence[t].tag = tag_names_[current];
    if (t == 0) break;
    current = backtrace[t][current];
  }
}

} // namespace phonemis::tagger