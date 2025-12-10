#include "tokenize.h"
#include "constants.h"
#include <algorithm>
#include <cctype>
#include <optional>
#include <stack>

// TODO: add saving the trailing white spaces to Tokens
// TODO: modify the script to resolve the trailing dot issue

namespace phonemize::tokenizer {

// Helper functions
namespace {
	
// Helper function - convert string to lowercase
std::string to_lower(const std::string& str) {
	std::string lower = str;
	std::transform(lower.begin(), lower.end(), lower.begin(),
								 [](unsigned char c){ return std::tolower(c); });

	return lower;
}

// Helper function - special character rule lookup
std::optional<rules::Separation> get_special_char_rule(char c) {
	for (const auto& sc : constants::kSpecialCharacters) {
			if (sc.character == c) return sc.sep_rule;
	}

	return std::nullopt;
}

// Helper function - soft separator (included in kSpecialCharacters)
bool is_soft_separator(char c) {
	return get_special_char_rule(c).has_value();
}

// Helper function - hard separator (not alnum and not in kSpecialCharacters) detection
bool is_hard_separator(char c) {
	if (std::isalnum(static_cast<unsigned char>(c))) return false;
	return !get_special_char_rule(c).has_value();
}

// Processes a single chunk (subword) recursively/iteratively based on special characters
void process_chunk(const std::string& chunk, std::vector<std::string>& tokens) {
	// Edge case - an empty word ("")
	if (chunk.empty()) return;

	// Special word set lookup
	// If an entire chunk is a special word, we should return it without
	// further divisions.
	if (constants::kSpecialWords.count(to_lower(chunk))) {
			tokens.push_back(chunk);
			return;
	}

	// Find first special character
	size_t special_pos = std::string::npos;
	rules::Separation rule;

	auto it = std::find_if(chunk.begin(), chunk.end(), 
												 [](char c) { return get_special_char_rule(c).has_value(); });
	if (it != chunk.end()) {
		special_pos = std::distance(chunk.begin(), it);
		rule = get_special_char_rule(*it).value();
	}

	// If no special character found, it's a simple token (an entire word)
	if (special_pos == std::string::npos) {
			tokens.push_back(chunk);
			return;
	}

	// If special character was found, then apply rules and divide into subwords
	std::string left = chunk.substr(0, special_pos);
	std::string right = chunk.substr(special_pos + 1);
	char special_char = chunk[special_pos];
	std::string special_str(1, special_char);

	switch (rule) {
			case rules::Separation::JOIN_LEFT:
					// xyz'abc -> xyz, 'abc (if xyz not empty)
					// if xyz empty -> 'abc
					if (!left.empty()) {
							process_chunk(left, tokens);
							process_chunk(special_str + right, tokens);
					} else {
							// Be careful for dots, as they are theoretically both soft and hard characters
							size_t next_dot = chunk.find('.');
							if (next_dot == std::string::npos)
								tokens.push_back(chunk);
							else {
								process_chunk(chunk.substr(0, next_dot), tokens);
								process_chunk(chunk.substr(next_dot), tokens);
							}
					}
					break;

			case rules::Separation::JOIN_RIGHT:
					// xyz-abc -> xyz-, abc (unless abc empty)
					if (!right.empty()) {
							process_chunk(left + special_str, tokens);
							process_chunk(right, tokens);
					} else {
							tokens.push_back(chunk);
					}
					break;

			case rules::Separation::TOTAL_JOIN:
					// xyz:abc -> xyz:abc (unless abc empty -> xyz, :)
					if (!right.empty()) {
							// Treat as one word (join from both sides)
							tokens.push_back(chunk);
					} else {
							// xyz: -> xyz, :
							process_chunk(left, tokens);
							tokens.push_back(special_str);
					}
					break;

			case rules::Separation::TOTAL_DIVIDE:
					// xyz.abc -> xyz, ., abc
					if (!left.empty()) process_chunk(left, tokens);
					tokens.push_back(special_str);
					if (!right.empty()) process_chunk(right, tokens);
					break;
	}
}

// Splits a raw word by hard separators and processes resulting parts
void process_raw_word(const std::string& raw_word, std::vector<std::string>& tokens) {
	std::string current_chunk;
	for (char c : raw_word) {
		if (is_hard_separator(c)) {
				if (!current_chunk.empty()) {
						process_chunk(current_chunk, tokens);
						current_chunk.clear();
				}
				tokens.push_back(std::string(1, c));
		} else {
				current_chunk += c;
		}
	}

	if (!current_chunk.empty()) {
			process_chunk(current_chunk, tokens);
	}
}

} // namespace

std::vector<std::string> tokenize(const std::string& text) {
	// A resulting list of tokens
	std::vector<std::string> tokens;

	// A dynamic buffer to collect characters to be processed
	std::string current_word;

	for (char c : text) {
			if (std::isspace(static_cast<unsigned char>(c))) {
					if (!current_word.empty()) {
							process_raw_word(current_word, tokens);
							current_word.clear();
					}
			} else {
					current_word += c;
			}
	}

	// One final processing when hitting the end of the text
	if (!current_word.empty()) {
			process_raw_word(current_word, tokens);
	}

	return tokens;
}

} // namespace phonemize::tokenizer
