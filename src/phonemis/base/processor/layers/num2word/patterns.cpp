#include "patterns.h"
#include "primitives.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

#include <algorithm>
#include <iterator>

namespace phonemis::processor::num2word {

using namespace utils;

namespace {
// What may not stand right before a number, besides letters and digits.
constexpr std::u32string_view kNumberBoundaries = U".,";

// What may not stand right before an amount: also a hyphen, as in i9-14900K.
constexpr std::u32string_view kAmountBoundaries = U".,-";

// Value of a short run of digits (a day, a month or an hour).
int small_value(std::u32string_view digits) {
  int value = 0;
  for (char32_t c : digits) {
    value = value * 10 + static_cast<int>(c - U'0');
  }
  return value;
}

// The numbers of an amount at a position - a range (10-15) or a single number - in the order a regex tries them.
std::vector<std::vector<Number>> amounts_at(std::u32string_view text, size_t position, const Notation& notation) {
  std::vector<std::vector<Number>> amounts;
  if (!is_clear_before(text, position, kAmountBoundaries)) {
    return amounts;
  }

  auto firsts = numbers_at(text, position, notation.group_separators, notation.decimal_separators);
  for (const Number& first : firsts) {
    size_t hyphen = first.span.end;
    if (hyphen < text.size() && text[hyphen] == U'-') {
      for (Number last : numbers_at(text, hyphen + 1, notation.group_separators, notation.decimal_separators)) {
        amounts.push_back({first, std::move(last)});
      }
    }
  }
  for (const Number& number : firsts) {
    amounts.push_back({number});
  }
  return amounts;
}

// Keys of a single capital letter (W, V), or all the others.
Keys single_letters(const Keys& keys, bool single) {
  Keys selected;
  std::ranges::copy_if(keys, std::back_inserter(selected), [single](const auto& key) {
    return (key.size() == 1 && key[0] >= U'A' && key[0] <= U'Z') == single;
  });
  return selected;
}
} // namespace

// ----------------------------
// ----- Generic patterns -----
// ----------------------------

std::vector<std::unique_ptr<Pattern>> generic_patterns(const Notation& notation) {
  std::vector<std::unique_ptr<Pattern>> patterns;
  patterns.push_back(std::make_unique<IsoDatePattern>());
  patterns.push_back(std::make_unique<InternationalPhonePattern>());
  patterns.push_back(std::make_unique<VersionPattern>(notation));
  patterns.push_back(std::make_unique<DifferencePattern>());
  patterns.push_back(std::make_unique<SumPattern>());
  patterns.push_back(std::make_unique<RangePattern>());
  patterns.push_back(std::make_unique<DimensionPattern>(notation));
  if (notation.fractions) {
    patterns.push_back(std::make_unique<FractionPattern>());
  }
  patterns.push_back(std::make_unique<SignPattern>());
  patterns.push_back(std::make_unique<TimePattern>(notation));
  if (!notation.time_suffixes.empty()) {
    patterns.push_back(std::make_unique<HourPattern>(notation));
  }
  patterns.push_back(std::make_unique<ScorePattern>());
  patterns.push_back(std::make_unique<PrefixedMoneyPattern>(notation));
  patterns.push_back(std::make_unique<MoneyPattern>(notation));
  patterns.push_back(std::make_unique<ScalePattern>(notation));
  patterns.push_back(std::make_unique<PercentPattern>(notation));
  patterns.push_back(std::make_unique<UnitPattern>(notation));
  patterns.push_back(std::make_unique<CompoundPattern>(notation));
  patterns.push_back(std::make_unique<CodePattern>());
  patterns.push_back(std::make_unique<NumberPattern>(notation));
  return patterns;
}

std::optional<Match> IsoDatePattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position, U".,/-") || digits_end(text, position) != position + 4) {
    return std::nullopt;
  }

  // YYYY-MM-DD, with months and days of one or two digits
  std::vector<Number> parts = {digits_at(text, position)};
  for (size_t p = position + 4; parts.size() < 3;) {
    size_t end = digits_end(text, p + 1);
    if (p >= text.size() || text[p] != U'-' || end == p + 1 || end > p + 3) {
      return std::nullopt;
    }
    parts.push_back(digits_at(text, p + 1));
    p = end;
  }

  size_t end = parts[2].span.end;
  int month = small_value(parts[1].integer), day = small_value(parts[2].integer);
  if (!is_clear_after(text, end) || is_digit_after(text, end, U".,/-") || month < 1 || month > 12 || day < 1 ||
      day > 31) {
    return std::nullopt;
  }
  return Match{
      .kind = Kind::DATE, .span = {position, end}, .numbers = {parts[2], parts[1], parts[0]}, .separator = U'-'};
}

