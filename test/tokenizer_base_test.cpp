#include "test.h"
#include <phonemis/base/tokenizer/tokenizer.h>
#include <phonemis/base/tokenizer/types.h>
#include <phonemis/utils/conversions.h>

#include <algorithm>
#include <iostream>

namespace phonemis::test {

using namespace tokenizer;
using namespace tokenizer::split;

// Helper function to compare token vectors
bool tokensEqual(const std::vector<Token>& actual, const std::vector<std::u32string>& expected) {
    return actual.size() == expected.size() &&
           std::equal(actual.begin(), actual.end(), expected.begin(), 
                      [](const Token& a, const std::u32string& b) { return a.text == b; });
}

REGISTER_TEST(tokenizer_base_basic_tokenization_test)
{
    // Create rules: hyphen uses TOTAL_DIVIDE (default), apostrophe uses KEEP_WITH_LEFT
    Rules rules = {
        {U'-', Rule::TOTAL_DIVIDE},   // "well-being" -> "well" + "-" + "being"
        {U'\'', Rule::KEEP_WITH_RIGHT}     // "don't" -> "don" + "'t"
    };

    Tokenizer tokenizer(&rules, nullptr);

    // Test 1: Simple whitespace-separated words
    auto tokens = tokenizer.tokenize(U"hello world");
    std::vector<std::u32string> expected = {U"hello", U"world"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));
    ASSERT_EQUALS(true, tokens[0].first);       // First token should be marked
    ASSERT_EQUALS(true, tokens[0].whitespace);  // A space after first token should be marked.
    ASSERT_EQUALS(false, tokens[1].whitespace);

    // Test 2: Soft separator with TOTAL_DIVIDE rule
    tokens = tokenizer.tokenize(U"well-being");
    expected = {U"well", U"-", U"being"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));
    ASSERT_EQUALS(false, tokens[0].whitespace);
    ASSERT_EQUALS(false, tokens[1].whitespace);

    // Test 3: Soft separator with JOIN_LEFT rule
    tokens = tokenizer.tokenize(U"don't");
    expected = {U"don", U"'t"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));
    ASSERT_EQUALS(false, tokens[0].whitespace);
    ASSERT_EQUALS(false, tokens[1].whitespace);

    // Test 4: Mixed - multiple separators
    tokens = tokenizer.tokenize(U"it's well-being");
    expected = {U"it", U"'s", U"well", U"-", U"being"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    return true;
}

REGISTER_TEST(tokenizer_base_rules_and_exceptions_test)
{
    // Create rules: hyphen should split phrases
    Rules rules = {
        {U'-', Rule::TOTAL_DIVIDE}
    };

    // Create exceptions: "well-being" should not be split even with hyphens
    Exceptions exceptions = {
        U"well-being"
    };

    Tokenizer tokenizer(&rules, &exceptions);

    // Test 1: Exception prevents splitting
    auto tokens = tokenizer.tokenize(U"well-being");
    std::vector<std::u32string> expected = {U"well-being"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 2: Non-exception words still follow rules
    tokens = tokenizer.tokenize(U"good-bye");
    expected = {U"good", U"-", U"bye"};  // Follows TOTAL_JOIN rule
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 3: Multiple words with exceptions
    tokens = tokenizer.tokenize(U"well-being and good-bye");
    expected = {U"well-being", U"and", U"good", U"-", U"bye"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));
    ASSERT_EQUALS(true, tokens[0].first);       // First token marked
    ASSERT_EQUALS(true, tokens[0].whitespace);
    ASSERT_EQUALS(true, tokens[1].whitespace);
    ASSERT_EQUALS(false, tokens[2].whitespace);

    return true;
}

