#pragma once

#include "layer.h"

#include <string>
#include <string_view>

namespace phonemis::processor {

/**
 * Preprocessor layer that collapses multiple consecutive whitespaces into a single space.
 * Also trims leading and trailing whitespaces.
 */
class TrimLayer : public Layer {
public:
  TrimLayer() = default;
  ~TrimLayer() override = default;

  std::u32string transform(std::u32string_view input) const override;
};

} // namespace phonemis::processor
