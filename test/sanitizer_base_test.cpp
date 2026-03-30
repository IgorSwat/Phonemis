#include "test.h"
#include <phonemis/base/preprocessor/sanitizer_layer.h>
#include <phonemis/utils/io.h>

#include <memory>

namespace phonemis::test {

using namespace preprocessor;

static const std::unordered_set<char32_t> TEST_KEEP_FILTER = {U'a', U'b', U'1', U' '};
static const std::unordered_set<char32_t> TEST_REJECT_FILTER = {U'x', U'y'};
static const std::unordered_map<char32_t, char32_t> TEST_MAPPER = {{U'a', U'A'}, {U'1', U'!'}};

REGISTER_TEST(sanitizer_filter_modes_test)
{
    SanitizerLayer layer;

    // KEEP mode
    layer.setupFilter(&TEST_KEEP_FILTER, SanitizerLayer::Mode::KEEP);
    ASSERT_EQUALS(U"abba ba", layer.transform(U"abcba cba"));
    ASSERT_EQUALS(U"a1a", layer.transform(U"a123a"));

    // REJECT mode
    layer.setupFilter(&TEST_REJECT_FILTER, SanitizerLayer::Mode::REJECT);
    ASSERT_EQUALS(U"abc", layer.transform(U"axbyc"));
    ASSERT_EQUALS(U"hello", layer.transform(U"hello"));

    // KEEP_ALPHABETICAL mode
    layer.setupFilter(&TEST_KEEP_FILTER, SanitizerLayer::Mode::KEEP_ALPHABETICAL);
    // 'c' is alphabetical and not in filter -> omitted
    // '1', '!', ' ' are not alphabetical -> passed
    ASSERT_EQUALS(U"ab 123! ab", layer.transform(U"abc 123! abc"));

    return true;
}

REGISTER_TEST(sanitizer_mapping_test)
{
    SanitizerLayer layer;

    layer.setupMapper(&TEST_MAPPER);

    ASSERT_EQUALS(U"Abc!", layer.transform(U"abc1"));

    return true;
}

REGISTER_TEST(sanitizer_filter_and_mapping_combined_test)
{
    SanitizerLayer layer;
    
    layer.setupFilter(&TEST_KEEP_FILTER, SanitizerLayer::Mode::KEEP);
    layer.setupMapper(&TEST_MAPPER);

    // 'c' is filtered out. '1' is kept by filter then mapped to '!'
    ASSERT_EQUALS(U"Ab!", layer.transform(U"abc1"));

    return true;
}

} // namespace phonemis::test
