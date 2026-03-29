#pragma once

#include <codecvt>
#include <functional>
#include <string>

/**
 * Type conversions
 * 
 * We use two different formats to represent strings: UTF-8 and UTF-32.
 * UTF-8 is the standard encoding for storage and transport, but UTF-32
 * enables easier character-by-character iteration over non-ASCII characters,
 * such as most of phonemes from IPA.
 * 
 * TODO: Replace with manual conversions to get rid of 'deprecated' warnings.
 */
namespace phonemis::utils::conversions {

/**
 * Converts a single UTF-32 character to a UTF-8 string.
 * @param c The UTF-32 character to convert.
 * @return A UTF-8 encoded string representing the character.
 */
inline std::string char32_to_utf8(char32_t c) {
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
  return convert.to_bytes(&c, &c + 1);
}

/**
 * Converts a UTF-8 string to a UTF-32 string.
 * @param utf8 The UTF-8 string to convert.
 * @return A UTF-32 encoded string.
 */
inline std::u32string utf8_to_u32(const std::string& utf8) {
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
	return convert.from_bytes(utf8);
}

/**
 * Converts a UTF-32 string to a UTF-8 string.
 * @param u32 The UTF-32 string to convert.
 * @return A UTF-8 encoded string.
 */
inline std::string u32_to_utf8(const std::u32string& u32) {
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
	return convert.to_bytes(u32);
}

} // namespace phonemis::utilities::conversions