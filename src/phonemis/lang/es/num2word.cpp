#include "constants.h"
#include "num2word.h"
#include "patterns.h"
#include <phonemis/base/processor/layers/num2word/numerals.h>
#include <phonemis/base/processor/layers/num2word/patterns.h>
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::es {

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

std::u32string below_hundred(uint64_t n, One one) {
  std::u32string words(n < 30 ? kOnes[n] : kTens[n / 10]);
  if (n >= 30 && n % 10) {
    words += U" y " + std::u32string(kOnes[n % 10]);
  }

  // veintiún años, treinta y una personas
  if (words.ends_with(U"uno") && one != One::UNO) {
    words.resize(words.size() - 3);
    words += one == One::UNA ? U"una" : (words == U"veinti" ? U"ún" : U"un");
  }
  return words;
}

std::u32string below_thousand(uint64_t n, One one) {
  uint64_t h = n / 100, r = n % 100;
  if (h == 0) {
    return below_hundred(r, one);
  }
  if (n == 100) {
    return U"cien";
  }

  // doscientas personas
  std::u32string head(kHundreds[h]);
  if (one == One::UNA && h > 1) {
    head.replace(head.size() - 2, 2, U"as");
  }
  return head + (r ? U" " + below_hundred(r, one) : U"");
}

// A cardinal, ending with the given form of "uno": doscientas mil personas, veintiún años
std::u32string cardinal(uint64_t n, One one = One::UNO) {
  if (n < 1000) {
    return below_thousand(n, one);
  }
  if (n < 1000000) {
    uint64_t t = n / 1000, r = n % 1000;
    std::u32string head = t == 1 ? U"mil" : below_thousand(t, one == One::UNA ? One::UNA : One::UN) + U" mil";
    return head + (r ? U" " + below_thousand(r, one) : U"");
  }

  bool billions = n >= 1000000000000ULL;
  uint64_t scale = billions ? 1000000000000ULL : 1000000ULL, q = n / scale, r = n % scale;
  std::u32string head = q == 1 ? (billions ? U"un billón" : U"un millón") :
                                 cardinal(q, One::UN) + (billions ? U" billones" : U" millones");
  return head + (r ? U" " + cardinal(r, one) : U"");
}

// centésima trigésima séptima
std::u32string ordinal(uint64_t n, bool feminine) {
  std::vector<std::u32string> words;
  for (auto [scale, stems] :
       {std::pair{100, &kOrdinalHundreds}, std::pair{10, &kOrdinalTens}, std::pair{1, &kOrdinalOnes}}) {
    if (uint64_t digit = n / scale % 10) {
      words.push_back(std::u32string((*stems)[digit]) + (feminine ? U"a" : U"o"));
    }
  }
  return join(words);
}

std::u32string digits(std::u32string_view text) {
  std::vector<std::u32string> words;
  for (char32_t c : text) {
    if (unicode::isdigit(c)) {
      words.emplace_back(kOnes[c - U'0']);
    }
  }
  return join(words);
}

// -------------------
// ----- Context -----
// -------------------

bool feminine(std::u32string_view noun) {
  std::u32string word = strings::to_lower(noun);
  std::u32string singular = word;
  if (word.ends_with(U"iones")) {
    singular = word.substr(0, word.size() - 5) + U"ión";
  } else if (word.ends_with(U"as") || word.ends_with(U"es")) {
    singular.pop_back();
  }

  if (kFeminineNouns.contains(word) || kFeminineNouns.contains(singular)) {
    return true;
  }
  bool feminine_ending = std::ranges::any_of(kFeminineEndings, [&](const auto& e) { return singular.ends_with(e); });
  return !kMasculineInA.contains(singular) && feminine_ending;
}

// The form of "uno" from the noun after a number: veintiún años, veintiuna personas; a la 1
One one_of(const Context& context) {
  std::u32string word = context.after();
  std::u32string previous = context.before();
  if (previous == U"la" || previous == U"las") {
    return One::UNA;
  }
  if (word.empty() || !unicode::isalpha(word[0]) || kStopWords.contains(strings::to_lower(word))) {
    return One::UNO;
  }
  return feminine(word) ? One::UNA : One::UN;
}

// A lowercase noun follows: 10k seguidores
bool counted(const Context& context) {
  std::u32string word = context.after();
  bool lowercase_word = word.size() > 2 && std::ranges::all_of(word, [](char32_t c) { return unicode::islower(c); });
  return lowercase_word && !kStopWords.contains(word);
}
} // namespace

