#pragma once

#include "layer.h"

#include <memory>
#include <vector>

namespace phonemis::preprocessor {

// Can be customized in derived classes by adding different preprocessing layers.
class Preprocessor {
public:
  Preprocessor() = default;

  std::string process(std::string_view input) const;

protected:
  // Preprocessing layers - each layer performs some sort of
  // text -> text transformation on it's input.
  std::vector<std::unique_ptr<Layer>> layers_;
};

} // namespace phonemis::preprocessor