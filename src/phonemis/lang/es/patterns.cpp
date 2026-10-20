#include "constants.h"
#include "patterns.h"
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

#include <optional>
#include <string>

namespace phonemis::es {

using namespace constants::num2word;
using namespace processor;
using namespace processor::num2word;
using namespace utils;

std::optional<Match> LawPattern::match(std::u32string_view text, size_t position) const {
  // A law word, then spaces
  size_t word_end = position;
  while (word_end > 0 && unicode::isspace(text[word_end - 1])) {
    word_end--;
  }
  size_t word_begin = word_end;
  while (word_begin > 0 && unicode::isword(text[word_begin - 1])) {
    word_begin--;
  }
  if (word_end == position || !kLaws.contains(strings::to_lower(text.substr(word_begin, word_end - word_begin)))) {
    return std::nullopt;
  }

  // Its number, then a year of 19xx or 20xx
  Number number = digits_at(text, position);
  size_t year = number.span.end + 1;
  if (number.span.end == position || number.integer.size() > 3 || year >= text.size() ||
      !strings::contains(std::u32string_view(U"./-"), text[year - 1])) {
    return std::nullopt;
  }
  Number year_number = digits_at(text, year);
  if (year_number.integer.size() != 4 ||
      !(year_number.integer.starts_with(U"19") || year_number.integer.starts_with(U"20"))) {
    return std::nullopt;
  }
  size_t end = year_number.span.end;
  if (!is_clear_after(text, end) || is_digit_after(text, end, U".,/")) {
    return std::nullopt;
  }
  return Match{.kind = Kind::DATE, .span = {position, end}, .numbers = {number, year_number}};
}

} // namespace phonemis::es
