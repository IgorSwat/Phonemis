#include <phonemis/preprocessor/tools.h>
#include <phonemis/preprocessor/constants.h>
#include <phonemis/preprocessor/num2word.h>
#include <phonemis/utilities/string_utils.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <codecvt>
#include <string>

namespace phonemis::preprocessor {

// Sentence splitting implementation
std::vector<std::string> split_sentences(const std::string& text) {
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

// Verbalizing numbers implementation
std::string verbalize_numbers(const std::string& text) {
	// Dynamically expanded text after numbers get converted
	std::string expanded_text = "";

	// Main loop
	// In each step we search for the next number-like substring and convert it.
	size_t i = 0;
	while (i < text.size()) {
		// Check if current position starts a number
		bool is_number = std::isdigit(static_cast<unsigned char>(text[i]));
		if (!is_number && text[i] == '-' && i + 1 < text.size()) {
			is_number = std::isdigit(static_cast<unsigned char>(text[i + 1]));
		}

		// If not a number, append character and continue
		if (!is_number) {
			expanded_text += text[i];
			++i;
			continue;
		}

		// Parse the number string
		size_t start_index = i;
		if (text[i] == '-') ++i;
		
		while (i < text.size()) {
			if (std::isdigit(static_cast<unsigned char>(text[i]))) {
				++i;
			} else if (text[i] == '.' && i + 1 < text.size() && 
								std::isdigit(static_cast<unsigned char>(text[i + 1]))) {
				// Floating point detected
				++i; // consume dot
				while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) {
					++i;
				}
				break;
			} else {
				break;
			}
		}
		std::string number_str = text.substr(start_index, i - start_index);

		// Check for suffixes (Currency or Ordinal)
		bool currency_found = false;
		std::string currency_word;
		size_t suffix_len = 0;

		// 1. Check currency
		for (const auto& [symbol, words] : constants::kAvailableCurrencies) {
			std::string utf8_symbol = utilities::conversions::char32_to_utf8(symbol);
			if (text.substr(i, utf8_symbol.size()) == utf8_symbol) {
				currency_found = true;
				currency_word = words.first;
				suffix_len = utf8_symbol.size();
				break;
			}
		}

		// 2. Check ordinal suffix (only if no currency)
		bool ordinal_found = false;
		if (!currency_found) {
			for (const auto& suffix : constants::kOrdinalSuffixes) {
				if (text.substr(i, suffix.size()) == suffix) {
					ordinal_found = true;
					suffix_len = suffix.size();
					break;
				}
			}
		}

		// Convert and append
		if (ordinal_found) {
			expanded_text += num2words::convert<num2words::ConversionMode::ORDINAL>(number_str);
			i += suffix_len;
		} else {
			expanded_text += num2words::convert<num2words::ConversionMode::CARDINAL>(number_str);
			
			if (currency_found) {
				expanded_text += " " + currency_word;
				
				// Pluralize if abs(val) >= 2
				if (std::abs(std::stod(number_str)) >= 2.0) {
					expanded_text += "s";
				}

				i += suffix_len;
			}
		}
	}

	return expanded_text;
}

} // namespace phonemis::preprocessor