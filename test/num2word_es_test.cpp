#include "test.h"
#include <phonemis/lang/es/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace es;

REGISTER_TEST(num2word_es_basic_transform_test)
{
    Num2Word layer;

    // Integers and Floats
    ASSERT_EQUALS(U"cero", layer.transform(U"0"));
    ASSERT_EQUALS(U"veintiuno", layer.transform(U"21"));
    ASSERT_EQUALS(U"setenta", layer.transform(U"70"));
    ASSERT_EQUALS(U"setenta y uno", layer.transform(U"71"));
    ASSERT_EQUALS(U"ochenta", layer.transform(U"80"));
    ASSERT_EQUALS(U"noventa y nueve", layer.transform(U"99"));
    ASSERT_EQUALS(U"cien", layer.transform(U"100"));
    ASSERT_EQUALS(U"ciento veintitrés", layer.transform(U"123"));
    ASSERT_EQUALS(U"doscientos", layer.transform(U"200"));
    ASSERT_EQUALS(U"mil", layer.transform(U"1000"));
    ASSERT_EQUALS(U"dos mil", layer.transform(U"2000"));
    ASSERT_EQUALS(U"un millón", layer.transform(U"1000000"));
    ASSERT_EQUALS(U"ciento veintitrés coma cuarenta y cinco", layer.transform(U"123,45"));
    ASSERT_EQUALS(U"cero coma cero cinco", layer.transform(U"0,05"));

    // Fractions
    ASSERT_EQUALS(U"un medio", layer.transform(U"1/2"));
    ASSERT_EQUALS(U"un tercio", layer.transform(U"1/3"));
    ASSERT_EQUALS(U"un cuarto", layer.transform(U"1/4"));
    ASSERT_EQUALS(U"tres cuartos", layer.transform(U"3/4"));
    ASSERT_EQUALS(U"tres décimos", layer.transform(U"3/10"));

    // Currency
    ASSERT_EQUALS(U"un dólar", layer.transform(U"1$"));
    ASSERT_EQUALS(U"cincuenta dólares", layer.transform(U"50$"));
    ASSERT_EQUALS(U"un euro", layer.transform(U"1,00€"));
    ASSERT_EQUALS(U"dos libras", layer.transform(U"2£"));
    ASSERT_EQUALS(U"El precio es noventa y nueve euros.", layer.transform(U"El precio es 99€."));

    return true;
}

REGISTER_TEST(num2word_es_ordinal_test)
{
    Num2Word layer;

    // Potentialy ordinal
    ASSERT_EQUALS(U"primer", layer.transform(U"1er"));
    ASSERT_EQUALS(U"primera", layer.transform(U"1ª"));
    ASSERT_EQUALS(U"primero", layer.transform(U"1º"));
    ASSERT_EQUALS(U"segundo", layer.transform(U"2º"));
    ASSERT_EQUALS(U"tercer", layer.transform(U"3er"));
    ASSERT_EQUALS(U"décimo", layer.transform(U"10º"));
    ASSERT_EQUALS(U"duodécimo", layer.transform(U"12º"));
    ASSERT_EQUALS(U"vigésimo", layer.transform(U"20º"));
    ASSERT_EQUALS(U"centésimo", layer.transform(U"100º"));

    return true;
}

REGISTER_TEST(num2word_es_date_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"veintisiete de marzo de dos mil veintiséis", layer.transform(U"27.03.2026"));
    ASSERT_EQUALS(U"Hoy es el veintisiete de marzo de dos mil veintiséis.", layer.transform(U"Hoy es el 27-03-2026."));
    ASSERT_EQUALS(U"Hoy es el veintisiete de marzo de dos mil veintiséis.", layer.transform(U"Hoy es el 2026-03-27."));

    return true;
}

REGISTER_TEST(num2word_es_complex_test)
{
    processor::num2word::Config config;
    config.allow_general_ord_notation = true;
    Num2Word layer(config);

    std::u32string input = U"El 2026-03-27, el 1er corredor terminó 1/2 de la carrera. "
                            "Él estaba 1º en la fila, mientras que el 2º seguía a 12,5 segundos.";
    
    std::u32string expected = U"El veintisiete de marzo de dos mil veintiséis, el primer corredor terminó un medio de la carrera. "
                               "Él estaba primero en la fila, mientras que el segundo seguía a doce coma cinco segundos.";

    ASSERT_EQUALS(expected, layer.transform(input));

    return true;
}

} // namespace phonemis::test
