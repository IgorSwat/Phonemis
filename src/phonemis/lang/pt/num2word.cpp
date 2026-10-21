#include "constants.h"
#include "num2word.h"
#include <phonemis/base/processor/layers/num2word/numerals.h>
#include <phonemis/base/processor/layers/num2word/patterns.h>
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::pt {

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

bool is_ascii_letter(char32_t c) {
  return unicode::is_ascii(c) && unicode::isalpha(c);
}

// ------------------------
// ----- Number words -----
// ------------------------

std::u32string unit(uint64_t n, Gender gender) {
  if (gender == Gender::FEMININE && (n == 1 || n == 2)) {
    return n == 1 ? U"uma" : U"duas";
  }
  return std::u32string(kOnes[n]);
}

std::u32string below_hundred(uint64_t n, Gender gender) {
  if (n < 20) {
    return unit(n, gender);
  }
  return std::u32string(kTens[n / 10]) + (n % 10 ? U" e " + unit(n % 10, gender) : U"");
}

std::u32string below_thousand(uint64_t n, Gender gender) {
  uint64_t h = n / 100, r = n % 100;
  if (h == 0) {
    return below_hundred(r, gender);
  }
  if (n == 100) {
    return U"cem";
  }

  // duzentas pessoas
  std::u32string head(kHundreds[h]);
  if (gender == Gender::FEMININE && h > 1) {
    head.replace(head.size() - 2, 2, U"as");
  }
  return head + (r ? U" e " + below_hundred(r, gender) : U"");
}

// "e" joins the last group below 100 or of round hundreds: dois mil e vinte, mil quinhentos e vinte, mil duzentos e um
std::u32string cardinal(uint64_t n, Gender gender = Gender::MASCULINE) {
  std::vector<std::pair<uint64_t, std::u32string>> groups;
  for (auto [scale, one, many] :
       {std::tuple{1000000000000ULL, U"trilhão", U"trilhões"}, std::tuple{1000000000ULL, U"bilhão", U"bilhões"},
        std::tuple{1000000ULL, U"milhão", U"milhões"}}) {
    if (uint64_t q = n / scale) {
      groups.emplace_back(q, cardinal(q) + U" " + (q == 1 ? one : many));
    }
    n %= scale;
  }

  // Thousands agree with the noun: duas mil pessoas
  if (uint64_t q = n / 1000) {
    groups.emplace_back(q, q == 1 ? U"mil" : below_thousand(q, gender) + U" mil");
  }
  n %= 1000;
  if (n || groups.empty()) {
    groups.emplace_back(n, below_thousand(n, gender));
  }

  std::u32string words = groups[0].second;
  for (size_t i = 1; i < groups.size(); ++i) {
    uint64_t last = groups[i].first;
    bool joined = i + 1 == groups.size() && (last < 100 || last % 100 == 0);
    words += (joined ? U" e " : U" ") + groups[i].second;
  }
  return words;
}

// décimo terceiro, vigésima primeira, milésimo
std::u32string ordinal(uint64_t n, bool feminine) {
  char32_t ending = feminine ? U'a' : U'o';
  std::vector<std::u32string> words;
  if (uint64_t thousands = n / 1000) {
    words.push_back((thousands == 1 ? U"" : cardinal(thousands) + U" ") + U"milésim" + ending);
  }
  for (auto [scale, stems] :
       {std::pair{100, &kOrdinalHundreds}, std::pair{10, &kOrdinalTens}, std::pair{1, &kOrdinalOnes}}) {
    if (uint64_t digit = n / scale % 10) {
      words.push_back(std::u32string((*stems)[digit]) + ending);
    }
  }
  return join(words);
}

std::u32string digits(std::u32string_view text) {
  std::vector<std::u32string> words;
  for (char32_t c : text) {
    words.push_back(unit(c - U'0', Gender::MASCULINE));
  }
  return join(words);
}

// um milhão de reais, but dez mil reais
std::u32string with_de(const std::u32string& words) {
  bool scale = std::ranges::any_of(kScaleWords, [&](const auto& w) { return words.ends_with(w); });
  return scale ? words + U" de" : words;
}

// -------------------
// ----- Context -----
// -------------------

bool feminine(std::u32string_view noun) {
  std::u32string word = strings::to_lower(noun);
  std::u32string singular = word;
  if (word.ends_with(U"ões") || word.ends_with(U"ães")) {
    singular = word.substr(0, word.size() - 3) + U"ão";
  } else if (word.ends_with(U"res") || word.ends_with(U"zes") || word.ends_with(U"les")) {
    singular.resize(word.size() - 2);
  } else if (word.ends_with(U's')) {
    singular.pop_back();
  }

  if (kMasculineInA.contains(singular) || kMasculineInA.contains(word)) {
    return false;
  }
  bool feminine_ending = std::ranges::any_of(kFeminineEndings, [&](const auto& e) { return singular.ends_with(e); });
  return kFeminineNouns.contains(singular) || kFeminineNouns.contains(word) || feminine_ending;
}

// The gender of the noun after a number; an hour is feminine: às duas, duas da tarde
Gender gender_of(const Context& context) {
  std::u32string word = context.after();
  if ((word == U"da" || word == U"de") && kDayParts.contains(strings::to_lower(context.after(2)))) {
    return Gender::FEMININE;
  }
  if (word.empty() || !unicode::isalpha(word[0]) || kStopWords.contains(strings::to_lower(word))) {
    return kHourWords.contains(context.before()) ? Gender::FEMININE : Gender::MASCULINE;
  }
  return feminine(word) ? Gender::FEMININE : Gender::MASCULINE;
}

// A lowercase noun follows: 500k seguidores
bool counted(const Context& context) {
  std::u32string word = context.after();
  bool lowercase_word = word.size() > 2 && std::ranges::all_of(word, [](char32_t c) { return unicode::islower(c); });
  return lowercase_word && !kStopWords.contains(word);
}

// A range touching letters: A320-200, 3-4x
bool lettered(const Match& range, const Context& context) {
  std::u32string_view head = context.head(), tail = context.tail();
  size_t before = range.numbers[0].span.begin, after = range.numbers[1].span.end - range.span.end;
  return (before > 0 && is_ascii_letter(head[before - 1])) || (after < tail.size() && is_ascii_letter(tail[after]));
}
} // namespace

