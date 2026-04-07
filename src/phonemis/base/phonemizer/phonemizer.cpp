#include "phonemizer.h"

namespace phonemis::phonemizer {

using tokenizer::Token;

std::u32string Phonemizer::phonemize(std::span<const Token> tokens) {
  std::u32string result;
  result.reserve(5 * tokens.size());

  for (size_t i = 0; i < tokens.size(); ++i) {
    const auto& token = tokens[i];

    // Update context before processing next token
    update_context(tokens, i);

    auto phonemes = phonemize(token);

    if (phonemes.has_value()) {
      result += *phonemes;

      if (token.whitespace) {
        result += U' ';
      }
    }
  }

  return result;
}

} // namespace phonemis::phonemizer