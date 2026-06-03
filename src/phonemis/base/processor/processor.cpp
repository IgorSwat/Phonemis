#include "processor.h"

namespace phonemis::processor {

void Processor::add_layer(std::unique_ptr<Layer> layer) {
  layers_.push_back(std::move(layer));
}

std::u32string Processor::process(std::u32string_view input) const {
  std::u32string result(input);

  for (const auto& layer : layers_) {
    result = layer->transform(result);
  }

  return result;
}

} // namespace phonemis::processor
