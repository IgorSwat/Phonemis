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

namespace phonemis::es::constants {

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

  // Up to 29, each a single word
  inline constexpr std::array<std::u32string_view, 30> kOnes = {
    U"cero",       U"uno",        U"dos",         U"tres",        U"cuatro",       U"cinco",
    U"seis",       U"siete",      U"ocho",        U"nueve",       U"diez",         U"once",
    U"doce",       U"trece",      U"catorce",     U"quince",      U"dieciséis",    U"diecisiete",
    U"dieciocho",  U"diecinueve", U"veinte",      U"veintiuno",   U"veintidós",    U"veintitrés",
    U"veinticuatro", U"veinticinco", U"veintiséis", U"veintisiete", U"veintiocho", U"veintinueve"
  };

  inline constexpr std::array<std::u32string_view, 10> kTens = {
    U"", U"", U"", U"treinta", U"cuarenta", U"cincuenta", U"sesenta", U"setenta", U"ochenta", U"noventa"
  };

  inline constexpr std::array<std::u32string_view, 10> kHundreds = {
    U"",           U"ciento",      U"doscientos",  U"trescientos", U"cuatrocientos",
    U"quinientos", U"seiscientos", U"setecientos", U"ochocientos", U"novecientos"
  };

  // Ordinal stems, completed by -o or -a: centésima trigésima séptima
  inline constexpr std::array<std::u32string_view, 10> kOrdinalOnes = {
    U"", U"primer", U"segund", U"tercer", U"cuart", U"quint", U"sext", U"séptim", U"octav", U"noven"
  };
  inline constexpr std::array<std::u32string_view, 10> kOrdinalTens = {
    U"",           U"décim",      U"vigésim",     U"trigésim",   U"cuadragésim",
    U"quincuagésim", U"sexagésim", U"septuagésim", U"octogésim", U"nonagésim"
  };
  inline constexpr std::array<std::u32string_view, 10> kOrdinalHundreds = {
    U"",             U"centésim",      U"ducentésim",     U"tricentésim",   U"cuadringentésim",
    U"quingentésim", U"sexcentésim",   U"septingentésim", U"octingentésim", U"noningentésim"
  };

  inline const std::vector<std::u32string> kMonths = {
    U"enero", U"febrero", U"marzo",      U"abril",   U"mayo",      U"junio",
    U"julio", U"agosto",  U"septiembre", U"octubre", U"noviembre", U"diciembre"
  };

  inline const std::unordered_map<std::u32string, Forms> kCurrencies = {
    {U"$", {U"dólar", U"dólares"}}, {U"US$", {U"dólar", U"dólares"}}, {U"€", {U"euro", U"euros"}},
    {U"£", {U"libra", U"libras"}}
  };

  inline const std::unordered_map<std::u32string, std::u32string> kScales = {
    {U"millones", U"millones"}, {U"millón", U"millón"}, {U"mil", U"mil"}, {U"k", U"mil"}, {U"K", U"mil"}
  };

