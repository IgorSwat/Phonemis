#include "segment.h"

namespace phonemis {

std::u32string join(std::span<const Segment> segments) {
  std::u32string result;
  result.reserve(segments.size() * 6); // Heuristic allocation

  for (const auto& segment : segments) {
    result.append(segment.phonemes);
    if (segment.whitespace) {
      result.push_back(U' ');
    }
  }

  return result;
}

} // namespace phonemis
