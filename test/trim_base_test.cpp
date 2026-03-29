#include "test.h"
#include <phonemis/base/preprocessor/trim_layer.h>

namespace phonemis::test {

using namespace preprocessor;

REGISTER_TEST(trim_layer_base_basic_whitespace_test)
{
    TrimLayer layer;

    // Consecutive whitespaces
    ASSERT_EQUALS("hello world", layer.transform("hello    world"));
    ASSERT_EQUALS("a b c", layer.transform("a  b   c"));
    
    // Different whitespace characters
    ASSERT_EQUALS("line1 line2", layer.transform("line1\n\t line2"));

    return true;
}

REGISTER_TEST(trim_layer_base_leading_trailing_test)
{
    TrimLayer layer;

    // Leading and trailing whitespaces
    ASSERT_EQUALS("trimmed", layer.transform("   trimmed   "));
    ASSERT_EQUALS("multiple words trimmed", layer.transform("\n  multiple words trimmed\t "));
    
    // Empty or only whitespace input
    ASSERT_EQUALS("", layer.transform("   "));
    ASSERT_EQUALS("", layer.transform(""));

    return true;
}

} // namespace phonemis::test
