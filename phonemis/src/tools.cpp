#include <phonemis/preprocessor/tools.h>
#include <phonemis/preprocessor/constants.h>
#include <phonemis/preprocessor/num2word.h>
#include <phonemis/utilities/string_utils.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <regex>
#include <string>

namespace phonemis::preprocessor {

// Sentence splitting implementation
std::vector<std::string> split_sentences(const std::string& text) {
	std::vector<std::string> sentences;

	size_t text_length = text.size();
	auto it = text.begin();	// Use iterators & STL algorithms to reduce the code size

	// Read through the text, splitting into sentences
	while (it != text.end()) {
		// First, find the end-of-sentence character
		auto end_char_it = std::find_if(it, text.end(), [](char c) -> bool { 
			return constants::kEndOfSentenceCharacters.contains(c);
		});

		// Then consume all the consecutive end-of-sentence characters first and then the white spaces
		if (end_char_it != text.end()) {
			end_char_it = std::find_if(end_char_it + 1, text.end(), [](char c) -> bool {
				return !constants::kEndOfSentenceCharacters.contains(c);
			});
			end_char_it = std::find_if(end_char_it, text.end(), [](char c) -> bool {
				return !std::isspace(c);
			});
		}

		// Now extract the sentence and move on
		sentences.emplace_back(it, end_char_it);	// Calls the appropriate range-based constructor
		it = end_char_it;
	}

	return sentences;
}

// Verbalizing numbers implementation
std::string verbalize_numbers(const std::string& text) {
	std::string output;

	// Since verbalizing the number creates at least as many characters
	// as before, we can reserve some space to optmize the creation of the output
	output.reserve(text.size());

	// Define regex for int or float number
	std::regex number_regex(R"([+-]?(?:\d+\.\d+|\.\d+|\d+))");

	std::sregex_iterator reg_it(text.begin(), text.end(), number_regex), last;
	size_t last_pos = 0;

	for (; reg_it != last; reg_it++) {
		// Find the next match (next number)
		const std::smatch& match = *reg_it;

		auto match_begin = text.begin() + match.position();
		auto match_end = match_begin + match.length();
		auto p_dist = std::distance(text.begin(), match_end);

		// Copy all the characters before the match
		output.append(text.begin() + last_pos, match_begin);

		// Check for suffixes (currency or ordinal)
		bool currency_found = false;
		std::string currency_word;
		size_t suffix_len = 0;

		// 1. Check currency
		for (const auto& [symbol, words] : constants::kAvailableCurrencies) {
			std::string utf8_symbol = utilities::string_utils::char32_to_utf8(symbol);
			if (text.substr(p_dist, utf8_symbol.size()) == utf8_symbol) {
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
				if (text.substr(p_dist, suffix.size()) == suffix) {
					ordinal_found = true;
					suffix_len = suffix.size();
					break;
				}
			}
		}

		// Convert and append
		if (ordinal_found) {
			output.append(num2words::convert<num2words::ConversionMode::ORDINAL>(match.str()));
		} else {
			output.append(num2words::convert<num2words::ConversionMode::CARDINAL>(match.str()));
			
			if (currency_found) {
				output += " " + currency_word;
				
				// Pluralize if abs(val) >= 2
				if (std::abs(std::stod(match.str())) >= 2.0) {
					output += "s";
				}
			}
		}

		last_pos = p_dist + suffix_len;
	}

	// Add the remaining characters
	output.append(text.begin() + last_pos, text.end());

	return output;
}

} // namespace phonemis::preprocessor