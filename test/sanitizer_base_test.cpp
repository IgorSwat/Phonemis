#include "processing.h"
#include "test.h"
#include <phonemis/base/processor/layers/sanitizer/layer.h>
#include <phonemis/utils/unicode.h>
#include <phonemis/utils/io.h>

#include <unordered_set>
#include <unordered_map>

namespace phonemis::test {

using namespace processor::sanitizer;

static const std::unordered_set<char32_t> TEST_KEEP_SET = {U'a', U'b', U'1', U' '};
static const std::unordered_set<char32_t> TEST_REJECT_SET = {U'x', U'y'};
static const std::unordered_map<char32_t, char32_t> TEST_MAP = {{U'a', U'A'}, {U'1', U'!'}};

REGISTER_TEST(sanitizer_filter_modes_test)
{
    // KEEP mode
    SanitizerLayer layer_keep([](char32_t c) { return TEST_KEEP_SET.contains(c); });
    ASSERT_EQUALS(U"abba ba", apply(layer_keep, U"abcba cba"));
    ASSERT_EQUALS(U"a1a", apply(layer_keep, U"a123a"));

    // REJECT mode
    SanitizerLayer layer_reject([](char32_t c) { return !TEST_REJECT_SET.contains(c); });
    ASSERT_EQUALS(U"abc", apply(layer_reject, U"axbyc"));
    ASSERT_EQUALS(U"hello", apply(layer_reject, U"hello"));

    // KEEP_ALPHABETICAL mode
    SanitizerLayer layer_keep_alpha([](char32_t c) { 
        if (utils::unicode::isalpha(c)) {
            return TEST_KEEP_SET.contains(c);
        }
        return true;
    });
    
    // 'c' is alphabetical and not in filter -> omitted
    // '1', '!', ' ' are not alphabetical -> passed
    ASSERT_EQUALS(U"ab 123! ab", apply(layer_keep_alpha, U"abc 123! abc"));

    return true;
}

REGISTER_TEST(sanitizer_mapping_test)
{
    SanitizerLayer layer_map([](char32_t) { return true; }, [](char32_t c) {
        if (TEST_MAP.contains(c)) return TEST_MAP.at(c);
        return c;
    });

    ASSERT_EQUALS(U"Abc!", apply(layer_map, U"abc1"));

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
    ASSERT_EQUALS(U"Ab!", apply(layer_combined, U"abc1"));

    return true;
}

} // namespace phonemis::test