  inline const std::unordered_map<std::u32string, Forms> kUnits = {
    {U"km", {U"kilómetro", U"kilómetros"}},         {U"m", {U"metro", U"metros"}},
    {U"cm", {U"centímetro", U"centímetros"}},       {U"mm", {U"milímetro", U"milímetros"}},
    {U"nm", {U"nanómetro", U"nanómetros"}},         {U"m²", {U"metro cuadrado", U"metros cuadrados"}},
    {U"km²", {U"kilómetro cuadrado", U"kilómetros cuadrados"}}, {U"m³", {U"metro cúbico", U"metros cúbicos"}},
    {U"ha", {U"hectárea", U"hectáreas"}},           {U"kg", {U"kilogramo", U"kilogramos"}},
    {U"g", {U"gramo", U"gramos"}},                  {U"mg", {U"miligramo", U"miligramos"}},
    {U"t", {U"tonelada", U"toneladas"}},            {U"l", {U"litro", U"litros"}},
    {U"L", {U"litro", U"litros"}},                  {U"ml", {U"mililitro", U"mililitros"}},
    {U"TB", {U"terabyte", U"terabytes"}},           {U"GB", {U"gigabyte", U"gigabytes"}},
    {U"MB", {U"megabyte", U"megabytes"}},           {U"KB", {U"kilobyte", U"kilobytes"}},
    {U"Hz", {U"hercio", U"hercios"}},               {U"kHz", {U"kilohercio", U"kilohercios"}},
    {U"MHz", {U"megahercio", U"megahercios"}},      {U"GHz", {U"gigahercio", U"gigahercios"}},
    {U"W", {U"vatio", U"vatios"}},                  {U"kW", {U"kilovatio", U"kilovatios"}},
    {U"V", {U"voltio", U"voltios"}},                {U"km/h", {U"kilómetro por hora", U"kilómetros por hora"}},
    {U"MPH", {U"milla por hora", U"millas por hora"}}, {U"mph", {U"milla por hora", U"millas por hora"}},
    {U"°C", {U"grado Celsius", U"grados Celsius"}}, {U"°", {U"grado", U"grados"}}
  };

  inline const std::map<std::pair<uint64_t, uint64_t>, std::u32string> kFractions = {
    {{1, 2}, U"un medio"}, {{1, 3}, U"un tercio"}, {{2, 3}, U"dos tercios"}, {{1, 4}, U"un cuarto"},
    {{3, 4}, U"tres cuartos"}
  };

  // Words before the number of a law with its year: Ley 2/2018
  inline const std::unordered_set<std::u32string> kLaws = {
    U"ley", U"decreto", U"legislativo", U"sentencia", U"resolución", U"declaración", U"orden", U"acuerdo",
    U"circular", U"reglamento", U"directiva", U"constitucionalidad", U"expediente", U"norma", U"instrucción"
  };

  // Words before a time with a dot, which make it a time: a las 18.30
  inline const std::unordered_set<std::u32string> kTimeWords = {U"las", U"la", U"a"};

  // Nouns in -a, -ión, -dad, ... are feminine unless listed below; other feminine nouns are listed
  inline const std::unordered_set<std::u32string> kMasculineInA = {
    U"día", U"mapa", U"problema", U"sistema", U"programa", U"tema", U"idioma", U"clima", U"planeta", U"poema",
    U"dilema", U"drama", U"esquema", U"sofá", U"papá", U"cometa", U"fantasma", U"panorama", U"diagrama", U"teorema"
  };
  inline const std::unordered_set<std::u32string> kFeminineNouns = {
    U"vez", U"veces", U"mano", U"foto", U"moto", U"radio", U"mujer", U"flor", U"noche", U"clase", U"parte",
    U"gente", U"leche", U"muerte", U"suerte", U"calle", U"carne", U"red", U"voz", U"luz", U"paz", U"ley", U"piel",
    U"fase", U"base", U"frase", U"nube", U"llave", U"fuente", U"mente", U"torre", U"tarde", U"serie", U"especie",
    U"superficie", U"imagen", U"orden", U"edición"
  };
  inline const std::vector<std::u32string> kFeminineEndings = {U"a", U"ión", U"dad", U"tad", U"tud", U"umbre", U"sis", U"itis"};

  // Words after a number which are not the noun it counts
  inline const std::unordered_set<std::u32string> kStopWords = {
    U"de", U"del", U"en", U"con", U"por", U"para", U"sin", U"que", U"y", U"e", U"o", U"u", U"a", U"al", U"los",
    U"las", U"el", U"la", U"lo", U"un", U"una", U"unos", U"unas", U"es", U"son", U"era", U"sea", U"pero", U"como",
    U"más", U"menos", U"hasta", U"desde", U"entre", U"sobre", U"contra", U"hacia", U"tras", U"según", U"durante",
    U"mediante", U"cada", U"toda", U"todas", U"todo", U"todos", U"esta", U"estas", U"este", U"estos", U"esa",
    U"esas", U"ese", U"esos", U"otra", U"otras", U"otro", U"otros", U"nada", U"ya", U"ahora", U"cuando", U"donde",
    U"mientras", U"muy", U"casi", U"también", U"así", U"se", U"le", U"les", U"me", U"te", U"nos"
  };