VersionPattern::VersionPattern(const Notation& notation) : Pattern(Kind::VERSION), group_(notation.group_separators) {}

std::optional<Match> VersionPattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position, kNumberBoundaries) || !unicode::isdigit(text[position])) {
    return std::nullopt;
  }

  std::vector<Number> parts = {digits_at(text, position)};
  while (parts.back().span.end + 1 < text.size() && text[parts.back().span.end] == U'.' &&
         unicode::isdigit(text[parts.back().span.end + 1])) {
    parts.push_back(digits_at(text, parts.back().span.end + 1));
  }

  size_t end = parts.back().span.end;
  if (parts.size() < 3 || !is_clear_after(text, end)) {
    return std::nullopt;
  }

  // 1.000.000 is a number
  bool thousands =
      strings::contains(group_, U'.') && parts[0].integer.size() <= 3 &&
      std::all_of(parts.begin() + 1, parts.end(), [](const Number& part) { return part.integer.size() == 3; });
  if (thousands) {
    return std::nullopt;
  }
  return Match{.kind = Kind::VERSION, .span = {position, end}, .numbers = parts, .separator = U'.'};
}

std::optional<Match> DifferencePattern::match(std::u32string_view text, size_t position) const {
  if (position == 0 || position + 3 >= text.size() || text.substr(position, 3) != U" - " ||
      !unicode::isdigit(text[position - 1]) || !unicode::isdigit(text[position + 3])) {
    return std::nullopt;
  }
  return Match{.kind = Kind::DIFFERENCE, .span = {position, position + 3}, .separator = U'-'};
}

std::optional<Match> SumPattern::match(std::u32string_view text, size_t position) const {
  if (position == 0 || !unicode::isdigit(text[position - 1])) {
    return std::nullopt;
  }
  size_t plus = position;
  while (plus < text.size() && unicode::isspace(text[plus])) {
    plus++;
  }
  size_t end = plus + 1;
  while (end < text.size() && unicode::isspace(text[end])) {
    end++;
  }
  if (plus >= text.size() || text[plus] != U'+' || end >= text.size() || !unicode::isdigit(text[end])) {
    return std::nullopt;
  }
  return Match{.kind = Kind::SUM, .span = {position, end}, .separator = U'+'};
}

std::optional<Match> RangePattern::match(std::u32string_view text, size_t position) const {
  if (position == 0 || !unicode::isdigit(text[position - 1])) {
    return std::nullopt;
  }

  // A hyphen (normalization writes dashes between numbers as one: 2020 – 2022)
  size_t end = position + 1;
  if (text[position] != U'-' || !is_digit_after(text, end)) {
    return std::nullopt;
  }

  // The digits on both sides
  size_t start = position;
  while (start > 0 && unicode::isdigit(text[start - 1])) {
    start--;
  }
  return Match{.kind = Kind::RANGE,
               .span = {position, end},
               .numbers = {digits_at(text, start), digits_at(text, end)},
               .separator = U'-'};
}

DimensionPattern::DimensionPattern(const Notation& notation)
    : Pattern(Kind::DIMENSION), units_(single_letters(notation.units, false)) {}

