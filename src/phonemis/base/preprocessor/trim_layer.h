#pragma once

#include "layer.h"

#include <string>
#include <string_view>

namespace phonemis::preprocessor {

/**
 * Preprocessor layer that collapses multiple consecutive whitespaces into a single space.
 * Also trims leading and trailing whitespaces.
 */
class TrimLayer : public Layer {
public:
  TrimLayer() = default;
  ~TrimLayer() override = default;

  std::string transform(std::string_view input) const override;
};

} // namespace phonemis::preprocessor
