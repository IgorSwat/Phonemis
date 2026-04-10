#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace phonemis::en::constants {
namespace num2word {
  // Cards map: basic number -> word
  inline const std::unordered_map<int32_t, std::u32string> kCardinals = {
		{0, U"zero"}, {1, U"one"}, {2, U"two"}, {3, U"three"}, {4, U"four"}, {5, U"five"},
		{6, U"six"}, {7, U"seven"}, {8, U"eight"}, {9, U"nine"}, {10, U"ten"},
		{11, U"eleven"}, {12, U"twelve"}, {13, U"thirteen"}, {14, U"fourteen"},
		{15, U"fifteen"}, {16, U"sixteen"}, {17, U"seventeen"}, {18, U"eighteen"},
		{19, U"nineteen"}, {20, U"twenty"}, {30, U"thirty"}, {40, U"forty"},
		{50, U"fifty"}, {60, U"sixty"}, {70, U"seventy"}, {80, U"eighty"}, {90, U"ninety"}
  };

  // Ordinal exceptions: cardinal word -> ordinal word
  inline const std::unordered_map<std::u32string, std::u32string> kOrdinals = {
		{U"one", U"first"}, {U"two", U"second"}, {U"three", U"third"}, {U"five", U"fifth"},
		{U"eight", U"eighth"}, {U"nine", U"ninth"}, {U"twelve", U"twelfth"}
  };

  // Large scale names: scale value -> name
  inline const std::map<std::int64_t, std::u32string> kLargeCardinals = {
		{100LL, U"hundred"}, {1000LL, U"thousand"}, {1000000LL, U"million"},
		{1000000000LL, U"billion"}, {1000000000000LL, U"trillion"}
  };

  // Currencies
  inline const std::unordered_map<char32_t, std::u32string> kCurrencies = {
		{U'$', U"dollars"}, {U'€', U"euros"}, {U'£', U"pounds"},
  };

  // Months
  inline const std::vector<std::u32string> kMonths = {
		U"", U"January", U"February", U"March", U"April", U"May", U"June",
		U"July", U"August", U"September", U"October", U"November", U"December"
  };
} // namespace num2word

namespace alphabet {
	inline const std::u32string kVowels = U"aeiouy";  // Written vowels
  inline const std::u32string kConsosants = U"bcdfghjklmnpqrstvwxz";  // Written consosants
} // namespace alphabet

inline const std::unordered_map<char32_t, std::u32string> kAddSymbols = {
  {U'.', U"dot"},
  {U'/', U"slash"}
};

inline const std::unordered_map<char, std::u32string> kSymbols = {
  {U'%', U"percent"},
  {U'&', U"and"},
  {U'+', U"plus"},
  {U'@', U"at"},
  {U'=', U"equals"}
};

inline const std::u32string kUSTaus = U"AIOWYiuæɑəɛɪɹʊʌ";

} // namespace phonemis::en::constants
