#pragma once

#include "../../layer.h"

#include <functional>

namespace phonemis::processor::sanitizer {

/**
 * Sanitizer layer is responsible for filtering out (and/or replacing) unrecognizable
 * or undesirable characters in given language.
 *
 * It is controlled by a filter function (boolean check) and a mapping function (replacement).
 * 
 * IMPORTANT: Should only be applied after Num2Word and Sym2Word layers.
 */
class SanitizerLayer : public Layer {
public:
  using Filter = std::function<bool(char32_t)>;
  using Mapper = std::function<char32_t(char32_t)>;

  SanitizerLayer(Filter filter = [](char32_t c) { return true; }, Mapper mapper = [](char32_t c) { return c; });

  void apply(Document& document) const override;

private:
  Filter filter_;
  Mapper mapper_;
};

} // namespace phonemis::processor::sanitizer
