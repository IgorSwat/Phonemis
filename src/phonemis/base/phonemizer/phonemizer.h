#pragma once

#include "types.h"
#include "../tokenizer/token.h"

#include <span>
#include <string>

namespace phonemis::phonemizer {

using tokenizer::Token;

/**
 * Interface for phonemization pipeline.
 * Processes tokens sequentially, supporting context-aware transitions.
 */
class Phonemizer {
public:
  virtual ~Phonemizer() = default;

  /**
   * Main entry point: converts a sequence of tokens into a phoneme string.
   * @param tokens Sequence of tokens to process.
   * @return A consolidated phoneme string in UTF-32.
   */
  std::u32string phonemize(std::span<const Token> tokens);

  /**
   * Phonemize a single token. Returns nullopt if no mapping exists.
   */
  virtual std::optional<std::u32string> phonemize(const Token& token) const = 0;

  /**
   * Updates internal state before phonemizing the current token (idx).
   */
  virtual void update_context(size_t idx, std::span<const Token> tokens) = 0;
};

} // namespace phonemis::phonemizer