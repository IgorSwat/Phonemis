#include "layer.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::processor::trim {

namespace {
// Whether a position starts punctuation which takes no space before it: "a ," -> "a,", but "a ..." stays
bool is_closing_punctuation(std::u32string_view text, size_t position) {
  if (position >= text.size()) {
    return false;
  }
  bool ellipsis = text.substr(position).starts_with(U"...");
  return utils::strings::contains(std::u32string_view(U",!?;:"), text[position]) ||
         (text[position] == U'.' && !ellipsis);
}
} // namespace

void TrimLayer::apply(Document& document) const {
  std::u32string_view text = document.text();

  for (size_t i = 0; i < text.size();) {
    if (!utils::unicode::isspace(text[i])) {
      i++;
      continue;
    }

    // Find the whole run of whitespaces
    size_t end = i;
    bool newline = false;
    while (end < text.size() && utils::unicode::isspace(text[end])) {
      newline |= text[end] == U'\n';
      end++;
    }

    // Edges and spaces before punctuation lose the run entirely, the middle keeps a single character
    bool closing = !newline && is_closing_punctuation(text, end);
    std::u32string replacement = i == 0 || end == text.size() || closing ? U"" : newline ? U"\n" : U" ";
    if (text.substr(i, end - i) != replacement) {
      document.replace({i, end}, replacement);
    }

    i = end;
  }
}

} // namespace phonemis::processor::trim
