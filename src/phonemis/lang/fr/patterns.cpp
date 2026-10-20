#include "patterns.h"
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/conversions.h>

#include <array>
#include <optional>
#include <string>

namespace phonemis::fr {

using namespace processor;
using namespace processor::num2word;

namespace {
int roman_digit(char32_t c) {
  switch (c) {
    case U'I':
      return 1;
    case U'V':
      return 5;
    case U'X':
      return 10;
    case U'L':
      return 50;
    case U'C':
      return 100;
    default:
      return 0;
  }
}

// The value of a Roman numeral written in its canonical form (XIX, not XVIIII).
std::optional<int> roman_value(std::u32string_view numeral) {
  int value = 0;
  for (size_t i = 0; i < numeral.size(); ++i) {
    int digit = roman_digit(numeral[i]);
    value += i + 1 < numeral.size() && digit < roman_digit(numeral[i + 1]) ? -digit : digit;
  }

  static constexpr std::array<std::pair<int, std::u32string_view>, 9> kSymbols = {
      {{100, U"C"}, {90, U"XC"}, {50, U"L"}, {40, U"XL"}, {10, U"X"}, {9, U"IX"}, {5, U"V"}, {4, U"IV"}, {1, U"I"}}};
  std::u32string canonical;
  for (int rest = value; const auto& [symbol_value, symbol] : kSymbols) {
    for (; rest >= symbol_value; rest -= symbol_value) {
      canonical += symbol;
    }
  }
  return canonical == numeral ? std::optional(value) : std::nullopt;
}
} // namespace

std::optional<Match> RomanOrdinalPattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position)) {
    return std::nullopt;
  }

  // Two Roman digits or more, or a single I (Ier)
  size_t end = position;
  while (end < text.size() && roman_digit(text[end])) {
    end++;
  }
  std::u32string_view numeral = text.substr(position, end - position);
  if (numeral.size() < 2 && numeral != U"I") {
    return std::nullopt;
  }
  auto value = roman_value(numeral);
  if (!value) {
    return std::nullopt;
  }

  for (auto suffix : keys_at(text, end, {U"er", U"re", U"ère", U"e", U"ème"})) {
    if (is_clear_after(text, end + suffix.size())) {
      std::u32string digits = utils::conversions::utf8_to_u32(std::to_string(*value));
      return Match{.kind = Kind::ORDINAL,
                   .span = {position, end + suffix.size()},
                   .numbers = {{.span = {position, end}, .integer = digits}},
                   .suffix = suffix};
    }
  }
  return std::nullopt;
}

} // namespace phonemis::fr
