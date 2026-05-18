#include "test.h"
#include <phonemis/lang/de/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace de;

REGISTER_TEST(num2word_de_basic_transform_test)
{
    Num2Word layer;

    // Integers and Floats
    ASSERT_EQUALS(U"null", layer.transform(U"0"));
    ASSERT_EQUALS(U"einundzwanzig", layer.transform(U"21"));
    ASSERT_EQUALS(U"siebzig", layer.transform(U"70"));
    ASSERT_EQUALS(U"einundsiebzig", layer.transform(U"71"));
    ASSERT_EQUALS(U"achtzig", layer.transform(U"80"));
    ASSERT_EQUALS(U"neunundneunzig", layer.transform(U"99"));
    ASSERT_EQUALS(U"einhundert", layer.transform(U"100"));
    ASSERT_EQUALS(U"einhundertdreiundzwanzig", layer.transform(U"123"));
    ASSERT_EQUALS(U"zweihundert", layer.transform(U"200"));
    ASSERT_EQUALS(U"eintausend", layer.transform(U"1000"));
    ASSERT_EQUALS(U"zweitausend", layer.transform(U"2000"));
    ASSERT_EQUALS(U"eine Million", layer.transform(U"1000000"));
    ASSERT_EQUALS(U"eine Milliarde", layer.transform(U"1000000000"));
    ASSERT_EQUALS(U"einhundertdreiundzwanzig Komma vier fünf", layer.transform(U"123,45"));
    ASSERT_EQUALS(U"null Komma null fünf", layer.transform(U"0,05"));
    ASSERT_EQUALS(U"zwölf Komma fünf", layer.transform(U"12,5"));

    // Fractions
    ASSERT_EQUALS(U"ein Halb", layer.transform(U"1/2"));
    ASSERT_EQUALS(U"ein Drittel", layer.transform(U"1/3"));
    ASSERT_EQUALS(U"ein Viertel", layer.transform(U"1/4"));
    ASSERT_EQUALS(U"drei Viertel", layer.transform(U"3/4"));
    ASSERT_EQUALS(U"drei Zehntel", layer.transform(U"3/10"));
    ASSERT_EQUALS(U"zwei Drittel", layer.transform(U"2/3"));

    // Currency
    ASSERT_EQUALS(U"ein Dollar", layer.transform(U"1$"));
    ASSERT_EQUALS(U"fünfzig Dollar", layer.transform(U"50$"));
    ASSERT_EQUALS(U"ein Euro", layer.transform(U"1,00€"));
    ASSERT_EQUALS(U"zwei Pfund", layer.transform(U"2£"));
    ASSERT_EQUALS(U"drei Euro", layer.transform(U"3€"));
    ASSERT_EQUALS(U"Der Preis ist neunundneunzig Euro.", layer.transform(U"Der Preis ist 99€."));

    return true;
}

REGISTER_TEST(num2word_de_date_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"siebenundzwanzigsten März zweitausendsechsundzwanzig", layer.transform(U"27.03.2026"));
    ASSERT_EQUALS(U"Am siebenundzwanzigsten März zweitausendsechsundzwanzig.", layer.transform(U"Am 27.03.2026."));
    ASSERT_EQUALS(U"Heute ist der siebenundzwanzigste März zweitausendsechsundzwanzig.", layer.transform(U"Heute ist der 27.03.2026."));
    ASSERT_EQUALS(U"Heute ist der siebenundzwanzigste März zweitausendsechsundzwanzig.", layer.transform(U"Heute ist der 2026-03-27."));

    return true;
}

REGISTER_TEST(num2word_de_complex_test)
{
    processor::num2word::Config config;
    config.allow_general_ord_notation = true;
    Num2Word layer(config);

    std::u32string input = U"Am 2026-03-27, beendete der 1. Läufer 1/2 des Rennens. "
                            "Er war der 1. in der Reihe, während der 2. mit 12,5 Sekunden Rückstand folgte.";
    
    std::u32string expected = U"Am siebenundzwanzigsten März zweitausendsechsundzwanzig, beendete der erste Läufer ein Halb des Rennens. "
                               "Er war der erste in der Reihe, während der zweite mit zwölf Komma fünf Sekunden Rückstand folgte.";

    ASSERT_EQUALS(expected, layer.transform(input));

    return true;
}

} // namespace phonemis::test