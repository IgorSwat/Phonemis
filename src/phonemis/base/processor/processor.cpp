#include "processor.h"

namespace phonemis::processor {

void Processor::add_layer(std::unique_ptr<Layer> layer) {
  layers_.push_back(std::move(layer));
}

std::u32string Processor::process(std::u32string_view input, Alignment* alignment) const {
  std::u32string result(input);

  if (alignment) {
    *alignment = identity(input.size());
  }

  for (const auto& layer : layers_) {
    if (alignment) {
      Alignment layer_alignment;
      result = layer->transform(result, &layer_alignment);
      *alignment = compose(layer_alignment, *alignment);
    } else {
      result = layer->transform(result);
    }
  }

  return result;
}

} // namespace phonemis::processor
