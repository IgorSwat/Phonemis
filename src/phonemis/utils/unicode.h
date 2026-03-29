#pragma once

#include "unicode_tables.h"
#include "conversions.h"

#include <limits>
#include <iostream>
#include <string>

using namespace phonemis::utils;

/**
 * A minimalistic unicode library to extend basic utf8 operations
 * to u32string format.
 */
namespace std {

constexpr inline bool is_ascii(char32_t c) {
  return CHAR_MIN <= c && c <= CHAR_MAX;
}

constexpr inline bool isalpha(char32_t c) {
  return is_ascii(c) && isalpha(static_cast<char>(c)) ||
         unicode::kAlphaLowerToUpper.contains(c) || unicode::kAlphaUpperToLower.contains(c);
}

constexpr inline bool isdigit(char32_t c) {
  return is_ascii(c) && isdigit(static_cast<char>(c));
}

constexpr inline bool isspace(char32_t c) {
  return is_ascii(c) && isspace(static_cast<char>(c));
}

constexpr inline char32_t tolower(char32_t c) {
  return is_ascii(c) ? tolower(static_cast<char>(c)) :
         unicode::kAlphaUpperToLower.contains(c) ? unicode::kAlphaUpperToLower.at(c) : c;
}

constexpr inline char32_t toupper(char32_t c) {
  return is_ascii(c) ? toupper(static_cast<char>(c)) :
         unicode::kAlphaLowerToUpper.contains(c) ? unicode::kAlphaLowerToUpper.at(c) : c;
}

inline std::ostream& operator<<(std::ostream& os, const std::u32string& u32) {
  return os << phonemis::utils::conversions::u32_to_utf8(u32);
}

inline std::ostream& operator<<(std::ostream& os, const char32_t* u32) {
  return os << phonemis::utils::conversions::u32_to_utf8(u32);
}

} // namespace std