Num2Word::Num2Word() : Num2WordLayer(kNotation, kJoiners) {
  add_pattern(std::make_unique<DatePattern>(kDateFormat));
  for (const auto& format : kPhoneFormats) {
    add_pattern(std::make_unique<PhonePattern>(format));
  }
  add_pattern(std::make_unique<SuffixPattern>(Kind::ORDINAL, kOrdinalSigns, kOrdinalSignFormat));
}

// ----------------------
// ----- Resolution -----
// ----------------------

bool Num2Word::accept(const Match& match, const Context& context) const {
  switch (match.kind) {
    case Kind::PHONE: {
      // 2021-2022: a range of years; 213 456 789 is a count, unless a word says it is a phone number
      bool years = std::ranges::all_of(match.numbers, [](const Number& group) {
        return group.integer.size() == 4 && value(group) >= 1900 && value(group) <= 2100;
      });
      return !years && (match.separator != U' ' || kPhoneWords.contains(context.before()));
    }
    case Kind::FRACTION:
      return kFractions.contains({value(match.numbers[0]), value(match.numbers[1])});
    case Kind::TIME:
      return match.numbers.size() < 2 || (value(match.numbers[0]) <= 24 && value(match.numbers[1]) <= 59);
    case Kind::SCALE:
      // 4K, 2K: a resolution; but 500k is quinhentos mil
      return (match.scale != U"k" && match.scale != U"K") || value(match.numbers.back()) >= 10 || counted(context);
    case Kind::ORDINAL:
      return value(match.numbers[0]) > 0;
    default:
      return true;
  }
}

NumberReading Num2Word::resolve(const Match& match, const Context& context) const {
  NumberReading reading{.gender = gender_of(context),
                        .day = strings::to_lower(context.after()) == U"de" &&
                               std::ranges::find(kMonths, strings::to_lower(context.after(2))) != kMonths.end(),
                        .heading = kHeadings.contains(context.before())};
  if (match.kind == Kind::RANGE) {
    reading.lettered = lettered(match, context);
  }

  // R$ 50 reais
  if (match.kind == Kind::MONEY) {
    const Forms& currency = kCurrencies.at(std::u32string(match.unit));
    std::u32string next = strings::to_lower(context.after());
    reading.named = next == currency.one || next == currency.many;
  }
  return reading;
}

// --------------------
// ----- Readings -----
// --------------------

std::u32string Num2Word::read_with(const Number& number, Gender gender) const {
  if (number.fraction.empty()) {
    auto n = value_of(number.integer);
    return !n || spoken_digits(number, 7) ? digits(number.integer) : cardinal(*n, gender);
  }

  // The separator is read as written: dois vírgula cinco, dois ponto cinco
  std::u32string fraction;
  if (number.fraction.size() > 2) {
    fraction = digits(number.fraction);
  } else {
    std::vector<std::u32string> words;
    size_t zeros = number.fraction.find_first_not_of(U'0');
    words.insert(words.end(), std::min(zeros, number.fraction.size()), U"zero");
    if (zeros != std::u32string::npos) {
      words.push_back(cardinal(value_of(number.fraction).value_or(0)));
    }
    fraction = join(words);
  }
  std::u32string head = number.integer.empty() ? U"" : cardinal(value(number), gender) + U" ";
  return head + (number.decimal_mark == U',' ? U"vírgula " : U"ponto ") + fraction;
}

