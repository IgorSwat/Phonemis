#include "test.h"
#include <phonemis/lang/fr/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace fr;

REGISTER_TEST(num2word_fr_basic_transform_test)
{
    Num2Word layer;

    // Integers and Floats
    ASSERT_EQUALS(U"zéro", layer.transform(U"0"));
    ASSERT_EQUALS(U"vingt et un", layer.transform(U"21"));
    ASSERT_EQUALS(U"soixante dix", layer.transform(U"70"));
    ASSERT_EQUALS(U"soixante et onze", layer.transform(U"71"));
    ASSERT_EQUALS(U"quatre vingts", layer.transform(U"80"));
    ASSERT_EQUALS(U"quatre vingt dix neuf", layer.transform(U"99"));
    ASSERT_EQUALS(U"cent", layer.transform(U"100"));
    ASSERT_EQUALS(U"cent vingt trois", layer.transform(U"123"));
    ASSERT_EQUALS(U"deux cents", layer.transform(U"200"));
    ASSERT_EQUALS(U"mille", layer.transform(U"1000"));
    ASSERT_EQUALS(U"deux mille", layer.transform(U"2000"));
    ASSERT_EQUALS(U"un million", layer.transform(U"1000000"));
    ASSERT_EQUALS(U"cent vingt trois virgule quarante cinq", layer.transform(U"123,45"));
    ASSERT_EQUALS(U"zéro virgule zéro cinq", layer.transform(U"0,05"));

    // Fractions
    ASSERT_EQUALS(U"un demi", layer.transform(U"1/2"));
    ASSERT_EQUALS(U"un tiers", layer.transform(U"1/3"));
    ASSERT_EQUALS(U"un quart", layer.transform(U"1/4"));
    ASSERT_EQUALS(U"trois quarts", layer.transform(U"3/4"));
    ASSERT_EQUALS(U"trois dixièmes", layer.transform(U"3/10"));

    // Currency
    ASSERT_EQUALS(U"un dollar", layer.transform(U"1$"));
    ASSERT_EQUALS(U"cinquante dollars", layer.transform(U"50$"));
    ASSERT_EQUALS(U"un euro", layer.transform(U"1,00€"));
    ASSERT_EQUALS(U"deux livres", layer.transform(U"2£"));
    ASSERT_EQUALS(U"Le prix est quatre vingt dix neuf euros.", layer.transform(U"Le prix est 99€."));

    return true;
}

REGISTER_TEST(num2word_fr_ordinal_test)
{
    Num2Word layer;

    // Potentialy ordinal
    ASSERT_EQUALS(U"premier", layer.transform(U"1er"));
    ASSERT_EQUALS(U"première", layer.transform(U"1re"));
    ASSERT_EQUALS(U"deuxième", layer.transform(U"2ème"));
    ASSERT_EQUALS(U"troisième", layer.transform(U"3e"));
    ASSERT_EQUALS(U"dixième", layer.transform(U"10e"));
    ASSERT_EQUALS(U"douzième", layer.transform(U"12ème"));
    ASSERT_EQUALS(U"vingt et unième", layer.transform(U"21e"));
    ASSERT_EQUALS(U"centième", layer.transform(U"100e"));

    return true;
}

REGISTER_TEST(num2word_fr_date_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"vingt sept mars deux mille vingt six", layer.transform(U"27.03.2026"));
    ASSERT_EQUALS(U"Aujourd'hui c'est le vingt sept mars deux mille vingt six.", layer.transform(U"Aujourd'hui c'est le 27-03-2026."));
    ASSERT_EQUALS(U"Aujourd'hui c'est le vingt sept mars deux mille vingt six.", layer.transform(U"Aujourd'hui c'est le 2026-03-27."));

    return true;
}

REGISTER_TEST(num2word_fr_complex_test)
{
    processor::num2word::Config config;
    config.allow_general_ord_notation = true;
    Num2Word layer(config);

    std::u32string input = U"Le 2026-03-27, le 1er coureur a terminé 1/2 de la course. "
                            "Il était 1. dans la file, tandis que le 2e suivait à 12,5 secondes.";
    
    std::u32string expected = U"Le vingt sept mars deux mille vingt six, le premier coureur a terminé un demi de la course. "
                               "Il était premier dans la file, tandis que le deuxième suivait à douze virgule cinq secondes.";

    ASSERT_EQUALS(expected, layer.transform(input));

    return true;
}

} // namespace phonemis::test