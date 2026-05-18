#include "test.h"
#include <phonemis/base/tokenizer/tokenizer.h>
#include <phonemis/base/tokenizer/types.h>
#include <phonemis/utils/conversions.h>

#include <algorithm>
#include <iostream>

namespace phonemis::test {

using namespace tokenizer;
using namespace tokenizer::split;
using namespace phonemis::utils::conversions;

// Helper class to compare token vectors
struct TokenList : public std::vector<Token> {
    using std::vector<Token>::vector;

    TokenList(const std::vector<Token>& other) : std::vector<Token>(other) {}

    bool operator==(const std::vector<std::u32string>& expected) const {
        if (size() != expected.size()) return false;
        for (size_t i = 0; i < size(); ++i) {
            if ((*this)[i].text != expected[i]) return false;
        }
        return true;
    }

    friend bool operator==(const std::vector<std::u32string>& expected, const TokenList& tokens) {
        return tokens == expected;
    }

    friend std::ostream& operator<<(std::ostream& os, const TokenList& tokens) {
        os << "[";
        for (size_t i = 0; i < tokens.size(); ++i) {
            os << "\"" << u32_to_utf8(tokens[i].text) << "\"";
            if (i < tokens.size() - 1) os << ", ";
        }
        os << "]";
        return os;
    }
};

static std::ostream& operator<<(std::ostream& os, const std::vector<std::u32string>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << "\"" << u32_to_utf8(vec[i]) << "\"";
        if (i < vec.size() - 1) os << ", ";
    }
    os << "]";
    return os;
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
    TokenList tokens = tokenizer.tokenize(U"hello world");
    std::vector<std::u32string> expected = {U"hello", U"world"};
    ASSERT_EQUALS(expected, tokens);
    ASSERT_EQUALS(true, tokens[0].first);       // First token should be marked
    ASSERT_EQUALS(true, tokens[0].whitespace);  // A space after first token should be marked.
    ASSERT_EQUALS(false, tokens[1].whitespace);

    // Test 2: Soft separator with TOTAL_DIVIDE rule
    tokens = tokenizer.tokenize(U"well-being");
    expected = {U"well", U"-", U"being"};
    ASSERT_EQUALS(expected, tokens);
    ASSERT_EQUALS(false, tokens[0].whitespace);
    ASSERT_EQUALS(false, tokens[1].whitespace);

    // Test 3: Soft separator with JOIN_LEFT rule
    tokens = tokenizer.tokenize(U"don't");
    expected = {U"don", U"'t"};
    ASSERT_EQUALS(expected, tokens);
    ASSERT_EQUALS(false, tokens[0].whitespace);
    ASSERT_EQUALS(false, tokens[1].whitespace);

    // Test 4: Mixed - multiple separators
    tokens = tokenizer.tokenize(U"it's well-being");
    expected = {U"it", U"'s", U"well", U"-", U"being"};
    ASSERT_EQUALS(expected, tokens);

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
    TokenList tokens = tokenizer.tokenize(U"well-being");
    std::vector<std::u32string> expected = {U"well-being"};
    ASSERT_EQUALS(expected, tokens);

    // Test 2: Non-exception words still follow rules
    tokens = tokenizer.tokenize(U"good-bye");
    expected = {U"good", U"-", U"bye"};  // Follows TOTAL_JOIN rule
    ASSERT_EQUALS(expected, tokens);

    // Test 3: Multiple words with exceptions
    tokens = tokenizer.tokenize(U"well-being and good-bye");
    expected = {U"well-being", U"and", U"good", U"-", U"bye"};
    ASSERT_EQUALS(expected, tokens);
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
    TokenList tokens = tokenizer.tokenize(U"hello@world");
    std::vector<std::u32string> expected = {U"hello", U"@", U"world"};
    ASSERT_EQUALS(expected, tokens);

    // Test 2: Multiple hard separators
    tokens = tokenizer.tokenize(U"user#domain@server");
    expected = {U"user", U"#", U"domain", U"@", U"server"};
    ASSERT_EQUALS(expected, tokens);

    // Test 3: Mix of soft (hyphen) and hard separators (@)
    tokens = tokenizer.tokenize(U"user-name@domain-server");
    expected = {U"user-name", U"@", U"domain-server"};
    ASSERT_EQUALS(expected, tokens);

    // Test 4: Hard separator with no text after
    tokens = tokenizer.tokenize(U"email@");
    expected = {U"email", U"@"};
    ASSERT_EQUALS(expected, tokens);

    return true;
}

