#pragma once

#include "config.h"
#include "layers/normalize/types.h"
#include "layers/sanitizer/layer.h"
#include "layers/sym2word/layer.h"
#include "layers/term2word/types.h"
#include "processor.h"

#include <concepts>
#include <memory>
#include <unordered_map>

namespace phonemis::processor {

// Characters replaced by others before phonemization.
using Replacements = std::unordered_map<char32_t, char32_t>;

// The parts of a language's preprocessing, from its constants.
struct Language {
  const normalize::Characters* characters = nullptr; // Its normalized characters, if not the default ones
  const term2word::Terms* terms = nullptr;           // Its abbreviations and web words, if any
  const sym2word::Symbols* symbols = nullptr;        // Its symbol words, if any
  const Replacements* replacements = nullptr;        // Its character replacements, if any: ’ -> '
};

/**
 * Builds a language's preprocessing from the layers its configuration enables, in their fixed order:
 * Normalize, Trim, Term2Word, Num2Word, Sym2Word, Sanitizer.
 * @param config which layers run, and words added to the language's tables.
 * @param language the language's tables.
 * @param numbers the language's Num2Word layer, or nullptr if numbers are not read.
 */
Preprocessor make_preprocessor(const Config& config, const Language& language, std::unique_ptr<Layer> numbers);

/**
 * Builds a language's preprocessing, as above.
 * @tparam N2W the language's Num2Word layer, created only if numbers are read.
 */
template <std::derived_from<Layer> N2W>
Preprocessor make_preprocessor(const Config& config, const Language& language) {
  return make_preprocessor(config, language, config.numbers ? std::make_unique<N2W>() : nullptr);
}

} // namespace phonemis::processor
