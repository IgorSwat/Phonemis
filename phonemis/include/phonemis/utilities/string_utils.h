#pragma once

#include <algorithm>
#include <codecvt>
#include <optional>
#include <string>

namespace phonemis::utilities::string_utils {

// -------------------------------------
// String utils - byte format conversion
// -------------------------------------

inline std::string char32_to_utf8(char32_t c) {
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
  return convert.to_bytes(&c, &c + 1);
}

inline std::u32string utf8_to_u32string(const std::string& utf8) {
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
	return convert.from_bytes(utf8);
}

// ----------------------------------------
// String utils - capitalizing & lowerizing
// ----------------------------------------

// Capitalization (first letter only)
inline std::string capitalize(const std::string& str) {
	if (str.empty()) return "";

	std::string captilized = str;
	captilized[0] = std::toupper(captilized[0]);

	return captilized;
}

// Capitalization (an entire string)
inline std::string to_upper(const std::string& str) {
	std::string upper = str;
	std::transform(upper.cbegin(), upper.cend(), upper.begin(),
								 [](char c) { return std::toupper(c); });

	return upper;
}

// Lowerization (an entire string)
inline std::string to_lower(const std::string& str) {
	std::string lower = str;
	std::transform(lower.cbegin(), lower.cend(), lower.begin(), 
								 [](char c) { return std::tolower(c); });

	return lower;
}

// ------------------------------------
// String utils - other transformations
// ------------------------------------

// Replaces all the occurances of a character `a` with a character `b`.
// If `b` is not specified, then it removes all occurances of `a` without replacement.
// Works in place, without copies.
template <typename StringT, typename CharT>
void replace(StringT& str, CharT a, std::optional<CharT> b) {
	if (b.has_value())
		std::replace(str.begin(), str.end(), a, b.value());
	else
		str.erase(std::remove(str.begin(), str.end(), a), str.end());
}

} // phonemis::utilities::conversions