#include "num2word.h"
#include "constants.h"
#include <phonemis/utils/conversions.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace phonemis::it {

using namespace constants;
using namespace phonemis::utils;

namespace {

// Apocopates a trailing cardinal "uno" to "un" (e.g. "uno" → "un",
// "ventuno" → "ventun"). Used before singular masculine nouns such as
// currency units and fraction denominators.
std::u32string apocopate(std::u32string word) {
  constexpr std::u32string_view kUno = U"uno";
  if (word.size() >= kUno.size() &&
      std::u32string_view(word).substr(word.size() - kUno.size()) == kUno) {
    size_t pre = word.size() - kUno.size();
    // Only apocopate when "uno" is a standalone suffix (start of word or
    // preceded by a boundary), to avoid trimming unrelated words ending in
    // "uno".
    if (pre == 0 || word[pre - 1] == U' ' || word[pre - 1] == U'-') {
      word.replace(pre, kUno.size(), U"un");
    }
  }
  return word;
}

// Composes an ordinal from a cardinal word by stripping its final vowel and
// appending "esimo". Cardinals ending in "tré" drop the accent but keep the
// 'e' (e.g. "ventitré" → "ventitreesimo").
std::u32string cardinal_to_ordinal(std::u32string word) {
  constexpr std::u32string_view kTre = U"tré";
  if (word.size() >= kTre.size() &&
      std::u32string_view(word).substr(word.size() - kTre.size()) == kTre) {
    word.replace(word.size() - kTre.size(), kTre.size(), U"tre");
    return word + U"esimo";
  }
  if (!word.empty()) {
    word.pop_back();
  }
  return word + U"esimo";
}

} // namespace

std::u32string Num2Word::to_cardinal_int(int32_t value) const {
  if (value < 0) {
    return U"meno " + to_cardinal_int(std::abs(value));
  }

  if (auto it = num2word::kCardinals.find(value); it != num2word::kCardinals.end()) {
    return it->second;
  }

  if (value < 100) {
    // Italian fuses tens and units into a single word. The tens' final vowel
    // elides before "uno" (1) and "otto" (8); "tre" takes an accent as a
    // suffix ("ventitré", "trentatré", ...).
    int32_t tens = (value / 10) * 10;
    int32_t units = value % 10;
    std::u32string tens_word = num2word::kCardinals.at(tens);
    std::u32string units_word = (units == 3) ? U"tré" : num2word::kCardinals.at(units);
    if (units == 1 || units == 8) {
      tens_word.pop_back();
    }
    return tens_word + units_word;
  }

  // Large-scale decomposition (cento, mille, milione, miliardo) in descending
  // order.
  for (auto it = num2word::kLargeCardinals.rbegin(); it != num2word::kLargeCardinals.rend(); ++it) {
    int64_t base = it->first;
    if (static_cast<int64_t>(value) < base) continue;

    int32_t high = value / static_cast<int32_t>(base);
    int32_t low = value % static_cast<int32_t>(base);

    // Base 100/1000 fuse with the multiplier and the remainder into a single
    // word ("duecento", "duemilaventisei"), dropping "uno" at the multiplier
    // position ("cento", "mille"). Base 10^6 and above take a space and
    // require an explicit "un" for a multiplier of 1 ("un milione",
    // "due milioni").
    std::u32string head;
    std::u32string scale;
    bool space_before_low = false;

    if (base == 100LL) {
      head = (high == 1) ? U"" : to_cardinal_int(high);
      scale = U"cento";
    } else if (base == 1000LL) {
      head = (high == 1) ? U"" : to_cardinal_int(high);
      scale = (high == 1) ? U"mille" : U"mila";
    } else {
      head = (high == 1) ? U"un " : to_cardinal_int(high) + U" ";
      scale = (high == 1) ? it->second : num2word::kLargeCardinalsPlural.at(base);
      space_before_low = true;
    }

    std::u32string res = head + scale;
    if (low > 0) {
      res += (space_before_low ? U" " : U"") + to_cardinal_int(low);
    }
    return res;
  }

  return conversions::utf8_to_u32(std::to_string(value));
}

std::u32string Num2Word::to_cardinal_float(float value, std::u32string_view repr) const {
  if (value < 0.0f) {
    return U"meno " + to_cardinal_float(std::abs(value), repr);
  }

  // The base transform normalizes the decimal separator to '.', but also
  // accept ',' when this method is invoked directly with a raw representation.
  std::string value_str = conversions::u32_to_utf8(repr);
  std::replace(value_str.begin(), value_str.end(), ',', '.');

  // Trim trailing zeros and any dangling decimal point.
  auto rit = std::find_if(value_str.rbegin(), value_str.rend(),
                          [](char c) { return c != '0'; });
  value_str = std::string(value_str.begin(), rit.base());
  if (!value_str.empty() && value_str.back() == '.') {
    value_str.pop_back();
  }

  size_t dot_pos = value_str.find('.');
  if (dot_pos == std::string::npos) {
    return to_cardinal_int(value_str.empty() ? 0 : std::stoi(value_str));
  }

  std::string integer_part_str = value_str.substr(0, dot_pos);
  std::string fractional_part_str = value_str.substr(dot_pos + 1);
  int32_t integer_part = integer_part_str.empty() ? 0 : std::stoi(integer_part_str);

  // Leading zeros in the fractional part are spoken as "zero" words; the
  // remaining digits verbalize as a single cardinal (e.g. 0,05 → "zero virgola
  // zero cinque", 123,45 → "... virgola quarantacinque").
  std::u32string frac_words;
  size_t pos = 0;
  while (pos < fractional_part_str.size() && fractional_part_str[pos] == '0') {
    if (!frac_words.empty()) frac_words += U" ";
    frac_words += U"zero";
    pos++;
  }
  if (pos < fractional_part_str.size()) {
    if (!frac_words.empty()) frac_words += U" ";
    frac_words += to_cardinal_int(std::stoi(fractional_part_str.substr(pos)));
  }

  return to_cardinal_int(integer_part) + U" virgola " + frac_words;
}

