#include <phonemize/preprocessor/text2sentences.h>
#include <algorithm>
#include <string>
#include <cctype> // added for std::isspace

namespace phonemize::preprocessor::text2sentences {

// Sentence splitting implementation
std::vector<std::string> split(const std::string& text) {
    std::vector<std::string> sentences;

    // Helper to check if a character ends a sentence
    auto is_end_char = [&](char c) {
        return std::find(constants::kEndOfSentenceCharacters.begin(),
                         constants::kEndOfSentenceCharacters.end(),
                         c) != constants::kEndOfSentenceCharacters.end();
    };

    size_t text_length = text.size();
    size_t index = 0;

    // Read through the text, splitting into sentences
    while (index < text_length) {
        // Start of the current sentence
        size_t start_index = index;

        // Advance until we find an end-of-sentence character or reach end
        while (index < text_length && !is_end_char(text[index])) {
            ++index;
        }

        // If we reached end of text without an end char, push the remainder
        if (index >= text_length) {
            sentences.emplace_back(text.substr(start_index));
            break;
        }

        // Consume all consecutive end-of-sentence characters (e.g. "..." or "?!")
        while (index < text_length && is_end_char(text[index])) {
            ++index;
        }

        // Consume following whitespace so it stays with the current sentence
        while (index < text_length && std::isspace(static_cast<unsigned char>(text[index]))) {
            ++index;
        }

        // Add the sentence including trailing end characters and following whitespace
        sentences.emplace_back(text.substr(start_index, index - start_index));
    }

    return sentences;
}

} // namespace phonemize::preprocessor::text2sentences