#pragma once

#include "types.h"

#include <string_view>

namespace phonemis::processor::normalize::constants {

// The character classes of Echo (echo/preprocessing/characters.py), the default of every language
inline const Characters kCharacters = {
    {U"…", U"..."},                    // Ellipsis
    {U"«»„“”‟˝", U"\""},               // Quotes
    {U"‘’‚‛´`ʼ", U"'"},                // Apostrophes
    {U"\u200b\u200c\u200d\ufeff", U""} // Invisible: zero-width space, non-joiner, joiner, byte order mark
};

// Dashes besides "-": em, en, figure dash, horizontal bar
inline constexpr std::u32string_view kDashes = U"—–‒―";

} // namespace phonemis::processor::normalize::constants
