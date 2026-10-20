#include "constants.h"
#include "num2word.h"
#include "patterns.h"
#include <phonemis/base/processor/layers/num2word/numerals.h>
#include <phonemis/base/processor/layers/num2word/patterns.h>
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::fr {

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

std::u32string below_hundred(uint64_t n, bool feminine, bool plural_s) {
  if (n < 20) {
    return feminine && n == 1 ? U"une" : std::u32string(kOnes[n]);
  }

  // soixante-dix, quatre-vingt-dix
  uint64_t t = n / 10, u = n % 10;
  if (t == 7 || t == 9) {
    t--;
    u += 10;
  }
  if (u == 0) {
    return std::u32string(kTens[t]) + (t == 8 && plural_s ? U"s" : U"");
  }
  std::u32string unit = feminine && u == 1 ? U"une" : std::u32string(kOnes[u]);
  return std::u32string(kTens[t]) + ((u == 1 || u == 11) && t < 8 ? U" et " : U"-") + unit;
}

std::u32string below_thousand(uint64_t n, bool feminine, bool plural_s) {
  uint64_t h = n / 100, r = n % 100;
  if (h == 0) {
    return below_hundred(r, feminine, plural_s);
  }
  std::u32string head = h == 1 ? U"cent" : std::u32string(kOnes[h]) + U" cent" + (r == 0 && plural_s ? U"s" : U"");
  return head + (r ? U" " + below_hundred(r, feminine, plural_s) : U"");
}

// A cardinal; `plural_s` is false when "mille" follows (deux cent mille, quatre-vingt mille).
std::u32string cardinal(uint64_t n, bool feminine = false, bool plural_s = true) {
  if (n < 1000) {
    return below_thousand(n, feminine, plural_s);
  }
  if (n < 1000000) {
    uint64_t t = n / 1000, r = n % 1000;
    std::u32string head = t == 1 ? U"mille" : below_thousand(t, false, false) + U" mille";
    return head + (r ? U" " + below_thousand(r, feminine, plural_s) : U"");
  }
  for (auto [scale, word] : {std::pair{1000000000ULL, U"milliard"}, std::pair{1000000ULL, U"million"}}) {
    if (n >= scale) {
      uint64_t q = n / scale, r = n % scale;
      std::u32string head = cardinal(q) + U" " + std::u32string(word) + (q > 1 ? U"s" : U"");
      return head + (r ? U" " + cardinal(r, feminine, plural_s) : U"");
    }
  }
  return U"";
}

std::u32string ordinal(uint64_t n, bool feminine = false) {
  if (n == 1) {
    return feminine ? U"première" : U"premier";
  }

  // vingtième, cinquième, neuvième, onzième
  std::u32string words = cardinal(n);
  if (words.ends_with(U"vingts") || words.ends_with(U"cents")) {
    words.pop_back();
  }
  if (words.ends_with(U"neuf")) {
    words.back() = U'v';
  } else if (words.ends_with(U"cinq")) {
    words += U"u";
  }
  if (words.ends_with(U'e')) {
    words.pop_back();
  }
  return words + U"ième";
}

std::u32string digits(std::u32string_view text) {
  std::vector<std::u32string> words;
  for (char32_t c : text) {
    words.emplace_back(kOnes[c - U'0']);
  }
  return join(words);
}

// -------------------
// ----- Context -----
// -------------------

bool feminine(std::u32string_view noun) {
  std::u32string word = strings::to_lower(noun);
  word = word.substr(word.find_last_of(U'-') + 1);
  if (!kFeminineNouns.contains(word) && word.ends_with(U's')) {
    word.pop_back();
  }
  if (kFeminineNouns.contains(word)) {
    return true;
  }
  bool masculine_ending = std::ranges::any_of(kMasculineEndings, [&](const auto& e) { return word.ends_with(e); });
  bool date = std::ranges::find(kMonths, word) != kMonths.end() || kWeekdays.contains(word);
  return word.ends_with(U'e') && !kMasculineInE.contains(word) && !date && !masculine_ending;
}

// A lowercase noun follows: 10k abonnés
bool counted(const Context& context) {
  std::u32string word = context.after();
  bool lowercase_word = word.size() > 2 && std::ranges::all_of(word, [](char32_t c) { return unicode::islower(c); });
  return lowercase_word && !kStopWords.contains(word) && !kNumberWords.contains(word);
}

// A noun after a number is singular below 2: un virgule cinq kilomètre, deux kilomètres
const std::u32string& number_form(const Number& number, const Forms& forms) {
  return value(number) < 2 ? forms.one : forms.many;
}
} // namespace

