#include "conversions.h"

namespace phonemis::utils::conversions {

std::u32string utf8_to_u32(std::string_view utf8) {
  std::u32string result;
  result.reserve(utf8.size());

  for (size_t i = 0; i < utf8.size();) {
    // We read utf8 string byte by byte.
    unsigned char byte = utf8[i];

    char32_t codepoint = 0;
    size_t extra = 0;

    // Read the information byte
    if (byte <= 0x7F) {
      codepoint = byte;
      extra = 0;
    } else if ((byte & 0xE0) == 0xC0) { // '110xxxxx' pattern
      codepoint = byte & 0x1F;
      extra = 1;
    } else if ((byte & 0xF0) == 0xE0) { // '1110xxxx' pattern
      codepoint = byte & 0x0F;
      extra = 2;
    } else if ((byte & 0xF8) == 0xF0) { // '11110xxx' pattern
      codepoint = byte & 0x07;
      extra = 3;
    } else { // Invalid, not a utf-8 byte
      result.push_back(0xFFFD); // Emit unicode replacement character
      i++;
      continue;
    }

    if (i + extra >= utf8.size()) { // truncated
      break;
    }

    // Read all the traling '10xxxxxx' bytes
    for (size_t j = 1; j <= extra; j++) {
      unsigned char c = utf8[i + j];
      if ((c & 0xC0) != 0x80) { // invalid continuation (not a '10xxxxxx' byte)
        break;
      }

      codepoint = (codepoint << 6) | (c & 0x3F);
    }

    result.push_back(codepoint);
    i += extra + 1;
  }

  return result;
}

std::string u32_to_utf8(std::u32string_view u32) {
  std::string result;
  result.reserve(u32.size());

  for (char32_t c : u32) {
    if (c <= 0x7F) { // Standard ASCII character
      result.push_back(static_cast<char>(c));
    } else if (c <= 0x7FF) { // 2-byte unicode character (max 11 bits)
      result.push_back(static_cast<char>(0xC0 | (c >> 6)));    // '110' + 5 bits
      result.push_back(static_cast<char>(0x80 | (c & 0x3F))); // '10' + 6 bits
    } else if (c <= 0xFFFF) { // 3-byte unicode character (max 16 bits)
      result.push_back(static_cast<char>(0xE0 | (c >> 12)));         // '1110' + 4 bits
      result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); // '10' + 6 bits
      result.push_back(static_cast<char>(0x80 | (c & 0x3F)));        // '10' + 6 bits
    } else if (c <= 0x10FFFF) { // 4-byte unicode character (max 21 bits)
      result.push_back(static_cast<char>(0xF0 | (c >> 18)));          // '11110' + 3 bits
      result.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F))); // '10' + 6 bits
      result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));  // '10' + 6 bits
      result.push_back(static_cast<char>(0x80 | (c & 0x3F)));         // '10' + 6 bits
    }
  }

  return result;
}

} // namespace phonemis::utils::conversions
