#include "sanitizer_layer.h"

namespace phonemis::processor {

SanitizerLayer::SanitizerLayer(Filter filter, Mapper mapper)
  : filter_(std::move(filter)), mapper_(std::move(mapper)) {}

std::u32string SanitizerLayer::transform(std::u32string_view input, Alignment* alignment) const {
  AlignedWriter out(alignment, input.size());

  for (size_t i = 0; i < input.size(); ++i) {
    if (filter_(input[i])) {
      out.push(mapper_(input[i]), i, i + 1);
    }
  }

  return out.take();
}

} // namespace phonemis::processor

