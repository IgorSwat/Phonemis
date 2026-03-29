#include "layer.h"
#include <regex>
#include <vector>
#include <cctype>

namespace phonemis::preprocessor::num2word {

// A helper struct to keep track of global positioning of encountered numbers.
struct Match {
  size_t pos;
  size_t length;
  StringifiedNumber sn;
};

namespace {
// Regexes for numeric patterns - defined from the highest to lowest specifity.
// Note: We use capture groups to identify which part of the alternation matched.
// 1: DATE, 2: FRACTION, 3: FLOAT, 4: DOT_ORDINAL, 5: POT_ORDINAL, 6: INTEGER
static const std::regex combined_regex(
    R"((\b\d{1,4}[.-]\d{1,2}[.-]\d{1,4}\b)|)"          // 1: DATE
    R"((\b\d+/\d+\b)|)"                                // 2: FRACTION
    R"((\b\d+\.\d+\b)|)"                               // 3: FLOAT
    R"((\b\d+\.)|)"                                    // 4: DOT_ORDINAL
    R"((\b\d+[a-zA-Z]+\b)|)"                           // 5: POT_ORDINAL
    R"((\b\d+\b))",                                    // 6: INTEGER
    std::regex::optimize
);
} // namespace

std::string Num2WordLayer::transform(std::string_view input) const {
  std::string result;
  result.reserve(input.size() + 32);

  auto it = std::cregex_iterator(input.data(), input.data() + input.size(), combined_regex);
  auto end = std::cregex_iterator();

  size_t last_pos = 0;
  for (; it != end; ++it) {
    const std::cmatch& m = *it;
    size_t pos = m.position();
    size_t len = m.length();

    Mode mode = Mode::CARDINAL;
    if (m[1].matched) {
      mode = Mode::DATE;
    } else if (m[2].matched) {
      mode = Mode::FRACTION;
    } else if (m[3].matched) {
      mode = Mode::CARDINAL; // Float
    } else if (m[4].matched) {
      // Check dot context for ordinal
      size_t next_pos = pos + len;
      while (next_pos < input.size() && std::isspace(static_cast<unsigned char>(input[next_pos]))) {
        next_pos++;
      }
      if (!config_.allowGeneralOrdNotation ||
          next_pos >= input.size() || !std::islower(static_cast<unsigned char>(input[next_pos]))) {
        mode = Mode::CARDINAL;
        len--;  // Remove the trailing dot from the number representation.
      }
      else {
        mode = Mode::ORDINAL;
      }
    } else if (m[5].matched) {
      mode = Mode::POTENTIALY_ORDINAL;
    } else if (m[6].matched) {
      mode = Mode::CARDINAL;  // Integer
    }

    result.append(input.substr(last_pos, pos - last_pos));
    result.append(convert({std::string_view(input).substr(pos, len), mode}));
    last_pos = pos + len;
  }

  result.append(input.substr(last_pos));
  return result;
}

} // namespace phonemis::preprocessor::num2word
