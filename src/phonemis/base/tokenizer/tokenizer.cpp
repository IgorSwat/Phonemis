#include "tokenizer.h"
#include "constants.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::tokenizer {

using namespace utils;

Tokenizer::Tokenizer(const split::Rules* split_rules,
					           const split::Exceptions* split_exceptions)
	: rules_(split_rules), exceptions_(split_exceptions) {}

std::vector<Token>
Tokenizer::tokenize(std::u32string_view input) const {
  // A resulting list of tokens
	std::vector<Token> tokens;
  tokens.reserve(input.size() / 5);

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
				process_phrase(std::u32string_view(input.data() + currw_offset, currw_len), tokens);
				currw_len = 0;  // resets the current word view

				// The right-most token always gets the trailing white space flag, except for the last token in the sequence
				tokens.back().whitespace = true;
			}
    } else {
      currw_offset = currw_len == 0 ? idx : currw_offset;
      currw_len++;
    }
  }

  // Process last remaining word
  if (currw_len > 0) {
    process_phrase(std::u32string_view(input.data() + currw_offset, currw_len), tokens);
  }

	// Mark beginning tokens in each sentence.
  bool next_is_first = true;
  for (auto& token : tokens) {
    if (next_is_first) {
      token.first = true;
      next_is_first = false;
    }

    if (token.text.size() == 1 && is_hard_separator(token.text[0]) &&
        constants::kEosCharacters.contains(token.text[0])) {
      next_is_first = true;
    }
  }

  return tokens;
}

void Tokenizer::process_phrase(std::u32string_view word, 
                               std::vector<Token>& token_vec) const {
  // Similarly to tokenize() implementation, we represent string view with
  // offset and length. We avoid copying by producing subviews into `word`.
  size_t currw_offset = 0, currw_len = 0;

  for (size_t i = 0; i < word.size(); ++i) {
    char32_t c = word[i];

    // Hard seperator basically means the subphrases needs to be separated anyway,
    // with the separator being a separate token.
    if (is_hard_separator(c)) {
      if (currw_len > 0) {
        process_chunk(std::u32string_view(word.data() + currw_offset, currw_len), token_vec);
        currw_len = 0;
      }

      // Push separator as its own token (single-char view)
      token_vec.push_back(Token{std::u32string(word.data() + i, 1)});
    } else {
      currw_offset = (currw_len == 0) ? i : currw_offset;
      ++currw_len;
    }
  }

  if (currw_len > 0) {
    process_chunk(std::u32string_view(word.data() + currw_offset, currw_len), token_vec);
  }
}

void Tokenizer::process_chunk(std::u32string_view chunk, 
                              std::vector<Token>& token_vec) const {
  // Edge case - an empty chunk/word ("")
	if (chunk.empty()) return;

	// Special word set lookup
	// If an entire chunk is a special word/phrase, we should return it without
	// further divisions.
  // Note that the lookup is not case sensitive.
	if (exceptions_ && is_exception(strings::to_lower(chunk))) {
		token_vec.push_back({std::u32string(chunk)});
		return;
	}

	// Find first special character
	size_t special_pos = std::u32string_view::npos;
	split::Rule rule = split::Rule::TOTAL_DIVIDE;
	auto it = std::find_if(chunk.begin(), chunk.end(), 
												 [](char32_t c) { return !unicode::isalnum(c); });
	if (it != chunk.end()) {
		special_pos = std::distance(chunk.begin(), it);
		rule = get_rule(*it);
	}

	// If no special character found, it's a simple token (an entire word)
	if (special_pos == std::u32string_view::npos) {
		token_vec.push_back({std::u32string(chunk)});
		return;
	}

	// If special character was found, then apply rules and divide into subwords
	std::u32string_view left = chunk.substr(0, special_pos);
	std::u32string_view right = chunk.substr(special_pos + 1);
	std::u32string_view special_str(chunk.data() + special_pos, 1);

	switch (rule) {
		case split::Rule::KEEP_WITH_RIGHT:
			// xyz'abc -> xyz, 'abc (if xyz not empty)
			// if xyz empty -> 'abc
			if (!left.empty()) {
				process_chunk(left, token_vec);
				process_chunk(chunk.substr(special_pos), token_vec);
			} else {
				// Be careful for dots, as they are theoretically both soft and hard characters
				size_t next_dot = chunk.find(U'.');
				if (next_dot == std::u32string_view::npos)
					token_vec.push_back({std::u32string(chunk)});
				else {
					process_chunk(chunk.substr(0, next_dot), token_vec);
					process_chunk(chunk.substr(next_dot), token_vec);
				}
			}
			break;

		case split::Rule::KEEP_WITH_LEFT:
			// xyz-abc -> xyz-, abc (unless abc empty)
			if (!right.empty()) {
				process_chunk(chunk.substr(0, special_pos + 1), token_vec);
				process_chunk(right, token_vec);
			} else {
				token_vec.push_back({std::u32string(chunk)});
			}
			break;

		case split::Rule::TOTAL_JOIN:
			// xyz:abc or xyz: -> xyz:abc / xyz:
			// Treat as one word (join from both sides)
			token_vec.push_back({std::u32string(chunk)});
			break;

		case split::Rule::TOTAL_DIVIDE:
			// xyz.abc -> xyz, ., abc
			if (!left.empty()) process_chunk(left, token_vec);
			token_vec.push_back({std::u32string(special_str)});
			if (!right.empty()) process_chunk(right, token_vec);
			break;
	}
}

split::Rule Tokenizer::get_rule(char32_t c) const {
  return rules_ && rules_->contains(c) ?
         rules_->at(c) : split::Rule::TOTAL_DIVIDE;
}

bool Tokenizer::is_soft_separator(char32_t c) const {
  return rules_ && rules_->contains(c);
}

bool Tokenizer::is_hard_separator(char32_t c) const {
  return !unicode::isalnum(c) && !unicode::isspace(c) && !is_soft_separator(c);
}

bool Tokenizer::is_exception(const std::u32string& word) const {
  return exceptions_ && exceptions_->contains(word);
}

} // namespace phonemis::tokenizer