Num2Word::Num2Word() : Num2WordLayer(kNotation, kJoiners) {
  add_pattern(std::make_unique<LawPattern>());
  add_pattern(std::make_unique<DatePattern>(kDateFormat));
  for (const auto& format : kPhoneFormats) {
    add_pattern(std::make_unique<PhonePattern>(format));
  }
  add_pattern(std::make_unique<SuffixPattern>(Kind::ORDINAL, kOrdinalSigns, kOrdinalSignFormat));
  add_pattern(std::make_unique<SuffixPattern>(Kind::ORDINAL, kOrdinalSuffixes));
  add_pattern(std::make_unique<SuffixPattern>(Kind::DECADE, kDecadeSuffixes, kDecadeFormat));
}

// ----------------------
// ----- Resolution -----
// ----------------------

bool Num2Word::accept(const Match& match, const Context& context) const {
  switch (match.kind) {
    case Kind::PHONE:
      // 600 123 456 is a count, unless a word says it is a phone number
      return match.separator != U' ' || kPhoneWords.contains(context.before());
    case Kind::FRACTION:
      return kFractions.contains({value(match.numbers[0]), value(match.numbers[1])});
    case Kind::TIME: {
      // Only with minutes; a dot makes a time only after "a las": a las 18.30, but 1.13
      if (match.numbers.size() < 2) {
        return false;
      }
      bool decimal = match.separator == U'.' && !kTimeWords.contains(context.before());
      return value(match.numbers[0]) <= 24 && value(match.numbers[1]) <= 59 && !decimal;
    }
    case Kind::SCALE:
      // 4K, 2K: a letter, read as a code
      return (match.scale != U"k" && match.scale != U"K") || counted(context);
    case Kind::ORDINAL:
      // Ordinal words reach 999
      return value(match.numbers[0]) > 0 && value(match.numbers[0]) < 1000;
    default:
      return true;
  }
}

NumberReading Num2Word::resolve(const Match& match, const Context& context) const {
  // A decade or a compound stands alone: los 80s, 4-carriles
  bool alone = match.kind == Kind::DECADE || match.kind == Kind::COMPOUND;
  NumberReading reading{.one = alone ? One::UNO : one_of(context)};

  // $5.000 dólares
  if (match.kind == Kind::MONEY) {
    const Forms& currency = kCurrencies.at(std::u32string(match.unit));
    std::u32string next = strings::to_lower(context.after());
    reading.named = next == currency.one || next == currency.many || next == U"pesos";
  }
  return reading;
}

// --------------------
// ----- Readings -----
// --------------------

std::u32string Num2Word::read_with(const Number& number, One one) const {
  if (number.fraction.empty()) {
    auto n = value_of(number.integer);
    return !n || spoken_digits(number, 7) ? digits(number.integer) : cardinal(*n, one);
  }

  // The separator is read as written: tres coma cinco, tres punto cinco
  std::u32string fraction;
  if (number.fraction.size() > 2) {
    fraction = digits(number.fraction);
  } else {
    std::vector<std::u32string> words;
    size_t zeros = number.fraction.find_first_not_of(U'0');
    words.insert(words.end(), std::min(zeros, number.fraction.size()), U"cero");
    if (zeros != std::u32string::npos) {
      words.push_back(cardinal(value_of(number.fraction).value_or(0), one));
    }
    fraction = join(words);
  }
  std::u32string head = number.integer.empty() ? U"" : cardinal(value(number), one == One::UN ? One::UNO : one) + U" ";
  return head + (number.decimal_mark == U',' ? U"coma " : U"punto ") + fraction;
}

std::u32string Num2Word::read_number(const Number& number, const NumberReading& reading) const {
  return read_with(number, reading.one);
}

