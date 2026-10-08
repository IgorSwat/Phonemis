#include "lexicon_phonemizer.h"

#include <phonemis/utils/conversions.h>

namespace phonemis::zh {

using namespace utils;

LexiconPhonemizer::LexiconPhonemizer(const phonemizer::Config& config) : Base(config) {
  for (const auto& [key, _] : dict_) {
    auto phrase = conversions::utf8_to_u32(key);
    if (phrase.size() < 2) {
      continue;
    }
    for (size_t length = 1; length <= phrase.size(); length++) {
      phrase_prefixes_.insert(conversions::u32_to_utf8(phrase.substr(0, length)));
    }
  }
}

std::u32string LexiconPhonemizer::phonemize_word(std::u32string_view word) const {
  std::u32string phonemes;
  auto read = [&](std::u32string_view part) { phonemes += lookup(std::u32string{part}); };

  while (!word.empty()) {
    // The longest phrase among the prefixes of the remaining text that start a phrase.
    size_t phrase_length = 0;
    bool all_prefix = true;
    for (size_t length = 1; length <= word.size(); length++) {
      auto prefix = conversions::u32_to_utf8(word.substr(0, length));
      if (!phrase_prefixes_.contains(prefix)) {
        all_prefix = false;
        break;
      }
      if (length > 1 && is_known(prefix)) {
        phrase_length = length;
      }
    }

    // pypinyin reads every remaining character on its own when all of them only start
    // a longer phrase.
    if (phrase_length == 0 && all_prefix) {
      for (size_t i = 0; i < word.size(); i++) {
        read(word.substr(i, 1));
      }
      break;
    }

    size_t length = phrase_length > 0 ? phrase_length : 1;
    read(word.substr(0, length));
    word.remove_prefix(length);
  }

  return phonemes;
}

} // namespace phonemis::zh