std::optional<Match> DimensionPattern::match(std::u32string_view text, size_t position) const {
  // A number on the left, possibly followed by a unit: 3x4, 3 m x 4
  size_t number_end = 0;
  if (position > 0 && unicode::isdigit(text[position - 1])) {
    number_end = position;
  } else {
    for (const auto& unit : units_) {
      if (position < unit.size() || text.substr(position - unit.size(), unit.size()) != unit) {
        continue;
      }
      size_t start = position - unit.size();
      start = start > 0 && unicode::isspace(text[start - 1]) ? start - 1 : start;
      if (start > 0 && unicode::isdigit(text[start - 1])) {
        number_end = start;
        break;
      }
    }
  }

  // Not a hexadecimal number: 0x1F
  if (number_end == 0 || (text[number_end - 1] == U'0' && is_clear_before(text, number_end - 1))) {
    return std::nullopt;
  }

  size_t x = skip_space(text, position);
  if (x >= text.size() || (text[x] != U'x' && text[x] != U'×')) {
    return std::nullopt;
  }
  size_t end = skip_space(text, x + 1);
  if (!is_digit_after(text, end)) {
    return std::nullopt;
  }
  return Match{.kind = Kind::DIMENSION, .span = {position, end}, .separator = U'x'};
}

std::optional<Match> FractionPattern::match(std::u32string_view text, size_t position) const {
  size_t slash = digits_end(text, position);
  if (!is_clear_before(text, position, U"./") || slash == position || slash >= text.size() || text[slash] != U'/') {
    return std::nullopt;
  }

  size_t end = digits_end(text, slash + 1);
  if (end == slash + 1 || !is_clear_after(text, end, U"/") || is_digit_after(text, end, U".,")) {
    return std::nullopt;
  }
  return Match{.kind = Kind::FRACTION,
               .span = {position, end},
               .numbers = {digits_at(text, position), digits_at(text, slash + 1)},
               .separator = U'/'};
}

std::optional<Match> SignPattern::match(std::u32string_view text, size_t position) const {
  if ((text[position] != U'-' && text[position] != U'−') || !is_clear_before(text, position, kNumberBoundaries) ||
      !is_digit_after(text, position + 1)) {
    return std::nullopt;
  }
  return Match{.kind = Kind::SIGN, .span = {position, position + 1}};
}

TimePattern::TimePattern(const Notation& notation)
    : Pattern(Kind::TIME),
      separators_(notation.time_separators),
      suffixes_(notation.time_suffixes),
      measures_(single_letters(notation.units, false)) {
  measures_.push_back(U"min");
}

std::optional<Match> TimePattern::match(std::u32string_view text, size_t position) const {
  size_t hours_end = digits_end(text, position);
  if (!is_clear_before(text, position, kNumberBoundaries) || hours_end == position || hours_end > position + 2 ||
      hours_end >= text.size()) {
    return std::nullopt;
  }

  // A separator (18:30, 18.30), or an "h" with optional spaces around it (18h30, 18 h 30)
  char32_t separator = 0;
  size_t minutes = hours_end + 1;
  if (text[hours_end] != U'h' && strings::contains(separators_, text[hours_end])) {
    separator = text[hours_end];
  } else if (size_t h = skip_space(text, hours_end);
             strings::contains(separators_, U'h') && h < text.size() && text[h] == U'h') {
    separator = U'h';
    minutes = skip_space(text, h + 1);
  }
  if (separator == 0 || digits_end(text, minutes) < minutes + 2) {
    return std::nullopt;
  }

  Match time{.kind = Kind::TIME,
             .numbers = {digits_at(text, position),
                         {.span = {minutes, minutes + 2}, .integer = std::u32string(text.substr(minutes, 2))}},
             .separator = separator};

  // With a suffix first (10:30 pm), then without it
  size_t suffix = skip_space(text, minutes + 2);
  for (auto key : keys_at(text, suffix, suffixes_)) {
    if (ends_at(text, suffix + key.size())) {
      time.span = {position, suffix + key.size()};
      time.suffix = key;
      return time;
    }
  }
  if (ends_at(text, minutes + 2)) {
    time.span = {position, minutes + 2};
    return time;
  }
  return std::nullopt;
}

