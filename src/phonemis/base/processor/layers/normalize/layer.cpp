#include "layer.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::processor::normalize {

using namespace utils;

namespace {
// Whether a character is a dash or a hyphen
bool is_dash(char32_t c) {
  return c == U'-' || strings::contains(constants::kDashes, c);
}

// The end of a dash starting at a position: a dash character or two hyphens and more (--); the position if none
size_t dash_end(std::u32string_view text, size_t position) {
  if (position < text.size() && strings::contains(constants::kDashes, text[position])) {
    return position + 1;
  }
  size_t end = position;
  while (end < text.size() && text[end] == U'-') {
    end++;
  }
  return end - position >= 2 ? end : position;
}
} // namespace

NormalizeLayer::NormalizeLayer(const Characters& characters) {
  for (const auto& [members, canonical] : characters) {
    for (char32_t c : members) {
      canonical_[c] = canonical;
    }
  }
}

void NormalizeLayer::apply(Document& document) const {
  std::u32string_view text = document.text();

  for (size_t i = 0; i < text.size();) {
    if (auto dash = read_dash(text, i)) {
      document.replace(dash->span, std::move(dash->text));
      i = dash->span.end;
      continue;
    }
    if (auto it = canonical_.find(text[i]); it != canonical_.end()) {
      document.replace({i, i + 1}, it->second);
    }
    i++;
  }
}

std::optional<Document::Edit> NormalizeLayer::read_dash(std::u32string_view text, size_t position) const {
  size_t end = dash_end(text, position);
  if (end == position) {
    return std::nullopt;
  }

  // 1. Inside a word or a number: Szczurek–Żelazko, 1990–2000, a--b
  if (position > 0 && unicode::isword(text[position - 1]) && end < text.size() && unicode::isword(text[end])) {
    return Document::Edit{.span = {position, end}, .text = U"-"};
  }

  // 2. Elsewhere, the whole run of dashes with the spaces around it: "a – b", "a – — b", "a ---- b"
  size_t begin = position;
  while (begin > 0 && unicode::isspace(text[begin - 1])) {
    begin--;
  }
  for (size_t next = end; next < text.size();) {
    while (next < text.size() && unicode::isspace(text[next])) {
      next++;
    }
    if (next == text.size() || !is_dash(text[next])) {
      break;
    }
    while (next < text.size() && is_dash(text[next])) {
      next++;
    }
    end = next;
  }
  while (end < text.size() && unicode::isspace(text[end])) {
    end++;
  }

  // Between numbers it stays a range: 2020 – 2022 -> 2020-2022
  bool range = begin > 0 && unicode::isdigit(text[begin - 1]) && end < text.size() && unicode::isdigit(text[end]);
  return Document::Edit{.span = {begin, end}, .text = range ? U"-" : U" - "};
}

} // namespace phonemis::processor::normalize
