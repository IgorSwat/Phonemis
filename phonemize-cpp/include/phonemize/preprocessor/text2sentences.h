#pragma once

#include "constants.h"
#include <vector>

namespace phonemize::preprocessor::text2sentences {

  // Divides a monolit text into multiple sentences.
  // A sentence always ends with a end of sentence character (defined in constants.h).
  std::vector<std::string> split(const std::string& text);

} // namespace phonemize::preprocessor::text2sentences