REGISTER_TEST(tokenizer_base_hard_separators_test)
{
    // Create rules: hyphen uses TOTAL_JOIN
    // Characters not in rules (like @ and #) will be hard separators
    Rules rules = {
        {U'-', Rule::TOTAL_JOIN}
    };

    Tokenizer tokenizer(&rules, nullptr);

    // Test 1: Hard separator without rule (@ is not in rules)
    // Hard separators immediately separate and are their own tokens
    auto tokens = tokenizer.tokenize(U"hello@world");
    std::vector<std::u32string> expected = {U"hello", U"@", U"world"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 2: Multiple hard separators
    tokens = tokenizer.tokenize(U"user#domain@server");
    expected = {U"user", U"#", U"domain", U"@", U"server"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 3: Mix of soft (hyphen) and hard separators (@)
    tokens = tokenizer.tokenize(U"user-name@domain-server");
    expected = {U"user-name", U"@", U"domain-server"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 4: Hard separator with no text after
    tokens = tokenizer.tokenize(U"email@");
    expected = {U"email", U"@"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    return true;
}

REGISTER_TEST(tokenizer_base_no_rules_no_exceptions_test)
{
    // No rules and no exceptions - all special characters are hard separators
    Tokenizer tokenizer(nullptr, nullptr);

    // Test 1: Simple text with special character
    auto tokens = tokenizer.tokenize(U"hello-world");
    std::vector<std::u32string> expected = {U"hello", U"-", U"world"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 2: Multiple different special characters
    tokens = tokenizer.tokenize(U"one,two;three");
    expected = {U"one", U",", U"two", U";", U"three"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 3: Mixed alphanumeric and special
    tokens = tokenizer.tokenize(U"user@domain.com");
    expected = {U"user", U"@", U"domain", U".", U"com"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 4: With whitespace
    tokens = tokenizer.tokenize(U"hello-world test");
    expected = {U"hello", U"-", U"world", U"test"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));
    ASSERT_EQUALS(true, tokens[0].first);   // First token marked
    ASSERT_EQUALS(true, tokens[2].whitespace);

    return true;
}

REGISTER_TEST(tokenizer_base_complex_rules_test)
{
    // Complex rules: different separators with different behaviors
    Rules rules = {
        {U'-', Rule::TOTAL_DIVIDE},   // hyphen: split and remove
        {U'\'', Rule::KEEP_WITH_RIGHT},     // apostrophe: attach at the beginning of the right-side phrase
        {U':', Rule::TOTAL_JOIN}      // colon: keep together
    };

    Tokenizer tokenizer(&rules, nullptr);

    // Test 1: Multiple rule types in one string
    auto tokens = tokenizer.tokenize(U"it's well-being");
    std::vector<std::u32string> expected = {U"it", U"'s", U"well", U"-", U"being"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 2: TOTAL_JOIN rule keeps separator and both sides
    tokens = tokenizer.tokenize(U"ratio:1");
    expected = {U"ratio:1"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 3: TOTAL_JOIN with empty right side
    tokens = tokenizer.tokenize(U"ratio:");
    expected = {U"ratio", U":"};  // Empty right, so split
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 4: Complex combination
    tokens = tokenizer.tokenize(U"the ratio:5 and it's well-being");
    expected = {U"the", U"ratio:5", U"and", U"it", U"'s", U"well", U"-", U"being"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    return true;
}

REGISTER_TEST(tokenizer_base_edge_cases_test)
{
    Rules rules = {
        {U'-', Rule::KEEP_WITH_LEFT}
    };

    Tokenizer tokenizer(&rules, nullptr);

    // Test 1: Empty string
    auto tokens = tokenizer.tokenize(U"");
    ASSERT_EQUALS(true, tokens.empty());

    // Test 2: Only whitespace
    tokens = tokenizer.tokenize(U"   ");
    ASSERT_EQUALS(true, tokens.empty());

    // Test 3: Single word
    tokens = tokenizer.tokenize(U"hello");
    std::vector<std::u32string> expected = {U"hello"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));
    ASSERT_EQUALS(true, tokens[0].first);

    // Test 4: Special character at start
    tokens = tokenizer.tokenize(U"-word");
    expected = {U"-", U"word"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 5: Special character at end
    tokens = tokenizer.tokenize(U"word-");
    expected = {U"word-"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 6: Only special character
    tokens = tokenizer.tokenize(U"-");
    expected = {U"-"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    // Test 7: Multiple consecutive special characters
    tokens = tokenizer.tokenize(U"-word---");
    expected = {U"-", U"word-", U"-", U"-"};
    ASSERT_EQUALS(true, tokensEqual(tokens, expected));

    return true;
}

} // namespace phonemis::test
