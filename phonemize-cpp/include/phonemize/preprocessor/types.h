#pragma once

#include <string>

namespace phonemize::preprocessor {

// -------------------------
// num2word type definitions
// -------------------------
namespace num2words {
  // Conversion type
  // Either cardinal (example: five), ordinal (example: fifth) or special (year, etc.)
  enum ConversionMode {
    CARDINAL = 1,
    ORDINAL,
    YEAR,

    UNDEFINED
  };
} // namespace num2words

} // namespace phonemize::preprocessing