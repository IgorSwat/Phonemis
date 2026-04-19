#include "tagger.h"
#include "constants.h"

#include <algorithm>
#include <iterator>

namespace phonemis::tagger {

void Tagger::tag(std::span<Token> tokens) const {
  size_t start_idx = 0;

  while (start_idx < tokens.size()) {
    // 1. Identify sentence boundaries (End-Of-Sentence character with trailing whitespace)
    auto it = std::find_if(tokens.begin() + start_idx, tokens.end(), [](const Token& token) {
      return token.whitespace && token.text.size() == 1 &&
             constants::kEosCharacters.contains(token.text[0]);
    });

    // 2. Determine span size (including the punctuation token)
    size_t end_idx = (it == tokens.end()) 
        ? tokens.size() 
        : static_cast<size_t>(std::distance(tokens.begin(), it) + 1);
    
    size_t sentence_len = end_idx - start_idx;

    // 3. Process the sentence span
    tag_sentence(tokens.subspan(start_idx, sentence_len));

    start_idx = end_idx;
  }
}

} // namespace phonemis::tagger