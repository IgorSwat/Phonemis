#include "processing.h"
#include "test.h"
#include <phonemis/base/processor/layers/sym2word/layer.h>
#include <phonemis/lang/de/constants.h>
#include <phonemis/lang/es/constants.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using processor::sym2word::Sym2WordLayer;

REGISTER_TEST(sym2word_base_test) {
  Sym2WordLayer layer({{U"&", {U"and"}}, {U"+", {U"plus"}}, {U"=", {U"equals"}}, {U"*", {U""}}});

  // Symbols read as words, separated by single spaces
  ASSERT_EQUALS(U"Tom and Jerry", apply(layer, U"Tom & Jerry"));
  ASSERT_EQUALS(U"R and B", apply(layer, U"R&B"));
  ASSERT_EQUALS(U"plus equals x", apply(layer, U"+= x"));

  // No space before punctuation, and a symbol without words leaves one space
  ASSERT_EQUALS(U"a plus, b", apply(layer, U"a+, b"));
  ASSERT_EQUALS(U"a b", apply(layer, U"a * b"));

  return true;
}

REGISTER_TEST(sym2word_conditions_test) {
  // A symbol whose conditions all fail stays as written
  Sym2WordLayer layer({{U"+", {{U"plus", processor::after_name}}}});
  ASSERT_EQUALS(U"Google plus and 2 + 2", apply(layer, U"Google+ and 2 + 2"));

  // Spanish "+" after a name
  Sym2WordLayer spanish(es::constants::sym2word::kSymbols);
  ASSERT_EQUALS(U"Google plus y C más más", apply(spanish, U"Google+ y C++"));

  // German "§" after a word which takes its plural
  Sym2WordLayer german(de::constants::sym2word::kSymbols);
  ASSERT_EQUALS(U"im Paragrafen 5 und Paragraf 6", apply(german, U"im § 5 und § 6"));

  return true;
}

} // namespace phonemis::test
