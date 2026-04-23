#pragma once

#include <array>

namespace phonemis::processor::constants {

inline constexpr std::array<char32_t, 3> kCurrencies = {
  U'$', U'€', U'£', 
};

} // namespace phonemis::processor::constants