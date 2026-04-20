#include "trim_layer.h"
#include <phonemis/utils/unicode.h>

#include <algorithm>
#include <cctype>
#include <iterator>

namespace phonemis::preprocessor {

std::u32string TrimLayer::transform(std::u32string_view input) const {
  if (input.empty()) {
    return U"";
  }

  std::u32string result;
  result.reserve(input.size());

  // A hack to omit the leading whitespaces
  bool last_was_space = true;

  for (auto it = input.begin(); it != input.end(); ++it) {
    bool isSpace = utils::unicode::isspace(*it);
    if (isSpace && !last_was_space) {
      result += U' ';
      last_was_space = true;
    } else if (!isSpace) {
      result += *it;
      last_was_space = false;
    }
  }

  // Remove trailing space if it exists
  if (!result.empty() && utils::unicode::isspace(result.back())) {
    result.pop_back();
  }

  return result;
}

} // namespace phonemis::preprocessor
