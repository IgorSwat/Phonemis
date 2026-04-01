#include "tagger.h"
#include "constants.h"

#include <algorithm>
#include <iterator>

namespace phonemis::tagger {

void Tagger::tag(std::span<Token> tokens) const {
  auto sentence_start = tokens.begin();

  while (sentence_start != tokens.end()) {
    // And of a sentence is marked by a special, EOS character with a space following afterwards.
    auto sentence_end = std::find_if(sentence_start, tokens.end(), [](const Token& token) {
      return token.whitespace && token.text.size() == 1 &&
             constants::kEosCharacters.contains(token.text[0]);
    });

    auto next_sentence_start = (sentence_end == tokens.end()) ? 
                                tokens.end() : std::next(sentence_end);
    auto sentence_size = static_cast<size_t>(std::distance(sentence_start, next_sentence_start));

    std::span<Token> sentence = tokens.subspan(static_cast<size_t>(std::distance(tokens.begin(), sentence_start)), sentence_size);

    tagSentence(sentence);

    sentence_start = next_sentence_start;
  }
}

} // namespace phonemis::tagger