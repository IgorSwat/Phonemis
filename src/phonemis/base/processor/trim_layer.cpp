#include "trim_layer.h"
#include <phonemis/utils/unicode.h>

#include <algorithm>
#include <cctype>
#include <iterator>

namespace phonemis::processor {

std::u32string TrimLayer::transform(std::u32string_view input, Alignment* alignment) const {
  AlignedWriter out(alignment, input.size());

  // A hack to omit the leading whitespaces
  bool last_was_space = true;

  for (size_t i = 0; i < input.size(); ++i) {
    bool isSpace = utils::unicode::isspace(input[i]);
    if (isSpace && !last_was_space) {
      out.push(U' ', i, i + 1);
      last_was_space = true;
    } else if (!isSpace) {
      out.push(input[i], i, i + 1);
      last_was_space = false;
    }
  }

  // Remove trailing space if it exists
  if (!out.text().empty() && utils::unicode::isspace(out.text().back())) {
    out.pop();
  }

  return out.take();
}

} // namespace phonemis::processor
