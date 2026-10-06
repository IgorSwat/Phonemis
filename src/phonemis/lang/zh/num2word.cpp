#include "num2word.h"
#include "constants.h"

#include <algorithm>

namespace phonemis::zh {

using namespace constants::num2word;

namespace {

bool is_digit(char32_t c) {
  return (c >= U'0' && c <= U'9') || (c >= U'０' && c <= U'９');
}

int digit_value(char32_t c) {
  return c <= U'9' ? static_cast<int>(c - U'0') : static_cast<int>(c - U'０');
}

bool is_at(std::u32string_view s, size_t i, char32_t c) {
  return i < s.size() && s[i] == c;
}

// Length of the digit run starting at i.
size_t digits_at(std::u32string_view s, size_t i) {
  size_t j = i;
  while (j < s.size() && is_digit(s[j])) {
    j++;
  }
  return j - i;
}

// Length of a "\d+(\.\d+)?" number starting at i, or 0.
size_t decimal_at(std::u32string_view s, size_t i) {
  size_t integer = digits_at(s, i);
  if (integer > 0 && is_at(s, i + integer, U'.')) {
    if (size_t fraction = digits_at(s, i + integer + 1); fraction > 0) {
      return integer + 1 + fraction;
    }
  }
  return integer;
}

bool measure_word_at(std::u32string_view s, size_t i) {
  return std::ranges::any_of(kMeasureWords, [&](const std::u32string& word) {
    return s.substr(i).starts_with(word);
  });
}

// Python's str.replace(): every non-overlapping occurrence, left to right.
std::u32string replace(std::u32string_view s, std::u32string_view from, std::u32string_view to) {
  std::u32string result;
  for (size_t i = 0; i < s.size();) {
    if (s.substr(i).starts_with(from)) {
      result += to;
      i += from.size();
    } else {
      result += s[i++];
    }
  }
  return result;
}

// Replaces every match left to right, like re.sub(). `match` returns the length of the
// match at a position (0 for none), and `convert` its replacement, or nullopt to keep it.
// Every character of a replacement comes from the whole match it replaces.
template <typename Match, typename Convert>
std::u32string substitute(std::u32string_view s, SourceSpans* sources, Match match,
                          Convert convert) {
  std::u32string result;
  SourceSpans result_sources;
  for (size_t i = 0; i < s.size();) {
    if (size_t len = match(s, i); len > 0) {
      auto text = s.substr(i, len);
      result += convert(text).value_or(std::u32string{text});
      if (sources) {
        SourceSpan source = (*sources)[i];
        for (size_t j = i + 1; j < i + len; j++) {
          source = merge(source, (*sources)[j]);
        }
        result_sources.resize(result.size(), source);
      }
      i += len;
    } else {
      if (sources) {
        result_sources.push_back((*sources)[i]);
      }
      result += s[i++];
    }
  }
  if (sources) {
    *sources = std::move(result_sources);
  }
  return result;
}

// Reads the digits of a number without its sign and decimal part, e.g. "1024" -> "一千零二十四".
std::optional<std::u32string> integer_to_words(std::u32string_view digits) {
  size_t first = 0;
  while (first + 1 < digits.size() && digit_value(digits[first]) == 0) {
    first++;
  }
  digits = digits.substr(first);
  if (digits.size() > kUnits.size()) {
    return std::nullopt;
  }

  std::u32string words;
  for (size_t i = 0; i < digits.size(); i++) {
    int digit = digit_value(digits[i]);
    size_t power = digits.size() - i - 1;
    if (digit != 0) {
      words += kDigits[digit];
      words += kUnits[power];
    } else {
      if (power % 4 == 0) {
        words += kDigits[0];
        words += kUnits[power];
      }
      if (i > 0 && words.back() != kDigits[0]) {
        words += kDigits[0];
      }
    }
  }

  words = replace(words, U"零零", U"零");
  words = replace(words, U"零万", U"万");
  words = replace(words, U"零亿", U"亿");
  words = replace(words, U"亿万", U"亿");

  size_t start = words.find_first_not_of(kDigits[0]);
  words = start == std::u32string::npos
              ? U""
              : words.substr(start, words.find_last_not_of(kDigits[0]) - start + 1);

  // "万零三千" -> "万三千"
  std::u32string compact;
  for (size_t i = 0; i < words.size();) {
    if (i + 3 < words.size() && (words[i] == U'万' || words[i] == U'亿') &&
        words[i + 1] == kDigits[0] && words[i + 2] != kDigits[0] &&
        std::ranges::find(kDigits, words[i + 2]) != kDigits.end() && words[i + 3] == U'千') {
      compact += words[i];
      compact += words.substr(i + 2, 2);
      i += 4;
    } else {
      compact += words[i++];
    }
  }
  words = compact;

  if (words.starts_with(U"一十")) {
    words.erase(0, 1);
  }
  if (words.empty()) {
    words = kDigits[0];
  }
  return words;
}

} // namespace

std::optional<std::u32string> Num2Word::to_cardinal(std::u32string_view number) {
  std::u32string sign;
  if (number.starts_with(U'-')) {
    sign = kNegative;
    number.remove_prefix(1);
  }

  size_t point = number.find(U'.');
  auto integer = integer_to_words(number.substr(0, point));
  if (!integer.has_value()) {
    return std::nullopt;
  }

  std::u32string words = sign + *integer;
  if (point != std::u32string_view::npos && point + 1 < number.size()) {
    words += kPoint;
    for (char32_t c : number.substr(point + 1, kMaxDecimals)) {
      words += kDigits[digit_value(c)];
    }
  }
  return words;
}

std::u32string Num2Word::to_digits(std::u32string_view number) {
  std::u32string words;
  for (char32_t c : number) {
    if (c == U'-') {
      words += kNegative;
    } else if (c == U'.') {
      words += kPoint;
    } else {
      words += kDigits[digit_value(c)];
    }
  }
  return words;
}

std::u32string Num2Word::transform(std::u32string_view input) const {
  return transform(input, nullptr);
}

std::u32string Num2Word::transform(std::u32string_view input, SourceSpans* sources) const {
  // 1. Ranges before a measure word: "3-5个" -> "三到五个"
  std::u32string text = substitute(
      input, sources,
      [](std::u32string_view s, size_t i) -> size_t {
        size_t from = decimal_at(s, i);
        if (from == 0 || !is_at(s, i + from, U'-')) {
          return 0;
        }
        size_t to = decimal_at(s, i + from + 1);
        size_t end = i + from + 1 + to;
        return to > 0 && measure_word_at(s, end) ? end - i : 0;
      },
      [](std::u32string_view range) -> std::optional<std::u32string> {
        size_t dash = range.find(U'-');
        auto from = to_cardinal(range.substr(0, dash));
        auto to = to_cardinal(range.substr(dash + 1));
        if (!from || !to) {
          return std::nullopt;
        }
        return *from + kRangeTo + *to;
      });

  // 2. Dates: years digit by digit, months and days as numbers
  text = substitute(
      text, sources,
      [](std::u32string_view s, size_t i) -> size_t {
        size_t j = i;
        if (size_t n = digits_at(s, j); n >= 2 && n <= 4 && is_at(s, j + n, U'年')) {
          j += n + 1;
        }
        if (size_t n = digits_at(s, j); n >= 1 && n <= 2 && is_at(s, j + n, U'月')) {
          j += n + 1;
        }
        if (size_t n = digits_at(s, j); n >= 1 && n <= 2 && is_at(s, j + n, U'日')) {
          j += n + 1;
        }
        return j - i;
      },
      [](std::u32string_view date) -> std::optional<std::u32string> {
        std::u32string words;
        for (size_t i = 0; i < date.size();) {
          size_t n = digits_at(date, i);
          if (n == 0) {
            words += date[i++];
            continue;
          }
          auto digits = date.substr(i, n);
          words += is_at(date, i + n, U'年') ? to_digits(digits) : *to_cardinal(digits);
          i += n;
        }
        return words;
      });

  // 3. Fractions: "1/3" -> "三分之一"
  text = substitute(
      text, sources,
      [](std::u32string_view s, size_t i) -> size_t {
        size_t numerator = digits_at(s, i);
        if (numerator == 0 || !is_at(s, i + numerator, U'/')) {
          return 0;
        }
        size_t denominator = digits_at(s, i + numerator + 1);
        return denominator > 0 ? numerator + 1 + denominator : 0;
      },
      [](std::u32string_view fraction) -> std::optional<std::u32string> {
        size_t slash = fraction.find(U'/');
        auto numerator = to_cardinal(fraction.substr(0, slash));
        auto denominator = to_cardinal(fraction.substr(slash + 1));
        if (!numerator || !denominator) {
          return std::nullopt;
        }
        return *denominator + kFraction + *numerator;
      });

  // 4. Percentages: "-5%" -> "百分之负五"
  text = substitute(
      text, sources,
      [](std::u32string_view s, size_t i) -> size_t {
        size_t j = is_at(s, i, U'-') ? i + 1 : i;
        size_t integer = digits_at(s, j);
        if (integer == 0) {
          return 0;
        }
        // "1.%" is no percentage: a point must be followed by digits.
        size_t number = decimal_at(s, j);
        if (number == integer && is_at(s, j + integer, U'.')) {
          return 0;
        }
        return is_at(s, j + number, U'%') ? j + number + 1 - i : 0;
      },
      [](std::u32string_view percent) -> std::optional<std::u32string> {
        auto number = to_cardinal(percent.substr(0, percent.size() - 1));
        if (!number) {
          return std::nullopt;
        }
        return kPercent + *number;
      });

  // 5. Degrees Celsius: "25℃" -> "二十五摄氏度"
  text = substitute(
      text, sources,
      [](std::u32string_view s, size_t i) -> size_t {
        size_t n = digits_at(s, i);
        return n > 0 && is_at(s, i + n, U'℃') ? n + 1 : 0;
      },
      [](std::u32string_view degrees) -> std::optional<std::u32string> {
        auto number = to_cardinal(degrees.substr(0, degrees.size() - 1));
        if (!number) {
          return std::nullopt;
        }
        return *number + kCelsius;
      });

  // 6. Remaining numbers
  return substitute(
      text, sources,
      [](std::u32string_view s, size_t i) -> size_t {
        size_t j = is_at(s, i, U'-') ? i + 1 : i;
        size_t n = decimal_at(s, j);
        return n > 0 ? j + n - i : 0;
      },
      [](std::u32string_view number) { return to_cardinal(number); });
}

} // namespace phonemis::zh
