#include "preprocessor.h"

namespace phonemis::preprocessor {

std::u32string Preprocessor::process(std::u32string_view input) const {
  std::u32string result(input);

  for (const auto& layer : layers_) {
    result = layer->transform(result);
  }

  return result;
}

} // namespace phonemis::preprocessor
