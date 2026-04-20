#pragma once

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
std::u32string utf8_to_u32(std::string_view utf8);

/**
 * Converts a UTF-32 string to a UTF-8 string.
 * @param u32 The UTF-32 string to convert.
 * @return A UTF-8 encoded string.
 */
std::string u32_to_utf8(std::u32string_view u32);

} // namespace phonemis::utilities::conversions