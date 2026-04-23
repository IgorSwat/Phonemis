#include "test.h"
#include <phonemis/base/processor/trim_layer.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace processor;

REGISTER_TEST(trim_layer_base_basic_whitespace_test)
{
    TrimLayer layer;

    // Consecutive whitespaces
    ASSERT_EQUALS(U"hello world", layer.transform(U"hello    world"));
    ASSERT_EQUALS(U"a b c", layer.transform(U"a  b   c"));
    
    // Different whitespace characters
    ASSERT_EQUALS(U"line1 line2", layer.transform(U"line1\n\t line2"));

    return true;
}

REGISTER_TEST(trim_layer_base_leading_trailing_test)
{
    TrimLayer layer;

    // Leading and trailing whitespaces
    ASSERT_EQUALS(U"trimmed", layer.transform(U"   trimmed   "));
    ASSERT_EQUALS(U"multiple words trimmed", layer.transform(U"\n  multiple words trimmed\t "));
    
    // Empty or only whitespace input
    ASSERT_EQUALS(U"", layer.transform(U"   "));
    ASSERT_EQUALS(U"", layer.transform(U""));

    return true;
}

} // namespace phonemis::test
