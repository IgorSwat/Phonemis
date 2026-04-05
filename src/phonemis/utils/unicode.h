#pragma once

#include "conversions.h"
#include "unicode_tables.h"

#include <limits>
#include <iostream>
#include <string>

/**
 * A minimalistic unicode library to extend basic utf8 operations
 * to u32string format.
 */
namespace phonemis::utils::unicode {

constexpr inline bool is_ascii(char32_t c) {
  return (c & ~0x7F) == 0;
}

constexpr inline bool isalnum(char32_t c) {
  return isalpha(c) || isdigit(c);
}

constexpr inline bool isalpha(char32_t c) {
  return is_ascii(c) && std::isalpha(static_cast<unsigned char>(c)) ||
         kAlphaLowerToUpper.contains(c) || kAlphaUpperToLower.contains(c);
}

constexpr inline bool isdigit(char32_t c) {
  return is_ascii(c) && std::isdigit(static_cast<unsigned char>(c));
}

constexpr inline bool isspace(char32_t c) {
  return is_ascii(c) && std::isspace(static_cast<unsigned char>(c));
}

constexpr inline bool islower(char32_t c) {
  return is_ascii(c) ? std::islower(static_cast<unsigned char>(c)) : kAlphaLowerToUpper.contains(c);
}

constexpr inline bool isupper(char32_t c) {
  return is_ascii(c) ? std::isupper(static_cast<unsigned char>(c)) : kAlphaUpperToLower.contains(c);
}

constexpr inline char32_t tolower(char32_t c) {
  return is_ascii(c) ? std::tolower(static_cast<unsigned char>(c)) :
         unicode::kAlphaUpperToLower.contains(c) ? unicode::kAlphaUpperToLower.at(c) : c;
}

constexpr inline char32_t toupper(char32_t c) {
  return is_ascii(c) ? std::toupper(static_cast<unsigned char>(c)) :
         unicode::kAlphaLowerToUpper.contains(c) ? unicode::kAlphaLowerToUpper.at(c) : c;
}

} // namespace phonemis::utils::unicode