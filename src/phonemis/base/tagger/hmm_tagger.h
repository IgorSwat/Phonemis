#pragma once

#include "tagger.h"

#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace phonemis::tagger {

using tokenizer::Token;

// A HMM (Hidden Markov Model) tagger represents a family of taggers, which
// utilize precalculated HMM statistic tables to guess the tag for a given word in a sequence.
// Utilizes Viterbi algorithm.
class HMMTagger : public Tagger {
public:
  // The input data file should be a .json file with a specific HMM structure
  // (see the implementation in hmm_tagger.cpp).
  explicit HMMTagger(const std::string& hmm_data_path);

  void tagSentence(std::span<Token> sentence) const override;

private:
  // Set of possible tags (states)
  std::unordered_set<Tag> tags_;

  // Probability maps - loaded from the input json file.
  std::unordered_map<Tag, double> start_probs_ = {};
  std::unordered_map<Tag, std::unordered_map<std::string, double>>
    emission_probs_ = {}; // utf-8 instead of u32 to save memory
  std::unordered_map<Tag, std::unordered_map<Tag, double>>
    transition_probs_ = {};
};

} // namespace phonemis::tagger