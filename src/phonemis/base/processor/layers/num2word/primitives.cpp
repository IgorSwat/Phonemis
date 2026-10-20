#include "../../common/text.h"
#include "primitives.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

#include <algorithm>

namespace phonemis::processor::num2word {

using namespace utils;

Number digits_at(std::u32string_view text, size_t position) {
  size_t end = digits_end(text, position);
  return {.span = {position, end}, .integer = std::u32string(text.substr(position, end - position))};
}

std::vector<Number> numbers_at(std::u32string_view text, size_t position, std::u32string_view group,
                               std::u32string_view decimal) {
  size_t run = digits_end(text, position) - position;
  if (run == 0) {
    return {};
  }

  // Integer parts: 1-3 digits followed by whole groups (1 000 000), then a plain run of digits
  std::vector<Number> integers;
  if (run <= 3) {
    size_t end = position + run;
    while (end + 3 < text.size() && strings::contains(group, text[end]) && digits_end(text, end + 1) >= end + 4) {
      end += 4;
    }
    // A group may not be followed by a digit, nor by a separator and a digit
    bool complete = end > position + run && !is_digit_after(text, end) &&
                    !(end < text.size() && strings::contains(group, text[end]) && is_digit_after(text, end + 1));
    if (complete) {
      std::u32string digits;
      std::ranges::copy_if(text.substr(position, end - position), std::back_inserter(digits), unicode::isdigit);
      integers.push_back({.span = {position, end}, .integer = digits, .grouped = true});
    }
  }
  integers.push_back(digits_at(text, position));

  // Each with its fraction first, then without it
  std::vector<Number> numbers;
  for (const Number& integer : integers) {
    size_t end = integer.span.end;
    if (end < text.size() && strings::contains(decimal, text[end]) && is_digit_after(text, end + 1)) {
      Number number = integer;
      number.span.end = digits_end(text, end + 1);
      number.fraction = text.substr(end + 1, number.span.end - end - 1);
      number.decimal_mark = text[end];

      // A fraction shaped like a group is one: the 1.980 of 1.980,6
      if (!integer.grouped && integer.integer.size() <= 3 && number.fraction.size() == 3 &&
          strings::contains(group, number.decimal_mark)) {
        number = {.span = number.span, .integer = integer.integer + number.fraction, .grouped = true};
      }
      numbers.push_back(number);
    }
    numbers.push_back(integer);
  }
  return numbers;
}

} // namespace phonemis::processor::num2word
