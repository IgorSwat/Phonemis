#pragma once

#include <phonemis/base/tokenizer/types.h>

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace phonemis::es::constants {

// --- Number to Word Normalization ---
namespace num2word {
  // Base cardinals: number -> word.
  // Includes units, teens, all numbers 16-29 as single-word forms, round tens
  // 30-90, and full hundreds 100-900. 100 is "cien" alone but becomes "ciento"
  // in compounds; that distinction is handled at verbalization time.
  inline const std::unordered_map<int32_t, std::u32string> kCardinals = {
    {0,   U"cero"},        {1,   U"uno"},         {2,   U"dos"},          {3,   U"tres"},        {4,   U"cuatro"},
    {5,   U"cinco"},       {6,   U"seis"},        {7,   U"siete"},        {8,   U"ocho"},        {9,   U"nueve"},
    {10,  U"diez"},        {11,  U"once"},        {12,  U"doce"},         {13,  U"trece"},       {14,  U"catorce"},
    {15,  U"quince"},      {16,  U"dieciséis"},   {17,  U"diecisiete"},   {18,  U"dieciocho"},   {19,  U"diecinueve"},
    {20,  U"veinte"},      {21,  U"veintiuno"},   {22,  U"veintidós"},    {23,  U"veintitrés"},  {24,  U"veinticuatro"},
    {25,  U"veinticinco"}, {26,  U"veintiséis"},  {27,  U"veintisiete"},  {28,  U"veintiocho"},  {29,  U"veintinueve"},
    {30,  U"treinta"},     {40,  U"cuarenta"},    {50,  U"cincuenta"},    {60,  U"sesenta"},     {70,  U"setenta"},
    {80,  U"ochenta"},     {90,  U"noventa"},
    {100, U"cien"},        {200, U"doscientos"},  {300, U"trescientos"},  {400, U"cuatrocientos"},
    {500, U"quinientos"},  {600, U"seiscientos"}, {700, U"setecientos"},  {800, U"ochocientos"}, {900, U"novecientos"}
  };

  // Base ordinals in masculine singular form. Feminine/plural/apocopated
  // variants are derived at verbalization time from the provided suffix.
  inline const std::unordered_map<int32_t, std::u32string> kOrdinals = {
    {1,    U"primero"},      {2,    U"segundo"},      {3,    U"tercero"},        {4,    U"cuarto"},
    {5,    U"quinto"},       {6,    U"sexto"},        {7,    U"séptimo"},        {8,    U"octavo"},      {9,    U"noveno"},
    {10,   U"décimo"},       {11,   U"undécimo"},     {12,   U"duodécimo"},
    {20,   U"vigésimo"},     {30,   U"trigésimo"},    {40,   U"cuadragésimo"},   {50,   U"quincuagésimo"},
    {60,   U"sexagésimo"},   {70,   U"septuagésimo"}, {80,   U"octogésimo"},     {90,   U"nonagésimo"},
    {100,  U"centésimo"},    {1000, U"milésimo"},     {1000000, U"millonésimo"}
  };

  // Large-scale names (singular forms).
  inline const std::map<std::int64_t, std::u32string> kLargeCardinals = {
    {1000LL,             U"mil"},
    {1000000LL,          U"millón"},
    {1000000000000LL,    U"billón"}
  };

  // Plural forms for scales. "mil" is invariable; "millón"/"billón" pluralize
  // as regular nouns ("millones"/"billones").
  inline const std::unordered_map<std::int64_t, std::u32string> kLargeCardinalsPlural = {
    {1000LL,             U"mil"},
    {1000000LL,          U"millones"},
    {1000000000000LL,    U"billones"}
  };

  // Currencies (singular and plural forms; Spanish plurals are irregular
  // enough — dólar → dólares, libra → libras — to warrant an explicit table).
  inline const std::unordered_map<char32_t, std::u32string> kCurrencies = {
    {U'$', U"dólar"}, {U'€', U"euro"}, {U'£', U"libra"}
  };

  inline const std::unordered_map<char32_t, std::u32string> kCurrenciesPlural = {
    {U'$', U"dólares"}, {U'€', U"euros"}, {U'£', U"libras"}
  };

  // Months (index 1 = January)
  inline const std::vector<std::u32string> kMonths = {
    U"",        U"enero",     U"febrero",    U"marzo",      U"abril",     U"mayo",      U"junio",
    U"julio",   U"agosto",    U"septiembre", U"octubre",    U"noviembre", U"diciembre"
  };

  // Recognized Spanish ordinal suffixes attached to digits (e.g. "1º", "1ª",
  // "1er", plural "1ºs", "1ªs", "1ers").
  inline const std::unordered_set<std::u32string> kOrdinalSuffixes = {
    U"º", U"ª", U"ºs", U"ªs", U"er", U"ers"
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

} // namespace phonemis::es::constants
