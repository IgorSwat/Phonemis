#pragma once

#include "types.h"
#include "../tokenizer/token.h"

#include <span>

namespace phonemis::tagger {

using tokenizer::Token;

/**
 * Interface for token tagging (e.g., PoS, lemmatization).
 * Implementations add metadata to tokens within a sentence context.
 */
class Tagger {
public:
  virtual ~Tagger() = default;

  /**
   * Orchestrates tagging of a token stream by splitting it into sentences.
   * @param tokens The full token stream to be processed in-place.
   */
  void tag(std::span<Token> tokens) const;

  /**
   * To be implemented by derived classes: applies tags to a single sentence.
   * @param sentence A span of tokens representing one logical sentence.
   */
  virtual void tag_sentence(std::span<Token> sentence) const = 0;
};

} // namespace phonemis::tagger