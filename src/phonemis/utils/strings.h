#pragma once

#include "unicode.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

/**
 * String utilities.
 * 
 * A set of string formatting & transforming operations,
 * similar to those from Python, including lower/upper case transformations,
 * splitting, trimming & other.
 * 
 * NOTE: Inline (mutable) versions of each operation is marked with trailing __.
 */
namespace phonemis::utils::strings {

// ----------------------
// ----- Predicates -----
// ----------------------

/**
 * Checks if given string contains only alphabetical characters.
 * @param str The string to check.
 * @return True if all characters are alphabetic.
 */
template <typename StringT>
inline bool is_alpha(const StringT& str) {
	using std::isalpha;
	using unicode::isalpha;

	return std::all_of(str.cbegin(), str.cend(), 
										 [](auto c) -> bool { return isalpha(c); });
}

/**
 * Checks if given string contains only lowercase characters.
 * @param str The string to check.
 * @return True if all characters are lowercase.
 */
template <typename StringT>
inline bool is_all_lower(const StringT& str) {
	using std::islower;
	using unicode::islower;

	return std::all_of(str.cbegin(), str.cend(), 
										 [](auto c) -> bool { return islower(c); });
}

/**
 * Checks if given string contains only uppercase characters.
 * @param str The string to check.
 * @return True if all characters are uppercase.
 */
template <typename StringT>
inline bool is_all_upper(const StringT& str) {
	using std::isupper;
	using unicode::isupper;

	return std::all_of(str.cbegin(), str.cend(), 
										 [](auto c) -> bool { return isupper(c); });
}

/**
 * Checks if given string ends with a given suffix.
 * @param str The string to check.
 * @param suffix The suffix to look for.
 * @return True if str ends with suffix.
 */
template <typename StringT1, typename SuffixT>
inline bool ends_with(const StringT1& str, const SuffixT& suffix) {
	std::basic_string_view suffix_view{suffix};
	return str.size() >= suffix_view.size() &&
				 str.substr(str.size() - suffix_view.size()) == suffix_view;
}

/**
 * Checks if given string starts with a given suffix.
 * @param str The string to check.
 * @param prefix The prefix to look for.
 * @return True if str starts with prefix.
 */
template <typename StringT1, typename PrefixT>
inline bool starts_with(const StringT1& str, const PrefixT& prefix) {
	std::basic_string_view prefix_view{prefix};
	return str.size() >= prefix_view.size() &&
				 str.substr(0, prefix_view.size()) == prefix_view;
}


// --------------------------------------------
// ----- Lower/Upper case transformations -----
// --------------------------------------------

/**
 * Capitalization (first letter only).
 * Changes the first letter of given string to uppercase (if alphabetical).
 * @param str The string to capitalize in-place.
 */
template <typename StringT>
inline void capitalize__(StringT& str) {
	using std::toupper;
	using unicode::toupper;

	if (!str.empty()) {
		str[0] = toupper(str[0]);
	}
}

/**
 * Lowerization (first letter only).
 * Changes the first letter of given string to lowercase (if alphabetical).
 * @param str The string to lowerize in-place.
 */
template <typename StringT>
inline void lowerize__(StringT& str) {
	using std::tolower;
	using unicode::tolower;

	if (!str.empty()) {
		str[0] = tolower(str[0]);
	}
}

/**
 * Lowerization (an entire string).
 * @param str The string to convert to lower case in-place.
 */
template <typename StringT>
inline void to_lower__(StringT& str) {
	using std::tolower;
	using unicode::tolower;

	std::transform(str.cbegin(), str.cend(), str.begin(), 
								 [](auto c) { return tolower(c); });
}

/**
 * Capitalization (an entire string).
 * @param str The string to convert to upper case in-place.
 */
template <typename StringT>
inline void to_upper__(StringT& str) {
	using std::toupper;
	using unicode::toupper;

	std::transform(str.cbegin(), str.cend(), str.begin(),
								 [](auto c) { return toupper(c); });
}


// ------------------------------------------
// ----- Character-wise transformations -----
// ------------------------------------------

/**
 * Filters a given string and omits all the characters which
 * do not pass given predicate.
 * @param str The string to filter in-place.
 * @param pred The predicate function.
 */
template <typename StringT, typename Pred>
inline void filter__(StringT& str, Pred pred) {
	str.erase(std::remove_if(str.begin(), str.end(), pred), str.end());
}

/**
 * Replaces all the occurrences of a character `a` with a character `b`.
 * @param str The string to modify in-place.
 * @param a The character to find.
 * @param b The character to replace with.
 */
template <typename StringT, typename CharT>
inline void replace__(StringT& str, CharT a, CharT b) {
	std::replace(str.begin(), str.end(), a, b);
}

/**
 * Removes all occurrences of a character `a` from the string.
 * @param str The string to modify in-place.
 * @param a The character to remove.
 */
template <typename StringT, typename CharT>
inline void remove__(StringT& str, CharT a) {
	str.erase(std::remove(str.begin(), str.end(), a), str.end());
}

/**
 * Removes the leading and trailing characters equals to given character.
 * @param str The string to strip.
 * @param c The character to remove (optional). If not specified, makes it remove white spaces instead.
 * @return A new string with characters removed.
 */
template <typename StringT, typename CharT>
inline StringT strip(const StringT& str, std::optional<CharT> c = std::nullopt) {
	using std::isspace;
	using unicode::isspace;

	auto lbound = std::find_if(str.cbegin(), str.cend(), 
														 [&c](CharT a) -> bool { return c.has_value() ? a != c : !isspace(a); });
	auto rbound = std::find_if(str.crbegin(), str.crend(),
														 [&c](CharT a) -> bool { return c.has_value() ? a != c : !isspace(a); });
	
	auto rbound_base = rbound.base();

	return (lbound < rbound_base) ? StringT(lbound, rbound_base) : StringT();
}


// ------------------
// ----- Splits -----
// ------------------

/**
 * Splits the string by the given character.
 * @param str The string to split.
 * @param bpoint The character to split by.
 * @return A vector of substrings.
 */
template <typename StringT, typename CharT>
inline std::vector<StringT> split(const StringT& str, CharT bpoint) {
	std::vector<StringT> result = {};

	auto it = str.begin();
	while (it != str.end()) {
		auto next = std::find(it, str.end(), bpoint);
		result.emplace_back(it, next);

		it = next;
		if (it != str.end()) it++;
	}

	return result;
}


// --------------------------------
// ----- Immutable operations -----
// --------------------------------

// We define a custom mapping, which will allow to bind string_views directly
// with their corresponding storage classes.
// By default, we bind the type to the itself (for standard strings).
template <typename T>
struct string_storage {
    using type = T;
};

// Specializations for the view types
template <> struct string_storage<std::string_view>    { using type = std::string; };
template <> struct string_storage<std::u16string_view> { using type = std::u16string; };
template <> struct string_storage<std::u32string_view> { using type = std::u32string; };

// Generates non-mutating wrapper `name(...)` that calls `name__(...)`
// Used to create a non-inplace versions of the above functions.
#define MAKE_NON_INPLACE(name)                                      \
template<typename StringT, typename... Args>                        \
inline auto name(const StringT& str, Args&&... args) {           \
    using StorageT = string_storage<std::decay_t<StringT>>::type;           \
    StorageT tmp{str.begin(), str.end()};                                          \
    name##__(tmp, std::forward<Args>(args)...);                     \
    return tmp;                                                     \
}

MAKE_NON_INPLACE(capitalize)
MAKE_NON_INPLACE(to_lower)
MAKE_NON_INPLACE(to_upper)
MAKE_NON_INPLACE(filter)
MAKE_NON_INPLACE(replace)
MAKE_NON_INPLACE(remove)

} // namespace phonemis::utils::strings 

