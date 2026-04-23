#include "test.h"
#include <phonemis/base/processor/sanitizer_layer.h>
#include <phonemis/utils/io.h>
#include <phonemis/utils/unicode.h>

#include <unordered_set>
#include <unordered_map>

namespace phonemis::test {

using namespace processor;

static const std::unordered_set<char32_t> TEST_KEEP_SET = {U'a', U'b', U'1', U' '};
static const std::unordered_set<char32_t> TEST_REJECT_SET = {U'x', U'y'};
static const std::unordered_map<char32_t, char32_t> TEST_MAP = {{U'a', U'A'}, {U'1', U'!'}};

REGISTER_TEST(sanitizer_filter_modes_test)
{
    // KEEP mode
    SanitizerLayer layer_keep([](char32_t c) { return TEST_KEEP_SET.contains(c); });
    ASSERT_EQUALS(U"abba ba", layer_keep.transform(U"abcba cba"));
    ASSERT_EQUALS(U"a1a", layer_keep.transform(U"a123a"));

    // REJECT mode
    SanitizerLayer layer_reject([](char32_t c) { return !TEST_REJECT_SET.contains(c); });
    ASSERT_EQUALS(U"abc", layer_reject.transform(U"axbyc"));
    ASSERT_EQUALS(U"hello", layer_reject.transform(U"hello"));

    // KEEP_ALPHABETICAL mode
    SanitizerLayer layer_keep_alpha([](char32_t c) { 
        if (utils::unicode::isalpha(c)) {
            return TEST_KEEP_SET.contains(c);
        }
        return true;
    });
    
    // 'c' is alphabetical and not in filter -> omitted
    // '1', '!', ' ' are not alphabetical -> passed
    ASSERT_EQUALS(U"ab 123! ab", layer_keep_alpha.transform(U"abc 123! abc"));

    return true;
}

REGISTER_TEST(sanitizer_mapping_test)
{
    SanitizerLayer layer_map([](char32_t) { return true; }, [](char32_t c) {
        if (TEST_MAP.contains(c)) return TEST_MAP.at(c);
        return c;
    });

    ASSERT_EQUALS(U"Abc!", layer_map.transform(U"abc1"));

    return true;
}

REGISTER_TEST(sanitizer_filter_and_mapping_combined_test)
{
    SanitizerLayer layer_combined(
        [](char32_t c) { return TEST_KEEP_SET.contains(c); },
        [](char32_t c) {
            if (TEST_MAP.contains(c)) return TEST_MAP.at(c);
            return c;
        }
    );

    // 'c' is filtered out. '1' is kept by filter then mapped to '!'
    ASSERT_EQUALS(U"Ab!", layer_combined.transform(U"abc1"));

    return true;
}

} // namespace phonemis::test
