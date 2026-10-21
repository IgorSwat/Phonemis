#pragma once

#include <string_view>

namespace phonemis::processor::constants {

// Punctuation stripped from the words around a match
inline constexpr std::u32string_view kPunctuation = U".,;:!?\"'()[]«»„“”…";

} // namespace phonemis::processor::constants
