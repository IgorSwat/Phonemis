#pragma once

#include "alignment.h"

#include <string>
#include <string_view>

namespace phonemis::processor {

class Layer {
public:
    virtual ~Layer() = default;

    /**
     * A text -> text transformation (single preprocessing step).
     * @param input an input text to be transformed.
     * @param alignment if not null, receives the alignment of the result onto the input.
     */
    virtual std::u32string transform(std::u32string_view input, Alignment* alignment = nullptr) const = 0;
};

} // namespace phonemis::processor