std::u32string Num2Word::to_ordinal_int(int32_t value, std::u32string_view suffix) const {
  // The suffix disambiguates gender/number; the default (no suffix, e.g. from
  // a "1." token) is masculine singular.
  bool feminine = (suffix == U"ª" || suffix == U"ªs");
  bool plural   = (suffix == U"ºs" || suffix == U"ªs");

  auto inflect = [&](std::u32string word) {
    if (feminine && !word.empty() && word.back() == U'o') {
      word.back() = U'a';
    }
    if (plural && !word.empty()) {
      // Italian plural for masculine nouns: -o → -i; for feminine: -a → -e.
      if (word.back() == U'o')      word.back() = U'i';
      else if (word.back() == U'a') word.back() = U'e';
    }
    return word;
  };

  if (auto it = num2word::kOrdinals.find(value); it != num2word::kOrdinals.end()) {
    return inflect(it->second);
  }

  // Values without a dedicated ordinal entry derive the form from the cardinal
  // by stripping its final vowel and appending "esimo".
  return inflect(cardinal_to_ordinal(to_cardinal_int(value)));
}

std::u32string Num2Word::to_currency(char32_t currency, std::variant<int32_t, float> number) const {
  auto it = num2word::kCurrencies.find(currency);
  if (it == num2word::kCurrencies.end()) {
    return std::u32string(1, currency);
  }

  // Singular only for amounts that round to exactly 1.
  bool is_singular = std::visit([](auto&& arg) -> bool {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int32_t>) {
      return arg == 1;
    } else {
      return std::abs(arg - 1.0F) < 1e-6F;
    }
  }, number);

  return is_singular ? it->second : num2word::kCurrenciesPlural.at(currency);
}

std::u32string Num2Word::to_fraction(int32_t numerator, int32_t denominator) const {
  // Numerator apocopates "uno" because it precedes a masculine noun
  // ("un mezzo", "un terzo", "un quarto", "un decimo").
  std::u32string num_word = apocopate(to_cardinal_int(numerator));
  bool plural = std::abs(numerator) > 1;

  auto with_plural = [&](std::u32string_view base) -> std::u32string {
    std::u32string w(base);
    if (plural && !w.empty() && w.back() == U'o') w.back() = U'i';
    return num_word + U" " + w;
  };

  // Dedicated nouns for denominators 2/3/4; other denominators fall back to
  // the ordinal form, agreeing in number.
  if (denominator == 2) return with_plural(U"mezzo");
  if (denominator == 3) return with_plural(U"terzo");
  if (denominator == 4) return with_plural(U"quarto");

  std::u32string ord = to_ordinal_int(denominator);
  if (plural && !ord.empty() && ord.back() == U'o') ord.back() = U'i';
  return num_word + U" " + ord;
}

std::u32string Num2Word::to_day(uint32_t day) const {
  return to_cardinal_int(static_cast<int32_t>(day));
}

std::u32string Num2Word::to_month(uint32_t month) const {
  if (month >= 1 && month <= 12) {
    return num2word::kMonths[month];
  }
  return U"";
}

std::u32string Num2Word::to_year(uint32_t year) const {
  // Italian reads years as plain cardinals (e.g. 2026 → "duemilaventisei").
  return to_cardinal_int(static_cast<int32_t>(year));
}

bool Num2Word::is_ordinal_suffix(std::u32string_view suffix) const {
  return num2word::kOrdinalSuffixes.contains(std::u32string(suffix));
}

std::u32string Num2Word::verbalize(const processor::num2word::StringifiedNumber& number) const {
  using Mode = processor::num2word::Mode;

  std::u32string result = Num2WordLayer::verbalize(number);

  // The base produces "<cardinal> <currency>"; apocopate the cardinal half so
  // amounts of 1 (or ending in "uno") read as "un <noun>" instead of "uno
  // <noun>".
  if (number.conversionMode == Mode::CURRENCY) {
    size_t last_space = result.rfind(U' ');
    if (last_space != std::u32string::npos) {
      std::u32string head = result.substr(0, last_space);
      std::u32string tail = result.substr(last_space);
      result = apocopate(std::move(head)) + tail;
    }
  }

  return result;
}

} // namespace phonemis::it
