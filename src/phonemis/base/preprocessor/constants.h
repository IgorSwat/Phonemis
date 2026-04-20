#pragma once

#include <array>

namespace phonemis::preprocessor::constants {

inline constexpr std::array<char32_t, 3> kCurrencies = {
  U'$', U'€', U'£', 
};

} // namespace phonemis::preprocessor::constants