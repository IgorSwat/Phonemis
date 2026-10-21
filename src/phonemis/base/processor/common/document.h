#pragma once

#include "types.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace phonemis::processor {

/**
 * The text being preprocessed, with lazy edits.
 * 
 * Provides an abstraction which allows algorithms like Num2Word patterns operate freely, without
 * risking that some previous edit affects the future one or the algorithm.
 */
class Document {
public:
  // A span of a text, with the text in its place.
  struct Edit {
    Span span;
    std::u32string text;
  };

  // A piece of the current text: a copy of a span of the original text, or the text which replaced it.
  // A replaced piece maps to its original span only as a whole.
  struct Piece {
    Span original;
    size_t size = 0; // In the current text
    bool replaced = false;
  };

  explicit Document(std::u32string_view text);

  std::u32string_view text() const { return text_; }

  /**
   * Queues a replacement of a span of the current text.
   * @param span a range of the text as it was when the current layer started.
   * @param replacement the new content of that range.
   * @throws std::invalid_argument if the span is out of range or overlaps an already queued edit.
   */
  void replace(Span span, std::u32string replacement);

  /**
   * Applies all queued edits at once.
   */
  void commit();

  /**
   * The committed changes of the original text, in order: each replaced span of the original text with
   * the current text in its place.
   */
  std::vector<Edit> changes() const;

private:
  // The span of the current text an edit really changes: widened to whole replaced pieces it touches.
  Span widen(Span span, std::span<const size_t> starts) const;

  std::u32string text_;
  std::vector<Edit> edits_; // Sorted by position
  std::vector<Piece> pieces_;
  size_t original_size_;
};

} // namespace phonemis::processor
