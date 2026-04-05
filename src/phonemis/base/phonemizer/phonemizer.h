#pragma once

#include "types.h"
#include "../tokenizer/token.h"

#include <span>
#include <string>

namespace phonemis::phonemizer {

using tokenizer::Token;

/**
 * A common interface for all different phonemizers.
 * 
 * Note that phonemization in general is a sequential task consisting of
* multiple steps - subphonemizations of consecutive tokens.
 */
class Phonemizer {
public:
  Phonemizer() = default;

  /**
   * This is the main API method. It performs a sequential, token-by-token phonemization.
   * 
   * Before each token gets processed, there is a possibility of updating the 
   * phonemizer's context (if implemented). This way we can handle complex relationships between words,
   * which are difficult to express only by Token's optional fields.
   * 
   * @param tokens tokenized input to be phonemized.
   * @returns a single string with all the obtained phonemes.
   */
  std::u32string phonemize(std::span<const Token> tokens);

  // Template methods to be implemented by derived classes.
  virtual std::optional<std::u32string> phonemize(const Token& token) const = 0;  // Returns std::nullopt if not able to phonemize the token.
  virtual void updateContext(std::span<const Token> tokens, size_t nextTokenID) = 0;  // Called before each token gets phonemized.
};

} // namespace phonemis::phonemizer