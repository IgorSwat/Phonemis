#include "common/document.h"
#include "processor.h"

namespace phonemis::processor {

void Processor::add_layer(std::unique_ptr<Layer> layer) {
  layers_.push_back(std::move(layer));
}

std::u32string Processor::process(std::u32string_view input) const {
  // We start by initializing a document - an object to work for all the layers.
  Document document(input);

  for (const auto& layer : layers_) {
    layer->apply(document);
    document.commit();
  }

  return std::u32string(document.text());
}

} // namespace phonemis::processor
