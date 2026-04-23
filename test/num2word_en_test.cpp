#include "test.h"
#include <phonemis/lang/en/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace en;

REGISTER_TEST(num2word_en_basic_transform_test)
{
    Num2Word layer;

    // Integers and Floats
    ASSERT_EQUALS(U"one hundred and twenty three", layer.transform(U"123"));
    ASSERT_EQUALS(U"one hundred and twenty three point four five", layer.transform(U"123.45"));

    // Fractions
    ASSERT_EQUALS(U"one second", layer.transform(U"1/2"));
    ASSERT_EQUALS(U"three tenth", layer.transform(U"3/10"));  // TODO: change after fixing the num2word

    // Currency
    ASSERT_EQUALS(U"one dollar", layer.transform(U"1$"));
    ASSERT_EQUALS(U"fifty dollars", layer.transform(U"50$"));
    ASSERT_EQUALS(U"one euro", layer.transform(U"1.00€"));
    ASSERT_EQUALS(U"The price is ninety nine euros.", layer.transform(U"The price is 99€."));

    return true;
}

REGISTER_TEST(num2word_en_ordinal_test)
{
    Num2Word layer;

    // Potentialy ordinal
    ASSERT_EQUALS(U"first", layer.transform(U"1st"));
    ASSERT_EQUALS(U"second", layer.transform(U"2nd"));
    ASSERT_EQUALS(U"third", layer.transform(U"3rd"));
    ASSERT_EQUALS(U"twelfth", layer.transform(U"12th"));
    ASSERT_EQUALS(U"twenty first", layer.transform(U"21st"));

    return true;
}

REGISTER_TEST(num2word_en_date_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"twenty seventh March twenty twenty six", layer.transform(U"27.03.2026"));
    ASSERT_EQUALS(U"Today is twenty seventh March twenty twenty six.", layer.transform(U"Today is 27-03-2026."));
    ASSERT_EQUALS(U"Today is twenty seventh March twenty twenty six.", layer.transform(U"Today is 2026-03-27."));

    return true;
}

REGISTER_TEST(num2word_en_complex_test)
{
    processor::num2word::Config config;
    config.allow_general_ord_notation = true;
    Num2Word layer(config);

    std::u32string input = U"On 2026-03-27, the 1st runner finished 1/2 of the race. "
                            "He was 1. in line, while the 2nd followed at 12.5 seconds.";
    
    std::u32string expected = U"On twenty seventh March twenty twenty six, the first runner finished one second of the race. "
                               "He was first in line, while the second followed at twelve point five seconds.";

    ASSERT_EQUALS(expected, layer.transform(input));

    return true;
}

} // namespace phonemis::test
