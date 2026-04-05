#pragma once

#include "types.h"
#include "../tokenizer/token.h"

#include <span>

namespace phonemis::tagger {

using tokenizer::Token;

// By tagger we mean a part-of-speech tagger by default.
// Since there could be many different ways of tagging, we use an interface
// which should be implemented by all kinds of taggers.
class Tagger {
public:
  Tagger() = default;

  // Performs in-place tagging on given token vector.
  void tag(std::span<Token> tokens) const;

  // A template method to be implemented by derived classes.
  virtual void tagSentence(std::span<Token> sentence) const = 0;
};

} // namespace phonemis::tagger