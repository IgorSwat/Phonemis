#include "common/document.h"
#include "processor.h"

namespace phonemis::processor {

void Processor::add_layer(std::unique_ptr<Layer> layer) {
  layers_.push_back(std::move(layer));
}

void Processor::process(Document& document) const {
  for (const auto& layer : layers_) {
    layer->apply(document);
    document.commit();
  }
}

std::u32string Processor::process(std::u32string_view input) const {
  Document document(input);
  process(document);
  return std::u32string(document.text());
}

} // namespace phonemis::processor
