#pragma once

#include <string>
#include <vector>

namespace phonemis::processor::normalize {

// A class of characters written in one canonical form: all quotes as ".
struct CharacterClass {
  std::u32string characters; // « » „ “ ” ‟ ˝
  std::u32string canonical;  // "; empty to remove them
};

using Characters = std::vector<CharacterClass>;

} // namespace phonemis::processor::normalize
