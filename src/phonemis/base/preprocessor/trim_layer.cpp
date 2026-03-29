#include "trim_layer.h"

#include <algorithm>
#include <cctype>
#include <iterator>

namespace phonemis::preprocessor {

std::string TrimLayer::transform(std::string_view input) const {
  if (input.empty()) {
    return "";
  }

  std::string result;
  result.reserve(input.size());

  // A hack to omit the leading whitespaces
  bool last_was_space = true;

  for (auto it = input.begin(); it != input.end(); ++it) {
    bool isSpace = std::isspace(static_cast<unsigned char>(*it));
    if (isSpace && !last_was_space) {
      result += ' ';
      last_was_space = true;
    } else if (!isSpace) {
      result += *it;
      last_was_space = false;
    }
  }

  // Remove trailing space if it exists
  if (!result.empty() && std::isspace(static_cast<unsigned char>(result.back()))) {
    result.pop_back();
  }

  return result;
}

} // namespace phonemis::preprocessor
