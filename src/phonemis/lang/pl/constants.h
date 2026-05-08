#pragma once

#include <phonemis/base/tokenizer/types.h>

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace phonemis::pl::constants {

// --- Sanitization Rules ---
namespace sanitizer {
  // Character replacements for postprocessing (e.g. ʑ → ʒ).
  inline const std::unordered_map<char32_t, char32_t> kCharReplacements = {
    {U'ʑ', U'ʒ'}
  };
} // namespace sanitizer

// --- Number to Word Normalization ---
namespace num2word {
  // Polish nouns following a numeral take one of three plural forms depending
  // on the numeral's last one or two digits. This drives both scale word
  // selection ("tysiąc"/"tysiące"/"tysięcy") and currency pluralization. The
  // index assignment is also used to look up forms in the tables below.
  enum class PluralForm : std::size_t { SINGULAR = 0, FEW = 1, MANY = 2 };

  // Base cardinals (masculine nominative): units, teens, round tens, and full
  // hundreds. Hundreds 200-900 are listed since they aren't compositionally
  // derivable in Polish ("dwieście", "trzysta", ..., "dziewięćset").
  inline const std::unordered_map<int32_t, std::u32string> kCardinals = {
    {0,   U"zero"},          {1,   U"jeden"},         {2,   U"dwa"},           {3,   U"trzy"},          {4,   U"cztery"},
    {5,   U"pięć"},          {6,   U"sześć"},         {7,   U"siedem"},        {8,   U"osiem"},         {9,   U"dziewięć"},
    {10,  U"dziesięć"},      {11,  U"jedenaście"},    {12,  U"dwanaście"},     {13,  U"trzynaście"},    {14,  U"czternaście"},
    {15,  U"piętnaście"},    {16,  U"szesnaście"},    {17,  U"siedemnaście"},  {18,  U"osiemnaście"},   {19,  U"dziewiętnaście"},
    {20,  U"dwadzieścia"},   {30,  U"trzydzieści"},   {40,  U"czterdzieści"},  {50,  U"pięćdziesiąt"},  {60,  U"sześćdziesiąt"},
    {70,  U"siedemdziesiąt"},{80,  U"osiemdziesiąt"}, {90,  U"dziewięćdziesiąt"},
    {100, U"sto"},           {200, U"dwieście"},      {300, U"trzysta"},       {400, U"czterysta"},
    {500, U"pięćset"},       {600, U"sześćset"},      {700, U"siedemset"},     {800, U"osiemset"},      {900, U"dziewięćset"}
  };

  // Large-scale forms keyed by base value, indexed by PluralForm.
  using PluralForms = std::array<std::u32string, 3>;
  inline const std::map<std::int64_t, PluralForms> kLargeCardinals = {
    {1000LL,         {U"tysiąc",  U"tysiące",  U"tysięcy"}},
    {1000000LL,      {U"milion",  U"miliony",  U"milionów"}},
    {1000000000LL,   {U"miliard", U"miliardy", U"miliardów"}}
  };

  // Currency word in singular/few/many forms. Euro is invariable.
  inline const std::unordered_map<char32_t, PluralForms> kCurrencies = {
    {U'$', {U"dolar", U"dolary", U"dolarów"}},
    {U'€', {U"euro",  U"euro",   U"euro"}},
    {U'£', {U"funt",  U"funty",  U"funtów"}}
  };

  // Months in genitive form, since Polish dates ("27 marca 2026") attach the
  // month as a genitive complement of the day.
  inline const std::vector<std::u32string> kMonths = {
    U"",            U"stycznia",    U"lutego",       U"marca",       U"kwietnia",    U"maja",        U"czerwca",
    U"lipca",       U"sierpnia",    U"września",     U"października",U"listopada",   U"grudnia"
  };

  // Ordinals in masculine nominative singular (e.g. "pierwszy", "dwudziesty").
  // Compound ordinals are built compositionally; gender/case variants are
  // derived at verbalization time.
  inline const std::unordered_map<int32_t, std::u32string> kOrdinals = {
    {1,    U"pierwszy"},      {2,    U"drugi"},          {3,    U"trzeci"},         {4,    U"czwarty"},      {5,    U"piąty"},
    {6,    U"szósty"},        {7,    U"siódmy"},         {8,    U"ósmy"},           {9,    U"dziewiąty"},
    {10,   U"dziesiąty"},     {11,   U"jedenasty"},      {12,   U"dwunasty"},       {13,   U"trzynasty"},    {14,   U"czternasty"},
    {15,   U"piętnasty"},     {16,   U"szesnasty"},      {17,   U"siedemnasty"},    {18,   U"osiemnasty"},   {19,   U"dziewiętnasty"},
    {20,   U"dwudziesty"},    {30,   U"trzydziesty"},    {40,   U"czterdziesty"},   {50,   U"pięćdziesiąty"},
    {60,   U"sześćdziesiąty"},{70,   U"siedemdziesiąty"},{80,   U"osiemdziesiąty"}, {90,   U"dziewięćdziesiąty"},
    {100,  U"setny"},         {1000, U"tysięczny"},      {1000000, U"milionowy"}
  };

  // Ordinals in masculine genitive (used for date days, e.g. "27 marca" reads
  // as "dwudziestego siódmego marca"). Days only ever fall in [1, 31], so
  // entries up to 30 cover every compositional case.
  inline const std::unordered_map<int32_t, std::u32string> kOrdinalsGen = {
    {1,    U"pierwszego"},      {2,    U"drugiego"},        {3,    U"trzeciego"},        {4,    U"czwartego"},      {5,    U"piątego"},
    {6,    U"szóstego"},        {7,    U"siódmego"},        {8,    U"ósmego"},           {9,    U"dziewiątego"},
    {10,   U"dziesiątego"},     {11,   U"jedenastego"},     {12,   U"dwunastego"},       {13,   U"trzynastego"},    {14,   U"czternastego"},
    {15,   U"piętnastego"},     {16,   U"szesnastego"},     {17,   U"siedemnastego"},    {18,   U"osiemnastego"},   {19,   U"dziewiętnastego"},
    {20,   U"dwudziestego"},    {30,   U"trzydziestego"}
  };

  // Recognized Polish ordinal suffixes attached to digits (e.g. "1szy",
  // "2gi", "3ci", "10ty"). All gender/number variants resolve to the same
  // masculine-nominative ordinal phrase since Polish ordinal notation is
  // primarily a phonetic abbreviation of the inflected ending.
  inline const std::unordered_set<std::u32string> kOrdinalSuffixes = {
    U"szy", U"sza", U"sze",
    U"gi",  U"ga",  U"gie",
    U"ci",  U"cia", U"cie",
    U"ty",  U"ta",  U"te",
    U"my",  U"ma",  U"me",
    U"ny",  U"na",  U"ne"
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

} // namespace phonemis::pl::constants
