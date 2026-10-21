#include "numerals.h"
#include "primitives.h"
#include <phonemis/utils/unicode.h>

namespace phonemis::processor::num2word {

using namespace utils;
std::optional<uint64_t> value_of(std::u32string_view digits) {
  if (digits.empty() || digits.size() > 18) {
    return std::nullopt;
  }

  uint64_t value = 0;
  for (char32_t c : digits) {
    value = value * 10 + static_cast<uint64_t>(c - U'0');
  }
  return value;
}

bool spoken_digits(const Number& number, size_t longest) {
  const auto& i = number.integer;
  return (i.size() > 1 && i[0] == U'0') || (i.size() >= longest && !number.grouped && !i.ends_with(U"000"));
}

std::vector<std::u32string_view> code_parts(std::u32string_view code) {
  auto is_mark = [](char32_t c) { return c == U'\'' || c == U'.' || c == U'-'; };

  std::vector<std::u32string_view> parts;
  for (size_t i = 0; i < code.size();) {
    size_t end = i + 1;
    if (unicode::isdigit(code[i])) {
      // 264, 2.5
      end = digits_end(code, i);
      if (end + 1 < code.size() && code[end] == U'.' && unicode::isdigit(code[end + 1])) {
        end = digits_end(code, end + 1);
      }
    } else if (code[i] == U'.' && i + 1 < code.size() && unicode::isdigit(code[i + 1])) {
      // .264
      end = digits_end(code, i + 1);
    } else if (!is_mark(code[i])) {
      while (end < code.size() && !unicode::isdigit(code[end]) && !is_mark(code[end])) {
        end++;
      }
    }
    parts.push_back(code.substr(i, end - i));
    i = end;
  }
  return parts;
}

} // namespace phonemis::processor::num2word
