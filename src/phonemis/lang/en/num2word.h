#pragma once

#include <phonemis/base/preprocessor/num2word/layer.h>
#include <string>
#include <vector>
#include <map>

namespace phonemis::en {

class Num2Word : public preprocessor::num2word::Num2WordLayer {
public:
    using Num2WordLayer::Num2WordLayer;

    std::u32string to_cardinal_int(int32_t number) const override;
    std::u32string to_cardinal_float(float number, std::u32string_view repr) const override;
    std::u32string to_ordinal_int(int32_t number, std::u32string_view suffix = U"") const override;
    std::u32string to_currency(char32_t currency, std::variant<int32_t, float> number) const override;
    std::u32string to_month(uint32_t month) const override;
    std::u32string to_year(uint32_t year) const override;
    bool is_ordinal_suffix(std::u32string_view suffix) const override;
};

} // namespace phonemis::en
