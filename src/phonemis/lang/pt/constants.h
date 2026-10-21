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

namespace phonemis::pt::constants {

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

  // Brazilian forms: dezesseis, bilhão
  inline constexpr std::array<std::u32string_view, 20> kOnes = {
    U"zero",  U"um",   U"dois",  U"três",  U"quatro",  U"cinco",  U"seis",      U"sete",      U"oito",    U"nove",
    U"dez",   U"onze", U"doze",  U"treze", U"catorze", U"quinze", U"dezesseis", U"dezessete", U"dezoito", U"dezenove"
  };

  inline constexpr std::array<std::u32string_view, 10> kTens = {
    U"", U"", U"vinte", U"trinta", U"quarenta", U"cinquenta", U"sessenta", U"setenta", U"oitenta", U"noventa"
  };

  inline constexpr std::array<std::u32string_view, 10> kHundreds = {
    U"",           U"cento",      U"duzentos",   U"trezentos",  U"quatrocentos",
    U"quinhentos", U"seiscentos", U"setecentos", U"oitocentos", U"novecentos"
  };

  // Ordinal stems, completed by -o or -a: vigésima primeira
  inline constexpr std::array<std::u32string_view, 10> kOrdinalOnes = {
    U"", U"primeir", U"segund", U"terceir", U"quart", U"quint", U"sext", U"sétim", U"oitav", U"non"
  };
  inline constexpr std::array<std::u32string_view, 10> kOrdinalTens = {
    U"",             U"décim",     U"vigésim",     U"trigésim",  U"quadragésim",
    U"quinquagésim", U"sexagésim", U"septuagésim", U"octogésim", U"nonagésim"
  };
  inline constexpr std::array<std::u32string_view, 10> kOrdinalHundreds = {
    U"",             U"centésim",    U"ducentésim",     U"trecentésim",   U"quadringentésim",
    U"quingentésim", U"sexcentésim", U"septingentésim", U"octingentésim", U"noningentésim"
  };

  inline const std::vector<std::u32string> kMonths = {
    U"janeiro", U"fevereiro", U"março",    U"abril",   U"maio",     U"junho",
    U"julho",   U"agosto",    U"setembro", U"outubro", U"novembro", U"dezembro"
  };

  inline const std::unordered_map<std::u32string, Forms> kCurrencies = {
    {U"R$", {U"real", U"reais"}}, {U"US$", {U"dólar", U"dólares"}}, {U"$", {U"dólar", U"dólares"}},
    {U"€", {U"euro", U"euros"}},  {U"£", {U"libra", U"libras"}}
  };

  // Full words are kept as written; abbreviations take the singular below 2
  inline const std::unordered_map<std::u32string, Forms> kScales = {
    {U"mil", {U"mil", U"mil"}},                 {U"k", {U"mil", U"mil"}},                 {U"K", {U"mil", U"mil"}},
    {U"milhão", {U"milhão", U"milhão"}},        {U"milhões", {U"milhões", U"milhões"}},
    {U"bilhão", {U"bilhão", U"bilhão"}},        {U"bilhões", {U"bilhões", U"bilhões"}},
    {U"trilhão", {U"trilhão", U"trilhão"}},     {U"trilhões", {U"trilhões", U"trilhões"}},
    {U"mi", {U"milhão", U"milhões"}},           {U"bi", {U"bilhão", U"bilhões"}},
    {U"tri", {U"trilhão", U"trilhões"}}
  };

