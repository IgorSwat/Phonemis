#pragma once

#include <cstddef>
#include <span>
#include <string>

namespace phonemis {

/**
 * A piece of phonemized text, aligned with the text it was produced from.
 * Joining the phonemes of all segments (with a space after each segment
 * marked with `whitespace`) gives the phonemization of the whole text.
 */
struct Segment {
  // Position of the source text, as a half-open [begin, end) range of characters.
  size_t begin = 0;
  size_t end = 0;

  // Phonemes produced from the source text. Empty if it has no pronunciation.
  std::u32string phonemes;

  // True if the phonemes are followed by a space.
  bool whitespace = false;
};

// Joins the segments into a single phonemization.
std::u32string join(std::span<const Segment> segments);

} // namespace phonemis
