#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace phonemis::fr::constants {

// --- Number to Word Normalization ---
namespace num2word {
  // Base cardinals: number -> word.
  // Includes units (0-19) and round tens up to 60. Higher tens (70, 80, 90)
  // are derived compositionally (e.g. 70 = soixante-dix, 80 = quatre-vingts).
  inline const std::unordered_map<int32_t, std::u32string> kCardinals = {
    {0,  U"zéro"},     {1,  U"un"},        {2,  U"deux"},      {3,  U"trois"},     {4,  U"quatre"},
    {5,  U"cinq"},     {6,  U"six"},       {7,  U"sept"},      {8,  U"huit"},      {9,  U"neuf"},
    {10, U"dix"},      {11, U"onze"},      {12, U"douze"},     {13, U"treize"},    {14, U"quatorze"},
    {15, U"quinze"},   {16, U"seize"},     {17, U"dix sept"},  {18, U"dix huit"},  {19, U"dix neuf"},
    {20, U"vingt"},    {30, U"trente"},    {40, U"quarante"},  {50, U"cinquante"}, {60, U"soixante"}
  };

  // Large scale names: scale value -> name
  inline const std::map<std::int64_t, std::u32string> kLargeCardinals = {
    {100LL,           U"cent"},
    {1000LL,          U"mille"},
    {1000000LL,       U"million"},
    {1000000000LL,    U"milliard"}
  };

  // Currencies (singular form; pluralization is applied at verbalization time).
  inline const std::unordered_map<char32_t, std::u32string> kCurrencies = {
    {U'$', U"dollar"}, {U'€', U"euro"}, {U'£', U"livre"}
  };

  // Months (index 1 = January)
  inline const std::vector<std::u32string> kMonths = {
    U"",         U"janvier",   U"février",   U"mars",      U"avril",    U"mai",      U"juin",
    U"juillet",  U"août",      U"septembre", U"octobre",   U"novembre", U"décembre"
  };

  // Recognized French ordinal suffixes attached to digits (e.g. "1er", "2ème", "3e", "1re").
  inline const std::unordered_set<std::u32string> kOrdinalSuffixes = {
    U"er", U"ers", U"re", U"res", U"ère", U"ères",
    U"e",  U"es",  U"ème", U"èmes", U"ième", U"ièmes"
  };
} // namespace num2word

} // namespace phonemis::fr::constants