REGISTER_TEST(tokenizer_base_no_rules_no_exceptions_test)
{
    // No rules and no exceptions - all special characters are hard separators
    Tokenizer tokenizer(nullptr, nullptr);

    // Test 1: Simple text with special character
    TokenList tokens = tokenizer.tokenize(U"hello-world");
    std::vector<std::u32string> expected = {U"hello", U"-", U"world"};
    ASSERT_EQUALS(expected, tokens);

    // Test 2: Multiple different special characters
    tokens = tokenizer.tokenize(U"one,two;three");
    expected = {U"one", U",", U"two", U";", U"three"};
    ASSERT_EQUALS(expected, tokens);

    // Test 3: Mixed alphanumeric and special
    tokens = tokenizer.tokenize(U"user@domain.com");
    expected = {U"user", U"@", U"domain", U".", U"com"};
    ASSERT_EQUALS(expected, tokens);

    // Test 4: With whitespace
    tokens = tokenizer.tokenize(U"hello-world test");
    expected = {U"hello", U"-", U"world", U"test"};
    ASSERT_EQUALS(expected, tokens);
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
    TokenList tokens = tokenizer.tokenize(U"it's well-being");
    std::vector<std::u32string> expected = {U"it", U"'s", U"well", U"-", U"being"};
    ASSERT_EQUALS(expected, tokens);

    // Test 2: TOTAL_JOIN rule keeps separator and both sides
    tokens = tokenizer.tokenize(U"ratio:1");
    expected = {U"ratio:1"};
    ASSERT_EQUALS(expected, tokens);

    // Test 3: TOTAL_JOIN with empty right side
    tokens = tokenizer.tokenize(U"ratio:");
    expected = {U"ratio:"};  // No split
    ASSERT_EQUALS(expected, tokens);

    // Test 4: Complex combination
    tokens = tokenizer.tokenize(U"the ratio:5 and it's well-being");
    expected = {U"the", U"ratio:5", U"and", U"it", U"'s", U"well", U"-", U"being"};
    ASSERT_EQUALS(expected, tokens);

    return true;
}

REGISTER_TEST(tokenizer_base_edge_cases_test)
{
    Rules rules = {
        {U'-', Rule::KEEP_WITH_LEFT}
    };

    Tokenizer tokenizer(&rules, nullptr);

    // Test 1: Empty string
    TokenList tokens = tokenizer.tokenize(U"");
    ASSERT_EQUALS(true, tokens.empty());

    // Test 2: Only whitespace
    tokens = tokenizer.tokenize(U"   ");
    ASSERT_EQUALS(true, tokens.empty());

    // Test 3: Single word
    tokens = tokenizer.tokenize(U"hello");
    std::vector<std::u32string> expected = {U"hello"};
    ASSERT_EQUALS(expected, tokens);
    ASSERT_EQUALS(true, tokens[0].first);

    // Test 4: Special character at start
    tokens = tokenizer.tokenize(U"-word");
    expected = {U"-", U"word"};
    ASSERT_EQUALS(expected, tokens);

    // Test 5: Special character at end
    tokens = tokenizer.tokenize(U"word-");
    expected = {U"word-"};
    ASSERT_EQUALS(expected, tokens);

    // Test 6: Only special character
    tokens = tokenizer.tokenize(U"-");
    expected = {U"-"};
    ASSERT_EQUALS(expected, tokens);

    // Test 7: Multiple consecutive special characters
    tokens = tokenizer.tokenize(U"-word---");
    expected = {U"-", U"word-", U"-", U"-"};
    ASSERT_EQUALS(expected, tokens);

    return true;
}

REGISTER_TEST(tokenizer_base_foreign_characters_test)
{
    // Rules: hyphen should split
    Rules rules = {
        {U'-', Rule::TOTAL_DIVIDE}
    };

    Tokenizer tokenizer(&rules, nullptr);

    // Test 1: Polish characters (Dzieńdoberek panowie - co tam u was słychać?)
    // Note: 'ń', 'ł', 'ć' are multi-byte in UTF-8 but single char in U32
    TokenList tokens = tokenizer.tokenize(U"Dzieńdoberek panowie - co tam u was słychać?");
    std::vector<std::u32string> expected = {U"Dzieńdoberek", U"panowie", U"-", U"co", U"tam", U"u", U"was", U"słychać", U"?"};
    ASSERT_EQUALS(expected, tokens);
    ASSERT_EQUALS(true, tokens[0].first);
    ASSERT_EQUALS(true, tokens[0].whitespace); // Space after Dzieńdoberek
    ASSERT_EQUALS(false, tokens[7].whitespace);

    // Test 2: Other European characters
    tokens = tokenizer.tokenize(U"München és Kraków");
    expected = {U"München", U"és", U"Kraków"};
    ASSERT_EQUALS(expected, tokens);

    return true;
}

} // namespace phonemis::test
