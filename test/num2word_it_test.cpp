#include "test.h"
#include <phonemis/lang/it/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace it;

REGISTER_TEST(num2word_it_basic_transform_test)
{
    Num2Word layer;

    // Integers and Floats
    ASSERT_EQUALS(U"zero", layer.transform(U"0"));
    ASSERT_EQUALS(U"ventuno", layer.transform(U"21"));
    ASSERT_EQUALS(U"settanta", layer.transform(U"70"));
    ASSERT_EQUALS(U"settantuno", layer.transform(U"71"));
    ASSERT_EQUALS(U"ottanta", layer.transform(U"80"));
    ASSERT_EQUALS(U"novantanove", layer.transform(U"99"));
    ASSERT_EQUALS(U"cento", layer.transform(U"100"));
    ASSERT_EQUALS(U"centoventitré", layer.transform(U"123"));
    ASSERT_EQUALS(U"duecento", layer.transform(U"200"));
    ASSERT_EQUALS(U"mille", layer.transform(U"1000"));
    ASSERT_EQUALS(U"duemila", layer.transform(U"2000"));
    ASSERT_EQUALS(U"un milione", layer.transform(U"1000000"));
    ASSERT_EQUALS(U"centoventitré virgola quarantacinque", layer.transform(U"123,45"));
    ASSERT_EQUALS(U"zero virgola zero cinque", layer.transform(U"0,05"));

    // Fractions
    ASSERT_EQUALS(U"un mezzo", layer.transform(U"1/2"));
    ASSERT_EQUALS(U"un terzo", layer.transform(U"1/3"));
    ASSERT_EQUALS(U"un quarto", layer.transform(U"1/4"));
    ASSERT_EQUALS(U"tre quarti", layer.transform(U"3/4"));
    ASSERT_EQUALS(U"tre decimi", layer.transform(U"3/10"));

    // Currency
    ASSERT_EQUALS(U"un dollaro", layer.transform(U"1$"));
    ASSERT_EQUALS(U"cinquanta dollari", layer.transform(U"50$"));
    ASSERT_EQUALS(U"un euro", layer.transform(U"1,00€"));
    ASSERT_EQUALS(U"due sterline", layer.transform(U"2£"));
    ASSERT_EQUALS(U"Il prezzo è novantanove euro.", layer.transform(U"Il prezzo è 99€."));

    return true;
}

REGISTER_TEST(num2word_it_ordinal_test)
{
    Num2Word layer;

    // Potentialy ordinal
    ASSERT_EQUALS(U"primo", layer.transform(U"1º"));
    ASSERT_EQUALS(U"prima", layer.transform(U"1ª"));
    ASSERT_EQUALS(U"secondo", layer.transform(U"2º"));
    ASSERT_EQUALS(U"terzo", layer.transform(U"3º"));
    ASSERT_EQUALS(U"decimo", layer.transform(U"10º"));
    ASSERT_EQUALS(U"dodicesimo", layer.transform(U"12º"));
    ASSERT_EQUALS(U"ventunesimo", layer.transform(U"21º"));
    ASSERT_EQUALS(U"centesimo", layer.transform(U"100º"));

    return true;
}

REGISTER_TEST(num2word_it_date_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"ventisette marzo duemilaventisei", layer.transform(U"27.03.2026"));
    ASSERT_EQUALS(U"Oggi è il ventisette marzo duemilaventisei.", layer.transform(U"Oggi è il 27-03-2026."));
    ASSERT_EQUALS(U"Oggi è il ventisette marzo duemilaventisei.", layer.transform(U"Oggi è il 2026-03-27."));

    return true;
}

REGISTER_TEST(num2word_it_complex_test)
{
    processor::num2word::Config config;
    config.allow_general_ord_notation = true;
    Num2Word layer(config);

    std::u32string input = U"Il 2026-03-27, il 1º corridore ha terminato 1/2 della gara. "
                            "Era 1º in fila, mentre il 2º seguiva a 12,5 secondi.";
    
    std::u32string expected = U"Il ventisette marzo duemilaventisei, il primo corridore ha terminato un mezzo della gara. "
                               "Era primo in fila, mentre il secondo seguiva a dodici virgola cinque secondi.";

    ASSERT_EQUALS(expected, layer.transform(input));

    return true;
}

} // namespace phonemis::test