bool TimePattern::ends_at(std::u32string_view text, size_t position) const {
  if (!is_clear_after(text, position) || is_digit_after(text, position, U".,:")) {
    return false;
  }
  for (size_t start : {position, skip_space(text, position)}) {
    for (auto key : keys_at(text, start, measures_)) {
      if (is_clear_after(text, start + key.size())) {
        return false;
      }
    }
  }
  return true;
}

HourPattern::HourPattern(const Notation& notation) : Pattern(Kind::TIME), suffixes_(notation.time_suffixes) {}

std::optional<Match> HourPattern::match(std::u32string_view text, size_t position) const {
  size_t hours_end = digits_end(text, position);
  if (!is_clear_before(text, position, kNumberBoundaries) || hours_end == position || hours_end > position + 2) {
    return std::nullopt;
  }

  size_t suffix = skip_space(text, hours_end);
  for (auto key : keys_at(text, suffix, suffixes_)) {
    if (is_clear_after(text, suffix + key.size())) {
      return Match{.kind = Kind::TIME,
                   .span = {position, suffix + key.size()},
                   .numbers = {digits_at(text, position)},
                   .suffix = key};
    }
  }
  return std::nullopt;
}

std::optional<Match> ScorePattern::match(std::u32string_view text, size_t position) const {
  if (position == 0 || text[position] != U':' || !unicode::isdigit(text[position - 1]) ||
      !is_digit_after(text, position + 1)) {
    return std::nullopt;
  }
  return Match{.kind = Kind::SCORE, .span = {position, position + 1}, .separator = U':'};
}

PrefixedMoneyPattern::PrefixedMoneyPattern(const Notation& notation) : Pattern(Kind::MONEY), notation_(notation) {
  std::ranges::copy_if(notation.currencies, std::back_inserter(currencies_), [](const auto& currency) {
    return !std::ranges::all_of(currency, [](char32_t c) { return utils::unicode::isalpha(c); });
  });
}

std::optional<Match> PrefixedMoneyPattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position)) {
    return std::nullopt;
  }

  for (auto currency : keys_at(text, position, currencies_)) {
    size_t start = skip_space(text, position + currency.size());
    auto numbers = numbers_at(text, start, notation_.group_separators, notation_.decimal_separators);
    if (numbers.empty()) {
      continue;
    }

    // An optional scale after the amount: $1.5 million
    Match money{
        .kind = Kind::MONEY, .span = {position, numbers[0].span.end}, .numbers = {numbers[0]}, .unit = currency};
    size_t scale = skip_space(text, money.span.end);
    for (auto key : keys_at(text, scale, notation_.scales)) {
      if (is_clear_after(text, scale + key.size())) {
        money.scale = key;
        money.span.end = scale + key.size();
        break;
      }
    }
    return money;
  }
  return std::nullopt;
}

MoneyPattern::MoneyPattern(const Notation& notation) : Pattern(Kind::MONEY), notation_(notation) {}

std::optional<Match> MoneyPattern::match(std::u32string_view text, size_t position) const {
  for (auto& numbers : amounts_at(text, position, notation_)) {
    size_t end = numbers.back().span.end;

    // The currency, after an optional scale: 3 mln zł, 50$
    auto currency_at = [&](size_t start, std::u32string_view scale) -> std::optional<Match> {
      for (auto currency : keys_at(text, start, notation_.currencies)) {
        if (is_clear_after(text, start + currency.size())) {
          return Match{.kind = Kind::MONEY,
                       .span = {position, start + currency.size()},
                       .numbers = numbers,
                       .unit = currency,
                       .scale = scale};
        }
      }
      return std::nullopt;
    };

    size_t scale = skip_space(text, end);
    for (auto key : keys_at(text, scale, notation_.scales)) {
      if (auto money = currency_at(skip_space(text, scale + key.size()), key)) {
        return money;
      }
    }
    if (auto money = currency_at(skip_space(text, end), U"")) {
      return money;
    }
  }
  return std::nullopt;
}

