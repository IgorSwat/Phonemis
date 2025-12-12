#pragma once

#include <string>

namespace phonemis::utilities::conversions {

// char32_t to utf8 string conversion
static std::string char32_to_utf8(char32_t c) {
	std::string out;
	if (c <= 0x7F) {
			out += static_cast<char>(c);
	} else if (c <= 0x7FF) {
			out += static_cast<char>(0xC0 | ((c >> 6) & 0x1F));
			out += static_cast<char>(0x80 | (c & 0x3F));
	} else if (c <= 0xFFFF) {
			out += static_cast<char>(0xE0 | ((c >> 12) & 0x0F));
			out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (c & 0x3F));
	} else {
			out += static_cast<char>(0xF0 | ((c >> 18) & 0x07));
			out += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
			out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (c & 0x3F));
	}
  
	return out;
}

} // phonemis::utilities::conversions