std::u32string Num2Word::read_code_digits(std::u32string_view text) const {
  auto n = value_of(text);
  if ((text.size() > 1 && text[0] == U'0') || text.size() >= 7 || !n) {
    return digits(text);
  }
  return cardinal(*n);
}

std::u32string Num2Word::read_date(const Match& match, const NumberReading& reading) const {
  // A law: Ley dos de dos mil dieciocho
  if (match.numbers.size() == 2) {
    return cardinal(value(match.numbers[0])) + U" de " + cardinal(value(match.numbers[1]));
  }

  uint64_t day = value(match.numbers[0]);
  return (day == 1 ? U"primero" : cardinal(day)) + U" de " + kMonths[value(match.numbers[1]) - 1] + U" de " +
         cardinal(value(match.numbers[2]));
}

std::u32string Num2Word::read_phone(const Match& match, const NumberReading& reading) const {
  return digits(match.text);
}

std::u32string Num2Word::read_fraction(const Match& match, const NumberReading& reading) const {
  return kFractions.at({value(match.numbers[0]), value(match.numbers[1])});
}

std::u32string Num2Word::read_time(const Match& match, const NumberReading& reading) const {
  uint64_t minutes = value(match.numbers[1]);
  return cardinal(value(match.numbers[0]), One::UNA) + (minutes ? U" " + cardinal(minutes) : U"");
}

std::u32string Num2Word::read_ordinal(const Match& match, const NumberReading& reading) const {
  uint64_t n = value(match.numbers[0]);
  std::u32string_view suffix = match.suffix;
  if (suffix == U"er" && (n == 1 || n == 3)) {
    return n == 1 ? U"primer" : U"tercer";
  }
  bool feminine = suffix == U"ª" || suffix == U"era" || suffix == U"ra" || suffix == U"da" || suffix == U"ta" ||
                  suffix == U"va" || suffix == U"ma" || suffix == U"na";
  return ordinal(n, feminine);
}

std::u32string Num2Word::read_decade(const Match& match, const NumberReading& reading) const {
  return read_number(match.numbers[0], reading) + U"s"; // los 80s: los ochentas
}

std::u32string Num2Word::read_money(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& currency = kCurrencies.at(std::u32string(match.unit));
  One one = currency.one == U"libra" ? One::UNA : One::UN;

  // A price: cinco dólares con cincuenta
  if (number.fraction.size() == 2 && match.scale.empty()) {
    Number whole{.integer = number.integer, .grouped = number.grouped};
    return read_with(whole, one) + U" " + (number.integer == U"1" ? currency.one : currency.many) + U" con " +
           cardinal(value_of(number.fraction).value_or(0));
  }

  std::u32string words = match.scale.empty() ? read_with(number, one) : read_scale(number, match, reading);
  if (reading.named) {
    return words;
  }
  // tres millones de dólares, but cinco mil dólares
  bool of = !match.scale.empty() && kScales.at(std::u32string(match.scale)) != U"mil";
  return words + (of ? U" de " : U" ") + (is_one(number) && match.scale.empty() ? currency.one : currency.many);
}

std::u32string Num2Word::read_scale(const Number& number, const Match& match, const NumberReading& reading) const {
  const std::u32string& scale = kScales.at(std::u32string(match.scale));
  std::u32string word = scale == U"mil" ? U"mil" : (is_one(number) ? U"millón" : U"millones");
  return read_with(number, One::UN) + U" " + word;
}

std::u32string Num2Word::read_percent(const Number& number, const Match& match, const NumberReading& reading) const {
  std::u32string suffix = match.suffix.empty() ? U"" : U" " + std::u32string(match.suffix);
  return read_with(number, One::UNO) + U" por ciento" + suffix;
}

std::u32string Num2Word::read_unit(const Number& number, const Match& match, const NumberReading& reading) const {
  const Forms& unit = kUnits.at(std::u32string(match.unit));
  std::u32string_view first_word = std::u32string_view(unit.one).substr(0, unit.one.find(U' '));
  One one = feminine(first_word) ? One::UNA : One::UN;
  return read_with(number, one) + U" " + (is_one(number) ? unit.one : unit.many);
}

} // namespace phonemis::es
