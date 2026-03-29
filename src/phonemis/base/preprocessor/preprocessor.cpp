#include "preprocessor.h"

namespace phonemis::preprocessor {

std::string Preprocessor::process(std::string_view input) const {
  std::string result(input);

  for (const auto& layer : layers_) {
    result = layer->transform(result);
  }

  return result;
}

} // namespace phonemis::preprocessor
