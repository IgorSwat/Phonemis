#include "preprocessor.h"

namespace phonemis::preprocessor {

void Preprocessor::add_layer(std::unique_ptr<Layer> layer) {
  layers_.push_back(std::move(layer));
}

std::u32string Preprocessor::process(std::u32string_view input) const {
  if (layers_.empty()) {
    return std::u32string(input);
  }

  std::u32string result(input);

  for (const auto& layer : layers_) {
    result = layer->transform(result);
  }

  return result;
}

} // namespace phonemis::preprocessor
