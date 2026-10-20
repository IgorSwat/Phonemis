#pragma once

#include <phonemis/base/processor/layers/num2word/types.h>
#include <phonemis/base/processor/common/conditions.h>
#include <phonemis/base/processor/layers/num2word/notation.h>
#include <phonemis/base/processor/layers/term2word/types.h>
#include <phonemis/base/tokenizer/types.h>

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace phonemis::it::constants {

// --- Number to Word Normalization ---
namespace num2word {
  using ::phonemis::processor::num2word::DateFormat;
  using ::phonemis::processor::num2word::Forms;
  using ::phonemis::processor::num2word::Joiners;
  using ::phonemis::processor::num2word::Keys;
  using ::phonemis::processor::num2word::keys_of;
  using ::phonemis::processor::num2word::Notation;
  using ::phonemis::processor::num2word::PhoneFormat;
  using ::phonemis::processor::num2word::SuffixFormat;

  inline constexpr std::array<std::u32string_view, 20> kOnes = {
    U"zero",   U"uno",    U"due",         U"tre",      U"quattro",    U"cinque",      U"sei",
    U"sette",  U"otto",   U"nove",        U"dieci",    U"undici",     U"dodici",      U"tredici",
    U"quattordici", U"quindici", U"sedici", U"diciassette", U"diciotto", U"diciannove"
  };

  inline constexpr std::array<std::u32string_view, 10> kTens = {
    U"", U"", U"venti", U"trenta", U"quaranta", U"cinquanta", U"sessanta", U"settanta", U"ottanta", U"novanta"
  };

  // Ordinals up to ten; later ones are the cardinal with -esimo
  inline constexpr std::array<std::u32string_view, 11> kOrdinals = {
    U"", U"primo", U"secondo", U"terzo", U"quarto", U"quinto", U"sesto", U"settimo", U"ottavo", U"nono", U"decimo"
  };

  inline const std::vector<std::u32string> kMonths = {
    U"gennaio", U"febbraio", U"marzo",     U"aprile",  U"maggio",   U"giugno",
    U"luglio",  U"agosto",   U"settembre", U"ottobre", U"novembre", U"dicembre"
  };

  inline const std::unordered_map<std::u32string, Forms> kCurrencies = {
    {U"€", {U"euro", U"euro"}}, {U"$", {U"dollaro", U"dollari"}}, {U"£", {U"sterlina", U"sterline"}}
  };

  // Most scales are written as the word they are read with: 3 milioni
  inline const std::unordered_map<std::u32string, Forms> kScales = {
    {U"mila", {U"mila", U"mila"}},             {U"k", {U"mila", U"mila"}},           {U"K", {U"mila", U"mila"}},
    {U"milione", {U"milione", U"milione"}},    {U"milioni", {U"milioni", U"milioni"}},
    {U"miliardo", {U"miliardo", U"miliardo"}}, {U"miliardi", {U"miliardi", U"miliardi"}},
    {U"mln", {U"milione", U"milioni"}},        {U"mld", {U"miliardo", U"miliardi"}}
  };

  inline const std::unordered_map<std::u32string, Forms> kUnits = {
    {U"km", {U"chilometro", U"chilometri"}},          {U"m", {U"metro", U"metri"}},
    {U"cm", {U"centimetro", U"centimetri"}},          {U"mm", {U"millimetro", U"millimetri"}},
    {U"m²", {U"metro quadro", U"metri quadri"}},      {U"m2", {U"metro quadro", U"metri quadri"}},
    {U"km²", {U"chilometro quadrato", U"chilometri quadrati"}}, {U"m³", {U"metro cubo", U"metri cubi"}},
    {U"kg", {U"chilo", U"chili"}},                    {U"g", {U"grammo", U"grammi"}},
    {U"mg", {U"milligrammo", U"milligrammi"}},        {U"l", {U"litro", U"litri"}},
    {U"ml", {U"millilitro", U"millilitri"}},          {U"TB", {U"terabyte", U"terabyte"}},
    {U"GB", {U"giga", U"giga"}},                      {U"MB", {U"mega", U"mega"}},
    {U"Hz", {U"hertz", U"hertz"}},                    {U"kHz", {U"kilohertz", U"kilohertz"}},
    {U"MHz", {U"megahertz", U"megahertz"}},           {U"GHz", {U"gigahertz", U"gigahertz"}},
    {U"W", {U"watt", U"watt"}},                       {U"kW", {U"kilowatt", U"kilowatt"}},
    {U"kWh", {U"kilowattora", U"kilowattora"}},       {U"V", {U"volt", U"volt"}},
    {U"km/h", {U"chilometro orario", U"chilometri orari"}}, {U"°C", {U"grado", U"gradi"}},
    {U"°", {U"grado", U"gradi"}}
  };

  inline const std::map<std::pair<uint64_t, uint64_t>, std::u32string> kFractions = {
    {{1, 2}, U"un mezzo"}, {{1, 3}, U"un terzo"}, {{2, 3}, U"due terzi"}, {{1, 4}, U"un quarto"},
    {{3, 4}, U"tre quarti"}
  };

  // Words before a time with a dot, which make it a time: alle 21.30
  inline const std::unordered_set<std::u32string> kTimeWords = {
    U"alle", U"ore", U"dalle", U"le", U"verso", U"entro", U"per"
  };

  // --- Notation: how numbers are written ---
  inline const Notation kNotation = {
    .group_separators = U".", .decimal_separators = U",.", .time_separators = U":.",
    .currencies = keys_of(kCurrencies), .scales = keys_of(kScales), .units = keys_of(kUnits), .fractions = true
  };

  // Words between numbers: 3.8.10, 10.0.0.51, 10-15, 1920x1080, 2:1, -5, 2 + 2
  inline const Joiners kJoiners = {
    .point = U"punto", .dot = U"punto", .range = U"-", .by = U"per", .score = U" a ", .minus = U"meno", .plus = U"più"
  };

  // 24/03/2022
  inline const DateFormat kDateFormat = {.boundaries = U".,/-"};

  // 333 1234567, 06 1234 5678, 06 12345678
  inline const std::vector<PhoneFormat> kPhoneFormats = {
    {.groups = {{3, 3}, {6, 7}}, .separators = U" "},
    {.groups = {{2, 4}, {3, 4}, {4, 4}}, .separators = U" ", .leading_zero = true},
    {.groups = {{2, 4}, {6, 8}}, .separators = U" ", .leading_zero = true}
  };

  // 1º, 2 ª: a sign, possibly after a space
  inline const Keys kOrdinalSigns = {U"º", U"ª"};
  inline const SuffixFormat kOrdinalSignFormat = {.space = true};

  // 21esimo, 33esima
  inline const Keys kOrdinalSuffixes = {U"esimo", U"esima", U"esimi", U"esime"};
} // namespace num2word

// --- Symbols to Word Normalization ---
namespace sym2word {
  using ::phonemis::processor::Readings;

  // "&" only joins English names: AT&T, R&B
  inline const Readings kSymbols = {
    {U"&", {U"and"}},         {U"+", {U"più"}},       {U"=", {U"uguale a"}}, {U"@", {U"chiocciola"}}, {U"%", {U"per cento"}},
    {U"°", {U"gradi"}},       {U"#", {U"hashtag"}},   {U"²", {U"al quadrato"}}, {U"³", {U"al cubo"}}, {U"π", {U"pi greco"}},
    {U"×", {U"per"}},         {U"€", {U"euro"}},      {U"$", {U"dollari"}},  {U"£", {U"sterline"}},   {U"§", {U"paragrafo"}}
  };
} // namespace sym2word

// --- Terms to Word Normalization ---
namespace term2word {
  using ::phonemis::processor::before_number;
  using ::phonemis::processor::term2word::Terms;

  inline const Terms kTerms = {
    .abbreviations = {
      // Before a number only: n. 5
      {U"n.", {{U"numero", before_number}}},
      {U"N.", {{U"numero", before_number}}},
      {U"nr.", {{U"numero", before_number}}},
      {U"n°", {{U"numero", before_number}}},
      {U"N°", {{U"numero", before_number}}},
      {U"art.", {{U"articolo", before_number}}},
      {U"Art.", {{U"articolo", before_number}}},
      {U"pag.", {{U"pagina", before_number}}},
      {U"cap.", {{U"capitolo", before_number}}},
      {U"§", {{U"paragrafo", before_number}}},
    },
    .web = {.dot = U"punto", .at = U"chiocciola", .slash = U"slash"}
  };
} // namespace term2word

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
