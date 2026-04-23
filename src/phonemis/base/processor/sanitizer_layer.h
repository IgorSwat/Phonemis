#pragma once

#include "layer.h"

#include <functional>
#include <string>

namespace phonemis::processor {

/**
 * Sanitizer layer is responsible for filtering out (and/or replacing) unrecognizable
 * or undesirable characters in given language.
 * 
 * It is controlled by a filter function and a mapping function.
 */
class SanitizerLayer : public Layer {
public:
  using Filter = std::function<bool(char32_t)>;
  using Mapper = std::function<char32_t(char32_t)>;

  SanitizerLayer(Filter filter = [](char32_t c) { return true; },
                 Mapper mapper = [](char32_t c) { return c; });

  std::u32string transform(std::u32string_view input) const override;

private:
  Filter filter_;
  Mapper mapper_;
};

} // namespace phonemis::processor