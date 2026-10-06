#include "punctuation.h"
#include "constants.h"

namespace phonemis::zh {

namespace {

// Whitespace as Python's str.strip() sees it.
bool is_space(char32_t c) {
  return (c >= U'\t' && c <= U'\r') || (c >= U'\x1C' && c <= U'\x1F') || c == U' ' ||
         c == U'\x85' || c == U'\xA0' || c == U' ' || (c >= U' ' && c <= U' ') ||
         c == U' ' || c == U' ' || c == U' ' || c == U' ' || c == U'　';
}

} // namespace

std::u32string PunctuationLayer::transform(std::u32string_view input) const {
  std::u32string result;
  result.reserve(input.size());
  for (char32_t c : input) {
    auto it = constants::punctuation::kReplacements.find(c);
    if (it != constants::punctuation::kReplacements.end()) {
      result += it->second;
    } else {
      result += c;
    }
  }

  size_t start = 0;
  size_t end = result.size();
  while (start < end && is_space(result[start])) {
    start++;
  }
  while (end > start && is_space(result[end - 1])) {
    end--;
  }
  return result.substr(start, end - start);
}

} // namespace phonemis::zh
