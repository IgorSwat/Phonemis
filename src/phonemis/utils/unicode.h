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

// A Latin letter, accented ones included
constexpr inline bool is_latin(char32_t c) {
  return (c >= U'A' && c <= U'Z') || (c >= U'a' && c <= U'z') || (c >= U'À' && c <= U'Ö') || (c >= U'Ø' && c <= U'ö') ||
         (c >= U'ø' && c <= U'ɏ');
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

// A letter, a digit or "_": a word character of a regex (\w)
constexpr inline bool isword(char32_t c) {
  return isalnum(c) || c == U'_';
}

// Also the Unicode spaces: no-break, thin (14 h 20), ideographic, ...
constexpr inline bool isspace(char32_t c) {
  return (is_ascii(c) && std::isspace(static_cast<unsigned char>(c))) || c == U'\u0085' || c == U'\u00a0' ||
         c == U'\u1680' || (c >= U'\u2000' && c <= U'\u200a') || c == U'\u2028' || c == U'\u2029' || c == U'\u202f' ||
         c == U'\u205f' || c == U'\u3000';
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