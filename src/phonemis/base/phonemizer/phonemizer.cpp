#include "constants.h"
#include "phonemizer.h"

namespace phonemis::phonemizer {

using tokenizer::Token;

std::u32string Phonemizer::phonemize(std::span<const Token> tokens) {
  using namespace phonemis::phonemizer::constants;
  
  std::u32string result;
  result.reserve(5 * tokens.size());

  for (size_t i = 0; i < tokens.size(); ++i) {
    const auto& token = tokens[i];

    // Update context before processing current token
    update_context(i, tokens);

    auto phonemes = phonemize(token);

    if (phonemes.has_value()) {
      result += *phonemes;
    }

    // Handle reimaining punctation characters which tend to not have it's own phonemization,
    // but affect the local phonemes from surrounding characters.
    bool is_single_char = token.text.size() == 1;
    bool is_punctation = is_single_char && puncts::kPunctations.contains(token.text[0]);
    bool is_dot_or_hyphen = is_punctation && (token.text[0] == U'.' || token.text[0] == U'-');
    bool is_last_token = i == tokens.size() - 1;
    if (!phonemes.has_value() && (!is_dot_or_hyphen || !token.whitespace || is_last_token)) {
      result += std::u32string(1, token.text[0]);
    }

    // Add trailing whitespace if present.
    if (token.whitespace) {
      result += U' ';
    }
  }

  return result;
}

} // namespace phonemis::phonemizer