#pragma once

#include "../tagger/types.h"

#include <optional>
#include <string>

namespace phonemis::tokenizer {

struct Token {
  // Required fields - corresponding text
  std::u32string text;

  // Required fields - token context properties
  bool first = false;       // True if it's the first token in a sentence.
  bool whitespace = false;  // True if there is trailing whitespace implicitly included.

  // Optional fields
  // Those can be set using optional, decorator modules such as PoS tagger.
  std::optional<tagger::Tag> tag = std::nullopt;
};

} // namespace phonemis::tokenizer