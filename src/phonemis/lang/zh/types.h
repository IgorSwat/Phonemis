#pragma once

#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace phonemis::zh {

// For each character of a transformed text, the range [first, second) of the original
// text it came from. Characters inserted from nowhere, like the spaces between words,
// have kNoSource.
using SourceSpan = std::pair<size_t, size_t>;
using SourceSpans = std::vector<SourceSpan>;

inline constexpr SourceSpan kNoSource = {std::numeric_limits<size_t>::max(), 0};

// The smallest span covering both.
inline SourceSpan merge(SourceSpan a, SourceSpan b) {
  return {std::min(a.first, b.first), std::max(a.second, b.second)};
}

} // namespace phonemis::zh
