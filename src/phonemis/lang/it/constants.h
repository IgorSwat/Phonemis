#pragma once

#include <phonemis/base/tokenizer/types.h>

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace phonemis::it::constants {

// --- Number to Word Normalization ---
namespace num2word {
  // Base cardinals: number -> word. Includes units, teens, and round tens
  // 20-90. All other values are composed: tens+units fuse into a single word
  // with elision rules, hundreds/thousands concatenate, millions+ take a space.
  inline const std::unordered_map<int32_t, std::u32string> kCardinals = {
    {0,  U"zero"},       {1,  U"uno"},         {2,  U"due"},          {3,  U"tre"},         {4,  U"quattro"},
    {5,  U"cinque"},     {6,  U"sei"},         {7,  U"sette"},        {8,  U"otto"},        {9,  U"nove"},
    {10, U"dieci"},      {11, U"undici"},      {12, U"dodici"},       {13, U"tredici"},     {14, U"quattordici"},
    {15, U"quindici"},   {16, U"sedici"},      {17, U"diciassette"},  {18, U"diciotto"},    {19, U"diciannove"},
    {20, U"venti"},      {30, U"trenta"},      {40, U"quaranta"},     {50, U"cinquanta"},   {60, U"sessanta"},
    {70, U"settanta"},   {80, U"ottanta"},     {90, U"novanta"}
  };

  // Base ordinals in masculine singular form. Feminine/plural variants are
  // derived at verbalization time from the provided suffix.
  inline const std::unordered_map<int32_t, std::u32string> kOrdinals = {
    {1,  U"primo"},    {2,  U"secondo"},  {3,  U"terzo"},    {4,  U"quarto"},    {5,  U"quinto"},
    {6,  U"sesto"},    {7,  U"settimo"},  {8,  U"ottavo"},   {9,  U"nono"},      {10, U"decimo"}
  };

  // Large-scale names (singular forms).
  inline const std::map<std::int64_t, std::u32string> kLargeCardinals = {
    {100LL,           U"cento"},
    {1000LL,          U"mille"},
    {1000000LL,       U"milione"},
    {1000000000LL,    U"miliardo"}
  };

  // Plural forms for scales. "cento" is invariable as a multiplier prefix;
  // "mille" becomes "mila"; "milione"/"miliardo" pluralize as regular nouns.
  inline const std::unordered_map<std::int64_t, std::u32string> kLargeCardinalsPlural = {
    {100LL,           U"cento"},
    {1000LL,          U"mila"},
    {1000000LL,       U"milioni"},
    {1000000000LL,    U"miliardi"}
  };

  // Currencies (singular and plural). "euro" is invariable; "dollaro" and
  // "sterlina" follow regular noun pluralization.
  inline const std::unordered_map<char32_t, std::u32string> kCurrencies = {
    {U'$', U"dollaro"}, {U'€', U"euro"}, {U'£', U"sterlina"}
  };

  inline const std::unordered_map<char32_t, std::u32string> kCurrenciesPlural = {
    {U'$', U"dollari"}, {U'€', U"euro"}, {U'£', U"sterline"}
  };

  // Months (index 1 = January)
  inline const std::vector<std::u32string> kMonths = {
    U"",         U"gennaio",   U"febbraio",  U"marzo",     U"aprile",   U"maggio",   U"giugno",
    U"luglio",   U"agosto",    U"settembre", U"ottobre",   U"novembre", U"dicembre"
  };

  // Recognized Italian ordinal suffixes attached to digits (e.g. "1º", "1ª",
  // plural "1ºs", "1ªs").
  inline const std::unordered_set<std::u32string> kOrdinalSuffixes = {
    U"º", U"ª", U"ºs", U"ªs"
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

} // namespace phonemis::it::constants
