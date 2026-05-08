#include "num2word.h"
#include "constants.h"
#include <phonemis/utils/conversions.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>

namespace phonemis::hi {

using namespace constants;
using namespace phonemis::utils;

namespace {

// Hindi ordinals split into three gender/number forms. The numeric values
// align with the column order used in `kOrdinalsSpecial` and `kOrdinalEndings`.
enum class OrdinalForm : std::size_t { MASCULINE = 0, FEMININE = 1, PLURAL = 2 };

// Maps an ordinal suffix to a gender/number form. The base scanner only
// captures the alpha portion ("व") of a Devanagari suffix because matras and
// nasal marks aren't classified as alphabetic, so the masculine form is the
// only one selectable through digit-attached notation in raw text. Direct
// API callers can pass the full ending ("वाँ", "वीं", "वें") to pick a
// specific form.
OrdinalForm form_from_suffix(std::u32string_view suffix) {
  if (suffix == U"वीं") return OrdinalForm::FEMININE;
  if (suffix == U"वें") return OrdinalForm::PLURAL;
  return OrdinalForm::MASCULINE;
}

} // namespace

std::u32string Num2Word::to_cardinal_int(int32_t value) const {
  if (value < 0) {
    return U"माइनस " + to_cardinal_int(std::abs(value));
  }

  if (auto it = num2word::kCardinals.find(value); it != num2word::kCardinals.end()) {
    return it->second;
  }

  if (value < 1000) {
    // 100-999: "<digit> सौ" with an optional remainder. Hindi keeps "एक"
    // before "सौ" (one hundred reads as "एक सौ", not bare "सौ").
    int32_t hundreds_high = value / 100;
    int32_t rest = value % 100;
    std::u32string head = num2word::kCardinals.at(hundreds_high) + U" सौ";
    if (rest == 0) return head;
    return head + U" " + to_cardinal_int(rest);
  }

  // Indian-numbering decomposition (हज़ार / लाख / करोड़) in descending order.
  // Unlike Western numbering, the next scale up after हज़ार multiplies by 100
  // (lakh = 100k) and then 100 again (crore = 10M), so 1,000,000 reads as
  // "दस लाख" rather than as a million-scoped phrase.
  for (auto it = num2word::kLargeCardinals.rbegin(); it != num2word::kLargeCardinals.rend(); ++it) {
    int64_t base = it->first;
    if (static_cast<int64_t>(value) >= base) {
      int32_t high = value / static_cast<int32_t>(base);
      int32_t low = value % static_cast<int32_t>(base);

      // Hindi keeps the leading "एक" in front of every scale noun (e.g.
      // 1000 -> "एक हज़ार", 100000 -> "एक लाख"), unlike European languages
      // that drop the multiplier of one.
      std::u32string res = to_cardinal_int(high) + U" " + it->second;
      if (low > 0) {
        res += U" " + to_cardinal_int(low);
      }
      return res;
    }
  }

  return conversions::utf8_to_u32(std::to_string(value));
}

std::u32string Num2Word::to_cardinal_float(float value, std::u32string_view repr) const {
  if (value < 0.0f) {
    return U"माइनस " + to_cardinal_float(std::abs(value), repr);
  }

  std::string value_str = conversions::u32_to_utf8(repr);

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

  // Hindi reads the fractional part digit-by-digit (e.g. 0.05 -> "शून्य
  // दशमलव शून्य पाँच"), unlike languages that verbalize it as a whole number.
  std::u32string frac_words;
  for (char c : fractional_part_str) {
    if (!frac_words.empty()) frac_words += U" ";
    frac_words += to_cardinal_int(c - '0');
  }

  return to_cardinal_int(integer_part) + U" दशमलव " + frac_words;
}

std::u32string Num2Word::to_ordinal_int(int32_t value, std::u32string_view suffix) const {
  OrdinalForm form = form_from_suffix(suffix);

  if (auto it = num2word::kOrdinalsSpecial.find(value); it != num2word::kOrdinalsSpecial.end()) {
    return it->second[static_cast<std::size_t>(form)];
  }

  // Regular ordinal: cardinal phrase with the gender-specific ending fused
  // onto the last token (no separating space, e.g. "इक्कीसवाँ").
  return to_cardinal_int(value) + num2word::kOrdinalEndings[static_cast<std::size_t>(form)];
}

std::u32string Num2Word::to_currency(char32_t currency, std::variant<int32_t, float> /*number*/) const {
  // Hindi loanword currencies (डॉलर, यूरो, पाउंड) are invariant; the same
  // form is used regardless of the amount.
  if (auto it = num2word::kCurrencies.find(currency); it != num2word::kCurrencies.end()) {
    return it->second;
  }
  return std::u32string(1, currency);
}

std::u32string Num2Word::to_fraction(int32_t numerator, int32_t denominator) const {
  // Common fractions have dedicated Hindi nouns; everything else falls back
  // to the colloquial "<num> बटा <den>" pattern.
  if (numerator == 1 && denominator == 2) return U"आधा";
  if (denominator == 3) return to_cardinal_int(numerator) + U" तिहाई";
  if (denominator == 4) return to_cardinal_int(numerator) + U" चौथाई";
  return to_cardinal_int(numerator) + U" बटा " + to_cardinal_int(denominator);
}

std::u32string Num2Word::to_day(uint32_t day) const {
  // Hindi dates render the day as a plain cardinal (e.g. "27 मार्च").
  return to_cardinal_int(static_cast<int32_t>(day));
}

std::u32string Num2Word::to_month(uint32_t month) const {
  if (month >= 1 && month <= 12) {
    return num2word::kMonths[month];
  }
  return U"";
}

std::u32string Num2Word::to_year(uint32_t year) const {
  return to_cardinal_int(static_cast<int32_t>(year));
}

bool Num2Word::is_ordinal_suffix(std::u32string_view suffix) const {
  return num2word::kOrdinalSuffixes.contains(std::u32string(suffix));
}

} // namespace phonemis::hi