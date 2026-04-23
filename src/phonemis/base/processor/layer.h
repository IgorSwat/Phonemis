#pragma once

#include <string>
#include <string_view>

namespace phonemis::processor {

class Layer {
public:
    virtual ~Layer() = default;

    // A text -> text transformation (single preprocessing step).
    virtual std::u32string transform(std::u32string_view input) const = 0;
};
  
} // namespace phonemis::processor