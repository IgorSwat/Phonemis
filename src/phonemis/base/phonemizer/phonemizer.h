#pragma once

#include "../segment.h"
#include "../tokenizer/token.h"

#include <span>
#include <string>
#include <vector>

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
   * @details Non-const as it manages stateful context updates across the sequence.
   * @param tokens Sequence to process.
   */
  std::u32string phonemize(std::span<const Token> tokens);

  /**
   * Converts a sequence of tokens into one segment per token, positioned like the token.
   * Joining the segments gives the result of phonemize().
   * @param tokens Sequence to process.
   */
  std::vector<Segment> phonemize_segments(std::span<const Token> tokens);

  /**
   * Phonemize a single token. Returns nullopt if no mapping exists.
   * @details Const as it is a pure lookup based on current internal state.
   */
  virtual std::optional<std::u32string> phonemize(const Token& token) const = 0;

  /**
   * Updates internal state before phonemizing the current token (idx).
   */
  virtual void update_context(size_t idx, std::span<const Token> tokens) {};
};

} // namespace phonemis::phonemizer