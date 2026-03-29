#pragma once

#include "config.h"
#include "types.h"
#include "../layer.h"

#include <concepts>

namespace phonemis::preprocessor::num2word {

/**
 * A skeleton for all numeric verbalizations inside the package.
 */
class Num2WordLayer : public Layer {
public:
    Num2WordLayer() = default;
    explicit Num2WordLayer(const Config& config) : config_(config) {}

    // Utilizes template method pattern with abstract convertion (`convert()`) mechanism
    std::string transform(std::string_view input) const override;

    // Needs to be implemented by language-specyfic layers.
    virtual std::string convert(const StringifiedNumber& number) const = 0;
private:
    Config config_;
};

} // namespace phonemis::preprocessor::num2word