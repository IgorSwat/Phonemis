#include "patterns.h"
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/unicode.h>

#include <vector>

namespace phonemis::de {

using namespace utils;
using namespace processor;
using namespace processor::num2word;

std::optional<Match> PhonePattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position, U".,") || text[position] != U'0') {
    return std::nullopt;
  }

  // The area code and the number: written together (0301234567), or apart (030 1234567, 030/1234567)
  size_t end = digits_end(text, position);
  size_t length = end - position;
  if (length < 6) {
    bool apart = length >= 3 && end < text.size() && (text[end] == U' ' || text[end] == U'/' || text[end] == U'-') &&
                 digits_end(text, end + 1) >= end + 4;
    if (!apart) {
      return std::nullopt;
    }
    end = digits_end(text, end + 1);
  }

  // Then any further groups: 0800-123 45 67. Each could end the number, if nothing continues it there.
  std::vector<size_t> ends = {end};
  while (ends.back() + 1 < text.size() && (text[ends.back()] == U' ' || text[ends.back()] == U'-') &&
         unicode::isdigit(text[ends.back() + 1])) {
    ends.push_back(digits_end(text, ends.back() + 1));
  }
  for (auto it = ends.rbegin(); it != ends.rend(); ++it) {
    // Not followed by a digit, even after a word character, a dot or a comma
    bool continued = is_digit_after(text, *it) || (!is_clear_after(text, *it, U".,") && is_digit_after(text, *it + 1));
    if (!continued) {
      Match phone{.kind = Kind::PHONE, .span = {position, *it}};
      for (size_t i = position; i < *it;) {
        size_t digits = digits_end(text, i);
        if (digits > i) {
          phone.numbers.push_back(digits_at(text, i));
          i = digits;
        } else {
          i++;
        }
      }
      return phone;
    }
  }
  return std::nullopt;
}

} // namespace phonemis::de
