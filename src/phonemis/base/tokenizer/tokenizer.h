#pragma once

#include "token.h"
#include "types.h"

#include <vector>

namespace phonemis::tokenizer {

class Tokenizer {
public:
  Tokenizer(const split::Rules* splitRules = nullptr, 
            const split::Exceptions* splitExceptions = nullptr);

	/**
	 * Tokenizes to full words with regard to given special-char splitting rules.
	 * 
	 * @param input input text to be tokenized.
	 * @returns a token vector for given input text.
	 */
	std::vector<Token> tokenize(std::u32string_view input) const;

private:
	/**
	 * Processes a single phrase, which could either be a single word,
	 * or multiple words concatenated with a special character.
	 * The special character can be either soft (and then we process it together with left and right side)
	 * or hard (and then we have an immediate separation).
	 * 
	 * @param phrase a phrase - input substring to be processed.
	 * @param tokenVec a token vector to add results to.
	 */
	void processPhrase(std::u32string_view phrase, std::vector<Token>& tokenVec) const;

	/**
	 * Processes a single separable chunk which contains no hared separators.
	 * 
	 * @param chunk a chunk - input substring with no hard seperators to be processed.
	 * @param tokenVec a token vector to add results to.
	 */
	void processChunk(std::u32string_view chunk, std::vector<Token>& tokenVec) const;

	/**
	 * Helper function to obtain a rule for given special character.
	 * Returns the DEFAULT_RULE if no matching rule was found.
	 * 
	 * @param c a (special) character to find a rule for.
	 * @returns a rule for given character.
	 */
	split::Rule getRule(char32_t c) const;

	/**
	 * Returns true if given character has defined splitting rule.
	 */
	bool isSoftSeparator(char32_t c) const;

	/**
	 * Returns true if given character is a special character without
	 * splitting rule defined.
	 * This type of character always splits the two adjacent phrases.
	 */
	bool isHardSeparator(char32_t c) const;

	/**
	 * Returns true if given word exists in exception set and should not be splitted furthermore.
	 */
	bool isException(std::u32string_view word) const;

	const split::Rules* rules_ = nullptr;
	const split::Exceptions* exceptions_ = nullptr;
};

} // namespace phonemis::tokenizer