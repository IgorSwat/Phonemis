#include "num2word.h"
#include "constants.h"
#include <phonemis/base/processor/constants.h>
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/unicode.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>

namespace phonemis::de {

using namespace constants;
using namespace phonemis::utils;
using processor::num2word::Mode;
using processor::num2word::StringifiedNumber;

namespace {
// Checks if a character serves as a word boundary.
bool is_boundary(char32_t c) {
  return !utils::unicode::isalnum(c) && c != U'_';
}

// Counts and returns the number of consecutive digits from the start index
size_t count_digits(std::u32string_view input, size_t start) {
  auto it = std::find_if(
      input.begin() + start, input.end(),
      [](char32_t c) { return !utils::unicode::isdigit(c); });
  return static_cast<size_t>(it - (input.begin() + start));
}

// Checks if a character is a known currency symbol.
bool is_currency(char32_t c) {
  for (auto sym : processor::constants::kCurrencies) {
    if (c == sym) return true;
  }
  return false;
}

bool has_nominative_article(std::u32string_view input, size_t start) {
  size_t pos = start;
  while (pos > 0 && utils::unicode::isspace(input[pos - 1])) {
    pos--;
  }
  size_t end = pos;
  while (pos > 0 && utils::unicode::isalpha(input[pos - 1])) {
    pos--;
  }
  if (pos == end) return false;

  std::u32string word(input.substr(pos, end - pos));
  for (auto& ch : word) {
    ch = utils::unicode::tolower(ch);
  }

  return word == U"der" || word == U"die" || word == U"das" ||
         word == U"ein" || word == U"eine";
}
} // namespace

std::u32string Num2Word::transform(std::u32string_view input) const {
  char32_t dec_sep = decimal_separator();
  std::u32string owned;
  std::u32string_view work = input;
  if (dec_sep != U'.') {
    owned.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
      if (input[i] == dec_sep && i > 0 && i + 1 < input.size() &&
          utils::unicode::isdigit(input[i - 1]) && utils::unicode::isdigit(input[i + 1])) {
        owned.push_back(U'.');
      } else {
        owned.push_back(input[i]);
      }
    }
    work = owned;
  }

  std::u32string result;
  result.reserve(work.size());

