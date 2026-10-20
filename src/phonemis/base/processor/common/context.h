#pragma once

#include "types.h"

#include <string>
#include <string_view>
#include <vector>

namespace phonemis::processor {

/**
 * The surroundings of a span of a text: the words before and after it.
 * 
 * A word the span starts or ends inside of counts with its part outside the span.
 */
class Context {
public:
  Context(std::u32string_view text, Span span);

  /**
   * The k-th word before the span, lowercased and without surrounding punctuation.
   * Empty if there is no such word.
   */
  std::u32string before(size_t k = 1) const;

  /**
   * The k-th word after the span, case kept, without surrounding punctuation.
   * Empty if there is no such word, or if a punctuation mark directly follows the span.
   */
  std::u32string after(size_t k = 1) const;

  std::u32string_view head() const; // The text before the span
  std::u32string_view tail() const; // The text after the span
  std::u32string_view text() const; // The span itself

  Span span() const { return span_; }

private:
  std::u32string_view text_;
  Span span_;
};

// Whitespace-separated words of a text, like Python's str.split().
std::vector<std::u32string_view> words_of(std::u32string_view text);

// A word without the punctuation around it.
std::u32string_view stripped(std::u32string_view word);

} // namespace phonemis::processor
