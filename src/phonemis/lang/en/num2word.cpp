#include "constants.h"
#include "num2word.h"
#include <phonemis/base/processor/layers/num2word/numerals.h>
#include <phonemis/base/processor/layers/num2word/patterns.h>
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::en {

using namespace constants::num2word;
using namespace processor;
using namespace processor::num2word;
using namespace utils;

namespace {
// Number words

std::u32string cardinal(uint64_t n) {
  if (n < 20) {
    return std::u32string(kOnes[n]);
  }
  if (n < 100) {
    return std::u32string(kTens[n / 10]) + (n % 10 ? U"-" + std::u32string(kOnes[n % 10]) : U"");
  }
  if (n < 1000) {
    return std::u32string(kOnes[n / 100]) + U" hundred" + (n % 100 ? U" " + cardinal(n % 100) : U"");
  }
  for (const auto& [value, word] : kScaleWords) {
    if (n >= value) {
      return cardinal(n / value) + U" " + word + (n % value ? U" " + cardinal(n % value) : U"");
    }
  }
  return U"";
}

std::u32string ordinal(uint64_t n) {
  // Only the last word changes: twenty-first, one hundred and twelfth
  std::u32string words = cardinal(n);
  size_t start = words.find_last_of(U" -") + 1;
  std::u32string last = words.substr(start);
  words.erase(start);

  if (auto it = kIrregularOrdinals.find(last); it != kIrregularOrdinals.end()) {
    return words + it->second;
  }
  return words + (last.ends_with(U'y') ? last.substr(0, last.size() - 1) + U"ieth" : last + U"th");
}

// Year-style reading, also of 3-4 digit codes: nineteen oh five, two thousand seven, nine fifty-six
std::u32string pairs(uint64_t n) {
  uint64_t high = n / 100, low = n % 100;
  if (n >= 1000 && n % 1000 < 10) {
    return cardinal(n);
  }
  if (low == 0) {
    return cardinal(high) + U" hundred";
  }
  return cardinal(high) + U" " + (low < 10 ? U"oh " + std::u32string(kOnes[low]) : cardinal(low));
}

std::u32string digits(std::u32string_view text) {
  std::vector<std::u32string> words;
  for (char32_t c : text) {
    words.emplace_back(kOnes[c - U'0']);
  }
  return join(words);
}

// a.m. and p.m. as their lowercase abbreviations are said; AM and PM stay as written
std::u32string ampm(std::u32string_view suffix) {
  std::u32string base = strings::to_lower(suffix.substr(0, suffix.find_last_not_of(U'.') + 1));
  bool lowercase = std::ranges::none_of(suffix, [](char32_t c) { return unicode::isupper(c); });
  if (lowercase && (base == U"am" || base == U"pm")) {
    return base == U"am" ? U"a.m." : U"p.m.";
  }
  return std::u32string(suffix);
}

uint64_t value(const Number& number) {
  return value_of(number.integer).value_or(0);
}
} // namespace

Num2Word::Num2Word() : Num2WordLayer(kNotation, kJoiners) {
  add_pattern(std::make_unique<DatePattern>(kDateFormat));
  for (const auto& format : kPhoneFormats) {
    add_pattern(std::make_unique<PhonePattern>(format));
  }
  add_pattern(std::make_unique<SuffixPattern>(Kind::ORDINAL, kOrdinalSuffixes));
  add_pattern(std::make_unique<SuffixPattern>(Kind::DECADE, kDecadeSuffixes, kDecadeFormat));
}

// -----------------------
// ----- Resolution -----
// -----------------------

bool Num2Word::accept(const Match& match, const Context& context) const {
  switch (match.kind) {
    case Kind::FRACTION:
      return kFractions.contains({value(match.numbers[0]), value(match.numbers[1])});
    case Kind::SCALE:
      return match.scale != U"K"; // 4K: a resolution
    case Kind::TIME: {
      uint64_t hours = value(match.numbers[0]);
      // 7pm: an hour alone needs am or pm
      if (match.numbers.size() == 1) {
        return hours <= 12 && strings::contains(std::u32string_view(U"apAP"), match.suffix[0]);
      }
      // 1.13 is a decimal, unless a time is expected: at 10.30
      bool decimal = match.separator == U'.' && match.suffix.empty() && !kTimeWords.contains(context.before());
      return hours <= 24 && value(match.numbers[1]) <= 59 && !decimal;
    }
    default:
      return true;
  }
}

NumberReading Num2Word::resolve(const Match& match, const Context& context) const {
  std::u32string previous = context.before();
  std::u32string next = context.after();
  std::u32string next_lower = strings::to_lower(next);

  // 1999 people: a plural noun after a 4-digit number makes it a count, unless a year is expected (in 1999)
  bool plural_noun =
      kIrregularPlurals.contains(next_lower) || (next_lower.size() > 3 && next_lower.ends_with(U's') &&
                                                 !next_lower.ends_with(U"ss") && !kStopWords.contains(next_lower));
  bool year_cue = kYearCues.contains(previous) || kMonths.contains(previous);

  return {.after_month = kMonths.contains(previous),
          .counted = plural_noun && !year_cue,
          .attributive = !next.empty() && unicode::isalpha(next[0]) && !kStopWords.contains(next_lower)};
}

// --------------------
// ----- Readings -----
// --------------------

std::u32string Num2Word::read_integer(const Number& number, const NumberReading& reading) const {
  auto n = value_of(number.integer);
  if (!n || spoken_digits(number, 5)) {
    return digits(number.integer);
  }
  if (reading.after_month && *n >= 1 && *n <= 31) {
    return ordinal(*n);
  }
  if (number.integer.size() == 4 && !number.grouped) {
    return reading.counted ? cardinal(*n) : pairs(*n);
  }
  return cardinal(*n);
}

std::u32string Num2Word::read_number(const Number& number, const NumberReading& reading) const {
  if (number.fraction.empty()) {
    return read_integer(number, reading);
  }

  // .5 has no zero before its point
  std::u32string head = number.integer.empty() ? U"" : cardinal(value(number)) + U" ";
  return head + (number.fraction == U"0" ? U"point oh" : U"point " + digits(number.fraction));
}

std::u32string Num2Word::read_code_digits(std::u32string_view text) const {
  auto n = value_of(text);
  if (text.size() > 1 && text[0] == U'0') {
    std::vector<std::u32string> words;
    for (char32_t c : text) {
      words.emplace_back(c == U'0' ? U"oh" : kOnes[c - U'0']);
    }
    return join(words);
  }
  if (!n || text.size() > 4) {
    return digits(text);
  }
  return text.size() <= 2 ? cardinal(*n) : pairs(*n);
}

std::u32string Num2Word::read_date(const Match& match, const NumberReading& reading) const {
  std::u32string words = ordinal(value(match.numbers[0])) + U" " + kMonthNames[value(match.numbers[1]) - 1];
  if (match.numbers.size() < 3) {
    return words;
  }
  const Number& year = match.numbers[2];
  return words + U" " + (year.integer.size() == 4 ? pairs(value(year)) : cardinal(value(year)));
}

std::u32string Num2Word::read_phone(const Match& match, const NumberReading& reading) const {
  // 1-555-...: the leading one joins the area code
  std::vector<std::u32string> groups;
  for (const auto& number : match.numbers) {
    groups.push_back(number.integer);
  }
  if (match.text.starts_with(U"1-")) {
    groups[0] = U"1" + groups[0];
  }

  std::vector<std::u32string> words;
  for (const auto& group : groups) {
    words.push_back(digits(group));
  }
  return join(words, U", ");
}

std::u32string Num2Word::read_fraction(const Match& match, const NumberReading& reading) const {
  return kFractions.at({value(match.numbers[0]), value(match.numbers[1])});
}

std::u32string Num2Word::read_time(const Match& match, const NumberReading& reading) const {
  std::u32string hours = cardinal(value(match.numbers[0]));
  std::u32string suffix = ampm(match.suffix);
  if (match.numbers.size() == 1) {
    return hours + U" " + suffix;
  }

  // ten thirty, ten oh five, ten o'clock
  const auto& minutes = match.numbers[1].integer;
  std::u32string words = hours;
  if (minutes != U"00") {
    uint64_t m = value(match.numbers[1]);
    words += minutes[0] == U'0' ? U" oh " + std::u32string(kOnes[m]) : U" " + cardinal(m);
  }
  if (!suffix.empty()) {
    return words + U" " + suffix;
  }
  return words + (minutes == U"00" ? U" o'clock" : U"");
}

std::u32string Num2Word::read_ordinal(const Match& match, const NumberReading& reading) const {
  return ordinal(value(match.numbers[0]));
}

std::u32string Num2Word::read_decade(const Match& match, const NumberReading& reading) const {
  return *plural(read_number(match.numbers[0], reading));
}

std::u32string Num2Word::read_money(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& currency = kCurrencies.at(std::u32string(match.unit));

  // A price: four ninety-nine, ninety-nine cents
  if (number.fraction.size() == 2 && match.scale.empty()) {
    uint64_t cents_value = value_of(number.fraction).value_or(0);
    if (value(number) == 0) {
      return cardinal(cents_value) + U" cents";
    }
    std::u32string cents =
        number.fraction[0] == U'0' ? U"oh " + std::u32string(kOnes[cents_value]) : cardinal(cents_value);
    Number whole{.integer = number.integer, .grouped = number.grouped};
    return read_integer(whole, reading) + U" " + cents;
  }

  std::u32string words = read_number(number, reading);
  if (!match.scale.empty()) {
    words += U" " + kScales.at(std::u32string(match.scale));
  }
  bool one = number.integer == U"1" && number.fraction.empty() && match.scale.empty();
  return words + U" " + (one || reading.attributive ? currency.one : currency.many);
}

std::u32string Num2Word::read_scale(const Number& number, const Match& match, const NumberReading& reading) const {
  return read_number(number, reading) + U" " + kScales.at(std::u32string(match.scale));
}

std::u32string Num2Word::read_percent(const Number& number, const Match& match, const NumberReading& reading) const {
  return read_number(number, reading) + U" percent" +
         (match.suffix.empty() ? U"" : U" " + std::u32string(match.suffix));
}

std::u32string Num2Word::read_unit(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& unit = kUnits.at(std::u32string(match.unit));
  bool one = number.fraction.empty() && value(number) == 1;
  return read_number(number, reading) + U" " + (one || reading.attributive ? unit.one : unit.many);
}

std::u32string Num2Word::read_range(const Number& from, const Number& to, const NumberReading& reading) const {
  if (!from.fraction.empty() || !to.fraction.empty()) {
    return joiners_.range;
  }

  // 10-15 is "ten to fifteen"; but scores and abbreviated seasons keep "-": 51-44, 2016-17, 21-22
  uint64_t a = value(from), b = value(to);
  bool season = from.integer.size() == 2 && b == a + 1;
  return from.integer.size() == to.integer.size() && b > a && !season ? joiners_.range : U"-";
}

std::optional<std::u32string> Num2Word::plural(std::u32string_view words) const {
  if (words.ends_with(U'y')) {
    return std::u32string(words.substr(0, words.size() - 1)) + U"ies";
  }
  return std::u32string(words) + (words.ends_with(U'x') ? U"es" : U"s");
}

} // namespace phonemis::en