  size_t last_pos = 0;
  for (size_t i = 0; i < work.size(); ++i) {
    if (!utils::unicode::isdigit(work[i])) continue;
    if (i > 0 && !is_boundary(work[i - 1])) continue;

    size_t start = i;
    size_t len = 0;
    Mode mode = Mode::CARDINAL;

    size_t p1 = count_digits(work, start);
    size_t curr = start + p1;

    if (curr < work.size() && (work[curr] == U'.' || work[curr] == U'-' || work[curr] == U'/')) {
      char32_t sep = work[curr];
      size_t p2 = count_digits(work, curr + 1);

      if (p2 > 0) {
        size_t next_curr = curr + 1 + p2;

        if (next_curr < work.size() && work[next_curr] == sep && (sep == U'.' || sep == U'-')) {
          size_t p3 = count_digits(work, next_curr + 1);
          if (p3 > 0 &&
              (next_curr + 1 + p3 == work.size() || is_boundary(work[next_curr + 1 + p3]))) {
            if (p1 <= 4 && p2 <= 2 && p3 <= 4) {
              len = next_curr + 1 + p3 - start;
              mode = Mode::DATE;
            }
          }
        }

        if (len == 0 && (curr + 1 + p2 == work.size() || is_boundary(work[curr + 1 + p2]))) {
          len = curr + 1 + p2 - start;
          size_t after = start + len;
          if (sep != U'/' && after < work.size() && is_currency(work[after]) &&
              (after + 1 == work.size() || is_boundary(work[after + 1]))) {
            len++;
            mode = Mode::CURRENCY;
          } else {
            mode = (sep == U'/') ? Mode::FRACTION : Mode::CARDINAL;
          }
        }
      } else if (sep == U'.') {
        len = p1 + 1;
        mode = Mode::ORDINAL;

        size_t next_pos = start + len;
        while (next_pos < work.size() && utils::unicode::isspace(work[next_pos])) {
          next_pos++;
        }
        bool next_is_word = (next_pos < work.size()) && utils::unicode::isalpha(work[next_pos]);
        if (!config_.allow_general_ord_notation || !next_is_word) {
          mode = Mode::CARDINAL;
          len--;
        }
      }
    }

    if (len == 0) {
      if (curr < work.size() && is_currency(work[curr]) &&
          (curr + 1 == work.size() || is_boundary(work[curr + 1]))) {
        len = p1 + 1;
        mode = Mode::CURRENCY;
      } else {
        size_t alpha = 0;
        while (curr + alpha < work.size() && utils::unicode::isalpha(work[curr + alpha])) {
          alpha++;
        }

        if (alpha > 0 && (curr + alpha == work.size() || is_boundary(work[curr + alpha]))) {
          len = p1 + alpha;
          mode = Mode::POTENTIALLY_ORDINAL;
        } else if (curr == work.size() || is_boundary(work[curr])) {
          len = p1;
          mode = Mode::CARDINAL;
        }
      }
    }

    if (len > 0) {
      result.append(work.substr(last_pos, start - last_pos));

      if (mode == Mode::DATE) {
        char32_t sep = (work.substr(start, len).find(U'.') != std::u32string_view::npos) ? U'.' : U'-';
        size_t first_sep = work.find(sep, start);
        size_t second_sep = work.find(sep, first_sep + 1);

        std::u32string_view p1 = work.substr(start, first_sep - start);
        std::u32string_view p2 = work.substr(first_sep + 1, second_sep - first_sep - 1);
        std::u32string_view p3 = work.substr(second_sep + 1, start + len - second_sep - 1);

        auto val1 = as_int(p1);
        auto val2 = as_int(p2);
        auto val3 = as_int(p3);

        if (!val1 || !val2 || !val3) {
          result.append(work.substr(start, len));
        } else {
          bool nominative = has_nominative_article(work, start);
          std::u32string day_word;

          if (p1.size() == 4) {
            day_word = to_ordinal_int(static_cast<int32_t>(*val3), nominative ? U"e" : U"en");
            result.append(day_word + U" " + to_month(*val2) + U" " + to_year(*val1));
          } else {
            day_word = to_ordinal_int(static_cast<int32_t>(*val1), nominative ? U"e" : U"en");
            result.append(day_word + U" " + to_month(*val2) + U" " + to_year(*val3));
          }
        }
      } else {
        result.append(verbalize({work.substr(start, len), mode}));
      }

      last_pos = start + len;
      i = start + len - 1;
    }
  }

  result.append(work.substr(last_pos));
  return result;
}

