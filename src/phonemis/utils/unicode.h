#pragma once

#include "conversions.h"
#include "unicode_table.h"

#include <limits>
#include <string>

/**
 * A minimalistic unicode library to extend basic utf8 operations
 * to u32string format.
 */
namespace phonemis::utils::unicode {

constexpr inline bool is_ascii(char32_t c) {
  return (c & ~0x7F) == 0;
}

constexpr inline bool isalpha(char32_t c) {
  return (c < kLutSize) ? kUnicodeLut[c].is_alpha : false;
}

constexpr inline bool isdigit(char32_t c) {
  return is_ascii(c) && std::isdigit(static_cast<unsigned char>(c));
}

constexpr inline bool isalnum(char32_t c) {
  return isalpha(c) || isdigit(c);
}

constexpr inline bool isspace(char32_t c) {
  return is_ascii(c) && std::isspace(static_cast<unsigned char>(c));
}

constexpr inline bool islower(char32_t c) {
  return isalpha(c) && kUnicodeLut[c].upper != c;
}

constexpr inline bool isupper(char32_t c) {
  return isalpha(c) && kUnicodeLut[c].lower != c;
}

constexpr inline char32_t tolower(char32_t c) {
  return (c < kLutSize) ? kUnicodeLut[c].lower : c;
}

constexpr inline char32_t toupper(char32_t c) {
  return (c < kLutSize) ? kUnicodeLut[c].upper : c;
}

} // namespace phonemis::utils::unicode