  inline const std::unordered_map<std::u32string, Forms> kUnits = {
    {U"km", {U"quilômetro", U"quilômetros"}},        {U"m", {U"metro", U"metros"}},
    {U"cm", {U"centímetro", U"centímetros"}},        {U"mm", {U"milímetro", U"milímetros"}},
    {U"nm", {U"nanômetro", U"nanômetros"}},          {U"m²", {U"metro quadrado", U"metros quadrados"}},
    {U"km²", {U"quilômetro quadrado", U"quilômetros quadrados"}}, {U"m³", {U"metro cúbico", U"metros cúbicos"}},
    {U"ha", {U"hectare", U"hectares"}},              {U"kg", {U"quilo", U"quilos"}},
    {U"g", {U"grama", U"gramas"}},                   {U"mg", {U"miligrama", U"miligramas"}},
    {U"t", {U"tonelada", U"toneladas"}},             {U"l", {U"litro", U"litros"}},
    {U"L", {U"litro", U"litros"}},                   {U"ml", {U"mililitro", U"mililitros"}},
    {U"mL", {U"mililitro", U"mililitros"}},          {U"TB", {U"terabyte", U"terabytes"}},
    {U"GB", {U"gigabyte", U"gigabytes"}},            {U"MB", {U"megabyte", U"megabytes"}},
    {U"KB", {U"kilobyte", U"kilobytes"}},            {U"Hz", {U"hertz", U"hertz"}},
    {U"kHz", {U"quilohertz", U"quilohertz"}},        {U"MHz", {U"megahertz", U"megahertz"}},
    {U"GHz", {U"gigahertz", U"gigahertz"}},          {U"W", {U"watt", U"watts"}},
    {U"kW", {U"quilowatt", U"quilowatts"}},          {U"kWh", {U"quilowatt-hora", U"quilowatts-hora"}},
    {U"V", {U"volt", U"volts"}},                     {U"km/h", {U"quilômetro por hora", U"quilômetros por hora"}},
    {U"mph", {U"milha por hora", U"milhas por hora"}}, {U"°C", {U"grau Celsius", U"graus Celsius"}},
    {U"°", {U"grau", U"graus"}}
  };

  inline const std::map<std::pair<uint64_t, uint64_t>, std::u32string> kFractions = {
    {{1, 2}, U"meio"}, {{1, 3}, U"um terço"}, {{2, 3}, U"dois terços"}, {{1, 4}, U"um quarto"},
    {{3, 4}, U"três quartos"}
  };

  // Scale words which take "de" before a currency: um milhão de reais, but dez mil reais
  inline const std::vector<std::u32string> kScaleWords = {
    U"milhão", U"milhões", U"bilhão", U"bilhões", U"trilhão", U"trilhões"
  };

  // Words before an hour, which make it feminine: às duas, das dez
  inline const std::unordered_set<std::u32string> kHourWords = {U"às", U"à", U"das", U"pelas"};

  // Parts of the day after "da": duas da tarde
  inline const std::unordered_set<std::u32string> kDayParts = {U"manhã", U"tarde", U"noite", U"madrugada"};

  // Legal headings, after which an ordinal from 10 on reads as a cardinal: artigo 80º, artigo oitenta
  inline const std::unordered_set<std::u32string> kHeadings = {U"artigo", U"art", U"parágrafo", U"capítulo", U"inciso"};

  // Nouns in -a are feminine unless listed below; other feminine nouns are listed or end like one
  inline const std::unordered_set<std::u32string> kMasculineInA = {
    U"dia", U"mapa", U"problema", U"sistema", U"programa", U"tema", U"idioma", U"clima", U"planeta", U"poema",
    U"cometa", U"drama", U"esquema", U"sofá", U"diagrama", U"teorema", U"telefonema", U"cinema", U"prisma",
    U"pijama", U"aroma", U"dilema", U"fantasma", U"panorama", U"enigma", U"dogma", U"carisma", U"atleta", U"guia",
    U"pila"
  };
  inline const std::unordered_set<std::u32string> kFeminineNouns = {
    U"vez", U"mão", U"foto", U"moto", U"mulher", U"flor", U"noite", U"tarde", U"parte", U"gente", U"lei", U"luz",
    U"voz", U"paz", U"rede", U"sede", U"fase", U"base", U"frase", U"classe", U"chave", U"fonte", U"mente", U"morte",
    U"sorte", U"ponte", U"árvore", U"nuvem", U"ordem", U"série", U"espécie", U"superfície", U"cor", U"dor", U"mãe",
    U"pele", U"neve", U"febre", U"carne", U"torre", U"hora", U"semana", U"polegada", U"lição"
  };
  inline const std::vector<std::u32string> kFeminineEndings = {
    U"a", U"ção", U"são", U"dade", U"tude", U"gem", U"ise", U"ite", U"ez"
  };