std::u32string Num2Word::to_cardinal_int(int32_t value) const {
  if (value < 0) {
    return U"minus " + to_cardinal_int(std::abs(value));
  }

  if (value < 20) {
    return num2word::kCardinals.at(value);
  }

  if (value < 100) {
    int32_t units = value % 10;
    int32_t tens = (value / 10) * 10;
    if (units == 0) {
        return num2word::kCardinals.at(tens);
    }
    std::u32string unit_str = (units == 1) ? U"ein" : num2word::kCardinals.at(units);
    return unit_str + U"und" + num2word::kCardinals.at(tens);
  }

  for (auto it = num2word::kLargeCardinals.rbegin(); it != num2word::kLargeCardinals.rend(); ++it) {
    int64_t base = it->first;
    if (static_cast<int64_t>(value) >= base) {
      int32_t high = value / static_cast<int32_t>(base);
      int32_t low = value % static_cast<int32_t>(base);

      std::u32string head;
      if (high == 1) {
          head = (base >= 1000000LL) ? U"eine " : U"ein";
      } else {
          head = to_cardinal_int(high);
          if (base >= 1000000LL) {
              head += U" ";
          }
      }

      std::u32string scale = it->second;
      if (base >= 1000000LL && high > 1) {
          if (base == 1000000LL) scale = U"Millionen";
          else if (base == 1000000000LL) scale = U"Milliarden";
      }

      std::u32string res = head + scale;
      if (low > 0) {
          if (base >= 1000000LL) res += U" ";
          res += to_cardinal_int(low);
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

  std::string value_str = conversions::u32_to_utf8(repr);
  std::replace(value_str.begin(), value_str.end(), ',', '.');

  auto rit = std::find_if(value_str.rbegin(), value_str.rend(), [](char c) { return c != '0'; });
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

  std::u32string res = to_cardinal_int(integer_part) + U" Komma";
  for (char c : fractional_part_str) {
    if (isdigit(static_cast<unsigned char>(c))) {
      res += U" " + to_cardinal_int(c - '0');
    }
  }

  return res;
}

std::u32string Num2Word::to_ordinal_int(int32_t value, std::u32string_view suffix) const {
  if (value < 0) {
    return U"minus " + to_ordinal_int(std::abs(value), suffix);
  }

  std::u32string stem;
  if (value == 1) stem = U"erst";
  else if (value == 3) stem = U"dritt";
  else if (value == 7) stem = U"siebt";
  else if (value == 8) stem = U"acht";
  else if (value < 20) stem = to_cardinal_int(value) + U"t";
  else stem = to_cardinal_int(value) + U"st";

  std::u32string ending = U"e";
  if (!suffix.empty()) {
    char32_t last = utils::unicode::tolower(suffix.back());
    if (last == U'n') ending = U"en";
    else if (last == U'r') ending = U"er";
    else if (last == U's') ending = U"es";
    else if (last == U'm') ending = U"em";
  }

  return stem + ending;
}

std::u32string Num2Word::to_currency(char32_t currency, std::variant<int32_t, float> number) const {
  if (!num2word::kCurrencies.contains(currency)) {
    return std::u32string(1, currency);
  }
  return num2word::kCurrencies.at(currency);
}

std::u32string Num2Word::to_month(uint32_t month) const {
  if (month >= 1 && month <= 12) {
    return num2word::kMonths[month];
  }
  return U"";
}

std::u32string Num2Word::to_year(uint32_t year) const {
  if (year >= 1100 && year < 2000) {
    int32_t hundreds = year / 100;
    int32_t remainder = year % 100;
    std::u32string res = to_cardinal_int(hundreds) + U"hundert";
    if (remainder > 0) {
        res += to_cardinal_int(remainder);
    }
    return res;
  }
  return to_cardinal_int(static_cast<int32_t>(year));
}

bool Num2Word::is_ordinal_suffix(std::u32string_view suffix) const {
  return num2word::kOrdinalSuffixes.contains(std::u32string(suffix));
}

std::u32string Num2Word::to_fraction(int32_t numerator, int32_t denominator) const {
  std::u32string num_word = (numerator == 1) ? U"ein" : to_cardinal_int(numerator);
  std::u32string den_word;
  if (denominator == 2) {
    den_word = U"halb";
  } else if (denominator == 3) {
    den_word = U"drittel";
  } else if (denominator == 4) {
    den_word = U"viertel";
  } else if (denominator < 20) {
    den_word = to_cardinal_int(denominator) + U"tel";
  } else {
    den_word = to_cardinal_int(denominator) + U"stel";
  }

  if (!den_word.empty()) {
    den_word[0] = utils::unicode::toupper(den_word[0]);
  }
  return num_word + U" " + den_word;
}

std::u32string Num2Word::to_day(uint32_t day) const {
  return to_ordinal_int(static_cast<int32_t>(day));
}

std::u32string Num2Word::verbalize(const StringifiedNumber& number) const {
  if (number.conversionMode != Mode::CURRENCY) {
    return Num2WordLayer::verbalize(number);
  }

  char32_t currency = number.text.back();
  std::u32string_view num_part = number.text.substr(0, number.text.size() - 1);

  if (num_part.find(U'.') != std::u32string_view::npos) {
    auto val = as_float(num_part);
    if (!val) return U"";
    bool singular = std::abs(*val - 1.0F) < 1e-6F;
    std::u32string amount = singular ? U"ein" : to_cardinal_float(*val, num_part);
    return amount + U" " + to_currency(currency, *val);
  }

  auto val = as_int(num_part);
  if (!val) return U"";
  bool singular = (*val == 1);
  std::u32string amount = singular ? U"ein" : to_cardinal_int(*val);
  return amount + U" " + to_currency(currency, *val);
}

} // namespace phonemis::de
