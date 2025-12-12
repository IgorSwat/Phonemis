#pragma once

#include <string>

namespace phonemis::tokenizer {

namespace rules {
// Separation rules for special characters
enum class Separation {
	JOIN_LEFT,      // Join to the word on its left
	JOIN_RIGHT,     // Join to the word on its right
	TOTAL_DIVIDE,   // Always separate from both sides
	TOTAL_JOIN      // Always join both sides
};
} // namespace rules

struct SpecialCharacter {
	char character;
	rules::Separation sep_rule;
};

// Represents a single token extracted from text
struct Token {
	std::string text;
	std::string whitespace = ""; 	// Following whitespace
	bool is_first = false;				// Whether it is a first token in the sentence
};

} // namespace phonemis::tokenizer