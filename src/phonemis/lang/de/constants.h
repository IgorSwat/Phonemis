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

namespace phonemis::de::constants {

// --- Number to Word Normalization ---
namespace num2word {
  using ::phonemis::processor::num2word::DateFormat;
  using ::phonemis::processor::num2word::Forms;
  using ::phonemis::processor::num2word::Gender;
  using ::phonemis::processor::num2word::Joiners;
  using ::phonemis::processor::num2word::Keys;
  using ::phonemis::processor::num2word::keys_of;
  using ::phonemis::processor::num2word::Notation;
  using ::phonemis::processor::num2word::SuffixFormat;

  inline constexpr std::array<std::u32string_view, 20> kOnes = {
    U"null",   U"eins",    U"zwei",    U"drei",      U"vier",      U"fünf",      U"sechs",    U"sieben",    U"acht",     U"neun",
    U"zehn",   U"elf",     U"zwölf",   U"dreizehn",  U"vierzehn",  U"fünfzehn",  U"sechzehn", U"siebzehn",  U"achtzehn", U"neunzehn"
  };

  inline constexpr std::array<std::u32string_view, 10> kTens = {
    U"", U"", U"zwanzig", U"dreißig", U"vierzig", U"fünfzig", U"sechzig", U"siebzig", U"achtzig", U"neunzig"
  };

  inline const std::vector<std::u32string> kMonths = {
    U"Januar", U"Februar", U"März",      U"April",   U"Mai",      U"Juni",
    U"Juli",   U"August",  U"September", U"Oktober", U"November", U"Dezember"
  };

  // Ordinal stems which differ from their cardinals: erst, dritt, siebt, acht
  inline const std::unordered_map<std::u32string, std::u32string> kOrdinalStems = {
    {U"eins", U"ers"}, {U"drei", U"drit"}, {U"sieben", U"sieb"}, {U"acht", U"ach"}
  };

  inline const std::unordered_map<std::u32string, std::u32string> kCurrencies = {
    {U"€", U"Euro"}, {U"Euro", U"Euro"}, {U"$", U"Dollar"}, {U"Dollar", U"Dollar"}, {U"£", U"Pfund"},
    {U"CHF", U"Franken"}
  };

  inline const std::unordered_map<std::u32string, Forms> kScales = {
    {U"Mio.", {U"Million", U"Millionen"}},       {U"Million", {U"Million", U"Millionen"}},
    {U"Millionen", {U"Million", U"Millionen"}},  {U"Mrd.", {U"Milliarde", U"Milliarden"}},
    {U"Milliarde", {U"Milliarde", U"Milliarden"}}, {U"Milliarden", {U"Milliarde", U"Milliarden"}},
    {U"Tsd.", {U"tausend", U"tausend"}},         {U"k", {U"tausend", U"tausend"}}
  };

  inline const std::unordered_map<std::u32string, Forms> kUnits = {
    {U"km", {U"Kilometer", U"Kilometer"}},       {U"m", {U"Meter", U"Meter"}},
    {U"cm", {U"Zentimeter", U"Zentimeter"}},     {U"mm", {U"Millimeter", U"Millimeter"}},
    {U"nm", {U"Nanometer", U"Nanometer"}},       {U"m²", {U"Quadratmeter", U"Quadratmeter"}},
    {U"km²", {U"Quadratkilometer", U"Quadratkilometer"}}, {U"m³", {U"Kubikmeter", U"Kubikmeter"}},
    {U"ha", {U"Hektar", U"Hektar"}},             {U"kg", {U"Kilogramm", U"Kilogramm"}},
    {U"g", {U"Gramm", U"Gramm"}},                {U"mg", {U"Milligramm", U"Milligramm"}},
    {U"t", {U"Tonne", U"Tonnen"}},               {U"l", {U"Liter", U"Liter"}},
    {U"ml", {U"Milliliter", U"Milliliter"}},     {U"TB", {U"Terabyte", U"Terabyte"}},
    {U"GB", {U"Gigabyte", U"Gigabyte"}},         {U"MB", {U"Megabyte", U"Megabyte"}},
    {U"Hz", {U"Hertz", U"Hertz"}},               {U"kHz", {U"Kilohertz", U"Kilohertz"}},
    {U"MHz", {U"Megahertz", U"Megahertz"}},      {U"GHz", {U"Gigahertz", U"Gigahertz"}},
    {U"W", {U"Watt", U"Watt"}},                  {U"kW", {U"Kilowatt", U"Kilowatt"}},
    {U"kWh", {U"Kilowattstunde", U"Kilowattstunden"}}, {U"V", {U"Volt", U"Volt"}},
    {U"km/h", {U"Kilometer pro Stunde", U"Kilometer pro Stunde"}}, {U"°C", {U"Grad Celsius", U"Grad Celsius"}},
    {U"°", {U"Grad", U"Grad"}},                  {U"h", {U"Stunde", U"Stunden"}},
    {U"Std.", {U"Stunde", U"Stunden"}},          {U"min", {U"Minute", U"Minuten"}},
    {U"Min.", {U"Minute", U"Minuten"}},          {U"s", {U"Sekunde", U"Sekunden"}},
    {U"Sek.", {U"Sekunde", U"Sekunden"}},        {U"ct", {U"Cent", U"Cent"}},
    {U"PS", {U"PS", U"PS"}}
  };

