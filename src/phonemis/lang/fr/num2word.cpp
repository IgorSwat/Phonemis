#include "num2word.h"
#include "constants.h"
#include <phonemis/utils/conversions.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace phonemis::fr {

using namespace constants;
using namespace phonemis::utils;

namespace {

// Returns the position right after the last separator (space or hyphen) in `word`,
// which marks the start of the last atomic cardinal token.
size_t last_token_start(const std::u32string& word) {
  size_t pos = word.size();
  while (pos > 0 && word[pos - 1] != U' ' && word[pos - 1] != U'-') {
    pos--;
  }
  return pos;
}

// Converts a cardinal token to its ordinal form (e.g. "douze" -> "douzième").
std::u32string cardinal_to_ordinal_token(std::u32string_view token) {
  if (token == U"un")   return U"unième";
  if (token == U"cinq") return U"cinquième";
  if (token == U"neuf") return U"neuvième";
  // The plural 's' on "cents" / "quatre-vingts" is dropped for ordinals; other
  // tokens naturally ending in 's' (e.g. "trois") keep it.
  if (token == U"vingts") return U"vingtième";
  if (token == U"cents")  return U"centième";

  std::u32string stem(token);
  if (!stem.empty() && stem.back() == U'e') {
    stem.pop_back();
  }
  return stem + U"ième";
}

} // namespace

std::u32string Num2Word::to_cardinal_int(int32_t value) const {
  if (value < 0) {
    return U"moins " + to_cardinal_int(std::abs(value));
  }

  if (num2word::kCardinals.contains(value)) {
    return num2word::kCardinals.at(value);
  }

  if (value < 70) {
    // Regular tens-units composition with "et" insertion for *1 values (e.g. 21, 31...).
    int32_t tens = (value / 10) * 10;
    int32_t units = value % 10;
    std::u32string sep = (units == 1) ? U" et " : U" ";
    return num2word::kCardinals.at(tens) + sep + num2word::kCardinals.at(units);
  }

  if (value < 80) {
    // 70-79: "soixante" base with an extended units word (dix, onze, ..., dix neuf).
    int32_t units = value - 60;
    if (units == 11) return U"soixante et onze";
    return U"soixante " + num2word::kCardinals.at(units);
  }

  if (value < 100) {
    // 80-99: "quatre vingt(s)" base, no "et" even at 81/91.
    int32_t units = value - 80;
    if (units == 0) return U"quatre vingts";
    return U"quatre vingt " + num2word::kCardinals.at(units);
  }

  // Handle large numbers by iterating through the large scales map (cent, mille, million, ...)
  // in descending order to find the largest applicable base.
  for (auto it = num2word::kLargeCardinals.rbegin(); it != num2word::kLargeCardinals.rend(); ++it) {
    int64_t base = it->first;
    if (static_cast<int64_t>(value) >= base) {
      int32_t high = value / static_cast<int32_t>(base);
      int32_t low = value % static_cast<int32_t>(base);

      // "cent" and "mille" drop the leading "un" when used as multiplier of 1.
      std::u32string head;
      if (high == 1 && (base == 100LL || base == 1000LL)) {
        head = U"";
      } else {
        head = to_cardinal_int(high) + U" ";
      }

      std::u32string scale = it->second;
      // "cent" takes an 's' when multiplied and not followed by a smaller part.
      // "million"/"milliard" pluralize as normal nouns when multiplied.
      // "mille" is invariable.
      if (base == 100LL && high > 1 && low == 0) {
        scale += U"s";
      } else if (base >= 1000000LL && high > 1) {
        scale += U"s";
      }

      std::u32string res = head + scale;
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
    return U"moins " + to_cardinal_float(std::abs(value), repr);
  }

  // The base transform normalizes the decimal separator to '.', but accept ',' too
  // for direct callers.
  std::string value_str = conversions::u32_to_utf8(repr);
  std::replace(value_str.begin(), value_str.end(), ',', '.');

  // Trim trailing zeros in the fractional part and a dangling decimal point.
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

  // French verbalizes the fractional part as a single cardinal number
  // (e.g. "45" -> "quarante cinq"), but any leading zeros preserve their value
  // as spoken "zéro" words (e.g. "05" -> "zéro cinq").
  std::u32string frac_words;
  size_t pos = 0;
  while (pos < fractional_part_str.size() && fractional_part_str[pos] == '0') {
    if (!frac_words.empty()) frac_words += U" ";
    frac_words += U"zéro";
    pos++;
  }
  if (pos < fractional_part_str.size()) {
    if (!frac_words.empty()) frac_words += U" ";
    frac_words += to_cardinal_int(std::stoi(fractional_part_str.substr(pos)));
  }

  return to_cardinal_int(integer_part) + U" virgule " + frac_words;
}

std::u32string Num2Word::to_ordinal_int(int32_t value, std::u32string_view suffix) const {
  // "premier"/"première" are the only fully irregular ordinals; they do not appear
  // as a suffix in compounds (21 -> "vingt et unième"), so we only apply them
  // when the value is exactly 1. The suffix disambiguates masculine vs. feminine.
  if (value == 1) {
    bool feminine = (suffix == U"re" || suffix == U"res" ||
                     suffix == U"ère" || suffix == U"ères");
    return feminine ? U"première" : U"premier";
  }

  std::u32string card = to_cardinal_int(value);
  size_t split = last_token_start(card);
  std::u32string prefix = card.substr(0, split);
  std::u32string_view last = std::u32string_view(card).substr(split);

  return prefix + cardinal_to_ordinal_token(last);
}

std::u32string Num2Word::to_currency(char32_t currency, std::variant<int32_t, float> number) const {
  if (!num2word::kCurrencies.contains(currency)) {
    return std::u32string(1, currency);
  }

  std::u32string result = num2word::kCurrencies.at(currency);

  // French pluralization follows the same |value| > 1 heuristic as English:
  // singular only for an amount that rounds to exactly 1.
  bool is_singular = std::visit([](auto&& arg) -> bool {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int32_t>) {
      return arg == 1;
    } else {
      return std::abs(arg - 1.0F) < 1e-6F;
    }
  }, number);

  if (!is_singular) {
    result += U"s";
  }
  return result;
}

