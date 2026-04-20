#pragma once

#include <unordered_set>

namespace phonemis::tagger::constants {

inline const std::unordered_set<char32_t> kEosCharacters = {U'.', U'?', U'!', U';'};

} // namespace phonemis::tagger::constants