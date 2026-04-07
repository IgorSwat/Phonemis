#include "test.h"
#include <phonemis/base/phonemizer/nn/tokenizer.h>
#include <phonemis/base/phonemizer/nn/constants.h>
#include <phonemis/utils/io.h>

#include <vector>
#include <string_view>

namespace phonemis::test {

using namespace phonemizer::nn;
using namespace phonemizer::nn::constants;

REGISTER_TEST(neural_base_tokenization_test) {
    Tokenizer tokenizer;

    // Test Case 1: Simple word "hello"
    std::u32string_view input1 = U"hello";
    std::vector<int64_t> tokens1 = tokenizer.tokenize(input1);

    ASSERT_EQUALS(tokens1.size(), input1.size());
    for (size_t i = 0; i < input1.size(); ++i) {
        ASSERT_EQUALS(tokens1[i], DEFAULT_CHAR_TO_TOKEN.at(input1[i]));
    }

    // Test Case 2: Word with accented characters "àáâã"
    std::u32string_view input2 = U"àáâã";
    std::vector<int64_t> tokens2 = tokenizer.tokenize(input2);

    ASSERT_EQUALS(tokens2.size(), input2.size());
    for (size_t i = 0; i < input2.size(); ++i) {
        ASSERT_EQUALS(tokens2[i], DEFAULT_CHAR_TO_TOKEN.at(input2[i]));
    }

    // Test Case 3: Input with unknown characters (should be omitted)
    // Assume '#' and '@' are not in DEFAULT_CHAR_TO_TOKEN
    std::u32string_view input3 = U"a#b@c"; 
    std::vector<int64_t> tokens3 = tokenizer.tokenize(input3);

    // Only 'a', 'b', 'c' should remain
    ASSERT_EQUALS(tokens3.size(), 3ULL);
    ASSERT_EQUALS(tokens3[0], DEFAULT_CHAR_TO_TOKEN.at(U'a'));
    ASSERT_EQUALS(tokens3[1], DEFAULT_CHAR_TO_TOKEN.at(U'b'));
    ASSERT_EQUALS(tokens3[2], DEFAULT_CHAR_TO_TOKEN.at(U'c'));

    // Test Case 4: Decode mapping
    std::u32string decoded1 = tokenizer.decode(tokens1);
    ASSERT_EQUALS(decoded1, std::u32string(input1));

    return true;
}

} // namespace phonemis::test