  inline const std::map<std::pair<uint64_t, uint64_t>, std::u32string> kFractions = {
    {{1, 2}, U"ein halb"}, {{1, 3}, U"ein Drittel"}, {{2, 3}, U"zwei Drittel"}, {{1, 4}, U"ein Viertel"},
    {{3, 4}, U"drei Viertel"}
  };

  // Determiners after which an ordinal takes a weak ending: am dritten, der dritte (nominative)
  inline const std::unordered_set<std::u32string> kWeak = {
    U"den", U"dem", U"des", U"am", U"im", U"vom", U"zum", U"zur", U"beim", U"ins", U"diesem", U"diesen", U"dieses",
    U"jedem", U"jeden", U"einen", U"einem", U"einer", U"eines", U"keinen", U"keinem", U"keiner", U"keines",
    U"meinen", U"meinem", U"meiner", U"seinen", U"seinem", U"seiner", U"ihren", U"ihrem", U"ihrer", U"unseren",
    U"unserem", U"unserer"
  };
  inline const std::unordered_set<std::u32string> kWeakNominative = {
    U"der", U"die", U"das", U"diese", U"dieser", U"jede", U"jeder", U"jedes", U"eine", U"keine", U"meine", U"seine",
    U"ihre", U"unsere"
  };

  // ein-words, after which an ordinal takes a mixed ending: ein zweiter, ein drittes
  inline const std::unordered_set<std::u32string> kMixed = {
    U"ein", U"kein", U"mein", U"dein", U"sein", U"ihr", U"unser", U"euer"
  };

  // Prepositions by the case they govern
  inline const std::unordered_set<std::u32string> kDative = {
    U"mit", U"nach", U"bei", U"seit", U"von", U"zu", U"aus", U"gegenüber", U"in", U"an", U"auf", U"unter", U"über",
    U"vor", U"hinter", U"neben", U"zwischen", U"ab"
  };
  inline const std::unordered_set<std::u32string> kAccusative = {U"für", U"durch", U"gegen", U"ohne", U"um", U"bis"};
  inline const std::unordered_set<std::u32string> kGenitive = {
    U"wegen", U"während", U"trotz", U"statt", U"innerhalb", U"außerhalb"
  };

  // Words before a date (am 12. 3.), and before a year (im Jahr 1945, and months)
  inline const std::unordered_set<std::u32string> kDateCues = {U"am", U"vom", U"zum", U"bis", U"ab", U"seit", U"den", U"der"};
  inline const std::unordered_set<std::u32string> kYearCues = {
    U"jahr", U"jahre", U"jahren", U"im", U"seit", U"bis", U"von", U"ab", U"anfang", U"ende", U"mitte", U"sommer",
    U"winter", U"frühjahr", U"herbst", U"frühling", U"anno", U"januar", U"februar", U"märz", U"april", U"mai",
    U"juni", U"juli", U"august", U"september", U"oktober", U"november", U"dezember"
  };

  // Nouns which stay singular after a number: 1500 Stück. Like plural nouns, they make a 4-digit number a count.
  inline const std::unordered_set<std::u32string> kMeasureWords = {
    U"Stück", U"Mal", U"Prozent", U"Grad", U"Euro", U"Dollar", U"Pfund", U"Kilo", U"Gramm", U"Paar", U"Cent",
    U"Franken"
  };

