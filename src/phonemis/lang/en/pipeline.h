#pragma once

#include "lexicon_phonemizer.h"
#include "num2word.h"

#include <phonemis/base/config.h>
#include <phonemis/base/ipipeline.h>
#include <phonemis/base/phonemizer/hybrid_phonemizer.h>
#include <phonemis/base/preprocessor/preprocessor.h>
#include <phonemis/base/tagger/hmm_tagger.h>
#include <phonemis/base/tokenizer/tokenizer.h>

#include <memory>

namespace phonemis::en {

class Pipeline : public IPipeline {
public:
  explicit Pipeline(const Config& config);

  /**
   * Phonemizes given english text.
   * @param text an input text (utf-8) to be processed.
   * @returns phonemization (u32) of given input text.
   * @note This method is not thread-safe. Concurrent calls on the same
   * instance should be avoided.
   */
  std::u32string process(std::string_view text) override;

private:
  // Required submodules
  preprocessor::Preprocessor preprocessor_;
  tokenizer::Tokenizer tokenizer_;
  phonemizer::HybridPhonemizer<en::LexiconPhonemizer, phonemizer::nn::NeuralPhonemizer> phonemizer_;

  // Optional submodules
  std::unique_ptr<tagger::HMMTagger> tagger_ = nullptr;
};

} // namespace phonemis::en
