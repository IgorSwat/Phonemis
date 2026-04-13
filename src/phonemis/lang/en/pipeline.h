#pragma once

#include "lexicon_phonemizer.h"
#include "num2word.h"

#include <phonemis/base/config.h>
#include <phonemis/base/ipipeline.h>
#include <phonemis/base/phonemizer/hybrid_phonemizer.h>
#include <phonemis/base/preprocessor/preprocessor.h>
#include <phonemis/base/tagger/hmm_tagger.h>
#include <phonemis/base/tokenizer/tokenizer.h>

namespace phonemis::en {

class Pipeline : public IPipeline {
public:
  explicit Pipeline(const Config& config);

  /**
   * Phonemizes given english text.
   * @param text an input text (utf-8) to be processed.
   * @returns phonemization (u32) of given input text.
   */
  std::u32string process(std::string_view text) override;

private:
  preprocessor::Preprocessor preprocessor_;
  tokenizer::Tokenizer tokenizer_;
  tagger::HMMTagger tagger_;
  
  // Hybrid phonemizer using English-specific lexicon and generic neural model
  phonemizer::HybridPhonemizer<en::LexiconPhonemizer, phonemizer::nn::NeuralPhonemizer> phonemizer_;
};

} // namespace phonemis::en
