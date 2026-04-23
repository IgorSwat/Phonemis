#pragma once

#include "lexicon_phonemizer.h"
#include "num2word.h"

#include <phonemis/base/config.h>
#include <phonemis/base/ipipeline.h>
#include <phonemis/base/phonemizer/hybrid_phonemizer.h>
#include <phonemis/base/processor/processor.h>
#include <phonemis/base/tagger/hmm_tagger.h>
#include <phonemis/base/tokenizer/tokenizer.h>

#include <memory>

namespace phonemis::en {

class Pipeline : public IPipeline {
public:
  explicit Pipeline(const Config& config);

  // Performs a preprocessing stage of the pipeline.
  std::u32string preprocess(const std::u32string& input) override;

  // Performs a middle stage of the pipeline - including phonemization.
  std::u32string process(const std::u32string& input) override;

  // Performs a postprocessing stage of the pipeline.
  std::u32string postprocess(const std::u32string& input) override;

private:
  // Required submodules
  processor::Preprocessor preprocessor_;
  tokenizer::Tokenizer tokenizer_;
  phonemizer::HybridPhonemizer<en::LexiconPhonemizer, phonemizer::nn::NeuralPhonemizer> phonemizer_;

  // Optional submodules
  std::unique_ptr<tagger::HMMTagger> tagger_ = nullptr;
};

} // namespace phonemis::en