ScalePattern::ScalePattern(const Notation& notation) : Pattern(Kind::SCALE), notation_(notation) {}

std::optional<Match> ScalePattern::match(std::u32string_view text, size_t position) const {
  for (auto& numbers : amounts_at(text, position, notation_)) {
    size_t scale = skip_space(text, numbers.back().span.end);
    for (auto key : keys_at(text, scale, notation_.scales)) {
      if (is_clear_after(text, scale + key.size())) {
        return Match{.kind = Kind::SCALE, .span = {position, scale + key.size()}, .numbers = numbers, .scale = key};
      }
    }
  }
  return std::nullopt;
}

PercentPattern::PercentPattern(const Notation& notation) : Pattern(Kind::PERCENT), notation_(notation) {}

std::optional<Match> PercentPattern::match(std::u32string_view text, size_t position) const {
  for (auto& numbers : amounts_at(text, position, notation_)) {
    size_t percent = skip_space(text, numbers.back().span.end);
    if (percent >= text.size() || text[percent] != U'%') {
      continue;
    }

    // Letters right after it belong to it: 8%ige
    size_t end = percent + 1;
    while (end < text.size() && unicode::isword(text[end])) {
      end++;
    }
    return Match{.kind = Kind::PERCENT,
                 .span = {position, end},
                 .numbers = numbers,
                 .suffix = text.substr(percent + 1, end - percent - 1)};
  }
  return std::nullopt;
}

UnitPattern::UnitPattern(const Notation& notation)
    : Pattern(Kind::UNIT), notation_(notation), letters_(single_letters(notation.units, true)) {
  notation_.units = single_letters(notation.units, false);
}

std::optional<Match> UnitPattern::match(std::u32string_view text, size_t position) const {
  for (auto& numbers : amounts_at(text, position, notation_)) {
    size_t end = numbers.back().span.end;

    auto unit_at = [&](size_t start, const Keys& units) -> std::optional<Match> {
      for (auto unit : keys_at(text, start, units)) {
        size_t unit_end = start + unit.size();
        if (!is_clear_after(text, unit_end, U"'")) {
          continue;
        }
        // A dot closing both the unit and the text stays with the text: 5 Min.
        if (unit.ends_with(U'.') && is_sentence_end(text, unit_end)) {
          unit_end--;
        }
        return Match{.kind = Kind::UNIT, .span = {position, unit_end}, .numbers = numbers, .unit = unit};
      }
      return std::nullopt;
    };

    // A unit after an optional space (5 kg, 5kg), or a single letter after a space (230 V)
    for (size_t start : {skip_space(text, end), end}) {
      if (auto unit = unit_at(start, notation_.units)) {
        return unit;
      }
    }
    if (end < text.size() && unicode::isspace(text[end])) {
      if (auto unit = unit_at(end + 1, letters_)) {
        return unit;
      }
    }
  }
  return std::nullopt;
}

CompoundPattern::CompoundPattern(const Notation& notation) : Pattern(Kind::COMPOUND), notation_(notation) {}

std::optional<Match> CompoundPattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position, kAmountBoundaries)) {
    return std::nullopt;
  }

  for (auto& number : numbers_at(text, position, notation_.group_separators, notation_.decimal_separators)) {
    size_t hyphen = number.span.end;
    if (hyphen + 1 >= text.size() || text[hyphen] != U'-' || !unicode::is_latin(text[hyphen + 1])) {
      continue;
    }
    size_t end = hyphen + 1;
    while (end < text.size() && unicode::is_latin(text[end])) {
      end++;
    }
    return Match{.kind = Kind::COMPOUND,
                 .span = {position, end},
                 .numbers = {number},
                 .suffix = text.substr(hyphen + 1, end - hyphen - 1)};
  }
  return std::nullopt;
}

