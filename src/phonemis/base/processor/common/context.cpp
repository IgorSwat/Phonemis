#include "constants.h"
#include "context.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::processor {

using namespace utils;

Context::Context(std::u32string_view text, Span span) : text_(text), span_(span) {}

std::u32string Context::before(size_t k) const {
  // Walk back over k words of the head
  std::u32string_view head = this->head();
  size_t end = head.size();
  for (; k > 0; --k) {
    while (end > 0 && unicode::isspace(head[end - 1])) {
      end--;
    }
    if (end == 0) {
      return U"";
    }
    size_t begin = end;
    while (begin > 0 && !unicode::isspace(head[begin - 1])) {
      begin--;
    }
    if (k == 1) {
      return strings::to_lower(stripped(head.substr(begin, end - begin)));
    }
    end = begin;
  }

  return U"";
}

std::u32string Context::after(size_t k) const {
  std::u32string_view tail = this->tail();
  if (!tail.empty() && strings::contains(constants::kPunctuation, tail[0])) {
    return U"";
  }

  // Walk forward over k words of the tail
  size_t begin = 0;
  for (; k > 0; --k) {
    while (begin < tail.size() && unicode::isspace(tail[begin])) {
      begin++;
    }
    if (begin == tail.size()) {
      return U"";
    }
    size_t end = begin;
    while (end < tail.size() && !unicode::isspace(tail[end])) {
      end++;
    }
    if (k == 1) {
      return std::u32string(stripped(tail.substr(begin, end - begin)));
    }
    begin = end;
  }
  return U"";
}

std::u32string_view Context::head() const {
  return text_.substr(0, span_.begin);
}

std::u32string_view Context::tail() const {
  return text_.substr(span_.end);
}

std::u32string_view Context::text() const {
  return text_.substr(span_.begin, span_.end - span_.begin);
}

std::vector<std::u32string_view> words_of(std::u32string_view text) {
  std::vector<std::u32string_view> words;

  for (size_t i = 0; i < text.size();) {
    if (unicode::isspace(text[i])) {
      i++;
      continue;
    }
    size_t end = i;
    while (end < text.size() && !unicode::isspace(text[end])) {
      end++;
    }
    words.push_back(text.substr(i, end - i));
    i = end;
  }

  return words;
}

std::u32string_view stripped(std::u32string_view word) {
  size_t begin = word.find_first_not_of(constants::kPunctuation);
  if (begin == std::u32string_view::npos) {
    return U"";
  }

  size_t end = word.find_last_not_of(constants::kPunctuation);

  return word.substr(begin, end - begin + 1);
}

} // namespace phonemis::processor