Num2Word::Num2Word() : Num2WordLayer(kNotation, kJoiners) {
  add_pattern(std::make_unique<DatePattern>(kDateFormat));
  add_pattern(std::make_unique<PhonePattern>(kPhoneFormat));
  add_pattern(std::make_unique<SuffixPattern>(Kind::ORDINAL, kOrdinalSuffixes));
  add_pattern(std::make_unique<RomanOrdinalPattern>());
}

// -----------------------
// ----- Resolution -----
// -----------------------

bool Num2Word::accept(const Match& match, const Context& context) const {
  switch (match.kind) {
    case Kind::DATE:
      // Without a year only a zero-padded day or month is a date: 05/11, but not 3/4
      return match.numbers.size() == 3 || match.numbers[0].integer[0] == U'0' || match.numbers[1].integer[0] == U'0';
    case Kind::TIME:
      if (match.numbers.size() == 2) {
        bool decimal = match.separator == U'.' && !kTimeWords.contains(context.before());
        return value(match.numbers[0]) <= 24 && value(match.numbers[1]) <= 59 && !decimal;
      }
      return true;
    case Kind::SCALE:
      // 4K, 3k de Q: a letter, read as a code
      return kScales.at(std::u32string(match.scale)) != U"mille" || counted(context);
    default:
      return true;
  }
}

NumberReading Num2Word::resolve(const Match& match, const Context& context) const {
  std::u32string previous = context.before();
  return {.feminine = feminine(context.after()),
          .plural_s = previous != U"années" && previous != U"année",
          .equation = match.kind == Kind::DIMENSION && context.tail().find(U'=') != std::u32string_view::npos};
}

// --------------------
// ----- Readings -----
// --------------------

std::u32string Num2Word::read_gendered(const Number& number, const NumberReading& reading, bool feminine) const {
  if (number.fraction.empty()) {
    auto n = value_of(number.integer);
    return !n || spoken_digits(number, 5) ? digits(number.integer) : cardinal(*n, feminine, reading.plural_s);
  }

  // Up to three decimals read as a number: zéro virgule zéro cinq, deux virgule soixante-quinze
  std::u32string fraction;
  if (number.fraction.size() > 3) {
    fraction = digits(number.fraction);
  } else {
    std::vector<std::u32string> words;
    size_t zeros = number.fraction.find_first_not_of(U'0');
    words.insert(words.end(), std::min(zeros, number.fraction.size()), U"zéro");
    if (zeros != std::u32string::npos) {
      words.push_back(cardinal(value_of(number.fraction).value_or(0)));
    }
    fraction = join(words);
  }
  std::u32string head = number.integer.empty() ? U"" : cardinal(value(number), feminine) + U" ";
  return head + U"virgule " + fraction;
}

std::u32string Num2Word::read_number(const Number& number, const NumberReading& reading) const {
  return read_gendered(number, reading, number.fraction.empty() && reading.feminine);
}

std::u32string Num2Word::read_code_digits(std::u32string_view text) const {
  auto n = value_of(text);
  if ((text.size() > 1 && text[0] == U'0') || text.size() > 4 || !n) {
    return digits(text);
  }
  return cardinal(*n);
}

std::u32string Num2Word::read_date(const Match& match, const NumberReading& reading) const {
  uint64_t day = value(match.numbers[0]);
  std::u32string words = (day == 1 ? ordinal(1) : cardinal(day)) + U" " + kMonths[value(match.numbers[1]) - 1];
  return match.numbers.size() < 3 ? words : words + U" " + cardinal(value(match.numbers[2]));
}

std::u32string Num2Word::read_phone(const Match& match, const NumberReading& reading) const {
  // In pairs: zéro six douze trente-quatre ...
  std::vector<std::u32string> words;
  for (const auto& pair : match.numbers) {
    bool zero = pair.integer.size() == 2 && pair.integer[0] == U'0';
    if (pair.integer.size() > 3) {
      words.push_back(digits(pair.integer)); // A longer group of a foreign number
    } else {
      words.push_back(zero ? U"zéro " + std::u32string(kOnes[pair.integer[1] - U'0']) : cardinal(value(pair)));
    }
  }
  return join(words);
}

