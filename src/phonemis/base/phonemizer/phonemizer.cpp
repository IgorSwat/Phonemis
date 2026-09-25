#include "constants.h"
#include "phonemizer.h"

namespace phonemis::phonemizer {

using tokenizer::Token;

std::u32string Phonemizer::phonemize(std::span<const Token> tokens) {
  return join(phonemize_segments(tokens));
}

std::vector<Segment> Phonemizer::phonemize_segments(std::span<const Token> tokens) {
  using namespace phonemis::phonemizer::constants;

  std::vector<Segment> segments;
  segments.reserve(tokens.size());

  for (size_t i = 0; i < tokens.size(); ++i) {
    const auto& token = tokens[i];
    Segment segment{.begin = token.begin, .end = token.end, .whitespace = token.whitespace};

    // 1. Context Update
    update_context(i, tokens);

    // 2. Core Phonemization
    auto phonemes = phonemize(token);
    if (phonemes) {
      segment.phonemes = std::move(*phonemes);
    }

    // 3. Fallback/Punctuation Logic
    // Handles punctuation that doesn't have explicit phonemes but must be preserved.
    // Logic: If not phonemized, append raw character if it's punctuation,
    // unless it's a non-trailing dot/hyphen with whitespace (abbreviation/word-break case).
    if (!phonemes.has_value() && !token.text.empty()) {
      char32_t first_char = token.text[0];
      bool is_punct = (token.text.size() == 1) && puncts::kPunctuations.contains(first_char);
      bool is_soft_punct = (first_char == U'.' || first_char == U'-');
      bool is_last = (i == tokens.size() - 1);

      if (is_punct && (!is_soft_punct || token.whitespace || is_last)) {
        segment.phonemes.push_back(first_char);
      }
    }

    segments.push_back(std::move(segment));
  }

  return segments;
}

} // namespace phonemis::phonemizer
