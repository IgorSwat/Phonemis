#pragma once

#include "tagger.h"
#include "config.h"

#include <span>
#include <string>
#include <unordered_map>
#include <vector>

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

  // Tags are stored as a dense, sorted integer index: tag_names_[id] is the
  // tag string. Every probability table below is indexed by this id, so the
  // Viterbi hot path uses array lookups instead of hashing tag strings.
  std::vector<Tag> tag_names_;

  // start_probs_[id] : log P(first tag == id)
  std::vector<double> start_probs_;

  // transition_probs_[prev * T + curr] : log P(curr | prev), with
  // T == tag_names_.size(). Missing transitions are pre-filled with a small
  // log-epsilon, so the recursion needs no per-cell presence check.
  std::vector<double> transition_probs_;

  // emission_probs_[id] : word -> log P(word | tag == id). Words stay
  // string-keyed because the observation vocabulary is open-ended.
  std::vector<std::unordered_map<std::string, double>> emission_probs_;
};

} // namespace phonemis::tagger