std::u32string Num2Word::read_number(const Number& number, const NumberReading& reading) const {
  if (number.fraction.empty() && reading.day && number.integer == U"1") {
    return U"primeiro";
  }
  return read_with(number, number.fraction.empty() ? reading.gender : Gender::MASCULINE);
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
  return (day == 1 ? U"primeiro" : cardinal(day)) + U" de " + kMonths[value(match.numbers[1]) - 1] + U" de " +
         cardinal(value(match.numbers[2]));
}

std::u32string Num2Word::read_phone(const Match& match, const NumberReading& reading) const {
  std::vector<std::u32string> groups;
  for (const auto& group : match.numbers) {
    groups.push_back(group.integer == U"0800" ? U"zero oitocentos" : digits(group.integer));
  }
  return join(groups, U", ");
}

std::u32string Num2Word::read_fraction(const Match& match, const NumberReading& reading) const {
  return kFractions.at({value(match.numbers[0]), value(match.numbers[1])});
}

std::u32string Num2Word::read_time(const Match& match, const NumberReading& reading) const {
  // 14h30: catorze e trinta; 21h: vinte e uma horas
  uint64_t hours = value(match.numbers[0]);
  uint64_t minutes = match.numbers.size() > 1 ? value(match.numbers[1]) : 0;
  std::u32string hour = cardinal(hours, Gender::FEMININE);
  return minutes ? hour + U" e " + cardinal(minutes) : hour + (hours == 1 ? U" hora" : U" horas");
}

std::u32string Num2Word::read_ordinal(const Match& match, const NumberReading& reading) const {
  uint64_t n = value(match.numbers[0]);
  return n >= 10 && reading.heading ? cardinal(n) : ordinal(n, match.suffix == U"ª");
}

std::u32string Num2Word::read_compound(const Match& match, const NumberReading& reading) const {
  return read_with(match.numbers[0], Gender::MASCULINE) + U"-" + std::u32string(match.suffix);
}

std::u32string Num2Word::read_money(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& currency = kCurrencies.at(std::u32string(match.unit));
  NumberReading priced = reading;
  priced.gender = currency.one == U"libra" ? Gender::FEMININE : Gender::MASCULINE;

  // R$ 37,517: prices have two decimals, so three digits are a thousands group
  Number amount = number;
  if (number.decimal_mark == U',' && !number.integer.empty() && number.integer.size() <= 3 &&
      number.fraction.size() == 3) {
    amount = {.integer = number.integer + number.fraction};
  }

  if (reading.named) {
    return match.scale.empty() ? read_with(amount, priced.gender) : read_scale(amount, match, priced);
  }
  if (!match.scale.empty()) {
    return with_de(read_scale(amount, match, priced)) + U" " + currency.many;
  }

  // A price: vinte e nove reais e noventa centavos
  if (amount.fraction.size() == 2 && amount.decimal_mark == U',') {
    uint64_t cents = value_of(amount.fraction).value_or(0);
    std::u32string centavos = cardinal(cents) + (cents == 1 ? U" centavo" : U" centavos");
    if (value(amount) == 0) {
      return centavos;
    }
    Number whole{.integer = amount.integer, .grouped = amount.grouped};
    std::u32string words =
        with_de(read_with(whole, priced.gender)) + U" " + (amount.integer == U"1" ? currency.one : currency.many);
    return cents ? words + U" e " + centavos : words;
  }
  return with_de(read_with(amount, priced.gender)) + U" " + (is_one(amount) ? currency.one : currency.many);
}

std::u32string Num2Word::read_scale(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& scale = kScales.at(std::u32string(match.scale));
  if (scale.one == U"mil") {
    return read_with(number, reading.gender) + U" mil"; // vinte e duas mil pessoas
  }
  return read_with(number, Gender::MASCULINE) + U" " + (value(number) < 2 ? scale.one : scale.many);
}

std::u32string Num2Word::read_percent(const Number& number, const Match& match, const NumberReading& reading) const {
  std::u32string suffix = match.suffix.empty() ? U"" : U" " + std::u32string(match.suffix);
  return read_with(number, Gender::MASCULINE) + U" por cento" + suffix;
}

std::u32string Num2Word::read_unit(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& unit = kUnits.at(std::u32string(match.unit));
  std::u32string_view first_word = std::u32string_view(unit.one).substr(0, unit.one.find(U' '));
  Gender gender = feminine(first_word) ? Gender::FEMININE : Gender::MASCULINE;
  return read_with(number, gender) + U" " + (value(number) < 2 ? unit.one : unit.many);
}

std::u32string Num2Word::read_range(const Number& from, const Number& to, const NumberReading& reading) const {
  // 2016-2022 and 10-15 read "a"; scores and formations (4-3-3, 60-40) keep "-"
  bool plain = from.fraction.empty() && to.fraction.empty() && !from.grouped && !to.grouped;
  bool rising = from.integer.size() == to.integer.size() && value(to) > value(from);
  return plain && rising && !reading.lettered ? joiners_.range : U"-";
}

} // namespace phonemis::pt
