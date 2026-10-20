#include "processing.h"
#include "test.h"
#include <phonemis/lang/de/num2word.h>
#include <phonemis/lang/en/num2word.h>
#include <phonemis/lang/es/num2word.h>
#include <phonemis/lang/fr/num2word.h>
#include <phonemis/lang/it/num2word.h>
#include <phonemis/lang/pl/num2word.h>
#include <phonemis/lang/pt/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

// Phone numbers: international ones (+48 ...) in every language, and each language's own formats.
// Formats written with spaces are counts unless a word says they are phone numbers (Kup 600 123 456 jabłek).
REGISTER_TEST(num2word_phone_test) {
  en::Num2Word en;
  ASSERT_EQUALS(U"Call five five five, one two three, four five six seven, five five five, one two three, four five six seven or five five five, one two three, four five six seven.",
                apply(en, U"Call (555) 123-4567, 555.123.4567 or 555 123 4567."));
  ASSERT_EQUALS(U"Call plus one five five five, one two three, four five six seven or plus forty-four zero, two zero, seven nine four six, zero nine five eight.",
                apply(en, U"Call +1 555 123 4567 or +44 (0)20 7946 0958."));

  pl::Num2Word pl;
  ASSERT_EQUALS(U"Zadzwoń na plus czterdzieści osiem sześćset sto dwadzieścia trzy czterysta pięćdziesiąt sześć lub dwadzieścia dwa sto dwadzieścia trzy czterdzieści pięć sześćdziesiąt siedem.",
                apply(pl, U"Zadzwoń na +48 600 123 456 lub 22 123 45 67."));
  ASSERT_EQUALS(U"Infolinia: dwadzieścia dwa sto dwadzieścia trzy czterdzieści pięć sześćdziesiąt siedem, telefony sześćdziesiąt osiem trzysta dwadzieścia cztery dwadzieścia dwa pięćdziesiąt pięć lub osiemset jeden trzysta dwadzieścia siedem czterysta sześćdziesiąt dwa.",
                apply(pl, U"Infolinia: 22 123 45 67, telefony 68-324-22-55 lub 801-327-462."));
  ASSERT_EQUALS(U"Kup sześćset milionów sto dwadzieścia trzy tysiące czterysta pięćdziesiąt sześć jabłek.",
                apply(pl, U"Kup 600 123 456 jabłek."));

  fr::Num2Word fr;
  ASSERT_EQUALS(U"Appelez le plus trente-trois six douze trente-quatre cinquante-six soixante-dix-huit.",
                apply(fr, U"Appelez le +33 6 12 34 56 78."));

  es::Num2Word es;
  ASSERT_EQUALS(U"Llame al seis cero cero uno dos tres cuatro cinco seis o al más treinta y cuatro nueve uno dos tres cuatro cinco seis siete ocho.",
                apply(es, U"Llame al 600 123 456 o al +34 912 345 678."));
  ASSERT_EQUALS(U"Son seiscientos ciento veintitrés cuatrocientos cincuenta y seis euros.",
                apply(es, U"Son 600 123 456 euros."));

  it::Num2Word it;
  ASSERT_EQUALS(U"Chiama il tre tre tre uno due tre quattro cinque sei sette o il zero sei uno due tre quattro cinque sei sette otto.",
                apply(it, U"Chiama il 333 1234567 o il 06 1234 5678."));
  ASSERT_EQUALS(U"Chiama il più trentanove tre tre tre uno due tre quattro cinque sei sette.",
                apply(it, U"Chiama il +39 333 123 4567."));

  pt::Num2Word pt;
  ASSERT_EQUALS(U"Ligue para um um, nove oito sete seis cinco, quatro três dois um ou mais cinquenta e cinco um um, nove oito sete seis cinco, quatro três dois um.",
                apply(pt, U"Ligue para (11) 98765-4321 ou +55 11 98765-4321."));
  ASSERT_EQUALS(U"Ligue para dois um três, quatro cinco seis, sete oito nove.",
                apply(pt, U"Ligue para 213 456 789."));

  de::Num2Word de;
  ASSERT_EQUALS(U"Telefon: plus neunundvierzig drei null eins zwei drei vier fünf sechs sieben.",
                apply(de, U"Telefon: +49 30 1234567."));

  return true;
}

} // namespace phonemis::test
