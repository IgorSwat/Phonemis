#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// forward-declaration of tokenizer types etc. (optional, depending on project structure)

namespace phonemis::phonemizer::nn {

/**
 * A character-level tokenizer for NeuralPhonemizer.
 * 
 * Maps Unicode codepoints (char32_t) to integer IDs (int64_t) and vice versa.
 * Used for converting text to model inputs (encoding) and model outputs back to symbols (decoding).
 */
class Tokenizer {
public:
    Tokenizer();

    explicit Tokenizer(const std::unordered_map<char32_t, int64_t>& mapping);

    explicit Tokenizer(const std::string& filepath);

    /**
     * Transforms input text into a sequence of token IDs.
     * Character matching is exact. Characters not present in the mapping are silently ignored.
     * 
     * @param text The UTF-32 string view to tokenize.
     * @return A vector of token IDs.
     */
    std::vector<int64_t> tokenize(std::u32string_view text) const;

    /**
     * Decodes a sequence of token IDs back into a UTF-32 string.
     * Tokens not present in the reverse mapping are skipped.
     * 
     * @param tokens The vector of token IDs to decode.
     * @return The resulting decoded UTF-32 string.
     */
    std::u32string decode(const std::vector<int64_t>& tokens) const;

private:
    /**
     * Populates the token_to_char_ map based on the current contents of char_to_token_.
     * Should be called after any modification to the primary mapping.
     */
    void build_reverse_mapping();

    // Primary mapping: Character -> ID
    std::unordered_map<char32_t, int64_t> char_to_token_;
    
    // Reverse mapping: ID -> Character (used for decoding)
    std::unordered_map<int64_t, char32_t> token_to_char_;
};

} // namespace phonemis::phonemizer::nn
