#include "processing.h"
#include "test.h"
#include <phonemis/base/processor/layers/trim/layer.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace processor::trim;

REGISTER_TEST(trim_layer_base_basic_whitespace_test) {
  TrimLayer layer;

  // Consecutive whitespaces
  ASSERT_EQUALS(U"hello world", apply(layer, U"hello    world"));
  ASSERT_EQUALS(U"a b c", apply(layer, U"a  b   c"));

  // A run with a line break becomes one line break
  ASSERT_EQUALS(U"line1\nline2", apply(layer, U"line1\n\t line2"));
  ASSERT_EQUALS(U"line1\nline2", apply(layer, U"line1\n\n\nline2"));

  // No space before punctuation, but before an ellipsis
  ASSERT_EQUALS(U"Hello, world! Ok? Yes; no: maybe. And ...", apply(layer, U"Hello , world ! Ok ? Yes ; no : maybe . And ..."));

  return true;
}

REGISTER_TEST(trim_layer_base_leading_trailing_test) {
  TrimLayer layer;

  // Leading and trailing whitespaces
  ASSERT_EQUALS(U"trimmed", apply(layer, U"   trimmed   "));
  ASSERT_EQUALS(U"multiple words trimmed", apply(layer, U"\n  multiple words trimmed\t "));

  // Empty or only whitespace input
  ASSERT_EQUALS(U"", apply(layer, U"   "));
  ASSERT_EQUALS(U"", apply(layer, U""));

  return true;
}

} // namespace phonemis::test
