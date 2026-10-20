#include "layer.h"

namespace phonemis::processor::sanitizer {

SanitizerLayer::SanitizerLayer(Filter filter, Mapper mapper) : filter_(std::move(filter)), mapper_(std::move(mapper)) {}

void SanitizerLayer::apply(Document& document) const {
  std::u32string_view text = document.text();

  for (size_t i = 0; i < text.size(); ++i) {
    if (!filter_(text[i])) {
      document.replace({i, i + 1}, U"");
    } else if (char32_t c = mapper_(text[i]); c != text[i]) {
      document.replace({i, i + 1}, std::u32string(1, c));
    }
  }
}

} // namespace phonemis::processor::sanitizer
