#pragma once

#include <string>
#include <string_view>

namespace phonemis::preprocessor::num2word {

// Specializes the behavior of `convert()` method.
enum class Mode {
  CARDINAL = 0,

  POTENTIALY_ORDINAL,
  ORDINAL,

  FRACTION,

  MONTH,
  YEAR,
  DATE
};

// Conversion input - text representation + mode
struct StringifiedNumber {
  std::string_view text;
  Mode conversionMode = Mode::CARDINAL;
};

} // namespace phonemis::preprocessor::num2word