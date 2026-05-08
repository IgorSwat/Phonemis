#include "test.h"
#include <phonemis/lang/hi/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace hi;

REGISTER_TEST(num2word_hi_basic_transform_test)
{
    Num2Word layer;

    // Integers and Floats
    ASSERT_EQUALS(U"शून्य", layer.transform(U"0"));
    ASSERT_EQUALS(U"इक्कीस", layer.transform(U"21"));
    ASSERT_EQUALS(U"सत्तर", layer.transform(U"70"));
    ASSERT_EQUALS(U"इकहत्तर", layer.transform(U"71"));
    ASSERT_EQUALS(U"अस्सी", layer.transform(U"80"));
    ASSERT_EQUALS(U"निन्यानवे", layer.transform(U"99"));
    ASSERT_EQUALS(U"एक सौ", layer.transform(U"100"));
    ASSERT_EQUALS(U"एक सौ तेईस", layer.transform(U"123"));
    ASSERT_EQUALS(U"दो सौ", layer.transform(U"200"));
    ASSERT_EQUALS(U"एक हज़ार", layer.transform(U"1000"));
    ASSERT_EQUALS(U"दो हज़ार", layer.transform(U"2000"));
    ASSERT_EQUALS(U"पाँच हज़ार", layer.transform(U"5000"));
    ASSERT_EQUALS(U"एक लाख", layer.transform(U"100000"));
    ASSERT_EQUALS(U"दस लाख", layer.transform(U"1000000"));
    ASSERT_EQUALS(U"एक करोड़", layer.transform(U"10000000"));
    ASSERT_EQUALS(U"एक सौ तेईस दशमलव चार पाँच", layer.transform(U"123.45"));
    ASSERT_EQUALS(U"शून्य दशमलव शून्य पाँच", layer.transform(U"0.05"));

    // Fractions
    ASSERT_EQUALS(U"आधा", layer.transform(U"1/2"));
    ASSERT_EQUALS(U"एक तिहाई", layer.transform(U"1/3"));
    ASSERT_EQUALS(U"एक चौथाई", layer.transform(U"1/4"));
    ASSERT_EQUALS(U"तीन चौथाई", layer.transform(U"3/4"));
    ASSERT_EQUALS(U"तीन बटा दस", layer.transform(U"3/10"));

    // Currency
    ASSERT_EQUALS(U"एक डॉलर", layer.transform(U"1$"));
    ASSERT_EQUALS(U"पचास डॉलर", layer.transform(U"50$"));
    ASSERT_EQUALS(U"दो पाउंड", layer.transform(U"2£"));
    ASSERT_EQUALS(U"क़ीमत निन्यानवे यूरो है।", layer.transform(U"क़ीमत 99€ है।"));

    return true;
}

REGISTER_TEST(num2word_hi_ordinal_test)
{
    Num2Word layer;

    // Hindi ordinals are tested through the direct API: digit-attached
    // ordinal notation in Devanagari (e.g. "5वाँ") would lose its trailing
    // matras during the base layer's alphabetic scan, so the gender/number
    // form is instead selected here by passing the full ending explicitly.
    ASSERT_EQUALS(U"पहला",      layer.to_ordinal_int(1));
    ASSERT_EQUALS(U"पहली",      layer.to_ordinal_int(1, U"वीं"));
    ASSERT_EQUALS(U"पहले",      layer.to_ordinal_int(1, U"वें"));
    ASSERT_EQUALS(U"दूसरा",     layer.to_ordinal_int(2));
    ASSERT_EQUALS(U"तीसरा",     layer.to_ordinal_int(3));
    ASSERT_EQUALS(U"चौथा",      layer.to_ordinal_int(4));
    ASSERT_EQUALS(U"पाँचवाँ",    layer.to_ordinal_int(5));
    ASSERT_EQUALS(U"पाँचवीं",    layer.to_ordinal_int(5, U"वीं"));
    ASSERT_EQUALS(U"छठा",       layer.to_ordinal_int(6));
    ASSERT_EQUALS(U"दसवाँ",     layer.to_ordinal_int(10));
    ASSERT_EQUALS(U"बारहवाँ",   layer.to_ordinal_int(12));
    ASSERT_EQUALS(U"इक्कीसवाँ",  layer.to_ordinal_int(21));
    ASSERT_EQUALS(U"एक सौवाँ",  layer.to_ordinal_int(100));

    return true;
}

REGISTER_TEST(num2word_hi_date_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"सत्ताईस मार्च दो हज़ार छब्बीस",
                  layer.transform(U"27.03.2026"));
    ASSERT_EQUALS(U"आज सत्ताईस मार्च दो हज़ार छब्बीस है।",
                  layer.transform(U"आज 27-03-2026 है।"));
    ASSERT_EQUALS(U"आज सत्ताईस मार्च दो हज़ार छब्बीस है।",
                  layer.transform(U"आज 2026-03-27 है।"));

    return true;
}

} // namespace phonemis::test