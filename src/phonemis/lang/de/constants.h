#pragma once

#include <phonemis/base/tokenizer/types.h>

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace phonemis::de::constants {

// --- Number to Word Normalization ---
namespace num2word {
  inline const std::unordered_map<int32_t, std::u32string> kCardinals = {
    {0,  U"null"},     {1,  U"eins"},      {2,  U"zwei"},      {3,  U"drei"},      {4,  U"vier"},
    {5,  U"fünf"},     {6,  U"sechs"},     {7,  U"sieben"},    {8,  U"acht"},      {9,  U"neun"},
    {10, U"zehn"},     {11, U"elf"},       {12, U"zwölf"},     {13, U"dreizehn"},  {14, U"vierzehn"},
    {15, U"fünfzehn"}, {16, U"sechzehn"},  {17, U"siebzehn"},  {18, U"achtzehn"},  {19, U"neunzehn"},
    {20, U"zwanzig"},  {30, U"dreißig"},   {40, U"vierzig"},   {50, U"fünfzig"},   {60, U"sechzig"},
    {70, U"siebzig"},  {80, U"achtzig"},   {90, U"neunzig"}
  };

  inline const std::map<std::int64_t, std::u32string> kLargeCardinals = {
    {100LL,           U"hundert"},
    {1000LL,          U"tausend"},
    {1000000LL,       U"Million"},
    {1000000000LL,    U"Milliarde"}
  };

  inline const std::unordered_map<char32_t, std::u32string> kCurrencies = {
    {U'$', U"Dollar"}, {U'€', U"Euro"}, {U'£', U"Pfund"}
  };

  inline const std::vector<std::u32string> kMonths = {
    U"",         U"Januar",   U"Februar",   U"März",      U"April",    U"Mai",      U"Juni",
    U"Juli",     U"August",   U"September", U"Oktober",   U"November", U"Dezember"
  };

  inline const std::unordered_set<std::u32string> kOrdinalSuffixes = {
    U".", U"te", U"ste", U"ten", U"sten", U"ter", U"ster", U"tes", U"stes", U"tem", U"stem"
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

  inline const Exceptions kExceptions = {};
} // namespace tokenizer

} // namespace phonemis::de::constants
