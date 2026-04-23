#pragma once

#include "constants.h"

#include <phonemis/base/config.h>
#include <phonemis/base/ipipeline.h>
#include <phonemis/base/phonemizer/hybrid_phonemizer.h>
#include <phonemis/base/phonemizer/lexicon_phonemizer.h>
#include <phonemis/base/processor/processor.h>
#include <phonemis/base/processor/sanitizer_layer.h>
#include <phonemis/base/processor/trim_layer.h>
#include <phonemis/base/tokenizer/tokenizer.h>

#include <memory>

namespace phonemis::pl {

class Pipeline : public IPipeline {
public:
  explicit Pipeline(const Config& config)
      : tokenizer_(&constants::tokenizer::kSpecialCharacters,
                   &constants::tokenizer::kExceptions),
        phonemizer_(config.phonemizer) {

    // 1. Setup Preprocessing layers
    preprocessor_.add_layer(std::make_unique<processor::TrimLayer>());

    // 2. Setup Postprocessing layers
    postprocessor_.add_layer(std::make_unique<processor::SanitizerLayer>(
        [](char32_t) { return true; },
        [](char32_t c) {
          if (constants::sanitizer::kCharReplacements.contains(c)) {
            return constants::sanitizer::kCharReplacements.at(c);
          }
          return c;
        }));
  }

  // Performs a preprocessing stage of the pipeline.
  std::u32string preprocess(const std::u32string& input) override {
    return preprocessor_.process(input);
  }

  // Performs a middle stage of the pipeline - including phonemization.
  std::u32string process(const std::u32string& input) override {
    // 1. Tokenize
    auto tokens = tokenizer_.tokenize(input);

    // 2. Generate phonemes (Lexicon with Neural fallback)
    return phonemizer_.phonemize(tokens);
  }

  // Performs a postprocessing stage of the pipeline.
  std::u32string postprocess(const std::u32string& input) override {
    return postprocessor_.process(input);
  }

private:
  // Required submodules
  processor::Preprocessor preprocessor_;
  processor::Preprocessor postprocessor_;
  tokenizer::Tokenizer tokenizer_;
  phonemizer::HybridPhonemizer<phonemizer::LexiconPhonemizer, phonemizer::nn::NeuralPhonemizer> phonemizer_;
};

} // namespace phonemis::pl