std::u32string Num2Word::to_month(uint32_t month) const {
  if (month >= 1 && month <= 12) {
    return num2word::kMonths[month];
  }
  return U"";
}

std::u32string Num2Word::to_year(uint32_t year) const {
  // French reads years as plain cardinals (e.g. 2026 -> "deux mille vingt six"),
  // unlike the English "twenty twenty-six" style.
  return to_cardinal_int(static_cast<int32_t>(year));
}

bool Num2Word::is_ordinal_suffix(std::u32string_view suffix) const {
  return num2word::kOrdinalSuffixes.contains(std::u32string(suffix));
}

std::u32string Num2Word::to_fraction(int32_t numerator, int32_t denominator) const {
  // Common fractions have dedicated French nouns instead of the ordinal form,
  // and the denominator noun agrees in number with the numerator.
  std::u32string num_word = to_cardinal_int(numerator);
  bool plural = std::abs(numerator) > 1;

  auto with_plural = [&](std::u32string_view base) {
    std::u32string w(base);
    if (plural) w += U"s";
    return num_word + U" " + w;
  };

  if (denominator == 2) return with_plural(U"demi");
  if (denominator == 3) return with_plural(U"tiers");
  if (denominator == 4) return with_plural(U"quart");

  std::u32string ord = to_ordinal_int(denominator);
  if (plural) ord += U"s";
  return num_word + U" " + ord;
}

std::u32string Num2Word::to_day(uint32_t day) const {
  // French dates read the day as a cardinal, except for the first of the month.
  if (day == 1) return to_ordinal_int(1);
  return to_cardinal_int(static_cast<int32_t>(day));
}

} // namespace phonemis::fr
