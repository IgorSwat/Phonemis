#include "text.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

#include <algorithm>

namespace phonemis::processor {

using namespace utils;

bool is_clear_before(std::u32string_view text, size_t position, std::u32string_view also) {
  return position == 0 || (!unicode::isword(text[position - 1]) && !strings::contains(also, text[position - 1]));
}

bool is_clear_after(std::u32string_view text, size_t position, std::u32string_view also) {
  return position >= text.size() || (!unicode::isword(text[position]) && !strings::contains(also, text[position]));
}

bool is_digit_after(std::u32string_view text, size_t position, std::u32string_view marks) {
  if (position < text.size() && strings::contains(marks, text[position])) {
    position++;
  }

  return position < text.size() && unicode::isdigit(text[position]);
}

bool is_sentence_end(std::u32string_view text, size_t position, bool capital) {
  while (position < text.size() && unicode::isspace(text[position])) {
    position++;
  }

  return position == text.size() || (capital && unicode::isupper(text[position]));
}

size_t digits_end(std::u32string_view text, size_t position) {
  while (position < text.size() && unicode::isdigit(text[position])) {
    position++;
  }

  return position;
}

size_t skip_space(std::u32string_view text, size_t position) {
  return position < text.size() && unicode::isspace(text[position]) ? position + 1 : position;
}

std::vector<std::u32string_view> keys_at(std::u32string_view text, size_t position, const Keys& keys) {
  std::vector<std::u32string_view> found;
  for (const auto& key : keys) {
    if (text.substr(position).starts_with(key)) {
      found.push_back(key);
    }
  }

  std::ranges::sort(found, [](auto a, auto b) { return a.size() > b.size(); });

  return found;
}

std::u32string join(std::span<const std::u32string> words, std::u32string_view separator) {
  std::u32string result;
  for (const auto& word : words) {
    if (word.empty()) {
      continue;
    }
    if (!result.empty()) {
      result += separator;
    }
    result += word;
  }

  return result;
}

} // namespace phonemis::processor