std::optional<Match> CodePattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position, U"'.-")) {
    return std::nullopt;
  }

  // A token of word characters, apostrophes, dots and hyphens, with both a digit and a Latin letter
  size_t start = text[position] == U'.' ? position + 1 : position;
  bool digit = false, letter = false;
  for (size_t i = start;
       i < text.size() && (unicode::isword(text[i]) || text[i] == U'\'' || text[i] == U'.' || text[i] == U'-'); ++i) {
    digit |= unicode::isdigit(text[i]);
    letter |= unicode::is_latin(text[i]);
  }
  if (!digit || !letter || start >= text.size() || !unicode::isword(text[start])) {
    return std::nullopt;
  }

  // A dot only continues it before a digit: H.264
  size_t end = start + 1;
  while (end < text.size() && (unicode::isword(text[end]) || text[end] == U'\'' || text[end] == U'-' ||
                               (text[end] == U'.' && is_digit_after(text, end + 1)))) {
    end++;
  }
  return Match{.kind = Kind::CODE, .span = {position, end}};
}

NumberPattern::NumberPattern(const Notation& notation) : Pattern(Kind::NUMBER), notation_(notation) {}

std::optional<Match> NumberPattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position, kNumberBoundaries)) {
    return std::nullopt;
  }

  for (auto& number : numbers_at(text, position, notation_.group_separators, notation_.decimal_separators)) {
    if (is_clear_after(text, number.span.end)) {
      return Match{.kind = Kind::NUMBER, .span = number.span, .numbers = {number}};
    }
  }

  // .5: a fraction without its integer part
  if (text[position] == U'.' && strings::contains(notation_.decimal_separators, U'.') &&
      is_digit_after(text, position + 1)) {
    size_t end = digits_end(text, position + 1);
    if (is_clear_after(text, end)) {
      Number number{.span = {position, end},
                    .fraction = std::u32string(text.substr(position + 1, end - position - 1)),
                    .decimal_mark = U'.'};
      return Match{.kind = Kind::NUMBER, .span = number.span, .numbers = {number}};
    }
  }
  return std::nullopt;
}

// ---------------------------------
// ----- Configurable patterns -----
// ---------------------------------

DatePattern::DatePattern(DateFormat format) : Pattern(Kind::DATE), format_(std::move(format)) {}

std::optional<Match> DatePattern::match(std::u32string_view text, size_t position) const {
  size_t day_end = digits_end(text, position);
  if (!is_clear_before(text, position, format_.boundaries) || day_end == position || day_end > position + 2 ||
      day_end >= text.size() || !strings::contains(format_.separators, text[day_end])) {
    return std::nullopt;
  }

  char32_t separator = text[day_end];
  size_t month = format_.spaced ? skip_space(text, day_end + 1) : day_end + 1;
  size_t month_end = digits_end(text, month);
  if (month_end == month || month_end > month + 2) {
    return std::nullopt;
  }

  Match date{
      .kind = Kind::DATE, .numbers = {digits_at(text, position), digits_at(text, month)}, .separator = separator};
  int d = small_value(date.numbers[0].integer), m = small_value(date.numbers[1].integer);
  if (d < 1 || d > 31 || m < 1 || m > 12) {
    return std::nullopt;
  }
  date.span.begin = position;

  // A closing separator always follows the month (1.5.), otherwise only before a year (12/05/2024)
  bool closed = month_end < text.size() && text[month_end] == separator;
  if (format_.closing_separator && !closed) {
    return std::nullopt;
  }

  // With a year first, then (if allowed) without it
  if (closed) {
    size_t year = month_end + 1;
    size_t length = digits_end(text, year) - year;
    if (length == 4 || (length == 2 && format_.short_year)) {
      Match dated = date;
      dated.numbers.push_back(digits_at(text, year));
      if (auto result = complete(text, dated, year + length)) {
        return result;
      }
    }
  }
  if (format_.year_required) {
    return std::nullopt;
  }
  return complete(text, date, format_.closing_separator ? month_end + 1 : month_end);
}

