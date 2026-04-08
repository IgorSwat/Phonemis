#include "layer.h"
#include "../constants.h"
#include <phonemis/utils/unicode.h>
#include <phonemis/utils/conversions.h>

#include <algorithm>
#include <optional>
#include <iostream>

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
      result.append(verbalize({input.substr(start, len), mode})); // Append converted number
      last_pos = start + len;
      i = start + len - 1; // Fast-forward iterator to end of replaced segment
    }
  }

  result.append(input.substr(last_pos));
  return result;
}

std::u32string Num2WordLayer::verbalize(const StringifiedNumber& number) const {
  switch (number.conversionMode) {
    case Mode::CARDINAL: {
      if (number.text.find(U'.') != std::u32string_view::npos) {
        auto val = as_float(number.text);
        if (!val) return U"";
        return to_cardinal_float(*val, number.text);
      } else {
        auto val = as_int(number.text);
        if (!val) return U"";
        return to_cardinal_int(*val);
      }
    }
    case Mode::POTENTIALY_ORDINAL: {
      size_t i = 0;
      while (i < number.text.size() && std::isdigit(number.text[i])) i++;
      std::u32string_view num_part = number.text.substr(0, i);
      std::u32string_view suffix = number.text.substr(i);
      auto val = as_int(num_part);
      if (!val) return U"";
      if (is_ordinal_suffix(suffix)) {
        return to_ordinal_int(*val, suffix);
      } else {
        return to_ordinal_int(*val) + std::u32string(suffix);
      }
    }
    case Mode::ORDINAL: {
      std::u32string_view num_part = number.text.substr(0, number.text.size() - 1);
      auto val = as_int(num_part);
      if (!val) return U"";
      return to_ordinal_int(*val);
    }
    case Mode::FRACTION: {
      size_t pos = number.text.find(U'/');
      auto num = as_int(number.text.substr(0, pos));
      auto den = as_int(number.text.substr(pos + 1));
      if (!num || !den) return U"";
      return to_cardinal_int(*num) + U" " + to_ordinal_int(*den);
    }
    case Mode::CURRENCY: {
      char32_t currency = number.text.back();
      std::u32string_view num_part = number.text.substr(0, number.text.size() - 1);
      if (num_part.find(U'.') != std::u32string_view::npos) {
        auto val = as_float(num_part);
        if (!val) return U"";
        return to_cardinal_float(*val, num_part) + U" " + to_currency(currency, *val);
      } else {
        auto val = as_int(num_part);
        if (!val) return U"";
        return to_cardinal_int(*val) + U" " + to_currency(currency, *val);
      }
    }
    case Mode::DATE: {
      char32_t sep = (number.text.find(U'.') != std::u32string_view::npos) ? U'.' : U'-';
      size_t first_sep = number.text.find(sep);
      size_t second_sep = number.text.find(sep, first_sep + 1);

      std::u32string_view p1 = number.text.substr(0, first_sep);
      std::u32string_view p2 = number.text.substr(first_sep + 1, second_sep - first_sep - 1);
      std::u32string_view p3 = number.text.substr(second_sep + 1);

      auto val1 = as_int(p1);
      auto val2 = as_int(p2);
      auto val3 = as_int(p3);

      if (!val1 || !val2 || !val3) return U"";

      // ISO format: YYYY-MM-DD
      if (p1.size() == 4) {
        return to_ordinal_int(*val3) + U" " + to_month(*val2) + U" " + to_year(*val1);
      }
      // Common format: DD.MM.YYYY (or similar)
      return to_ordinal_int(*val1) + U" " + to_month(*val2) + U" " + to_year(*val3);
    }
    case Mode::MONTH: {
      auto m = as_int(number.text);
      if (!m) return U"";
      return (*m >= 1 && *m <= 12) ? to_month(*m) : to_cardinal_int(*m);
    }
    default:
      return std::u32string(number.text);
  }
}

std::optional<int32_t> Num2WordLayer::as_int(std::u32string_view s) const {
  try {
    return std::stoi(utils::conversions::u32_to_utf8(s));
  } catch (const std::exception& e) {
    std::cerr << "Error converting to int: " << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<float> Num2WordLayer::as_float(std::u32string_view s) const {
  try {
    return std::stof(utils::conversions::u32_to_utf8(s));
  } catch (const std::exception& e) {
    std::cerr << "Error converting to float: " << e.what() << std::endl;
    return std::nullopt;
  }
}

} // namespace phonemis::preprocessor::num2word
