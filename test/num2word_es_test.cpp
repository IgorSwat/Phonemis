#include "processing.h"
#include "test.h"
#include <phonemis/lang/es/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

// Spanish readings follow the Echo verbalizer; ISO dates are read by phonemis only.
REGISTER_TEST(num2word_es_test) {
  es::Num2Word layer;

  ASSERT_EQUALS(U"La Ley dos de dos mil dieciocho entró en vigor el catorce de julio de dos mil veintitrés.",
                apply(layer, U"La Ley 2/2018 entró en vigor el 14/07/2023."));
  ASSERT_EQUALS(U"Llame al seis cero cero uno dos tres cuatro cinco seis o al nueve uno dos tres cuatro cinco seis siete ocho.",
                apply(layer, U"Llame al 600-123-456 o al 912-345-678."));
  ASSERT_EQUALS(U"Tiene veintiún años y treinta y una personas, una casa y veintiuna mesas.",
                apply(layer, U"Tiene 21 años y 31 personas, 1 casa y 21 mesas."));
  ASSERT_EQUALS(U"A las dieciocho treinta o a las nueve quince, pero uno punto trece metros.",
                apply(layer, U"A las 18:30 o a las 9.15, pero 1.13 metros."));
  ASSERT_EQUALS(U"Cuesta cinco dólares con cincuenta o tres euros con noventa y nueve y una libra, son cinco mil dólares.",
                apply(layer, U"Cuesta $5,50 o 3,99 € y 1 £, son $5.000 dólares."));
  ASSERT_EQUALS(U"Ganó tres millones de euros y dos mil quinientos millones.",
                apply(layer, U"Ganó 3 millones de euros y 2.500 millones."));
  ASSERT_EQUALS(U"Tiene diez mil seguidores y una tarjeta cuatro K.",
                apply(layer, U"Tiene 10k seguidores y una tarjeta 4K."));
  ASSERT_EQUALS(U"Mide un kilómetro y veintiún kilómetros, uno coma cinco kilogramos de harina, una hectárea.",
                apply(layer, U"Mide 1 km y 21 km, 1,5 kg de harina, 1 ha."));
  ASSERT_EQUALS(U"Llegó primero y segunda, el tercer puesto y el primer día, la quinta.",
                apply(layer, U"Llegó 1º y 2ª, el 3er puesto y el 1er día, la 5ta."));
  ASSERT_EQUALS(U"En los ochentas y los noventas hubo cambios.",
                apply(layer, U"En los 80s y los 90's hubo cambios."));
  ASSERT_EQUALS(U"La versión tres punto ocho punto diez y el resultado dos-uno, el marcador tres a dos.",
                apply(layer, U"La versión 3.8.10 y el resultado 2-1, el marcador 3:2."));
  ASSERT_EQUALS(U"En el minuto noventa más seis y dos más dos.",
                apply(layer, U"En el minuto 90+6 y 2 + 2."));
  ASSERT_EQUALS(U"Alcanzó mil novecientos ochenta,6 millones de dólares.",
                apply(layer, U"Alcanzó 1.980,6 millones de dólares."));
  ASSERT_EQUALS(U"Son las una cinco y la una.",
                apply(layer, U"Son las 1:05 y la 1."));
  ASSERT_EQUALS(U"Pagó mil doscientos treinta y cuatro,56 € por dos libros.",
                apply(layer, U"Pagó 1.234,56 € por 2 libros."));

  return true;
}

} // namespace phonemis::test
