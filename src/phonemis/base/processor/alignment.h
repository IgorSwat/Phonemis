#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace phonemis::processor {

// A half-open [begin, end) range of character positions.
struct Span {
  size_t begin = 0;
  size_t end = 0;

  bool operator==(const Span&) const = default;
};

/**
 * Maps a transformed text back onto its input.
 * The i-th element is the input span the i-th output character was derived from.
 * Characters produced by a single replacement (e.g. "twenty five" from "25")
 * all share the span of the replaced input.
 */
using Alignment = std::vector<Span>;

// Returns the alignment of a text of given size onto itself.
Alignment identity(size_t size);

/**
 * Resolves the [begin, end) range of an aligned text to the input span it covers.
 * An empty range resolves to an empty span at the matching input position.
 */
Span source_span(const Alignment& alignment, size_t begin, size_t end);

/**
 * Chains two alignments: `outer` maps C onto B and `inner` maps B onto A,
 * the result maps C onto A.
 */
Alignment compose(const Alignment& outer, const Alignment& inner);

/**
 * Builds a transformed text while (optionally) recording its alignment onto the input.
 * With a null alignment it only builds the text, at no extra cost.
 */
class AlignedWriter {
public:
  explicit AlignedWriter(Alignment* alignment, size_t reserve = 0);

  // Appends input[begin, end) verbatim, mapping each character onto itself.
  void copy(std::u32string_view input, size_t begin, size_t end);

  // Appends a replacement text, mapping all its characters onto input[begin, end).
  void replace(std::u32string_view text, size_t begin, size_t end);

  // Appends a single character derived from input[begin, end).
  void push(char32_t c, size_t begin, size_t end);

  // Removes the last character.
  void pop();

  const std::u32string& text() const { return text_; }
  std::u32string take() { return std::move(text_); }

private:
  std::u32string text_;
  Alignment* alignment_;
};

} // namespace phonemis::processor
