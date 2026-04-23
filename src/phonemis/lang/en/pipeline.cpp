#include "pipeline.h"
#include "constants.h"

#include <phonemis/base/processor/trim_layer.h>
#include <phonemis/utils/conversions.h>

namespace phonemis::en {

using namespace utils;

Pipeline::Pipeline(const Config& config)
    : tokenizer_(&constants::tokenizer::kSpecialCharacters,
                 &constants::tokenizer::kExceptions),
      phonemizer_(config.phonemizer) {
  
  // 1. Setup Preprocessing layers
  preprocessor_.add_layer(std::make_unique<processor::TrimLayer>());
  preprocessor_.add_layer(std::make_unique<Num2Word>());

  // 2. Setup PoS tagger if specified
  if (config.tagger.has_value()) {
    tagger_ = std::make_unique<tagger::HMMTagger>(*config.tagger);
  }
}

std::u32string Pipeline::preprocess(const std::u32string& input) {
  return preprocessor_.process(input);
}

std::u32string Pipeline::process(const std::u32string& input) {
  // 1. Tokenize
  auto tokens = tokenizer_.tokenize(input);

  // 2. Apply Part-of-Speech tags
  if (tagger_) {
    tagger_->tag(tokens);
  }

  // 3. Generate phonemes (Lexicon with Neural fallback)
  return phonemizer_.phonemize(tokens);
}

std::u32string Pipeline::postprocess(const std::u32string& input) {
  // No postprocessing
  return input;
}

} // namespace phonemis::en
