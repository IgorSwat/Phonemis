#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace phonemis::processor {

// A half-open range [begin, end) of characters in a document's text.
struct Span {
  size_t begin = 0;
  size_t end = 0;
};

// Keys of a table matched in text (units, currencies, abbreviations); the longest matching one wins.
using Keys = std::vector<std::u32string>;

} // namespace phonemis::processor
