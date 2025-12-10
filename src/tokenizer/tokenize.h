#pragma once

#include "types.h"
#include <string>
#include <vector>

namespace phonemize::tokenizer {

// Tokenizes the input text into a vector of strings (tokens).
// Follows specific rules for special characters and special words.
std::vector<std::string> tokenize(const std::string& text);

} // namespace phonemize::tokenizer