  // --- Notation: how numbers are written ---
  inline const Notation kNotation = {
    .group_separators = U".,", .decimal_separators = U".,", .time_separators = U":.", .time_suffixes = {U"h", U"hrs"},
    .currencies = keys_of(kCurrencies), .scales = keys_of(kScales), .units = keys_of(kUnits), .fractions = true
  };

  // Words between numbers: 3.8.10, 10.0.0.51, 10-15, 1920x1080, 2:1, -5, 2 + 2
  inline const Joiners kJoiners = {
    .point = U"punto", .dot = U"punto", .range = U"-", .by = U"por", .score = U" a ", .minus = U"menos", .plus = U"más"
  };

  // 14/07/2023, 31/12/99
  inline const DateFormat kDateFormat = {};

  // 600-123-456, 600 123 456 (after a word of kPhoneWords)
  inline const std::vector<PhoneFormat> kPhoneFormats = {
    {.groups = {{3, 3}, {3, 3}, {3, 3}}, .separators = U"-", .boundaries = U".,-", .closing = U"-"},
    {.groups = {{3, 3}, {3, 3}, {3, 3}}, .separators = U" ", .boundaries = U".,-", .closing = U"-"}
  };

  // Words before a phone number written with spaces, which else reads as numbers: llame al 600 123 456
  inline const std::unordered_set<std::u32string> kPhoneWords = {
    U"al", U"teléfono", U"tel", U"móvil", U"número", U"llame", U"llamar", U"fax", U"whatsapp", U"contacto"
  };

  // 1º, 2 ª: a sign, possibly after a space
  inline const Keys kOrdinalSigns = {U"º", U"ª"};
  inline const SuffixFormat kOrdinalSignFormat = {.space = true, .word_end = false};

  // 1er, 3ero, 2da, 5ta
  inline const Keys kOrdinalSuffixes = {
    U"er", U"ero", U"era", U"ro", U"ra", U"do", U"da", U"to", U"ta", U"vo", U"va", U"mo", U"ma", U"no", U"na"
  };

  // 80s, 90's
  inline const Keys kDecadeSuffixes = {U"s"};
  inline const SuffixFormat kDecadeFormat = {.apostrophes = true, .boundaries = U".,-"};
} // namespace num2word

// --- Symbols to Word Normalization ---
namespace sym2word {
  using ::phonemis::processor::Readings;
  using ::phonemis::processor::after_name;

  // "+" is "plus" after a name (Google+), "más" otherwise (C++)
  inline const Readings kSymbols = {
    {U"&", {U"y"}},       {U"=", {U"igual a"}},     {U"@", {U"arroba"}},  {U"%", {U"por ciento"}}, {U"°", {U"grados"}},
    {U"#", {U"numeral"}}, {U"²", {U"al cuadrado"}}, {U"³", {U"al cubo"}}, {U"π", {U"pi"}},         {U"×", {U"por"}},
    {U"€", {U"euros"}},   {U"$", {U"dólares"}},     {U"£", {U"libras"}},  {U"+", {{U"plus", after_name}, U"más"}}
  };
} // namespace sym2word

// --- Terms to Word Normalization ---
namespace term2word {
  using ::phonemis::processor::before_number;
  using ::phonemis::processor::term2word::Terms;

  inline const Terms kTerms = {
    .abbreviations = {
      // Before a number only: Nº 5
      {U"Nº", {{U"número", before_number}}},
      {U"nº", {{U"número", before_number}}},
      {U"N°", {{U"número", before_number}}},
      {U"núm.", {{U"número", before_number}}},
      {U"pág.", {{U"página", before_number}}},
      {U"art.", {{U"artículo", before_number}}},
    },
    .web = {.dot = U"punto", .at = U"arroba", .slash = U"barra"}
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

} // namespace phonemis::es::constants
