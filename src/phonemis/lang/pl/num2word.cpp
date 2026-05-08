#include "num2word.h"
#include "constants.h"
#include <phonemis/utils/conversions.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <vector>

namespace phonemis::pl {

using namespace constants;
using namespace phonemis::utils;
using num2word::PluralForm;

namespace {

// Polish nouns following a numeral take one of three plural forms based on
// the numeral's last one or two digits: singular for 1, "few" (-y/-e endings)
// for 2-4 except 12-14, and "many" (genitive plural) for everything else.
PluralForm pick_plural_form(int32_t n) {
  int32_t abs_n = std::abs(n);
  if (abs_n == 1) return PluralForm::SINGULAR;
  int32_t last_two = abs_n % 100;
  int32_t last = abs_n % 10;
  if (last_two >= 12 && last_two <= 14) return PluralForm::MANY;
  if (last >= 2 && last <= 4) return PluralForm::FEW;
  return PluralForm::MANY;
}

// Inflects an ordinal word from its masculine-nominative form. The form code
// reuses PluralForm to share a single inflection table:
//   - SINGULAR  -> feminine singular ("druga", "trzecia")
//   - FEW       -> neuter / non-masc-plural ("drugie", "czwarte")
//   - MANY      -> genitive plural ("drugich", "czwartych")
// The ending shape depends on the masculine stem:
//   - hard stem (-y): drop "y", append plain ending (a / e / ych)
//   - soft stem (-i): drop "i", append i-prefixed ending (ia / ie / ich)
//   - velar stem (-gi/-ki): drop "i", "a" doesn't get the leading "i"
// Words not ending in -y or -i are returned unchanged so the helper is safe
// to apply across cardinal tokens of a compound ordinal phrase.
std::u32string inflect_ordinal(std::u32string_view word, PluralForm form) {
  if (word.empty()) return std::u32string(word);
  char32_t last = word.back();
  std::u32string stem(word);
  stem.pop_back();

  if (last == U'y') {
    if (form == PluralForm::SINGULAR) return stem + U"a";
    if (form == PluralForm::FEW)      return stem + U"e";
    return stem + U"ych";
  }
  if (last == U'i') {
    char32_t prev = stem.empty() ? U'\0' : stem.back();
    bool velar = (prev == U'g' || prev == U'k');
    if (form == PluralForm::SINGULAR) return stem + (velar ? U"a" : U"ia");
    if (form == PluralForm::FEW)      return stem + U"ie";
    return stem + U"ich";
  }
  return std::u32string(word);
}

// Builds the masculine-nominative ordinal phrase for `value`. Ordinals beyond
// 100 keep the upper magnitude as a cardinal phrase and only inflect the
// trailing sub-100 portion (e.g. 2026 -> "dwa tysiące dwudziesty szósty").
std::u32string ordinal_masc(int32_t value, const Num2Word& self) {
  if (auto it = num2word::kOrdinals.find(value); it != num2word::kOrdinals.end()) {
    return it->second;
  }

  if (value < 100) {
    int32_t tens = (value / 10) * 10;
    int32_t units = value % 10;
    return num2word::kOrdinals.at(tens) + U" " + num2word::kOrdinals.at(units);
  }

  if (value < 1000) {
    int32_t hundreds = (value / 100) * 100;
    int32_t rest = value % 100;
    if (rest == 0) {
      // 200-900 lack dedicated ordinal entries; the cardinal form is the
      // pragmatic fallback (the literal "dwusetny" family is rarely used).
      return num2word::kCardinals.at(hundreds);
    }
    return num2word::kCardinals.at(hundreds) + U" " + ordinal_masc(rest, self);
  }

  for (auto it = num2word::kLargeCardinals.rbegin(); it != num2word::kLargeCardinals.rend(); ++it) {
    int64_t base = it->first;
    if (static_cast<int64_t>(value) >= base) {
      int32_t high = value / static_cast<int32_t>(base);
      int32_t low = value % static_cast<int32_t>(base);
      const auto& forms = it->second;
      const auto& scale_word = forms[static_cast<std::size_t>(pick_plural_form(high))];

      if (low == 0) {
        // 1000 -> "tysięczny", 1000000 -> "milionowy" already in kOrdinals.
        // For 2000+ we fall back to "<cardinal high> <scale word>" since
        // round multiple ordinals ("dwutysięczny", ...) aren't tabulated.
        return self.to_cardinal_int(high) + U" " + scale_word;
      }

      std::u32string head = (high == 1 && base == 1000LL) ? U"" : (self.to_cardinal_int(high) + U" ");
      return head + scale_word + U" " + ordinal_masc(low, self);
    }
  }

  return self.to_cardinal_int(value);
}

// Inflects the trailing ordinal tokens of a compound phrase, leaving the
// preceding cardinal part untouched. Tokens are scanned from the end and
// inflected as long as they end in -y or -i (the hallmark of a masculine
// ordinal); the scan stops at the first non-ordinal token (e.g. "tysiące",
// "sto"), which is preserved verbatim.
std::u32string inflect_trailing_ordinals(const std::u32string& phrase, PluralForm form) {
  std::vector<std::u32string> parts;
  size_t pos = 0;
  while (pos <= phrase.size()) {
    size_t next = phrase.find(U' ', pos);
    if (next == std::u32string::npos) {
      parts.push_back(phrase.substr(pos));
      break;
    }
    parts.push_back(phrase.substr(pos, next - pos));
    pos = next + 1;
  }

  size_t boundary = parts.size();
  while (boundary > 0 && !parts[boundary - 1].empty()) {
    char32_t back = parts[boundary - 1].back();
    if (back != U'y' && back != U'i') break;
    boundary--;
  }

  std::u32string result;
  for (size_t i = 0; i < parts.size(); ++i) {
    if (i > 0) result += U" ";
    result += (i >= boundary) ? inflect_ordinal(parts[i], form) : parts[i];
  }
  return result;
}

// Maps an ordinal-suffix's terminal letter to the inflection class. Polish
// ordinal abbreviations encode the gender/number of the inflected ending in
// their last character: -y/-i = masculine nominative singular, -a = feminine
// singular, -e = neuter singular or non-masculine plural.
std::optional<PluralForm> form_from_suffix(std::u32string_view suffix) {
  if (suffix.empty()) return std::nullopt;
  switch (suffix.back()) {
    case U'a': return PluralForm::SINGULAR;
    case U'e': return PluralForm::FEW;
    default:   return std::nullopt;
  }
}

// Builds the masculine-genitive ordinal phrase used for date days. Days fall
// in [1, 31], so a flat lookup with a single tens+units fallback suffices.
std::u32string ordinal_gen(int32_t value) {
  if (auto it = num2word::kOrdinalsGen.find(value); it != num2word::kOrdinalsGen.end()) {
    return it->second;
  }
  int32_t tens = (value / 10) * 10;
  int32_t units = value % 10;
  return num2word::kOrdinalsGen.at(tens) + U" " + num2word::kOrdinalsGen.at(units);
}

} // namespace

std::u32string Num2Word::to_cardinal_int(int32_t value) const {
  if (value < 0) {
    return U"minus " + to_cardinal_int(std::abs(value));
  }

  if (auto it = num2word::kCardinals.find(value); it != num2word::kCardinals.end()) {
    return it->second;
  }

  if (value < 100) {
    // 21..99 (excluding round tens): tens word + units word, no connector.
    int32_t tens = (value / 10) * 10;
    int32_t units = value % 10;
    return num2word::kCardinals.at(tens) + U" " + num2word::kCardinals.at(units);
  }

  if (value < 1000) {
    int32_t hundreds = (value / 100) * 100;
    int32_t rest = value % 100;
    return num2word::kCardinals.at(hundreds) + U" " + to_cardinal_int(rest);
  }

  // Large-scale decomposition (tysiąc / milion / miliard) in descending order.
  for (auto it = num2word::kLargeCardinals.rbegin(); it != num2word::kLargeCardinals.rend(); ++it) {
    int64_t base = it->first;
    if (static_cast<int64_t>(value) >= base) {
      int32_t high = value / static_cast<int32_t>(base);
      int32_t low = value % static_cast<int32_t>(base);

      // Polish drops the leading "jeden" in front of any scale noun; "1000"
      // reads as "tysiąc" and "1000000" as "milion".
      std::u32string head = (high == 1) ? U"" : (to_cardinal_int(high) + U" ");
      const auto& forms = it->second;
      const auto& scale_word = forms[static_cast<std::size_t>(pick_plural_form(high))];

      std::u32string res = head + scale_word;
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
    return U"minus " + to_cardinal_float(std::abs(value), repr);
  }

  // The base transform normalizes the decimal separator to '.', but accept
  // ',' too when this method is invoked directly with a raw representation.
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

  // Leading zeros in the fractional part are spoken as "zero" tokens; the
  // remaining digits are verbalized as a single cardinal (e.g. 0,05 ->
  // "zero przecinek zero pięć", 123,45 -> "... przecinek czterdzieści pięć").
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

  return to_cardinal_int(integer_part) + U" przecinek " + frac_words;
}

std::u32string Num2Word::to_ordinal_int(int32_t value, std::u32string_view suffix) const {
  std::u32string base = ordinal_masc(value, *this);
  if (auto form = form_from_suffix(suffix)) {
    return inflect_trailing_ordinals(base, *form);
  }
  return base;
}

std::u32string Num2Word::to_currency(char32_t currency, std::variant<int32_t, float> number) const {
  auto it = num2word::kCurrencies.find(currency);
  if (it == num2word::kCurrencies.end()) {
    return std::u32string(1, currency);
  }

  PluralForm form = std::visit([](auto&& arg) -> PluralForm {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int32_t>) {
      return pick_plural_form(arg);
    } else {
      // Singular only for amounts that round to exactly 1; otherwise pick
      // based on the integer part as a coarse approximation.
      if (std::abs(arg - 1.0F) < 1e-6F) return PluralForm::SINGULAR;
      return pick_plural_form(static_cast<int32_t>(arg));
    }
  }, number);

  return it->second[static_cast<std::size_t>(form)];
}

std::u32string Num2Word::to_fraction(int32_t numerator, int32_t denominator) const {
  // The numerator agrees in gender (feminine) with the elided noun "część"
  // that the fraction implicitly modifies; only 1 and 2 have distinct forms
  // in this position ("jedna", "dwie"), other numerals are invariable here.
  std::u32string num_word;
  if (numerator == 1)      num_word = U"jedna";
  else if (numerator == 2) num_word = U"dwie";
  else                     num_word = to_cardinal_int(numerator);

  std::u32string ord_phrase = ordinal_masc(denominator, *this);
  return num_word + U" " + inflect_trailing_ordinals(ord_phrase, pick_plural_form(numerator));
}

std::u32string Num2Word::to_day(uint32_t day) const {
  // Polish dates always render the day as a masculine genitive ordinal.
  return ordinal_gen(static_cast<int32_t>(day));
}

std::u32string Num2Word::to_month(uint32_t month) const {
  if (month >= 1 && month <= 12) {
    return num2word::kMonths[month];
  }
  return U"";
}

std::u32string Num2Word::to_year(uint32_t year) const {
  // Polish reads years as plain cardinals (e.g. 2026 -> "dwa tysiące
  // dwadzieścia sześć"). The fully-inflected form ("dwa tysiące dwudziestego
  // szóstego roku") is reserved for formal contexts and not produced here.
  return to_cardinal_int(static_cast<int32_t>(year));
}

bool Num2Word::is_ordinal_suffix(std::u32string_view suffix) const {
  return num2word::kOrdinalSuffixes.contains(std::u32string(suffix));
}

} // namespace phonemis::pl
