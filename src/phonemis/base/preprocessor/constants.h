#pragma once

#include <unordered_set>

namespace phonemis::preprocessor::constants {

static const std::unordered_set<char32_t> kCurrencies = {
  U'$', U'€', U'£', 
};

} // namespace phonemis::preprocessor::constants