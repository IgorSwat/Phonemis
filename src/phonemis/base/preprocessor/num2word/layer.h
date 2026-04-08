#pragma once

#include "config.h"
#include "types.h"
#include "../layer.h"

#include <concepts>
#include <cstdint>
#include <variant>

namespace phonemis::preprocessor::num2word {

/**
 * A skeleton for all numeric verbalizations inside the package.
 */
class Num2WordLayer : public Layer {
public:
    Num2WordLayer() = default;
    explicit Num2WordLayer(const Config& config) : config_(config) {}

    // Utilizes template method pattern with abstract convertion (`convert()`) mechanism
    std::u32string transform(std::u32string_view input) const override;

    // Required methods - needs to be implemented by language-specyfic layers.
    virtual std::u32string to_cardinal_int(int32_t number) const = 0;
    virtual std::u32string to_cardinal_float(float number, std::u32string_view repr) const = 0;
    virtual std::u32string to_ordinal_int(int32_t number, std::u32string_view suffix = U"") const = 0;
    virtual std::u32string to_currency(char32_t currency, std::variant<int32_t, float> number) const = 0;
    virtual std::u32string to_month(uint32_t month) const = 0;
    virtual std::u32string to_year(uint32_t year) const = 0;
    virtual bool is_ordinal_suffix(std::u32string_view suffix) const = 0;

protected:
    std::u32string verbalize(const StringifiedNumber& number) const;

    // Conversion wrappers, with extra logic & verbose behavior on errors.
    std::optional<int32_t> as_int(std::u32string_view s) const;
    std::optional<float> as_float(std::u32string_view s) const;

    Config config_;
};

} // namespace phonemis::preprocessor::num2word