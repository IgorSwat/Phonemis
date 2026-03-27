#pragma once

#include <string>
#include <string_view>

namespace phonemis::preprocessor {

class Layer {
public:
    virtual ~Layer() = default;

    // A text -> text transformation (single preprocessing step).
    virtual std::string transform(std::string_view input) const = 0;
};
  
} // namespace phonemis::preprocessor