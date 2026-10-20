#include "processing.h"
#include "test.h"
#include <phonemis/lang/pl/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

// Polish readings follow the Echo verbalizer; ISO dates are read by phonemis only.
REGISTER_TEST(num2word_pl_test) {
  pl::Num2Word layer;

  ASSERT_EQUALS(U"Urodził się w tysiąc dziewięćset osiemdziesiątym piątym roku, pierwszego stycznia o osiemnastej trzydzieści.",
                apply(layer, U"Urodził się w 1985 roku, 1 stycznia o 18:30."));
  ASSERT_EQUALS(U"Otwieram pięćdziesiątą piątą sesję, około dwudziestu-trzydziestu minut, z trzema kolegami.",
                apply(layer, U"Otwieram 55. sesję, około 20-30 minut, z 3 kolegami."));
  ASSERT_EQUALS(U"To pięćset dwadzieścia cztery kilometry, jedenaście tysięcy trzysta czterdzieści dwa złote i czterdziestopięciometrowe mieszkania z lat siedemdziesiątych.",
                apply(layer, U"To 524 km, 11 342 zł i 45-metrowe mieszkania z lat 70-tych."));
  ASSERT_EQUALS(U"Od kwoty trzystu tysięcy, nie powinno przekraczać pięciu gramów.",
                apply(layer, U"Od kwoty 300 tysięcy, nie powinno przekraczać 5 g."));
  ASSERT_EQUALS(U"Dnia piętnastego sierpnia dwa tysiące dwudziestego roku przyszło dwie osoby, a w dwa tysiące dwudziestym roku było ich czterysta osiemdziesiąt.",
                apply(layer, U"Dnia 15.08.2020 r. przyszło 2 osoby, a w 2020 r. było ich 480."));
  ASSERT_EQUALS(U"Kosztuje dwanaście złotych dziewięćdziesiąt dziewięć groszy albo trzy miliony złotych, czyli dwadzieścia procent więcej niż półtora roku temu.",
                apply(layer, U"Kosztuje 12,99 zł albo 3 mln zł, czyli 20% więcej niż 1,5 roku temu."));
  ASSERT_EQUALS(U"Z lat siedemdziesiątych i osiemdziesiątych, w punkcie trzecim i rozdziale sześćset czternaście.",
                apply(layer, U"Z lat 70. i 80., w punkcie 3 i rozdziale 614."));
  ASSERT_EQUALS(U"Potrzebowali stu czterdziestu gramów mąki, stu trzydziestu gramów cukru.",
                apply(layer, U"Potrzebowali 140 g mąki, 130 g cukru."));
  ASSERT_EQUALS(U"Zadzwoń pod numer sześćset sto dwadzieścia trzy czterysta pięćdziesiąt sześć albo o siódmej piętnaście, numerem trzy przez trzydzieści osiem.",
                apply(layer, U"Zadzwoń pod numer 600 123 456 albo o 7.15, numerem 3x38."));
  ASSERT_EQUALS(U"W latach dwa tysiące dziesięć-dwa tysiące dwanaście zginęło tysiąc osób, w tym dwie kobiety.",
                apply(layer, U"W latach 2010-2012 zginęło 1 000 osób, w tym 2 kobiety."));

  return true;
}

} // namespace phonemis::test
