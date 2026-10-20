#include "processing.h"
#include "test.h"
#include <phonemis/lang/de/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

// German readings follow the Echo verbalizer; ISO dates are read by phonemis only.
REGISTER_TEST(num2word_de_test) {
  de::Num2Word layer;

  ASSERT_EQUALS(U"Am ersten Januar zweitausendzwanzig um achtzehn Uhr dreißig kostete es zwölf Euro neunundneunzig.",
                apply(layer, U"Am 1. Januar 2020 um 18:30 Uhr kostete es 12,99 €."));
  ASSERT_EQUALS(U"Er wird in zweiter und dritter Lesung beraten.",
                apply(layer, U"Er wird in 2. und 3. Lesung beraten."));
  ASSERT_EQUALS(U"Nur eine Person kam, eintausendneunhundertfünfundvierzig Einwohner, im Jahr neunzehnhundertfünfundvierzig.",
                apply(layer, U"Nur 1 Person kam, 1945 Einwohner, im Jahr 1945."));
  ASSERT_EQUALS(U"Die neunziger Jahre und die Neunziger, achtprozentige Lösung, Vier-Zimmer-Wohnung und dreißigjährige.",
                apply(layer, U"Die 90er Jahre und die 90er, 8%ige Lösung, 4-Zimmer-Wohnung und 30-jährige."));
  ASSERT_EQUALS(U"Ruf null drei null eins zwei drei vier fünf sechs sieben an, am zwölften März ist es so weit, drei Viertel und fünf durch sieben der Stimmen.",
                apply(layer, U"Ruf 030 1234567 an, am 12. 3. ist es so weit, 3/4 und 5/7 der Stimmen."));
  ASSERT_EQUALS(U"ein bis zwei Werktage, mit einer Stimme, Seite eins und fünf Kilogramm Mehl für einen Euro, zwei Komma fünf Millionen Euro.",
                apply(layer, U"1 bis 2 Werktage, mit 1 Stimme, Seite 1 und 5 kg Mehl für 1 €, 2,5 Mio. Euro."));
  ASSERT_EQUALS(U"Es ist eintausendneunhundertzwanzig mal eintausendachtzig und zehn bis fünfzehn Kilometer entfernt, zwanzig Prozent mehr.",
                apply(layer, U"Es ist 1920 mal 1080 und 10-15 km entfernt, 20 % mehr."));

  return true;
}

} // namespace phonemis::test
