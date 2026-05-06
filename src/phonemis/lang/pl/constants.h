#pragma once

#include <phonemis/base/tokenizer/types.h>

#include <unordered_map>

namespace phonemis::pl::constants {

// --- Sanitization Rules ---
namespace sanitizer {
  // Character replacements for postprocessing (e.g. ʑ → ʒ).
  inline const std::unordered_map<char32_t, char32_t> kCharReplacements = {
    {U'ʑ', U'ʒ'}
  };
} // namespace sanitizer

// --- Tokenization Rules ---
namespace tokenizer {
  using ::phonemis::tokenizer::split::Rule;
  using ::phonemis::tokenizer::split::Exceptions;

  inline const std::unordered_map<char32_t, Rule> kSpecialCharacters = {
    {U'\'', Rule::KEEP_WITH_RIGHT},
    {U'-',  Rule::TOTAL_DIVIDE},
    {U'.',  Rule::TOTAL_DIVIDE},
    {U':',  Rule::TOTAL_DIVIDE}
  };

  inline const Exceptions kExceptions = {};
} // namespace tokenizer

} // namespace phonemis::pl::constants
