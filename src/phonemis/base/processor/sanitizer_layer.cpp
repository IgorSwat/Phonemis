#include "sanitizer_layer.h"

namespace phonemis::processor {

SanitizerLayer::SanitizerLayer(Filter filter, Mapper mapper)
  : filter_(std::move(filter)), mapper_(std::move(mapper)) {}

std::u32string SanitizerLayer::transform(std::u32string_view input) const {
  std::u32string out;
  out.reserve(input.size());

  for (char32_t c : input) {
    if (filter_(c)) {
      out.push_back(mapper_(c));
    }
  }

  return out;
}

} // namespace phonemis::processor

