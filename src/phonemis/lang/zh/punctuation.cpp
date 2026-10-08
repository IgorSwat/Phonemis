#include "punctuation.h"
#include "constants.h"

namespace phonemis::zh {

namespace {

// Whitespace as Python's str.strip() sees it.
bool is_space(char32_t c) {
  return (c >= U'\t' && c <= U'\r') || (c >= U'\x1C' && c <= U'\x1F') || c == U' ' ||
         c == U'\x85' || c == U'\xA0' || c == U'\u1680' || (c >= U'\u2000' && c <= U'\u200A') ||
         c == U'\u2028' || c == U'\u2029' || c == U'\u202F' || c == U'\u205F' || c == U'\u3000';
}

} // namespace

std::u32string PunctuationLayer::transform(std::u32string_view input) const {
  return transform(input, nullptr);
}

std::u32string PunctuationLayer::transform(std::u32string_view input,
                                           SourceSpans* sources) const {
  std::u32string result;
  SourceSpans result_sources;
  result.reserve(input.size());
  for (size_t i = 0; i < input.size(); i++) {
    auto it = constants::punctuation::kReplacements.find(input[i]);
    if (it != constants::punctuation::kReplacements.end()) {
      result += it->second;
    } else {
      result += input[i];
    }
    if (sources) {
      result_sources.resize(result.size(), (*sources)[i]);
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
  if (sources) {
    *sources = SourceSpans(result_sources.begin() + start, result_sources.begin() + end);
  }
  return result.substr(start, end - start);
}

} // namespace phonemis::zh
