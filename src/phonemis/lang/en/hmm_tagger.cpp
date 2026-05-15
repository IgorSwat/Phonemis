#include "hmm_tagger.h"
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/strings.h>

#include <limits>

namespace phonemis::en {

using namespace utils;
using namespace tagger;

bool HMMTagger::should_lowercase(std::u32string_view word) const {
  std::string utf8_lower_word = conversions::u32_to_utf8(strings::to_lower(word));

  // Look for the tag with highest emission probability for this word
  Tag best_tag = "XX";
  double max_prob = -std::numeric_limits<double>::infinity();

  for (const auto& [tag, word_map] : emission_probs_) {
    auto it = word_map.find(utf8_lower_word);
    if (it != word_map.end()) {
      if (it->second > max_prob) {
        max_prob = it->second;
        best_tag = tag;
      }
    }
  }

  return best_tag != "XX" &&
         best_tag != "NNP" &&
         best_tag != "NNPS" &&
        (best_tag != "PRP" || word != U"I");
}

} // namespace phonemis::tagger::en