  // Words after a number which are not the noun it counts
  inline const std::unordered_set<std::u32string> kStopWords = {
    U"de", U"do", U"da", U"dos", U"das", U"em", U"no", U"na", U"nos", U"nas", U"com", U"por", U"para", U"sem", U"que",
    U"e", U"ou", U"a", U"o", U"os", U"as", U"um", U"uma", U"uns", U"umas", U"é", U"são", U"era", U"foi", U"mais",
    U"menos", U"até", U"desde", U"entre", U"sobre", U"contra", U"após", U"cada", U"todo", U"toda", U"todos", U"todas",
    U"este", U"esta", U"estes", U"estas", U"esse", U"essa", U"esses", U"essas", U"isso", U"isto", U"já", U"agora",
    U"quando", U"onde", U"muito", U"também", U"se", U"lhe", U"me", U"te", U"ao", U"à", U"aos", U"às", U"pelo",
    U"pela", U"pelos", U"pelas", U"não", U"sim", U"meu", U"minha", U"seu", U"sua"
  };

  // --- Notation: how numbers are written ---
  inline const Notation kNotation = {
    .group_separators = U".", .decimal_separators = U",.", .time_separators = U":h", .time_suffixes = {U"h"},
    .currencies = keys_of(kCurrencies), .scales = keys_of(kScales), .units = keys_of(kUnits), .fractions = true
  };

  // Words between numbers: 3.8.10, 10.0.0.51, 10-15, 1920x1080, 2:1, -5, 2 + 2
  inline const Joiners kJoiners = {
    .point = U"ponto", .dot = U"ponto", .range = U" a ", .by = U"por", .score = U" a ", .minus = U"menos", .plus = U"mais"
  };

  // 24/03/2022, 07-11-2018
  inline const DateFormat kDateFormat = {.separators = U"/-", .boundaries = U".,/-"};

  // (11) 98765-4321, 0800-123456, 3456-7890, 213 456 789 (after a word of kPhoneWords): the longest first
  inline const std::vector<PhoneFormat> kPhoneFormats = {
    {.groups = {{2, 2}, {4, 5}, {4, 4}}, .parenthesized = true, .closing = U"-"},
    {.groups = {{4, 5}, {4, 6}}, .separators = U"-", .boundaries = U".,-", .closing = U"-"},
    {.groups = {{3, 3}, {3, 3}, {3, 3}}, .separators = U" ", .boundaries = U".,-", .closing = U"-"}
  };

  // Words before a phone number written with spaces, which else reads as numbers: ligue para 213 456 789
  inline const std::unordered_set<std::u32string> kPhoneWords = {
    U"para", U"telefone", U"tel", U"ligue", U"contato", U"contacto", U"número", U"whatsapp", U"fax"
  };

  // 1º, 2 ª: a sign, possibly after a space
  inline const Keys kOrdinalSigns = {U"º", U"ª"};
  inline const SuffixFormat kOrdinalSignFormat = {.space = true, .word_end = false};
} // namespace num2word

// --- Symbols to Word Normalization ---
namespace sym2word {
  using ::phonemis::processor::Readings;

  inline const Readings kSymbols = {
    {U"&", {U"e"}},         {U"+", {U"mais"}},    {U"=", {U"igual a"}},     {U"@", {U"arroba"}},   {U"%", {U"por cento"}},
    {U"°", {U"graus"}},     {U"#", {U"hashtag"}}, {U"²", {U"ao quadrado"}}, {U"³", {U"ao cubo"}},  {U"π", {U"pi"}},
    {U"×", {U"vezes"}},     {U"€", {U"euros"}},   {U"$", {U"dólares"}},     {U"£", {U"libras"}},   {U"§", {U"parágrafo"}}
  };
} // namespace sym2word

// --- Terms to Word Normalization ---
namespace term2word {
  using ::phonemis::processor::before_number;
  using ::phonemis::processor::term2word::Terms;

  inline const Terms kTerms = {
    .abbreviations = {
      // Before a number only: nº 5
      {U"nº", {{U"número", before_number}}},
      {U"Nº", {{U"número", before_number}}},
      {U"n.º", {{U"número", before_number}}},
      {U"N.º", {{U"número", before_number}}},
      {U"núm.", {{U"número", before_number}}},
      {U"art.", {{U"artigo", before_number}}},
      {U"Art.", {{U"artigo", before_number}}},
      {U"pág.", {{U"página", before_number}}},
      {U"§", {{U"parágrafo", before_number}}},
    },
    .web = {.dot = U"ponto", .at = U"arroba", .slash = U"barra"}
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

} // namespace phonemis::pt::constants
