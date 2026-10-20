#include "processing.h"
#include "test.h"
#include <phonemis/base/processor/layers/term2word/layer.h>
#include <phonemis/lang/en/constants.h>
#include <phonemis/lang/pl/constants.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using processor::term2word::Term2WordLayer;

REGISTER_TEST(term2word_address_test) {
  Term2WordLayer layer(en::constants::term2word::kTerms);

  // Domains, hosts and paths
  ASSERT_EQUALS(U"Visit example dot com.", apply(layer, U"Visit example.com."));
  ASSERT_EQUALS(U"w w w dot example dot co dot uk slash news slash index dot html",
                apply(layer, U"www.example.co.uk/news/index.html"));
  ASSERT_EQUALS(U"(see blog dot example dot io slash)", apply(layer, U"(see https://blog.example.io/)"));

  // Emails
  ASSERT_EQUALS(U"Write to jan dot kowalski at firma dot pl!", apply(layer, U"Write to jan.kowalski@firma.pl!"));

  // Not addresses: no known domain, a number, an abbreviation
  ASSERT_EQUALS(U"file.txt, 3.5 and U.S.", apply(layer, U"file.txt, 3.5 and U.S."));

  return true;
}

REGISTER_TEST(term2word_abbreviation_test) {
  Term2WordLayer english(en::constants::term2word::kTerms);

  // Readings by context: before a number, before a name
  ASSERT_EQUALS(U"See page 12 and chapter 3, not p. a.", apply(english, U"See p. 12 and ch. 3, not p. a."));
  ASSERT_EQUALS(U"Saint Louis is on Main Street.", apply(english, U"St. Louis is on Main St."));
  ASSERT_EQUALS(U"Doctor Smith, Mister Brown and Dr. no", apply(english, U"Dr. Smith, Mr. Brown and Dr. no"));

  // A dot ending the text stays
  ASSERT_EQUALS(U"apples, pears et cetera.", apply(english, U"apples, pears etc."));

  // Polish, with a number right after the abbreviation
  Term2WordLayer polish(pl::constants::term2word::kTerms);
  ASSERT_EQUALS(U"numer 5, numer 7 i około 20 osób, między innymi Jan", apply(polish, U"nr 5, nr7 i ok. 20 osób, m.in. Jan"));

  return true;
}

} // namespace phonemis::test
