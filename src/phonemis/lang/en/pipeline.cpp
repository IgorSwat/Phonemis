#include "pipeline.h"
#include "constants.h"

#include <phonemis/base/preprocessor/trim_layer.h>
#include <phonemis/utils/conversions.h>

namespace phonemis::en {

using namespace utils;

Pipeline::Pipeline(const Config& config)
    : tokenizer_(&constants::tokenizer::kSpecialCharacters,
                 &constants::tokenizer::kExceptions),
      tagger_(config.tagger),
      phonemizer_(config.phonemizer) {
  
  // 1. Setup Preprocessing layers
  preprocessor_.add_layer(std::make_unique<preprocessor::TrimLayer>());
  preprocessor_.add_layer(std::make_unique<Num2Word>());
}

std::u32string Pipeline::process(std::string_view text) {
  // 1. Clean and normalize input
  // num2word + trim + (optionally) unicode related normalizations
  auto preprocessed = preprocessor_.process(conversions::utf8_to_u32(text));

  // 2. Tokenize
  auto tokens = tokenizer_.tokenize(preprocessed);

  // 3. Apply Part-of-Speech tags
  tagger_.tag(tokens);

  // 4. Generate phonemes (Lexicon with Neural fallback)
  return phonemizer_.phonemize(tokens);
}

} // namespace phonemis::en
