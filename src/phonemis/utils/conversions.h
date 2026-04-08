#pragma once

#include <codecvt>
#include <functional>
#include <string>
#include <string_view>

/**
 * Type conversions
 * 
 * We use two different formats to represent strings: UTF-8 and UTF-32.
 * UTF-8 is the standard encoding for storage and transport, but UTF-32
 * enables easier character-by-character iteration over non-ASCII characters,
 * such as most of phonemes from IPA.
 */
namespace phonemis::utils::conversions {

/**
 * Converts a UTF-8 string to a UTF-32 string.
 * @param utf8 The UTF-8 string to convert.
 * @return A UTF-32 encoded string.
 */
inline std::u32string utf8_to_u32(std::string_view utf8) {
	// std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
	// return convert.from_bytes(utf8);

	std::u32string result;
	result.reserve(utf8.size() / 4);

	for (size_t i = 0; i < utf8.size();) {
		// We read utf8 string byte by byte.
		unsigned char byte = utf8[i];

		char32_t codepoint = 0;
		size_t extra = 0;

		// Read the information byte
		if (byte <= 0x7F) {
			codepoint = byte;
			extra = 0;
		} else if ((byte & 0xE0) == 0xC0) {	// '110xxxxx' pattern
			codepoint = byte & 0x1F;
			extra = 1;
		} else if ((byte & 0xF0) == 0xE0) {	// '1110xxxx' pattern
			codepoint = byte & 0x0F;
			extra = 2;
		} else if ((byte & 0xF8) == 0xF0) {	// '11110xxx' pattern
			codepoint = byte & 0x07;
			extra = 3;
		} else {	// Invalid, not a utf-8 byte
			i++;
			continue;
		}

		if (i + extra >= utf8.size()) {	// truncated
			break;
		}
	
		// Read all the traling '10xxxxxx' bytes
		for (size_t j = 1; j <= extra; j++) {
			unsigned char c = utf8[i + j];
			if ((c & 0xC0) != 0x80) {	// invalid continuation (not a '10xxxxxx' byte)
				break;
			}
			
			codepoint = (codepoint << 6) | (c & 0x3F);
		}

		result.push_back(codepoint);
		i += extra + 1;
	}

	return result;
}

/**
 * Converts a UTF-32 string to a UTF-8 string.
 * @param u32 The UTF-32 string to convert.
 * @return A UTF-8 encoded string.
 */
inline std::string u32_to_utf8(std::u32string_view u32) {
	std::string result;
	result.reserve(u32.size());

	for (char32_t c : u32) {
		if (c <= 0x7F) {	// Standard ASCII character
			result.push_back(static_cast<char>(c));
		} else if (c <= 0x7FF) {	// 2-byte unicode character (max 11 bits)
			result.push_back(static_cast<char>(0xC0 | (c >> 6)));		// '110' + 5 bits
			result.push_back(static_cast<char>(0x80 | (c & 0x3F)));	// '10' + 6 bits
		} else if (c <= 0xFFFF) {	// 3-byte unicode character (max 16 bits)
			result.push_back(static_cast<char>(0xE0 | (c >> 12)));	// '1110' + 4 bits
			result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));	// '10' + 6 bits
			result.push_back(static_cast<char>(0x80 | (c & 0x3F)));	// '10' + 6 bits
		} else if (c <= 0x10FFFF) {	// 4-byte unicode character (max 21 bits)
			result.push_back(static_cast<char>(0xF0 | (c >> 18)));	// '11110' + 3 bits
			result.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));	// '10' + 6 bits
			result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));	// '10' + 6 bits
			result.push_back(static_cast<char>(0x80 | (c & 0x3F)));	// '10' + 6 bits
		}
	}

	return result;
}

} // namespace phonemis::utilities::conversions