std::optional<Match> DatePattern::complete(std::u32string_view text, Match date, size_t position) const {
  // An abbreviation after the year: 2024 r.
  if (date.numbers.size() == 3) {
    size_t suffix = skip_space(text, position);
    for (auto key : keys_at(text, suffix, format_.suffixes)) {
      size_t end = suffix + key.size();
      if (is_clear_after(text, end) && !is_digit_after(text, end, format_.boundaries)) {
        // A dot closing both the abbreviation and the sentence stays with the sentence
        date.suffix = key;
        date.span.end = key.ends_with(U'.') && is_sentence_end(text, end, true) ? end - 1 : end;
        return date;
      }
    }
  }

  if (!is_clear_after(text, position) || is_digit_after(text, position, format_.boundaries)) {
    return std::nullopt;
  }
  date.span.end = position;
  return date;
}

std::optional<Match> InternationalPhonePattern::match(std::u32string_view text, size_t position) const {
  if (text[position] != U'+' || !is_clear_before(text, position)) {
    return std::nullopt;
  }

  // 1. The country code: one to three digits
  Number country = digits_at(text, position + 1);
  if (country.integer.empty() || country.integer.size() > 3) {
    return std::nullopt;
  }
  Match phone{.kind = Kind::INTERNATIONAL_PHONE, .numbers = {country}};
  size_t end = country.span.end, length = country.integer.size();

  // 2. Groups of digits, each after a space, a hyphen or a dot, or in parentheses: +44 (0)20 7946-0958
  for (bool closed = false; end < text.size();) {
    size_t next = strings::contains(std::u32string_view(U" -."), text[end]) ? end + 1 : end;
    bool parenthesized = next < text.size() && text[next] == U'(';
    Number group = digits_at(text, next + parenthesized);
    size_t group_end = group.span.end;
    if (group.integer.empty() || (next == end && !parenthesized && !closed) ||
        (parenthesized && (group_end >= text.size() || text[group_end] != U')'))) {
      break;
    }
    phone.numbers.push_back(group);
    end = group_end + parenthesized;
    length += group.integer.size();
    closed = parenthesized; // (0)20: a group may follow a closing parenthesis directly
  }

  // 3. A whole number: 8 to 15 digits, as numbering plans allow
  if (phone.numbers.size() < 2 || length < 8 || length > 15 || !is_clear_after(text, end) ||
      is_digit_after(text, end, U".,")) {
    return std::nullopt;
  }
  phone.span = {position, end};
  return phone;
}

PhonePattern::PhonePattern(PhoneFormat format) : Pattern(Kind::PHONE), format_(std::move(format)) {}

std::optional<Match> PhonePattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position, format_.boundaries)) {
    return std::nullopt;
  }

  // With its optional prefix first (1-555-...), then without it
  if (!format_.prefix.empty() && text.substr(position).starts_with(format_.prefix)) {
    if (auto phone = groups_at(text, position, position + format_.prefix.size())) {
      return phone;
    }
  }
  return groups_at(text, position, position);
}

