#pragma once

#include "tagger.h"
#include "config.h"

#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace phonemis::tagger {

using tokenizer::Token;

/**
 * HMM (Hidden Markov Model) Tagger implementation.
 * Uses the Viterbi algorithm to determine the most likely sequence of tags (hidden states)
 * based on observed words and precomputed probability tables.
 */
class HMMTagger : public Tagger {
public:
  explicit HMMTagger(const Config& config);

  /**
   * Applies the Viterbi algorithm to assign tags to a sentence.
   * @param sentence Span of tokens to tag in-place.
   */
  void tag_sentence(std::span<Token> sentence) const override;

protected:
  /**
   * Returns true if the (first) word should be transformed to lowercase
   * before processing.
   */
  virtual bool should_lowercase(std::u32string_view word) const { return false; }

  // Set of all possible tags extracted from the model
  std::unordered_set<Tag> tags_;

  // Probability tables
  std::unordered_map<Tag, double> start_probs_;
  // Tag -> Word -> Probability (UTF-8 word keys to save memory)
  std::unordered_map<Tag, std::unordered_map<std::string, double>> emission_probs_;
  // PrevTag -> NextTag -> Probability
  std::unordered_map<Tag, std::unordered_map<Tag, double>> transition_probs_;
};

} // namespace phonemis::tagger