#pragma once

#include "token.h"
#include "types.h"

#include <vector>

namespace phonemis::tokenizer {

class Tokenizer {
public:
  Tokenizer(const split::Rules* split_rules = nullptr, 
            const split::Exceptions* split_exceptions = nullptr);
	virtual ~Tokenizer() = default;

	/**
	 * Tokenizes to full words with regard to given special-char splitting rules.
	 * 
	 * @param input input text to be tokenized.
	 * @returns a token vector for given input text.
	 */
	virtual std::vector<Token> tokenize(std::u32string_view input) const;

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
	void process_phrase(std::u32string_view phrase, std::vector<Token>& token_vec) const;

	/**
	 * Processes a single separable chunk which contains no hared separators.
	 * 
	 * @param chunk a chunk - input substring with no hard seperators to be processed.
	 * @param tokenVec a token vector to add results to.
	 */
	void process_chunk(std::u32string_view chunk, std::vector<Token>& token_vec) const;

	/**
	 * Helper function to obtain a rule for given special character.
	 * Returns the DEFAULT_RULE if no matching rule was found.
	 * 
	 * @param c a (special) character to find a rule for.
	 * @returns a rule for given character.
	 */
	split::Rule get_rule(char32_t c) const;

	/**
	 * Returns true if given character has defined splitting rule.
	 */
	bool is_soft_separator(char32_t c) const;

	/**
	 * Returns true if given character is a special character without
	 * splitting rule defined.
	 * This type of character always splits the two adjacent phrases.
	 */
	bool is_hard_separator(char32_t c) const;

	/**
	 * Returns true if given word exists in exception set and should not be splitted furthermore.
	 */
	bool is_exception(const std::u32string& word) const;

	const split::Rules* rules_ = nullptr;
	const split::Exceptions* exceptions_ = nullptr;
};

} // namespace phonemis::tokenizer