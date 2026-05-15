#pragma once

#include <phonemis/base/tokenizer/types.h>

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace phonemis::en::constants {

// --- Number to Word Normalization ---
namespace num2word {
  // Cards map: basic number -> word
  inline const std::unordered_map<int32_t, std::u32string> kCardinals = {
    {0,  U"zero"},     {1,  U"one"},      {2,  U"two"},       {3,  U"three"},    {4,  U"four"},
    {5,  U"five"},     {6,  U"six"},      {7,  U"seven"},     {8,  U"eight"},    {9,  U"nine"},
    {10, U"ten"},      {11, U"eleven"},   {12, U"twelve"},    {13, U"thirteen"}, {14, U"fourteen"},
    {15, U"fifteen"},  {16, U"sixteen"},  {17, U"seventeen"}, {18, U"eighteen"}, {19, U"nineteen"},
    {20, U"twenty"},   {30, U"thirty"},   {40, U"forty"},     {50, U"fifty"},    {60, U"sixty"},
    {70, U"seventy"},  {80, U"eighty"},   {90, U"ninety"}
  };

  // Ordinal exceptions: cardinal word -> ordinal word
  inline const std::unordered_map<std::u32string, std::u32string> kOrdinals = {
    {U"one",   U"first"},  {U"two",   U"second"}, {U"three", U"third"},  {U"five",  U"fifth"},
    {U"eight", U"eighth"}, {U"nine",  U"ninth"},  {U"twelve", U"twelfth"}
  };

  // Large scale names: scale value -> name
  inline const std::map<std::int64_t, std::u32string> kLargeCardinals = {
    {100LL,           U"hundred"},
    {1000LL,          U"thousand"},
    {1000000LL,       U"million"},
    {1000000000LL,    U"billion"},
    {1000000000000LL, U"trillion"}
  };

  // Currencies
  inline const std::unordered_map<char32_t, std::u32string> kCurrencies = {
    {U'$', U"dollar"}, {U'€', U"euro"}, {U'£', U"pound"}
  };

  // Months
  inline const std::vector<std::u32string> kMonths = {
    U"",         U"January",   U"February",  U"March",     U"April",    U"May",      U"June",
    U"July",     U"August",    U"September", U"October",   U"November", U"December"
  };
} // namespace num2word

// --- Tokenization Rules ---
namespace tokenizer {
  using ::phonemis::tokenizer::split::Rule;
  using ::phonemis::tokenizer::split::Exceptions;

  inline const std::unordered_map<char32_t, Rule> kSpecialCharacters = {
    {U'\'', Rule::KEEP_WITH_RIGHT},
    {U'-',  Rule::TOTAL_DIVIDE},
    {U'.',  Rule::TOTAL_DIVIDE},
    {U':',  Rule::TOTAL_DIVIDE}
  };

  // String literal pointers are valid for the lifetime of the program,
  // so u32string_view elements are safe here.
  inline const Exceptions kExceptions = {
    // Contractions
    U"'bout",    U"'d",        U"'em",       U"'ll",       U"'m",        U"'re",       U"'s",        U"'ve",
    U"can't",    U"goin'",     U"let's",     U"ma'am",     U"n't",       U"nothin'",   U"o'clock",   U"o'er",
    U"somethin'", U"what's",    U"y'know",    U"y'all",    U"mr.",       U"Mr.",       U"mrs.",      U"Mrs."
  };
} // namespace tokenizer

// --- Alphabet & Phone Sets ---
namespace alphabet {
  // Written characters
  inline const std::unordered_set<char32_t> kVowels = {U'a', U'e', U'i', U'o', U'u', U'y'};
  inline const std::unordered_set<char32_t> kConsonants = {
    U'b', U'c', U'd', U'f', U'g', U'h', U'j', U'k', U'l', U'm',
    U'n', U'p', U'q', U'r', U's', U't', U'v', U'w', U'x', U'z'
  };
} // namespace alphabet

namespace ipa {
  // Spoken phones
  inline const std::unordered_set<char32_t> kVowels = {
    U'A', U'I', U'O', U'Q', U'W', U'Y', U'a', U'i', U'u', U'æ',
    U'ɑ', U'ɒ', U'ɔ', U'ə', U'ɛ', U'ɜ', U'ɪ', U'ʊ', U'ʌ', U'ᵻ'
  };

  inline const std::unordered_set<char32_t> kConsonants = {
    U'b', U'd', U'f', U'h', U'j', U'k', U'l', U'm', U'n', U'p',
    U's', U't', U'v', U'w', U'z', U'ð', U'ŋ', U'ɡ', U'ɹ', U'ɾ',
    U'ʃ', U'ʒ', U'ʤ', U'ʧ', U'θ'
  };

  // US Taus set for specific phonetic rules
  inline const std::unordered_set<char32_t> kUSTaus = {
    U'A', U'I', U'O', U'Q', U'W', U'Y', U'a', U'i', U'u', U'æ',
    U'ɑ', U'ɒ', U'ɔ', U'ə', U'ɛ', U'ɜ', U'ɪ', U'ʊ', U'ʌ', U'ᵻ'
  };
} // namespace ipa

// --- Symbols & Punctuation Mapping ---
inline const std::unordered_map<char32_t, char32_t> kSanitizerReplacements = {
  {U'’', U'\''}
};

inline const std::unordered_map<char32_t, std::u32string> kAddSymbols = {
  {U'.', U"dot"},
  {U'/', U"slash"}
};

inline const std::unordered_map<char32_t, std::u32string> kSymbols = {
  {U'%', U"percent"},
  {U'&', U"and"},
  {U'+', U"plus"},
  {U'@', U"at"},
  {U'=', U"equals"}
};

} // namespace phonemis::en::constants
