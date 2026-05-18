#include "test.h"
#include <phonemis/base/tokenizer/tokenizer.h>
#include <phonemis/base/tokenizer/types.h>
#include <phonemis/lang/hi/constants.h>
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

REGISTER_TEST(tokenizer_special_hindi_purna_viram_test)
{
    // The Hindi Purna Viram (U+0964) is a hard separator.
    // It should behave like '.' or '?' in English.
    
    Tokenizer tokenizer(&phonemis::hi::constants::tokenizer::kSpecialCharacters,
                        &phonemis::hi::constants::tokenizer::kExceptions);

    // Test text provided by user
    std::u32string hindi_text = U"आज सुबह मौसम बहुत सुहावना था और हल्की ठंडी हवा चल रही थी। मैं अपने दोस्तों के साथ पार्क में घूमने गया और वहाँ हमने काफी समय बिताया।";
    
    TokenList tokens = tokenizer.tokenize(hindi_text);
    
    // Expected tokens split by whitespace and the Purna Viram (।)
    std::vector<std::u32string> expected = {
        U"आज", U"सुबह", U"मौसम", U"बहुत", U"सुहावना", U"था", U"और", U"हल्की", U"ठंडी", U"हवा", U"चल", U"रही", U"थी", U"।",
        U"मैं", U"अपने", U"दोस्तों", U"के", U"साथ", U"पार्क", U"में", U"घूमने", U"गया", U"और", U"वहाँ", U"हमने", U"काफी", U"समय", U"बिताया", U"।"
    };

    // Note: The specific splitting of "घूमने" vs "घूूमने" depends on unicode normalization, 
    // but we expect the Purna Viram to be a standalone token.
    
    ASSERT_EQUALS(expected, tokens);
    
    // Check if the Purna Viram was correctly identified as a separator (whitespace=true for the word before it)
    // "थी।" -> "थी" (token 12) should have whitespace=false, "।" (token 13) should have whitespace=true (since there is a space after it)
    ASSERT_EQUALS(false, tokens[12].whitespace);
    ASSERT_EQUALS(true, tokens[13].whitespace);
    ASSERT_EQUALS(true, tokens[14].first); // "मैं" is the first token of the next sentence segment

    return true;
}

} // namespace phonemis::test
