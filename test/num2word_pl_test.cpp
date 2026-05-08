#include "test.h"
#include <phonemis/lang/pl/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace pl;

REGISTER_TEST(num2word_pl_basic_transform_test)
{
    Num2Word layer;

    // Integers and Floats
    ASSERT_EQUALS(U"zero", layer.transform(U"0"));
    ASSERT_EQUALS(U"dwadzieścia jeden", layer.transform(U"21"));
    ASSERT_EQUALS(U"siedemdziesiąt", layer.transform(U"70"));
    ASSERT_EQUALS(U"siedemdziesiąt jeden", layer.transform(U"71"));
    ASSERT_EQUALS(U"osiemdziesiąt", layer.transform(U"80"));
    ASSERT_EQUALS(U"dziewięćdziesiąt dziewięć", layer.transform(U"99"));
    ASSERT_EQUALS(U"sto", layer.transform(U"100"));
    ASSERT_EQUALS(U"sto dwadzieścia trzy", layer.transform(U"123"));
    ASSERT_EQUALS(U"dwieście", layer.transform(U"200"));
    ASSERT_EQUALS(U"tysiąc", layer.transform(U"1000"));
    ASSERT_EQUALS(U"dwa tysiące", layer.transform(U"2000"));
    ASSERT_EQUALS(U"pięć tysięcy", layer.transform(U"5000"));
    ASSERT_EQUALS(U"milion", layer.transform(U"1000000"));
    ASSERT_EQUALS(U"sto dwadzieścia trzy przecinek czterdzieści pięć", layer.transform(U"123,45"));
    ASSERT_EQUALS(U"zero przecinek zero pięć", layer.transform(U"0,05"));

    // Fractions
    ASSERT_EQUALS(U"jedna druga", layer.transform(U"1/2"));
    ASSERT_EQUALS(U"jedna trzecia", layer.transform(U"1/3"));
    ASSERT_EQUALS(U"jedna czwarta", layer.transform(U"1/4"));
    ASSERT_EQUALS(U"trzy czwarte", layer.transform(U"3/4"));
    ASSERT_EQUALS(U"trzy dziesiąte", layer.transform(U"3/10"));

    // Currency
    ASSERT_EQUALS(U"jeden dolar", layer.transform(U"1$"));
    ASSERT_EQUALS(U"pięćdziesiąt dolarów", layer.transform(U"50$"));
    ASSERT_EQUALS(U"dwa funty", layer.transform(U"2£"));
    ASSERT_EQUALS(U"Cena to dziewięćdziesiąt dziewięć euro.", layer.transform(U"Cena to 99€."));

    return true;
}

REGISTER_TEST(num2word_pl_ordinal_test)
{
    Num2Word layer;

    // Potentially ordinal (digit + alphabetic Polish suffix)
    ASSERT_EQUALS(U"pierwszy", layer.transform(U"1szy"));
    ASSERT_EQUALS(U"pierwsza", layer.transform(U"1sza"));
    ASSERT_EQUALS(U"pierwsze", layer.transform(U"1sze"));
    ASSERT_EQUALS(U"drugi", layer.transform(U"2gi"));
    ASSERT_EQUALS(U"trzeci", layer.transform(U"3ci"));
    ASSERT_EQUALS(U"dziesiąty", layer.transform(U"10ty"));
    ASSERT_EQUALS(U"dziesiąta", layer.transform(U"10ta"));
    ASSERT_EQUALS(U"dwunasty", layer.transform(U"12ty"));
    ASSERT_EQUALS(U"dwunaste", layer.transform(U"12te"));
    ASSERT_EQUALS(U"dwudziesty pierwszy", layer.transform(U"21szy"));
    ASSERT_EQUALS(U"setny", layer.transform(U"100ny"));

    return true;
}

REGISTER_TEST(num2word_pl_date_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"dwudziestego siódmego marca dwa tysiące dwadzieścia sześć",
                  layer.transform(U"27.03.2026"));
    ASSERT_EQUALS(U"Dzisiaj jest dwudziestego siódmego marca dwa tysiące dwadzieścia sześć.",
                  layer.transform(U"Dzisiaj jest 27-03-2026."));
    ASSERT_EQUALS(U"Dzisiaj jest dwudziestego siódmego marca dwa tysiące dwadzieścia sześć.",
                  layer.transform(U"Dzisiaj jest 2026-03-27."));

    return true;
}

REGISTER_TEST(num2word_pl_complex_test)
{
    processor::num2word::Config config;
    config.allow_general_ord_notation = true;
    Num2Word layer(config);

    std::u32string input = U"Dnia 2026-03-27 1szy zawodnik ukończył 1/2 trasy. "
                            "Był 1. w kolejce, a 2. szedł 12,5 sekundy później.";

    std::u32string expected = U"Dnia dwudziestego siódmego marca dwa tysiące dwadzieścia sześć pierwszy zawodnik ukończył jedna druga trasy. "
                               "Był pierwszy w kolejce, a drugi szedł dwanaście przecinek pięć sekundy później.";

    ASSERT_EQUALS(expected, layer.transform(input));

    return true;
}

} // namespace phonemis::test
