#pragma once

#include <phonemis/base/processor/num2word/layer.h>
#include <string>
#include <variant>

namespace phonemis::es {

class Num2Word : public processor::num2word::Num2WordLayer {
public:
    using Num2WordLayer::Num2WordLayer;

    char32_t decimal_separator() const override { return U','; }
    std::u32string to_cardinal_int(int32_t number) const override;
    std::u32string to_cardinal_float(float number, std::u32string_view repr) const override;
    std::u32string to_ordinal_int(int32_t number, std::u32string_view suffix = U"") const override;
    std::u32string to_currency(char32_t currency, std::variant<int32_t, float> number) const override;
    std::u32string to_fraction(int32_t numerator, int32_t denominator) const override;
    std::u32string to_day(uint32_t day) const override;
    std::u32string to_month(uint32_t month) const override;
    std::u32string to_year(uint32_t year) const override;
    bool is_ordinal_suffix(std::u32string_view suffix) const override;

protected:
    // Overridden so we can insert the Spanish "de" between date parts and
    // apply apocope ("uno" → "un") to currency amounts preceding the noun.
    std::u32string verbalize(const processor::num2word::StringifiedNumber& number) const override;
};

} // namespace phonemis::es