std::optional<Match> PhonePattern::groups_at(std::u32string_view text, size_t start, size_t position) const {
  Match phone{.kind = Kind::PHONE};

  // (555) 123-4567: the parenthesis opens the first group
  bool parenthesized = format_.parenthesized;
  if (parenthesized) {
    if (position >= text.size() || text[position] != U'(') {
      return std::nullopt;
    }
    position++;
  }

  for (size_t i = 0; i < format_.groups.size(); ++i) {
    // The separator before a group, which a closing parenthesis replaces
    bool after_parenthesis = i == 1 && parenthesized;
    if (i > 0 && !after_parenthesis && position < text.size() &&
        strings::contains(format_.separators, text[position])) {
      phone.separator = phone.separator ? phone.separator : text[position];
      position++;
    } else if (i > 0 && !after_parenthesis && !format_.optional_separators) {
      return std::nullopt;
    }
    if (i == 0 && format_.leading_zero && (position >= text.size() || text[position] != U'0')) {
      return std::nullopt;
    }

    // Without separators, a run of digits splits into groups of their longest length
    auto [shortest, longest] = format_.groups[i];
    size_t length = digits_end(text, position) - position;
    length = format_.optional_separators ? std::min(length, longest) : length;
    if (length < shortest || length > longest) {
      return std::nullopt;
    }
    phone.numbers.push_back(
        {.span = {position, position + length}, .integer = std::u32string(text.substr(position, length))});
    position += length;

    if (i == 0 && parenthesized) {
      if (position >= text.size() || text[position] != U')') {
        return std::nullopt;
      }
      position = skip_space(text, position + 1);
    }
  }

  if (!is_clear_after(text, position, format_.closing) || is_digit_after(text, position, U".,")) {
    return std::nullopt;
  }
  phone.span = {start, position};
  return phone;
}

SuffixPattern::SuffixPattern(Kind kind, Keys suffixes, SuffixFormat format)
    : Pattern(kind), suffixes_(std::move(suffixes)), format_(std::move(format)) {}

std::optional<Match> SuffixPattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position, format_.boundaries)) {
    return std::nullopt;
  }

  // '90s: an apostrophe before the digits
  size_t digits = format_.apostrophes && text[position] == U'\'' ? position + 1 : position;
  size_t end = digits_end(text, digits);
  if (end == digits) {
    return std::nullopt;
  }

  // 90's, 1 º: an apostrophe or a space before the suffix
  size_t suffix = end;
  if (format_.apostrophes && suffix < text.size() && text[suffix] == U'\'') {
    suffix++;
  }
  if (format_.space) {
    suffix = skip_space(text, suffix);
  }

  for (auto key : keys_at(text, suffix, suffixes_)) {
    size_t suffix_end = suffix + key.size();
    if (format_.word_end && !is_clear_after(text, suffix_end)) {
      continue;
    }
    // A dot closing both the suffix and the sentence stays with the sentence: 2020 r.
    if (format_.sentence_dot && key.ends_with(U'.') && is_sentence_end(text, suffix_end, true)) {
      suffix_end--;
    }
    return Match{.kind = kind(), .span = {position, suffix_end}, .numbers = {digits_at(text, digits)}, .suffix = key};
  }
  return std::nullopt;
}

DotOrdinalPattern::DotOrdinalPattern(bool before_punctuation, bool sentence_dot)
    : Pattern(Kind::ORDINAL), before_punctuation_(before_punctuation), sentence_dot_(sentence_dot) {}

std::optional<Match> DotOrdinalPattern::match(std::u32string_view text, size_t position) const {
  size_t dot = digits_end(text, position);
  if (!is_clear_before(text, position, kNumberBoundaries) || dot == position || dot >= text.size() ||
      text[dot] != U'.') {
    return std::nullopt;
  }

  // Followed by a word, or (optionally) by a comma or the end of the text
  size_t next = dot + 1;
  while (next < text.size() && unicode::isspace(text[next])) {
    next++;
  }
  bool word = next > dot + 1 && next < text.size();
  bool punctuation = before_punctuation_ && (dot + 1 == text.size() || text[dot + 1] == U',');
  if (!word && !punctuation) {
    return std::nullopt;
  }
  size_t end = sentence_dot_ && is_sentence_end(text, dot + 1, true) ? dot : dot + 1;
  return Match{.kind = Kind::ORDINAL, .span = {position, end}, .numbers = {digits_at(text, position)}, .suffix = U"."};
}

} // namespace phonemis::processor::num2word
