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
    // KEEP mode
    SanitizerLayer layer_keep(&TEST_KEEP_FILTER, nullptr, SanitizerLayer::Mode::KEEP);
    ASSERT_EQUALS(U"abba ba", layer_keep.transform(U"abcba cba"));
    ASSERT_EQUALS(U"a1a", layer_keep.transform(U"a123a"));

    // REJECT mode
    SanitizerLayer layer_reject(&TEST_REJECT_FILTER, nullptr, SanitizerLayer::Mode::REJECT);
    ASSERT_EQUALS(U"abc", layer_reject.transform(U"axbyc"));
    ASSERT_EQUALS(U"hello", layer_reject.transform(U"hello"));

    // KEEP_ALPHABETICAL mode
    SanitizerLayer layer_keep_alpha(&TEST_KEEP_FILTER, nullptr, SanitizerLayer::Mode::KEEP_ALPHABETICAL);
    // 'c' is alphabetical and not in filter -> omitted
    // '1', '!', ' ' are not alphabetical -> passed
    ASSERT_EQUALS(U"ab 123! ab", layer_keep_alpha.transform(U"abc 123! abc"));

    return true;
}

REGISTER_TEST(sanitizer_mapping_test)
{
    SanitizerLayer layer_map(nullptr, &TEST_MAPPER);

    ASSERT_EQUALS(U"Abc!", layer_map.transform(U"abc1"));

    return true;
}

REGISTER_TEST(sanitizer_filter_and_mapping_combined_test)
{
    SanitizerLayer layer_combined(&TEST_KEEP_FILTER, &TEST_MAPPER, SanitizerLayer::Mode::KEEP);

    // 'c' is filtered out. '1' is kept by filter then mapped to '!'
    ASSERT_EQUALS(U"Ab!", layer_combined.transform(U"abc1"));

    return true;
}

} // namespace phonemis::test
