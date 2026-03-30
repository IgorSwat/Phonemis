#include "layer.h"
#include "../constants.h"
#include <phonemis/utils/unicode.h>

#include <cctype>
#include <string_view>

namespace phonemis::preprocessor::num2word {

namespace {
// Checks if a character serves as a word boundary.
// In other words, it's either white space or special character.
bool is_boundary(char32_t c) {
  return !std::isalnum(c) && c != U'_';
}

// Counts and returns the number of consecutive digits from the start index
size_t count_digits(std::u32string_view input, size_t start) {
  auto it = std::find_if(
      input.begin() + start, input.end(),
      [](unsigned char c) { return !std::isdigit(c); });
  return static_cast<size_t>(it - (input.begin() + start));
}
} // namespace

std::u32string Num2WordLayer::transform(std::u32string_view input) const {
  std::u32string result;

  // A bit of a heuristic for estimating output size to minimize number of reallocs.
  result.reserve(input.size() + 32);

  size_t last_pos = 0;
  for (size_t i = 0; i < input.size(); ++i) {
    // Look for the start of a number, ensuring it's at a word boundary
    if (!std::isdigit(input[i])) continue;
    if (i > 0 && !is_boundary(input[i - 1])) continue;

    size_t start = i;
    size_t len = 0;
    Mode mode = Mode::CARDINAL;

    // Parse the first group of digits
    size_t p1 = count_digits(input, start);
    size_t curr = start + p1;

    // Check for compound numbers like dates, floats, fractions, or dot-ordinals
    if (curr < input.size() && (input[curr] == U'.' || input[curr] == U'-' || input[curr] == U'/')) {
      char sep = input[curr];
      size_t p2 = count_digits(input, curr + 1);
      
      if (p2 > 0) {
        size_t next_curr = curr + 1 + p2;
        
        // Try to match a DATE pattern (e.g., DD.MM.YYYY, YYYY-MM-DD)
        if (next_curr < input.size() && input[next_curr] == sep && (sep == U'.' || sep == U'-')) {
          size_t p3 = count_digits(input, next_curr + 1);
          // Ensure the date ends at a boundary and parts have reasonable lengths
          if (p3 > 0 && (next_curr + 1 + p3 == input.size() || is_boundary(input[next_curr + 1 + p3]))) {
            if (p1 <= 4 && p2 <= 2 && p3 <= 4) {
              len = next_curr + 1 + p3 - start;
              mode = Mode::DATE;
            }
          }
        }
        
        // If it's not a date, evaluate for FLOAT or FRACTION
        if (len == 0 && (curr + 1 + p2 == input.size() || is_boundary(input[curr + 1 + p2]))) {
          len = curr + 1 + p2 - start;
          mode = (sep == U'/') ? Mode::FRACTION : Mode::CARDINAL;
        }
      } else if (sep == U'.') {
        // No second digit part found, might be an ORDINAL ending with a dot (e.g. '1.')
        len = p1 + 1;
        mode = Mode::ORDINAL;
        
        // Check the trailing context for ordinal notation validation (typically expects a lowercase word next)
        size_t next_pos = start + len;
        while (next_pos < input.size() && std::isspace(input[next_pos])) {
          next_pos++;
        }
        if (!config_.allowGeneralOrdNotation || next_pos >= input.size() || !std::islower(input[next_pos])) {
          // Revert to plain Cardinal if context does not match ordinal expectations
          mode = Mode::CARDINAL;
          len--; // Exclude the trailing dot
        }
      }
    }
    
    // If no compound pattern was successfully identified
    if (len == 0) {
      // Check for CURRENCY (number followed by single currency symbol)
      if (curr < input.size() && constants::kCurrencies.contains(input[curr]) &&
          (curr + 1 == input.size() || is_boundary(input[curr + 1]))) {
        len = p1 + 1;
        mode = Mode::CURRENCY;
      } else {
        size_t alpha = 0;
        while (curr + alpha < input.size() && std::isalpha(input[curr + alpha])) {
          alpha++;
        }

        // Check for POTENTIALLY ORDINAL values (numbers directly followed by letters, e.g., '1st', '2nd')
        if (alpha > 0 && (curr + alpha == input.size() || is_boundary(input[curr + alpha]))) {
          len = p1 + alpha;
          mode = Mode::POTENTIALY_ORDINAL;
        }
        // Otherwise, assume it's a standard integer CARDINAL
        else if (curr == input.size() || is_boundary(input[curr])) {
          len = p1;
          mode = Mode::CARDINAL;
        }
      }
    }

    // Apply translation if a valid numeric chunk was found
    if (len > 0) {
      result.append(input.substr(last_pos, start - last_pos)); // Append preceding text
      result.append(convert({input.substr(start, len), mode})); // Append converted number
      last_pos = start + len;
      i = start + len - 1; // Fast-forward iterator to end of replaced segment
    }
  }

  result.append(input.substr(last_pos));
  return result;
}

} // namespace phonemis::preprocessor::num2word
