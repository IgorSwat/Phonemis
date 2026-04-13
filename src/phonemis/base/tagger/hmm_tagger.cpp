#include "hmm_tagger.h"
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/io.h>
#include <phonemis/utils/unicode.h>

#include <algorithm>
#include <stdexcept>

namespace phonemis::tagger {

using namespace utils;

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

  // 1. Start Probabilities (defines the tag set)
  for (const auto& [tag, prob] : json["start_prob"].items()) {
    tags_.insert(tag);
    start_probs_[tag] = prob.get<double>();
  }

  // 2. Emission Probabilities: P(Word | Tag)
  for (const auto& [tag, words] : json["emission"].items()) {
    for (const auto& [word, prob] : words.items()) {
      emission_probs_[tag][word] = prob.get<double>();
    }
  }

  // 3. Transition Probabilities: P(Tag_i | Tag_{i-1})
  for (const auto& [prev_tag, next_tags] : json["transition"].items()) {
    for (const auto& [next_tag, prob] : next_tags.items()) {
      transition_probs_[prev_tag][next_tag] = prob.get<double>();
    }
  }
}

void HMMTagger::tag_sentence(std::span<Token> sentence) const {
  if (sentence.empty()) return;

  constexpr double EPSILON = 1e-6; // Fallback for unseen observations

  // Viterbi tables: [step][state]
  std::vector<std::unordered_map<Tag, double>> dp(sentence.size());
  std::vector<std::unordered_map<Tag, Tag>> backtrace(sentence.size());

  // Step 0: Initialization
  std::string first_word = conversions::u32_to_utf8(sentence[0].text);
  for (const auto& tag : tags_) {
    double emit_p = emission_probs_.at(tag).contains(first_word) 
                    ? emission_probs_.at(tag).at(first_word) : EPSILON;
    dp[0][tag] = start_probs_.at(tag) * emit_p;

    // Check lowercase variant for better robustness
    if (unicode::isalpha(sentence[0].text[0])) {
      std::u32string lower_text = sentence[0].text;
      lower_text[0] = unicode::tolower(lower_text[0]);
      std::string lower_utf8 = conversions::u32_to_utf8(lower_text);

      double lower_emit_p = emission_probs_.at(tag).contains(lower_utf8)
                            ? emission_probs_.at(tag).at(lower_utf8) : EPSILON;
      dp[0][tag] = std::max(dp[0][tag], start_probs_.at(tag) * lower_emit_p);
    }
  }

  // Steps 1..N: Viterbi Recursion
  for (size_t t = 1; t < sentence.size(); ++t) {
    std::string word = conversions::u32_to_utf8(sentence[t].text);

    for (const auto& curr_tag : tags_) {
      double max_p = -1.0;
      Tag best_prev;

      double emit_p = emission_probs_.at(curr_tag).contains(word) 
                      ? emission_probs_.at(curr_tag).at(word) : EPSILON;

      for (const auto& prev_tag : tags_) {
        double trans_p = transition_probs_.at(prev_tag).contains(curr_tag)
                         ? transition_probs_.at(prev_tag).at(curr_tag) : EPSILON;
        
        double p = dp[t - 1][prev_tag] * trans_p * emit_p;
        if (p > max_p) {
          max_p = p;
          best_prev = prev_tag;
        }
      }
      dp[t][curr_tag] = max_p;
      backtrace[t][curr_tag] = best_prev;
    }
  }

  // Backtracking: Find most likely end state and reverse-walk the path
  auto last_idx = sentence.size() - 1;
  Tag current_tag = *std::max_element(tags_.begin(), tags_.end(), [&](const Tag& a, const Tag& b) {
    return dp[last_idx][a] < dp[last_idx][b];
  });

  for (size_t t = last_idx; ; --t) {
    sentence[t].tag = current_tag;
    if (t == 0) break;
    current_tag = backtrace[t][current_tag];
  }
}

} // namespace phonemis::tagger