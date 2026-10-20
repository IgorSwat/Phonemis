#include "layers/normalize/layer.h"
#include "layers/term2word/layer.h"
#include "layers/trim/layer.h"
#include "preprocessor.h"

namespace phonemis::processor {

namespace {
// A table with extra entries, which replace those with the same key
Readings merged(Readings table, const Readings& extra) {
  for (const auto& [key, readings] : extra) {
    table.insert_or_assign(key, readings);
  }
  return table;
}
} // namespace

Preprocessor make_preprocessor(const Config& config, const Language& language, std::unique_ptr<Layer> numbers) {
  Preprocessor preprocessor;

  // Characters normalized first, so that every layer sees one form of each
  if (config.normalize) {
    preprocessor.add_layer(std::make_unique<normalize::NormalizeLayer>(
        language.characters ? *language.characters : normalize::constants::kCharacters));
  }

  if (config.trim) {
    preprocessor.add_layer(std::make_unique<trim::TrimLayer>());
  }

  // Terms read before numbers, so that "nr 5" still sees its digit
  if (config.terms && (language.terms || !config.abbreviations.empty())) {
    term2word::Terms terms = language.terms ? *language.terms : term2word::Terms{};
    terms.abbreviations = merged(std::move(terms.abbreviations), config.abbreviations);
    preprocessor.add_layer(std::make_unique<term2word::Term2WordLayer>(std::move(terms)));
  }

  if (numbers) {
    preprocessor.add_layer(std::move(numbers));
  }

  // Symbols read after numbers, which read the ones they own: 20%, $5
  if (config.symbols && (language.symbols || !config.symbol_words.empty())) {
    sym2word::Symbols symbols = language.symbols ? *language.symbols : sym2word::Symbols{};
    preprocessor.add_layer(std::make_unique<sym2word::Sym2WordLayer>(merged(std::move(symbols), config.symbol_words)));
  }

  if (config.sanitize && language.replacements) {
    const Replacements& replacements = *language.replacements;
    auto mapper = [&replacements](char32_t c) {
      auto it = replacements.find(c);
      return it != replacements.end() ? it->second : c;
    };
    preprocessor.add_layer(std::make_unique<sanitizer::SanitizerLayer>([](char32_t) { return true; }, mapper));
  }

  return preprocessor;
}

} // namespace phonemis::processor
