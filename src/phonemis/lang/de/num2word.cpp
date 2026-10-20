#include "constants.h"
#include "num2word.h"
#include "patterns.h"
#include <phonemis/base/processor/layers/num2word/numerals.h>
#include <phonemis/base/processor/layers/num2word/patterns.h>
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

#include <array>

namespace phonemis::de {

using namespace constants::num2word;
using namespace processor;
using namespace processor::num2word;
using namespace utils;

namespace {
uint64_t value(const Number& number) {
  return value_of(number.integer).value_or(0);
}

// ------------------------
// ----- Number words -----
// ------------------------

std::u32string cardinal(uint64_t n);

// A cardinal as the first part of a compound: ein, einundzwanzig, einhundertein
std::u32string stem(uint64_t n) {
  std::u32string words = cardinal(n);
  return words.ends_with(U"eins") ? words.substr(0, words.size() - 1) : words;
}

std::u32string cardinal(uint64_t n) {
  if (n < 20) {
    return std::u32string(kOnes[n]);
  }
  if (n < 100) {
    return (n % 10 ? stem(n % 10) + U"und" : U"") + std::u32string(kTens[n / 10]);
  }
  if (n < 1000) {
    return stem(n / 100) + U"hundert" + (n % 100 ? cardinal(n % 100) : U"");
  }
  if (n < 1000000) {
    return stem(n / 1000) + U"tausend" + (n % 1000 ? cardinal(n % 1000) : U"");
  }
  for (auto [scale, one, many] :
       {std::tuple{1000000000ULL, U"Milliarde", U"Milliarden"}, std::tuple{1000000ULL, U"Million", U"Millionen"}}) {
    if (n >= scale) {
      uint64_t q = n / scale, r = n % scale;
      std::u32string head = q == 1 ? U"eine " + std::u32string(one) : cardinal(q) + U" " + std::u32string(many);
      return head + (r ? U" " + cardinal(r) : U"");
    }
  }
  return U"";
}

// 1100-1999 in hundreds (neunzehnhundertneunzig); later years read like cardinals
std::u32string year(uint64_t n) {
  if (n >= 1100 && n <= 1999) {
    return cardinal(n / 100) + U"hundert" + (n % 100 ? cardinal(n % 100) : U"");
  }
  return cardinal(n);
}

std::u32string ordinal(uint64_t n, std::u32string_view ending) {
  std::u32string words = cardinal(n);
  uint64_t r = n % 100;
  if (r > 0 && r < 20) {
    std::u32string one(kOnes[r]);
    auto it = kOrdinalStems.find(one);
    std::u32string base = words.substr(0, words.size() - one.size());
    return base + (it != kOrdinalStems.end() ? it->second : one) + U"t" + std::u32string(ending);
  }
  return words + U"st" + std::u32string(ending);
}

std::u32string digits(std::u32string_view text) {
  std::vector<std::u32string> words;
  for (char32_t c : text) {
    words.emplace_back(kOnes[c - U'0']);
  }
  return join(words);
}

// "1" said like an article, by case and gender: eine Stunde, mit einem Mal
std::u32string article(Case c, Gender g) {
  static const std::array<std::array<std::u32string_view, 3>, 4> forms = {{
      {U"ein", U"eine", U"ein"},      // Nominative
      {U"eines", U"einer", U"eines"}, // Genitive
      {U"einem", U"einer", U"einem"}, // Dative
      {U"einen", U"eine", U"ein"},    // Accusative
  }};
  size_t row = c == Case::GENITIVE ? 1 : c == Case::DATIVE ? 2 : c == Case::ACCUSATIVE ? 3 : 0;
  return std::u32string(forms[row][static_cast<size_t>(g)]);
}

// The adjective ending of an ordinal: am dritten, der dritte, ein zweiter, dritter
std::u32string_view ending(const NumberReading& reading) {
  static const std::array<std::array<std::u32string_view, 3>, 4> strong = {{
      {U"er", U"e", U"es"},  // Nominative
      {U"en", U"er", U"en"}, // Genitive
      {U"em", U"er", U"em"}, // Dative
      {U"en", U"e", U"es"},  // Accusative
  }};
  switch (reading.declension) {
    case Declension::WEAK:
      return reading.grammatical_case == Case::NOMINATIVE ? U"e" : U"en";
    case Declension::MIXED:
      return reading.gender == Gender::NEUTER ? U"es" : U"er";
    default:
      Case c = reading.grammatical_case;
      size_t row = c == Case::GENITIVE ? 1 : c == Case::DATIVE ? 2 : c == Case::ACCUSATIVE ? 3 : 0;
      return strong[row][static_cast<size_t>(reading.gender)];
  }
}

// -------------------
// ----- Context -----
// -------------------

Gender gender_of(std::u32string_view noun) {
  std::u32string word = strings::to_lower(noun);

  // The longest known ending decides, then the regular ones
  size_t longest = 0;
  Gender gender = Gender::MASCULINE;
  for (const auto& [key, key_gender] : kGenders) {
    if (key.size() > longest && word.ends_with(key)) {
      longest = key.size();
      gender = key_gender;
    }
  }
  if (longest > 0) {
    return gender;
  }

  auto ends_with_any = [&](const std::vector<std::u32string>& endings) {
    return std::ranges::any_of(endings, [&](const auto& e) { return word.ends_with(e); });
  };
  return ends_with_any(kFeminineEndings) ? Gender::FEMININE :
         ends_with_any(kNeuterEndings)   ? Gender::NEUTER :
                                           Gender::MASCULINE;
}

Case case_after(std::u32string_view word) {
  std::u32string w(word);
  return kDative.contains(w)     ? Case::DATIVE :
         kAccusative.contains(w) ? Case::ACCUSATIVE :
         kGenitive.contains(w)   ? Case::GENITIVE :
                                   Case::NOMINATIVE;
}

bool capitalized(std::u32string_view word) {
  return !word.empty() && unicode::isupper(word[0]);
}

bool all_digits(std::u32string_view word) {
  return !word.empty() && std::ranges::all_of(word, unicode::isdigit);
}

// A number right after a capitalized noun is its label: Seite 1, Platz 1
bool labelled(const Context& context) {
  auto head = words_of(context.head());
  return !head.empty() && capitalized(head.back()) &&
         !(head.size() == 1 || strings::contains(std::u32string_view(U".!?"), head[head.size() - 2].back()));
}

// The ending of an ordinal from the words before it, and the noun it qualifies.
void resolve_ending(const Context& context, std::u32string_view noun, NumberReading& reading) {
  std::vector<std::u32string> before;
  for (auto word : words_of(context.head())) {
    before.push_back(strings::to_lower(stripped(word)));
  }

  // "in 2. und 3. Lesung", "vom 1. 5. bis 3. 5.": both take the ending of the first
  static const std::array<std::u32string_view, 4> joining = {U"und", U"oder", U"bis", U"sowie"};
  while (before.size() >= 2 && std::ranges::find(joining, before.back()) != joining.end() &&
         all_digits(before[before.size() - 2])) {
    before.resize(before.size() - 2);
    while (!before.empty() && all_digits(before.back())) {
      before.pop_back();
    }
  }
  std::u32string w1 = before.empty() ? U"" : before.back();
  std::u32string w2 = before.size() < 2 ? U"" : before[before.size() - 2];

  reading.gender = gender_of(noun);
  if (kWeak.contains(w1) || (w1 == U"der" && (kDative.contains(w2) || kGenitive.contains(w2)))) {
    reading.declension = Declension::WEAK;
    reading.grammatical_case = Case::DATIVE;
  } else if (kWeakNominative.contains(w1)) {
    reading.declension = Declension::WEAK;
    reading.grammatical_case = Case::NOMINATIVE;
  } else if (kMixed.contains(w1)) {
    reading.declension = Declension::MIXED;
  } else {
    reading.declension = Declension::STRONG;
    reading.grammatical_case = case_after(w1);
  }
}
} // namespace

Num2Word::Num2Word() : Num2WordLayer(kNotation, kJoiners) {
  add_pattern(std::make_unique<DatePattern>(kDateFormat));
  add_pattern(std::make_unique<PhonePattern>());
  add_pattern(std::make_unique<DotOrdinalPattern>());
  add_pattern(std::make_unique<SuffixPattern>(Kind::DECADE, kDecadeSuffixes, kDecadeFormat));
}

// -----------------------
// ----- Resolution -----
// -----------------------

bool Num2Word::accept(const Match& match, const Context& context) const {
  switch (match.kind) {
    case Kind::DATE:
      // "am 12. 3." is a date, "Platz 1. 2 Tore" is not
      return match.text.find(U' ') == std::u32string_view::npos || kDateCues.contains(context.before());
    case Kind::TIME:
      if (match.numbers.size() == 2) {
        bool decimal = match.separator == U'.' && match.suffix.empty();
        return value(match.numbers[0]) <= 24 && value(match.numbers[1]) <= 59 && !decimal;
      }
      return true;
    case Kind::ORDINAL: {
      // "Platz 3. Danach", "Platz 1. 2 Tore": a sentence end, not an ordinal
      static const std::unordered_set<std::u32string> inflecting = [] {
        std::unordered_set<std::u32string> words = {U"und", U"oder", U"bis"};
        for (const auto* set : {&kWeak, &kWeakNominative, &kMixed, &kDative, &kAccusative, &kGenitive}) {
          words.insert(set->begin(), set->end());
        }
        return words;
      }();
      auto words = words_of(context.tail());
      std::u32string_view next = words.empty() ? U"" : words[0];
      bool month = std::ranges::find(kMonths, next) != kMonths.end();
      bool sentence = capitalized(next) && !month && !inflecting.contains(context.before());
      return !next.empty() && !unicode::isdigit(next[0]) && !sentence;
    }
    default:
      return true;
  }
}

NumberReading Num2Word::resolve(const Match& match, const Context& context) const {
  NumberReading reading;

  switch (match.kind) {
    case Kind::DATE:
      resolve_ending(context, U"Monat", reading);
      return reading;

    case Kind::ORDINAL: {
      // The noun is the first capitalized word after it: der 3. Platz
      std::u32string_view noun;
      for (auto word : words_of(context.tail())) {
        if (capitalized(word)) {
          size_t end = word.find_last_not_of(U".,;:!?");
          noun = word.substr(0, end == std::u32string_view::npos ? 0 : end + 1);
          break;
        }
      }
      resolve_ending(context, noun, reading);
      return reading;
    }

    case Kind::DECADE: {
      std::u32string next = context.after();
      reading.capitalized = next != U"Jahre" && next != U"Jahren" && next != U"Jahrgang";
      return reading;
    }

    default:
      break;
  }

  // "1" before a noun is said like an article, in the case of the word before it: mit einer Stimme
  std::u32string next = context.after();
  std::u32string noun = next;
  if ((next == U"bis" || next == U"oder") && all_digits(context.after(2))) {
    // "1 bis 2 Werktage": gendered by the plural noun
    noun = context.after(3);
    if (noun.ends_with(U"en")) {
      noun.pop_back();
    } else if (noun.ends_with(U'e')) {
      noun.pop_back();
    }
  }
  reading.article = capitalized(noun) && noun != U"Quadrat" && (noun != next || !labelled(context));
  reading.grammatical_case = case_after(context.before());
  reading.gender = gender_of(noun);

  // 1998 Abi gemacht, im Jahr 1945; but 1945 Einwohner, 1920 mal 1080
  bool month = std::ranges::find(kMonths, next) != kMonths.end();
  bool measure = next.ends_with(U'e') || next.ends_with(U'n') || next.ends_with(U"er") || next.ends_with(U's') ||
                 kMeasureWords.contains(next);
  bool counted = capitalized(next) && measure && !month;
  std::u32string previous = context.before();
  bool times = next == U"mal" || previous == U"mal";
  reading.year = !times && (kYearCues.contains(previous) || !counted);
  return reading;
}

// --------------------
// ----- Readings -----
// --------------------

std::u32string Num2Word::read_number(const Number& number, const NumberReading& reading) const {
  if (!number.fraction.empty()) {
    std::u32string head = number.integer.empty() ? U"" : cardinal(value(number)) + U" ";
    return head + U"Komma " + digits(number.fraction);
  }

  auto n = value_of(number.integer);
  if (!n || spoken_digits(number, 5)) {
    return digits(number.integer);
  }
  if (*n == 1 && reading.article) {
    return article(reading.grammatical_case, reading.gender);
  }
  if (number.integer.size() == 4 && !number.grouped && reading.year) {
    return year(*n);
  }
  return cardinal(*n);
}

std::u32string Num2Word::read_code_digits(std::u32string_view text) const {
  auto n = value_of(text);
  if ((text.size() > 1 && text[0] == U'0') || text.size() > 4 || !n) {
    return digits(text);
  }
  return year(*n);
}

std::u32string Num2Word::read_date(const Match& match, const NumberReading& reading) const {
  std::u32string words =
      ordinal(value(match.numbers[0]), ending(reading)) + U" " + kMonths[value(match.numbers[1]) - 1];
  return match.numbers.size() < 3 ? words : words + U" " + year(value(match.numbers[2]));
}

std::u32string Num2Word::read_phone(const Match& match, const NumberReading& reading) const {
  std::u32string all;
  for (const auto& group : match.numbers) {
    all += group.integer;
  }
  return digits(all);
}

std::u32string Num2Word::read_fraction(const Match& match, const NumberReading& reading) const {
  uint64_t a = value(match.numbers[0]), b = value(match.numbers[1]);
  auto it = kFractions.find({a, b});
  return it != kFractions.end() ? it->second : cardinal(a) + U" durch " + cardinal(b);
}

std::u32string Num2Word::read_time(const Match& match, const NumberReading& reading) const {
  std::u32string words = stem(value(match.numbers[0])) + U" Uhr";
  if (match.numbers.size() == 2 && value(match.numbers[1]) > 0) {
    words += U" " + cardinal(value(match.numbers[1]));
  }
  return words;
}

std::u32string Num2Word::read_ordinal(const Match& match, const NumberReading& reading) const {
  return ordinal(value(match.numbers[0]), ending(reading));
}

std::u32string Num2Word::read_decade(const Match& match, const NumberReading& reading) const {
  // 90er, 90ern: neunziger, neunzigern; 1990er: neunzehnhundertneunziger
  const auto& digits = match.numbers[0].integer;
  std::u32string number = digits.size() > 1 && digits[0] == U'0' ? U"null" + cardinal(value(match.numbers[0])) :
                                                                   year(value(match.numbers[0]));
  std::u32string words = number + std::u32string(match.suffix);
  if (reading.capitalized) {
    words[0] = unicode::toupper(words[0]);
  }
  return words;
}

std::u32string Num2Word::read_compound(const Match& match, const NumberReading& reading) const {
  // 30-jährige: dreißigjährige; 4-Zimmer-Wohnung: Vier-Zimmer-Wohnung
  const Number& number = match.numbers[0];
  std::u32string words =
      number.fraction.empty() && !number.grouped ? stem(value(number)) : read_number(number, reading);
  if (capitalized(match.suffix)) {
    words[0] = unicode::toupper(words[0]);
    return words + U"-" + std::u32string(match.suffix);
  }
  return words + std::u32string(match.suffix);
}

std::u32string Num2Word::read_money(const Number& number, const Match& match, const NumberReading& reading) const {
  const std::u32string& noun = kCurrencies.at(std::u32string(match.unit));
  std::u32string one = article(reading.grammatical_case, gender_of(noun));
  if (!match.scale.empty()) {
    return read_scale(number, match, reading) + U" " + noun;
  }

  // A price: drei Euro fünfzig, neunundneunzig Cent
  if (number.fraction.size() == 2) {
    uint64_t cents = value_of(number.fraction).value_or(0);
    if (value(number) == 0) {
      return cardinal(cents) + U" Cent";
    }
    Number whole{.integer = number.integer, .grouped = number.grouped};
    std::u32string words = number.integer == U"1" ? one : read_number(whole, reading);
    return words + U" " + noun + U" " + cardinal(cents);
  }

  bool single = number.integer == U"1" && number.fraction.empty();
  return (single ? one : read_number(number, reading)) + U" " + noun;
}

std::u32string Num2Word::read_scale(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& scale = kScales.at(std::u32string(match.scale));
  if (scale.many == U"tausend") {
    return read_number(number, reading) + U"tausend";
  }
  bool single = number.integer == U"1" && number.fraction.empty();
  return single ? U"eine " + scale.one : read_number(number, reading) + U" " + scale.many;
}

std::u32string Num2Word::read_percent(const Number& number, const Match& match, const NumberReading& reading) const {
  std::u32string words = read_number(number, reading);
  // 8%ige: achtprozentige
  return match.suffix.empty() ? words + U" Prozent" : words + U"prozent" + std::u32string(match.suffix);
}

std::u32string Num2Word::read_unit(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& unit = kUnits.at(std::u32string(match.unit));
  if (number.integer == U"1" && number.fraction.empty()) {
    return article(reading.grammatical_case, gender_of(unit.one)) + U" " + unit.one;
  }
  return read_number(number, reading) + U" " + unit.many;
}

} // namespace phonemis::de
