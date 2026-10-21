#include "constants.h"
#include "num2word.h"
#include "patterns.h"
#include <phonemis/base/processor/layers/num2word/numerals.h>
#include <phonemis/base/processor/layers/num2word/patterns.h>
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/strings.h>

namespace phonemis::it {

using namespace constants::num2word;
using namespace processor;
using namespace processor::num2word;
using namespace utils;

namespace {
uint64_t value(const Number& number) {
  return value_of(number.integer).value_or(0);
}

// The number "1" as written, which counts a singular noun
bool is_one(const Number& number) {
  return number.integer == U"1" && number.fraction.empty();
}

// ------------------------
// ----- Number words -----
// ------------------------

// ventuno, ventotto: a ten drops its vowel before uno and otto
std::u32string below_hundred(uint64_t n) {
  if (n < 20) {
    return std::u32string(kOnes[n]);
  }
  std::u32string tens(kTens[n / 10]);
  uint64_t unit = n % 10;
  if (unit == 1 || unit == 8) {
    tens.pop_back();
  }
  return tens + (unit ? std::u32string(kOnes[unit]) : U"");
}

// centotto, novecentottanta: a hundred drops its vowel before otto and ottanta
std::u32string below_thousand(uint64_t n) {
  uint64_t h = n / 100, r = n % 100;
  std::u32string head = h == 0 ? U"" : (h == 1 ? U"cento" : std::u32string(kOnes[h]) + U"cento");
  if (!head.empty() && (r == 8 || (r >= 80 && r <= 89))) {
    head.pop_back();
  }
  return head + (r || head.empty() ? below_hundred(r) : U"");
}

// ventitré, centotré, milletré: a compound ending in tre takes the accent
std::u32string accented(std::u32string_view words) {
  std::vector<std::u32string> out;
  for (auto word : strings::split(words, U' ')) {
    std::u32string w(word);
    if (w.size() > 3 && w.ends_with(U"tre")) {
      w.replace(w.size() - 3, 3, U"tré");
    }
    out.push_back(w);
  }
  return join(out);
}

// One word below a million (duemilaventidue), then un milione duecentomila
std::u32string cardinal(uint64_t n) {
  std::vector<std::u32string> words;
  for (auto [scale, one, many] :
       {std::tuple{1000000000ULL, U"un miliardo", U"miliardi"}, std::tuple{1000000ULL, U"un milione", U"milioni"}}) {
    if (uint64_t q = n / scale) {
      words.push_back(q == 1 ? std::u32string(one) : cardinal(q) + U" " + many);
    }
    n %= scale;
  }

  uint64_t thousands = n / 1000;
  n %= 1000;
  std::u32string head = thousands == 0 ? U"" : (thousands == 1 ? U"mille" : below_thousand(thousands) + U"mila");
  if (!head.empty() || n || words.empty()) {
    words.push_back(head + (n || head.empty() ? below_thousand(n) : U""));
  }
  return accented(join(words));
}

// primo ... decimo, then the cardinal with -esimo: undicesimo, ventitreesimo, millesimo
std::u32string ordinal(uint64_t n, char32_t ending) {
  if (n <= 10) {
    std::u32string word(kOrdinals[n]);
    word.back() = ending;
    return word;
  }

  std::u32string words = cardinal(n);
  if (words.ends_with(U"tré")) {
    words.back() = U'e';
  }
  if (words.ends_with(U"tre") || words.ends_with(U"sei")) {
    return words + U"esim" + ending;
  }
  if (words.ends_with(U"mila")) {
    return words.substr(0, words.size() - 4) + U"millesim" + ending;
  }
  return words.substr(0, words.size() - 1) + U"esim" + ending;
}

std::u32string digits(std::u32string_view text) {
  std::vector<std::u32string> words;
  for (char32_t c : text) {
    words.emplace_back(kOnes[c - U'0']);
  }
  return join(words);
}
} // namespace

Num2Word::Num2Word() : Num2WordLayer(kNotation, kJoiners) {
  add_pattern(std::make_unique<DatePattern>(kDateFormat));
  for (const auto& format : kPhoneFormats) {
    add_pattern(std::make_unique<PhonePattern>(format));
  }
  add_pattern(std::make_unique<SuffixPattern>(Kind::ORDINAL, kOrdinalSigns, kOrdinalSignFormat));
  add_pattern(std::make_unique<SuffixPattern>(Kind::ORDINAL, kOrdinalSuffixes));
  add_pattern(std::make_unique<DecadePattern>());
}

// ----------------------
// ----- Resolution -----
// ----------------------

bool Num2Word::accept(const Match& match, const Context& context) const {
  switch (match.kind) {
    case Kind::FRACTION:
      return kFractions.contains({value(match.numbers[0]), value(match.numbers[1])});
    case Kind::TIME: {
      // With a dot, a time comes after "alle" or from 13 on: alle 9.30, 21.30, but versione 2.79
      uint64_t hours = value(match.numbers[0]);
      bool decimal = match.separator == U'.' && !kTimeWords.contains(context.before()) && (hours < 13 || hours > 23);
      return hours <= 24 && value(match.numbers[1]) <= 59 && !decimal;
    }
    case Kind::SCALE:
      // 4K: a resolution
      return (match.scale != U"k" && match.scale != U"K") || value(match.numbers.back()) >= 10;
    case Kind::ORDINAL:
      return value(match.numbers[0]) > 0;
    default:
      return true;
  }
}

NumberReading Num2Word::resolve(const Match& match, const Context& context) const {
  return {.month = std::ranges::find(kMonths, strings::to_lower(context.after())) != kMonths.end()};
}

// --------------------
// ----- Readings -----
// --------------------

std::u32string Num2Word::read_number(const Number& number, const NumberReading& reading) const {
  if (number.fraction.empty()) {
    auto n = value_of(number.integer);
    if (!n || spoken_digits(number, 7)) {
      return digits(number.integer);
    }
    return *n == 1 && reading.month ? U"primo" : cardinal(*n);
  }

  // The separator is read as written: due virgola cinque, due punto cinque
  std::vector<std::u32string> fraction;
  size_t zeros = number.fraction.find_first_not_of(U'0');
  fraction.insert(fraction.end(), std::min(zeros, number.fraction.size()), U"zero");
  if (zeros != std::u32string::npos) {
    fraction.push_back(cardinal(value_of(number.fraction).value_or(0)));
  }
  std::u32string head = number.integer.empty() ? U"" : cardinal(value(number)) + U" ";
  return head + (number.decimal_mark == U',' ? U"virgola " : U"punto ") + join(fraction);
}

std::u32string Num2Word::read_code_digits(std::u32string_view text) const {
  auto n = value_of(text);
  if ((text.size() > 1 && text[0] == U'0') || text.size() >= 7 || !n) {
    return digits(text);
  }
  return cardinal(*n);
}

std::u32string Num2Word::read_date(const Match& match, const NumberReading& reading) const {
  uint64_t day = value(match.numbers[0]);
  return (day == 1 ? U"primo" : cardinal(day)) + U" " + kMonths[value(match.numbers[1]) - 1] + U" " +
         cardinal(value(match.numbers[2]));
}

std::u32string Num2Word::read_phone(const Match& match, const NumberReading& reading) const {
  // Digit by digit: tre tre tre, uno due tre ...
  std::vector<std::u32string> groups;
  for (const auto& group : match.numbers) {
    groups.push_back(digits(group.integer));
  }
  return join(groups);
}

std::u32string Num2Word::read_fraction(const Match& match, const NumberReading& reading) const {
  return kFractions.at({value(match.numbers[0]), value(match.numbers[1])});
}

std::u32string Num2Word::read_time(const Match& match, const NumberReading& reading) const {
  // alle 21.30: ventuno e trenta
  uint64_t hours = value(match.numbers[0]), minutes = value(match.numbers[1]);
  std::u32string hour = hours == 1 ? U"una" : cardinal(hours);
  return minutes ? hour + U" e " + cardinal(minutes) : hour;
}

std::u32string Num2Word::read_ordinal(const Match& match, const NumberReading& reading) const {
  char32_t ending = match.suffix == U"ª" ? U'a' : (match.suffix == U"º" ? U'o' : match.suffix.back());
  return ordinal(value(match.numbers[0]), ending);
}

std::u32string Num2Word::read_decade(const Match& match, const NumberReading& reading) const {
  return cardinal(value(match.numbers[0])); // gli anni '80: gli anni ottanta
}

std::u32string Num2Word::read_money(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& currency = kCurrencies.at(std::u32string(match.unit));

  // 3 mln €: tre milioni di euro, but 10 mila €: diecimila euro
  if (!match.scale.empty()) {
    bool thousands = kScales.at(std::u32string(match.scale)).one == U"mila";
    return read_scale(number, match, reading) + (thousands ? U" " : U" di ") + currency.many;
  }

  // 10,50 €: dieci euro e cinquanta
  if (number.fraction.size() == 2 && number.decimal_mark == U',') {
    Number whole{.integer = number.integer.empty() ? U"0" : number.integer, .grouped = number.grouped};
    uint64_t cents = value_of(number.fraction).value_or(0);
    return read_number(whole, reading) + U" " + (number.integer == U"1" ? currency.one : currency.many) +
           (cents ? U" e " + cardinal(cents) : U"");
  }
  return read_number(number, reading) + U" " + (is_one(number) ? currency.one : currency.many);
}

std::u32string Num2Word::read_scale(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& scale = kScales.at(std::u32string(match.scale));

  // 74 mila: settantaquattromila
  if (scale.one == U"mila" && number.fraction.empty()) {
    return cardinal(value(number) * 1000);
  }
  bool singular = value(number) < 2 && number.fraction.empty();
  return read_number(number, reading) + U" " + (singular ? scale.one : scale.many);
}

std::u32string Num2Word::read_percent(const Number& number, const Match& match, const NumberReading& reading) const {
  std::u32string suffix = match.suffix.empty() ? U"" : U" " + std::u32string(match.suffix);
  return read_number(number, reading) + U" per cento" + suffix;
}

std::u32string Num2Word::read_unit(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& unit = kUnits.at(std::u32string(match.unit));
  return read_number(number, reading) + U" " + (is_one(number) ? unit.one : unit.many);
}

} // namespace phonemis::it
