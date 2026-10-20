#include "processing.h"
#include "test.h"
#include <phonemis/lang/hi/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace hi;

REGISTER_TEST(num2word_hi_basic_transform_test) {
  Num2Word layer;

  // Integers and decimals
  ASSERT_EQUALS(U"शून्य", apply(layer, U"0"));
  ASSERT_EQUALS(U"इक्कीस", apply(layer, U"21"));
  ASSERT_EQUALS(U"सत्तर", apply(layer, U"70"));
  ASSERT_EQUALS(U"इकहत्तर", apply(layer, U"71"));
  ASSERT_EQUALS(U"अस्सी", apply(layer, U"80"));
  ASSERT_EQUALS(U"निन्यानवे", apply(layer, U"99"));
  ASSERT_EQUALS(U"एक सौ", apply(layer, U"100"));
  ASSERT_EQUALS(U"एक सौ तेईस", apply(layer, U"123"));
  ASSERT_EQUALS(U"दो सौ", apply(layer, U"200"));
  ASSERT_EQUALS(U"एक हज़ार", apply(layer, U"1000"));
  ASSERT_EQUALS(U"दो हज़ार", apply(layer, U"2000"));
  ASSERT_EQUALS(U"पाँच हज़ार", apply(layer, U"5000"));
  ASSERT_EQUALS(U"एक लाख", apply(layer, U"100000"));
  ASSERT_EQUALS(U"दस लाख", apply(layer, U"1000000"));
  ASSERT_EQUALS(U"एक करोड़", apply(layer, U"10000000"));
  ASSERT_EQUALS(U"एक सौ तेईस दशमलव चार पाँच", apply(layer, U"123.45"));
  ASSERT_EQUALS(U"शून्य दशमलव शून्य पाँच", apply(layer, U"0.05"));

  // Fractions
  ASSERT_EQUALS(U"आधा", apply(layer, U"1/2"));
  ASSERT_EQUALS(U"एक तिहाई", apply(layer, U"1/3"));
  ASSERT_EQUALS(U"एक चौथाई", apply(layer, U"1/4"));
  ASSERT_EQUALS(U"तीन चौथाई", apply(layer, U"3/4"));
  ASSERT_EQUALS(U"तीन बटा दस", apply(layer, U"3/10"));

  // Currency
  ASSERT_EQUALS(U"एक डॉलर", apply(layer, U"1$"));
  ASSERT_EQUALS(U"पचास डॉलर", apply(layer, U"50$"));
  ASSERT_EQUALS(U"दो पाउंड", apply(layer, U"2£"));
  ASSERT_EQUALS(U"क़ीमत निन्यानवे यूरो है।", apply(layer, U"क़ीमत 99€ है।"));

  return true;
}

REGISTER_TEST(num2word_hi_ordinal_test) {
  Num2Word layer;

  // The suffix gives the form: masculine (वाँ), feminine (वीं) or plural (वें)
  ASSERT_EQUALS(U"पहला", apply(layer, U"1वाँ"));
  ASSERT_EQUALS(U"पहली", apply(layer, U"1वीं"));
  ASSERT_EQUALS(U"पहले", apply(layer, U"1वें"));
  ASSERT_EQUALS(U"दूसरा", apply(layer, U"2वाँ"));
  ASSERT_EQUALS(U"तीसरा", apply(layer, U"3वाँ"));
  ASSERT_EQUALS(U"चौथा", apply(layer, U"4वाँ"));
  ASSERT_EQUALS(U"पाँचवाँ", apply(layer, U"5वाँ"));
  ASSERT_EQUALS(U"पाँचवीं", apply(layer, U"5वीं"));
  ASSERT_EQUALS(U"छठा", apply(layer, U"6वाँ"));
  ASSERT_EQUALS(U"दसवाँ", apply(layer, U"10वाँ"));
  ASSERT_EQUALS(U"बारहवाँ", apply(layer, U"12वाँ"));
  ASSERT_EQUALS(U"इक्कीसवाँ", apply(layer, U"21वाँ"));
  ASSERT_EQUALS(U"एक सौवाँ", apply(layer, U"100वाँ"));
  ASSERT_EQUALS(U"यह पाँचवीं बार है।", apply(layer, U"यह 5वीं बार है।"));

  return true;
}

REGISTER_TEST(num2word_hi_date_test) {
  Num2Word layer;

  ASSERT_EQUALS(U"सत्ताईस मार्च दो हज़ार छब्बीस", apply(layer, U"27.03.2026"));
  ASSERT_EQUALS(U"आज सत्ताईस मार्च दो हज़ार छब्बीस है।", apply(layer, U"आज 27-03-2026 है।"));
  ASSERT_EQUALS(U"आज सत्ताईस मार्च दो हज़ार छब्बीस है।", apply(layer, U"आज 2026-03-27 है।"));

  return true;
}

} // namespace phonemis::test
