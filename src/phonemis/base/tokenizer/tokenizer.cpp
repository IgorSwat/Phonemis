#include "tokenizer.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::tokenizer {

using namespace utils;

Tokenizer::Tokenizer(const split::Rules* splitRules,
					           const split::Exceptions* splitExceptions)
	: rules_(splitRules), exceptions_(splitExceptions) {}

std::vector<Token>
Tokenizer::tokenize(std::u32string_view input) const {
  // A resulting list of tokens
	std::vector<Token> tokens;
  tokens.reserve(input.size() / 5);  // Reserve space to reduce reallocations

  // A dynamic buffer to collect characters to be processed.
  // Since we try to avoid copying strings, we represent the buffer as a view
  // parametrized by offset (start) and length (len), similarly to string_view.
	size_t currw_offset = 0, currw_len = 0;

  for (size_t idx = 0; idx < input.size(); ++idx) {
    char32_t c = input[idx];

    if (unicode::isspace(c)) {
      // We assume the input gor preprocessed in a correct way (i.e. with TrimLayer)
      // and there cannot be 2 or more consecutive white characters.

      // If we have a pending word, process it
			if (currw_len > 0) {  // equivalent to !curr_word.empty()
				processPhrase(std::u32string_view(input.data() + currw_offset, currw_len), tokens);
				currw_len = 0;  // resets the current word view

				// The right-most token always gets the trailing white space flag
				tokens.back().whitespace = true;
			}
    } else {
      currw_offset = currw_len == 0 ? idx : currw_offset;
      currw_len++;
    }
  }

  // Process last remaining word
  if (currw_len > 0) {
    processPhrase(std::u32string_view(input.data() + currw_offset, currw_len), tokens);
  }

  // Add a mark for the first token in the sequence
  if (!tokens.empty()) {
    tokens.front().first = true;
  }

  return tokens;
}

void Tokenizer::processPhrase(std::u32string_view word, 
                              std::vector<Token>& tokenVec) const {
  // Similarly to tokenize() implementation, we represent string view with
  // offset and length. We avoid copying by producing subviews into `word`.
  size_t currw_offset = 0, currw_len = 0;

  for (size_t i = 0; i < word.size(); ++i) {
    char32_t c = word[i];

    // Hard seperator basically means the subphrases needs to be separated anyway,
    // with the separator being a separate token.
    if (isHardSeparator(c)) {
      if (currw_len > 0) {
        processChunk(std::u32string_view(word.data() + currw_offset, currw_len), tokenVec);
        currw_len = 0;
      }

      // Push separator as its own token (single-char view)
      tokenVec.push_back(Token{std::u32string(word.data() + i, 1)});
    } else {
      currw_offset = (currw_len == 0) ? i : currw_offset;
      ++currw_len;
    }
  }

  if (currw_len > 0) {
    processChunk(std::u32string_view(word.data() + currw_offset, currw_len), tokenVec);
  }
}

void Tokenizer::processChunk(std::u32string_view chunk, 
                             std::vector<Token>& tokenVec) const {
  // Edge case - an empty chunk/word ("")
	if (chunk.empty()) return;

	// Special word set lookup
	// If an entire chunk is a special word/phrase, we should return it without
	// further divisions.
  // Note that the lookup is not case sensitive.
	if (isException(strings::to_lower(chunk))) {
		tokenVec.push_back({std::u32string(chunk)});
		return;
	}

	// Find first special character
	size_t special_pos = std::string::npos;
	split::Rule rule = split::DEFAULT_RULE;

	auto it = std::find_if(chunk.begin(), chunk.end(), 
												 [](char32_t c) { return !unicode::isalnum(c); });
	if (it != chunk.end()) {
		special_pos = std::distance(chunk.begin(), it);
		rule = getRule(*it);
	}

	// If no special character found, it's a simple token (an entire word)
	if (special_pos == std::string::npos) {
		tokenVec.push_back({std::u32string(chunk)});
		return;
	}

	// If special character was found, then apply rules and divide into subwords
	std::u32string_view left = chunk.substr(0, special_pos);
	std::u32string_view right = chunk.substr(special_pos + 1);
	char32_t special_char = chunk[special_pos];
	std::u32string_view special_str(chunk.data() + special_pos, 1);

	switch (rule) {
		case split::Rule::JOIN_LEFT:
			// xyz'abc -> xyz, 'abc (if xyz not empty)
			// if xyz empty -> 'abc
			if (!left.empty()) {
				processChunk(left, tokenVec);
				processChunk(chunk.substr(special_pos), tokenVec);
			} else {
				// Be careful for dots, as they are theoretically both soft and hard characters
				size_t next_dot = chunk.find(U'.');
				if (next_dot == std::u32string_view::npos)
					tokenVec.push_back({std::u32string(chunk)});
				else {
					processChunk(chunk.substr(0, next_dot), tokenVec);
					processChunk(chunk.substr(next_dot), tokenVec);
				}
			}
			break;

		case split::Rule::JOIN_RIGHT:
			// xyz-abc -> xyz-, abc (unless abc empty)
			if (!right.empty()) {
				processChunk(chunk.substr(0, special_pos + 1), tokenVec);
				processChunk(right, tokenVec);
			} else {
				tokenVec.push_back({std::u32string(chunk)});
			}
			break;

		case split::Rule::TOTAL_JOIN:
			// xyz:abc -> xyz:abc (unless abc empty -> xyz, :)
			if (!right.empty()) {
				// Treat as one word (join from both sides)
				tokenVec.push_back({std::u32string(chunk)});
			} else {
				// xyz: -> xyz, :
				processChunk(left, tokenVec);
				tokenVec.push_back({std::u32string(special_str)});
			}
			break;

		case split::Rule::TOTAL_DIVIDE:
			// xyz.abc -> xyz, ., abc
			if (!left.empty()) processChunk(left, tokenVec);
			tokenVec.push_back({std::u32string(special_str)});
			if (!right.empty()) processChunk(right, tokenVec);
			break;
	}
}

split::Rule Tokenizer::getRule(char32_t c) const {
  return rules_ && rules_->contains(c) ?
         rules_->at(c) : split::DEFAULT_RULE;
}

bool Tokenizer::isSoftSeparator(char32_t c) const {
  return rules_ && rules_->contains(c);
}

bool Tokenizer::isHardSeparator(char32_t c) const {
  return !unicode::isalnum(c) && !isSoftSeparator(c);
}

bool Tokenizer::isException(std::u32string_view word) const {
  return exceptions_ && exceptions_->contains(word);
}

} // namespace phonemis::tokenizer