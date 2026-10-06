#pragma once

#include "types.h"

#include <phonemis/base/processor/layer.h>

#include <string>
#include <string_view>

namespace phonemis::zh {

/**
 * Maps full-width punctuation onto the forms Kokoro reads ("，" -> ", ") and trims
 * surrounding whitespace, including the ideographic space.
 */
class PunctuationLayer : public processor::Layer {
public:
  std::u32string transform(std::u32string_view input) const override;

  // The same, also tracking where each character of the result came from.
  std::u32string transform(std::u32string_view input, SourceSpans* sources) const;
};

} // namespace phonemis::zh
