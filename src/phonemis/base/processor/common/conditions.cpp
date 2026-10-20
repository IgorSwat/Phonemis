#include "conditions.h"
#include "text.h"
#include <phonemis/utils/unicode.h>

namespace phonemis::processor {

using namespace utils;

std::optional<std::u32string> read(const std::vector<Reading>& readings, const Context& context) {
  for (const auto& reading : readings) {
    if (!reading.when || reading.when(context)) {
      return reading.words;
    }
  }
  return std::nullopt;
}

bool after_name(const Context& context) {
  std::u32string_view head = context.head();
  return head.size() >= 2 && unicode::isalpha(head[head.size() - 1]) && unicode::isalpha(head[head.size() - 2]);
}

bool before_number(const Context& context) {
  std::u32string_view tail = context.tail();
  return is_digit_after(tail, skip_space(tail, 0));
}

bool before_name(const Context& context) {
  std::u32string word = context.after();
  return !word.empty() && unicode::isupper(word[0]);
}

Condition after_any(std::unordered_set<std::u32string> words) {
  return [words = std::move(words)](const Context& context) { return words.contains(context.before()); };
}

} // namespace phonemis::processor
