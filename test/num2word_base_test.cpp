#include "test.h"
#include <phonemis/base/preprocessor/num2word/layer.h>

namespace phonemis::test {

using namespace preprocessor::num2word;

// A dumb class to simplify the testing procedure.
class TestNum2Word : public Num2WordLayer {
public:
    using Num2WordLayer::Num2WordLayer;

    std::string convert(const StringifiedNumber& number) const override {
        switch (number.conversionMode) {
            case Mode::CARDINAL: return "[CARDINAL]";
            case Mode::POTENTIALY_ORDINAL: return "[POTENTIALY_ORDINAL]";
            case Mode::ORDINAL: return "[ORDINAL]";
            case Mode::FRACTION: return "[FRACTION]";
            case Mode::MONTH: return "[MONTH]";
            case Mode::YEAR: return "[YEAR]";
            case Mode::DATE: return "[DATE]";
            default: return "[UNKNOWN]";
        }
    }
};

REGISTER_TEST(num2word_base_basic_transform_test)
{
    TestNum2Word layer;

    // Integers and Floats
    ASSERT_EQUALS("[CARDINAL]", layer.transform("123"));
    ASSERT_EQUALS("[CARDINAL]", layer.transform("123.45"));
    
    // Fractions
    ASSERT_EQUALS("[FRACTION]", layer.transform("1/2"));

    // Mixed text
    ASSERT_EQUALS("Value: [CARDINAL], Ratio: [FRACTION].", layer.transform("Value: 42, Ratio: 1/3."));

    return true;
}

REGISTER_TEST(num2word_base_ordinal_dot_test)
{
    Config config;
    config.allowGeneralOrdNotation = true;
    TestNum2Word layer(config);

    // Potentialy ordinal
    ASSERT_EQUALS("[POTENTIALY_ORDINAL]", layer.transform("1st"));
    ASSERT_EQUALS("[POTENTIALY_ORDINAL]", layer.transform("2nd"));
    ASSERT_EQUALS("[POTENTIALY_ORDINAL]", layer.transform("12th"));

    // Ordinal with dot followed by lowercase
    ASSERT_EQUALS("[ORDINAL] word", layer.transform("1. word"));
    
    // Not an ordinal (uppercase follows or no following alphabetical character)
    ASSERT_EQUALS("[CARDINAL]. Word", layer.transform("1. Word"));
    ASSERT_EQUALS("[CARDINAL].", layer.transform("1."));

    // Disabled by config
    config.allowGeneralOrdNotation = false;
    TestNum2Word layer_disabled(config);
    ASSERT_EQUALS("[CARDINAL]. word", layer_disabled.transform("1. word"));

    return true;
}

REGISTER_TEST(num2word_base_date_test)
{
    TestNum2Word layer;

    ASSERT_EQUALS("[DATE]", layer.transform("2026-03-27"));
    ASSERT_EQUALS("[DATE]", layer.transform("27.03.2026"));
    ASSERT_EQUALS("Today is [DATE].", layer.transform("Today is 27-03-2026."));

    return true;
}
REGISTER_TEST(num2word_base_complex_sequence_test)
{
    TestNum2Word layer;
    Config config;
    config.allowGeneralOrdNotation = true;
    TestNum2Word layer_with_ord(config);

    // Mixed content: ordinals, dates, fractions, and potential ordinals
    std::string input = "On 2026-03-27, the 1st runner finished 1/2 of the race. "
                        "He was 1. in line, while the 2nd followed at 12.5 seconds.";
    
    std::string expected = "On [DATE], the [POTENTIALY_ORDINAL] runner finished [FRACTION] of the race. "
                           "He was [ORDINAL] in line, while the [POTENTIALY_ORDINAL] followed at [CARDINAL] seconds.";

    ASSERT_EQUALS(expected, layer_with_ord.transform(input));

    return true;
}

} // namespace phonemis::test
