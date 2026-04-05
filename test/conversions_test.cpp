#include "test.h"
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/io.h>

#include <string>

namespace phonemis::test {

using namespace utils::conversions;

REGISTER_TEST(utf8_to_u32_test)
{
    // Basic ASCII
    ASSERT_EQUALS(U"hello", utf8_to_u32("hello"));
    
    // 2-byte UTF-8 (Polish, Cyrillic, Greek)
    ASSERT_EQUALS(U"привет", utf8_to_u32("привет"));
    ASSERT_EQUALS(U"stanisław żółkiewski", utf8_to_u32("stanisław żółkiewski"));
    
    // 3-byte UTF-8 (Hindi, Japanese, IPA)
    ASSERT_EQUALS(U"नमस्ते", utf8_to_u32("नमस्ते"));
    ASSERT_EQUALS(U"こんにちは", utf8_to_u32("こんにちは"));
    
    // 4-byte UTF-8 (Emojis)
    ASSERT_EQUALS(U"🌍🌎🌏", utf8_to_u32("🌍🌎🌏"));

    // Mixed content
    ASSERT_EQUALS(U"Hello 🌍!", utf8_to_u32("Hello 🌍!"));

    // Empty string
    ASSERT_EQUALS(U"", utf8_to_u32(""));

    return true;
}

REGISTER_TEST(u32_to_utf8_test)
{
    // Basic ASCII
    ASSERT_EQUALS("hello", u32_to_utf8(U"hello"));
    
    // 2-byte UTF-8
    ASSERT_EQUALS("привет", u32_to_utf8(U"привет"));
    
    // 3-byte UTF-8
    ASSERT_EQUALS("नमस्ते", u32_to_utf8(U"नमस्ते"));
    ASSERT_EQUALS("こんにちは", u32_to_utf8(U"こんにちは"));
    
    // 4-byte UTF-8
    ASSERT_EQUALS("🌍🌎🌏", u32_to_utf8(U"🌍🌎🌏"));

    // Mixed content
    ASSERT_EQUALS("Hello 🌍!", u32_to_utf8(U"Hello 🌍!"));

    // Empty string
    ASSERT_EQUALS("", u32_to_utf8(U""));

    return true;
}

} // namespace phonemis::test
