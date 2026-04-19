#include "test.h"
#include <phonemis/base/preprocessor/num2word/layer.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace preprocessor::num2word;

// A dumb class to simplify the testing procedure.
class TestNum2Word : public Num2WordLayer {
public:
    using Num2WordLayer::Num2WordLayer;

    std::u32string to_cardinal_int(int32_t number) const override { return U"[CARDINAL]"; }
    std::u32string to_cardinal_float(float number, std::u32string_view repr) const override { return U"[CARDINAL]"; }
    std::u32string to_ordinal_int(int32_t number, std::u32string_view suffix = U"") const override {
        if (!suffix.empty() || is_ordinal_suffix(suffix)) return U"[POTENTIALLY_ORDINAL]";
        return U"[ORDINAL]";
    }
    std::u32string to_currency(char32_t currency, std::variant<int32_t, float> number) const override { return U"[CURRENCY]"; }
    std::u32string to_month(uint32_t month) const override { return U"[MONTH]"; }
    std::u32string to_year(uint32_t year) const override { return U"[YEAR]"; }
    bool is_ordinal_suffix(std::u32string_view suffix) const override {
        return suffix == U"st" || suffix == U"nd" || suffix == U"rd" || suffix == U"th";
    }
};

REGISTER_TEST(num2word_base_basic_transform_test)
{
    TestNum2Word layer;

    // Integers and Floats
    ASSERT_EQUALS(U"[CARDINAL]", layer.transform(U"123"));
    ASSERT_EQUALS(U"[CARDINAL]", layer.transform(U"123.45"));

    // Fractions
    ASSERT_EQUALS(U"[CARDINAL] [ORDINAL]", layer.transform(U"1/2"));

    // Currency
    ASSERT_EQUALS(U"[CARDINAL] [CURRENCY]", layer.transform(U"50$"));
    ASSERT_EQUALS(U"The price is [CARDINAL] [CURRENCY].", layer.transform(U"The price is 99€."));

    // Mixed text
    ASSERT_EQUALS(U"Value: [CARDINAL], Ratio: [CARDINAL] [ORDINAL].", layer.transform(U"Value: 42, Ratio: 1/3."));

    return true;
}

REGISTER_TEST(num2word_base_ordinal_dot_test)
{
    Config config;
    config.allow_general_ord_notation = true;
    TestNum2Word layer(config);

    // Potentialy ordinal
    ASSERT_EQUALS(U"[POTENTIALLY_ORDINAL]", layer.transform(U"1st"));
    ASSERT_EQUALS(U"[POTENTIALLY_ORDINAL]", layer.transform(U"2nd"));
    ASSERT_EQUALS(U"[POTENTIALLY_ORDINAL]", layer.transform(U"12th"));

    // Ordinal with dot followed by lowercase
    ASSERT_EQUALS(U"[ORDINAL] word", layer.transform(U"1. word"));
    
    // Not an ordinal (uppercase follows or no following alphabetical character)
    ASSERT_EQUALS(U"[CARDINAL]. Word", layer.transform(U"1. Word"));
    ASSERT_EQUALS(U"[CARDINAL].", layer.transform(U"1."));

    // Disabled by config
    config.allow_general_ord_notation = false;
    TestNum2Word layer_disabled(config);
    ASSERT_EQUALS(U"[CARDINAL]. word", layer_disabled.transform(U"1. word"));

    return true;
}

REGISTER_TEST(num2word_base_date_test)
{
    TestNum2Word layer;

    ASSERT_EQUALS(U"[ORDINAL] [MONTH] [YEAR]", layer.transform(U"2026-03-27"));
    ASSERT_EQUALS(U"[ORDINAL] [MONTH] [YEAR]", layer.transform(U"27.03.2026"));
    ASSERT_EQUALS(U"Today is [ORDINAL] [MONTH] [YEAR].", layer.transform(U"Today is 27-03-2026."));

    return true;
}
REGISTER_TEST(num2word_base_complex_sequence_test)
{
    TestNum2Word layer;
    Config config;
    config.allow_general_ord_notation = true;
    TestNum2Word layer_with_ord(config);

    // Mixed content: ordinals, dates, fractions, and potential ordinals
    std::u32string input = U"On 2026-03-27, the 1st runner finished 1/2 of the race. "
                            "He was 1. in line, while the 2nd followed at 12.5 seconds.";
    
    std::u32string expected = U"On [ORDINAL] [MONTH] [YEAR], the [POTENTIALLY_ORDINAL] runner finished [CARDINAL] [ORDINAL] of the race. "
                               "He was [ORDINAL] in line, while the [POTENTIALLY_ORDINAL] followed at [CARDINAL] seconds.";

    ASSERT_EQUALS(expected, layer_with_ord.transform(input));

    return true;
}

} // namespace phonemis::test
