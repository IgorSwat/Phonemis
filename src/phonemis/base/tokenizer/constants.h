#pragma once

#include <unordered_set>

namespace phonemis::tokenizer::constants {

inline const std::unordered_set<char32_t> kEosCharacters = {
    U'.',
    U'?',
    U'!',
    U';',
    U'…', // Ellipsis
    U'|', // ASCII Pipe (often used as Hindi Purna Viram)
    U'।', // Hindi Purna Viram (U+0964)
    U'॥', // Hindi Deergh Viram (U+0965)
    U'¿', // Spanish Inverted Question Mark (U+00BF)
    U'¡', // Spanish Inverted Exclamation Mark (U+00A1)
};

} // namespace phonemis::tagger::constants