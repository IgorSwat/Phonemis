#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace phonemis::phonemizer::nn {

/**
 * A helper class for character-by-character tokenization of given word.
 * 
 * Note that it supports both hardcoded mappings (and uses one by default)
 * and external .json file mappings.
 */
class Tokenizer {
public:
    Tokenizer();
    explicit Tokenizer(const std::unordered_map<char32_t, int64_t>& mapping);
    explicit Tokenizer(const std::string& filepath);

    /**
     * Transforms input word to a vector of tokens, where each token corresponds to 
     * exactly one character.
     * Omits unrecognizable characters, which means number of tokens <= number of characters.
     * 
     * @param text The input UTF-32 string to tokenize.
     * @return A vector of token IDs representing the input text.
     */
    std::vector<int64_t> tokenize(std::u32string_view text) const;

    /**
     * A reverse transformation - from tokens back to characters.
     * 
     * @param tokens The vector of token IDs to decode.
     * @return The resulting UTF-32 string.
     */
    std::u32string decode(const std::vector<int64_t>& tokens) const;

private:
    // Creates token_to_char map from existing char_to_token map.
    void initialize_reverse_map();

    // Use forward & reverse mappings to speed up the tokenization process.
    std::unordered_map<char32_t, int64_t> char_to_token_;
    std::unordered_map<int64_t, char32_t> token_to_char_;
};

} // namespace phonemis::phonemizer::nn