std::u32string Num2Word::read_fraction(const Match& match, const NumberReading& reading) const {
  uint64_t a = value(match.numbers[0]), b = value(match.numbers[1]);
  auto it = kFractions.find({a, b});
  return it != kFractions.end() ? it->second : cardinal(a) + U" sur " + cardinal(b);
}

std::u32string Num2Word::read_time(const Match& match, const NumberReading& reading) const {
  uint64_t hours = value(match.numbers[0]);
  std::u32string words = cardinal(hours, true) + (hours < 2 ? U" heure" : U" heures");
  if (match.numbers.size() == 2 && value(match.numbers[1]) > 0) {
    words += U" " + cardinal(value(match.numbers[1]), true);
  }
  return words;
}

std::u32string Num2Word::read_ordinal(const Match& match, const NumberReading& reading) const {
  std::u32string_view suffix = match.suffix;
  std::u32string words;
  if (suffix.starts_with(U"nd")) {
    words = suffix.starts_with(U"nde") ? U"seconde" : U"second";
  } else {
    bool feminine = suffix == U"re" || suffix == U"ère" || suffix == U"res" || suffix == U"ères";
    words = ordinal(value(match.numbers[0]), feminine);
  }
  return words + (suffix.ends_with(U's') ? U"s" : U"");
}

std::u32string Num2Word::read_money(const Number& number, const Match& match, const NumberReading& reading) const {
  Forms currency = kCurrencies.at(std::u32string(match.unit));
  std::u32string scale(match.scale);
  if (match.unit == U"M€") {
    scale = U"millions";
    currency = {U"euro", U"euros"};
  }

  if (!scale.empty()) {
    // trois millions d'euros, but dix mille euros
    std::u32string words = read_scale(number, Match{.scale = scale}, reading);
    if (kScales.at(scale) == U"mille") {
      return words + U" " + currency.many;
    }
    bool vowel = strings::contains(std::u32string_view(U"aeiouyéh"), currency.many[0]);
    return words + (vowel ? U" d'" : U" de ") + currency.many;
  }

  // A price: trois euros cinquante, quatre-vingt-dix-neuf centimes
  bool feminine = currency.one == U"livre";
  if (number.fraction.size() == 2) {
    uint64_t cents = value_of(number.fraction).value_or(0);
    if (value(number) == 0) {
      return cardinal(cents) + U" centimes";
    }
    Number whole{.integer = number.integer, .grouped = number.grouped};
    return read_gendered(whole, reading, feminine) + U" " + (number.integer == U"1" ? currency.one : currency.many) +
           U" " + cardinal(cents);
  }
  return read_gendered(number, reading, feminine) + U" " + number_form(number, currency);
}

std::u32string Num2Word::read_scale(const Number& number, const Match& match, const NumberReading& reading) const {
  const std::u32string& word = kScales.at(std::u32string(match.scale));
  if (word == U"mille") {
    return read_gendered(number, reading, false) + U" mille";
  }

  // trois millions, un virgule cinq million
  std::u32string one = word.ends_with(U's') ? word.substr(0, word.size() - 1) : word;
  return read_gendered(number, reading, false) + U" " + number_form(number, {one, one + U"s"});
}

std::u32string Num2Word::read_percent(const Number& number, const Match& match, const NumberReading& reading) const {
  std::u32string suffix = match.suffix.empty() ? U"" : U" " + std::u32string(match.suffix);
  return read_gendered(number, reading, false) + U" pour cent" + suffix;
}

std::u32string Num2Word::read_unit(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& unit = kUnits.at(std::u32string(match.unit));
  std::u32string_view first_word = std::u32string_view(unit.one).substr(0, unit.one.find(U' '));
  return read_gendered(number, reading, feminine(first_word)) + U" " + number_form(number, unit);
}

std::u32string Num2Word::read_joiner(const Match& match, const NumberReading& reading) const {
  if (match.kind == Kind::DIMENSION) {
    return reading.equation ? U" fois " : U" par ";
  }
  return Num2WordLayer::read_joiner(match, reading);
}

} // namespace phonemis::fr