  // Noun gender by ending: the exceptions that matter after numbers (matched as compound endings), then the rules
  inline const std::vector<std::pair<std::u32string, Gender>> kGenders = {
    {U"jahrhundert", Gender::NEUTER}, {U"jahrtausend", Gender::NEUTER}, {U"semester", Gender::NEUTER},
    {U"mitglied", Gender::NEUTER},    {U"kapitel", Gender::NEUTER},     {U"quartal", Gender::NEUTER},
    {U"prozent", Gender::NEUTER},     {U"zimmer", Gender::NEUTER},      {U"rennen", Gender::NEUTER},
    {U"person", Gender::FEMININE},    {U"gramm", Gender::NEUTER},       {U"stück", Gender::NEUTER},
    {U"spiel", Gender::NEUTER},       {U"leben", Gender::NEUTER},       {U"liter", Gender::MASCULINE},
    {U"meter", Gender::MASCULINE},    {U"monat", Gender::MASCULINE},    {U"kunde", Gender::MASCULINE},
    {U"junge", Gender::MASCULINE},    {U"kilo", Gender::NEUTER},        {U"jahr", Gender::NEUTER},
    {U"kind", Gender::NEUTER},        {U"glas", Gender::NEUTER},        {U"haus", Gender::NEUTER},
    {U"land", Gender::NEUTER},        {U"buch", Gender::NEUTER},        {U"bild", Gender::NEUTER},
    {U"ende", Gender::NEUTER},        {U"pfund", Gender::NEUTER},       {U"euro", Gender::MASCULINE},
    {U"name", Gender::MASCULINE},     {U"käse", Gender::MASCULINE},     {U"grad", Gender::MASCULINE},
    {U"mal", Gender::NEUTER},         {U"tor", Gender::NEUTER},         {U"ei", Gender::NEUTER}
  };
  inline const std::vector<std::u32string> kFeminineEndings = {
    U"e", U"ung", U"heit", U"keit", U"schaft", U"ion", U"tät", U"ik", U"ur", U"enz", U"anz", U"ei", U"in", U"ie"
  };
  inline const std::vector<std::u32string> kNeuterEndings = {U"chen", U"lein", U"ment", U"um", U"tum", U"ma", U"o"};

  // --- Notation: how numbers are written ---
  inline const Notation kNotation = {
    .group_separators = U". \u00a0\u202f", .decimal_separators = U",.", .time_separators = U":.", .time_suffixes = {U"Uhr"},
    .currencies = keys_of(kCurrencies), .scales = keys_of(kScales), .units = keys_of(kUnits), .fractions = true
  };

  // Words between numbers: 3.8.10, 10.0.0.51, 10-15, 1920x1080, 2:1, -5, 2 + 2
  inline const Joiners kJoiners = {
    .point = U"Komma", .dot = U"Punkt", .range = U" bis ", .by = U"mal", .score = U" zu ", .minus = U"minus", .plus = U"plus"
  };

  // 1.5.2024, 1.5., 12. 3.: the year is optional, a dot closes the month
  inline const DateFormat kDateFormat = {
    .separators = U".", .boundaries = U".,", .year_required = false, .closing_separator = true, .spaced = true
  };

  // 90er, 90ern, 90ers
  inline const Keys kDecadeSuffixes = {U"er", U"ern", U"ers"};
  inline const SuffixFormat kDecadeFormat = {.boundaries = U".,-"};
} // namespace num2word

// --- Symbols to Word Normalization ---
namespace sym2word {
  using ::phonemis::processor::Readings;
  using ::phonemis::processor::after_any;

  // Words before "§" which take its plural: im § 5 (in dem Paragrafen)
  inline const std::unordered_set<std::u32string> kParagraphPlurals = {
    U"im", U"vom", U"zum", U"beim", U"dem", U"des", U"den"
  };

  inline const Readings kSymbols = {
    {U"&", {U"und"}},       {U"€", {U"Euro"}},      {U"$", {U"Dollar"}},    {U"£", {U"Pfund"}},    {U"+", {U"plus"}},
    {U"=", {U"gleich"}},    {U"@", {U"at"}},        {U"%", {U"Prozent"}},   {U"°", {U"Grad"}},     {U"#", {U"Hashtag"}},
    {U"²", {U"Quadrat"}},   {U"³", {U"hoch drei"}}, {U"¹", {U"hoch eins"}}, {U"π", {U"Pi"}},       {U"×", {U"mal"}},
    {U"½", {U"ein halb"}},  {U"§§", {U"Paragrafen"}}, {U"§", {{U"Paragrafen", after_any(kParagraphPlurals)}, U"Paragraf"}}
  };
} // namespace sym2word

// --- Terms to Word Normalization ---
namespace term2word {
  using ::phonemis::processor::before_number;
  using ::phonemis::processor::term2word::Terms;

  inline const Terms kTerms = {
    .abbreviations = {
      // Before a number only: Abs. 5
      {U"Abs.", {{U"Absatz", before_number}}},
      {U"Nr.", {{U"Nummer", before_number}}},
      {U"S.", {{U"Seite", before_number}}},
      {U"Art.", {{U"Artikel", before_number}}},
      {U"Kap.", {{U"Kapitel", before_number}}},
      {U"ca.", {{U"circa", before_number}}},
      {U"Bd.", {{U"Band", before_number}}},
    },
    .web = {.dot = U"Punkt", .at = U"at", .slash = U"Schrägstrich"}
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

} // namespace phonemis::de::constants
