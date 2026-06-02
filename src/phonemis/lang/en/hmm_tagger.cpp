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

  for (size_t id = 0; id < emission_probs_.size(); ++id) {
    auto it = emission_probs_[id].find(utf8_lower_word);
    if (it != emission_probs_[id].end() && it->second > max_prob) {
      max_prob = it->second;
      best_tag = tag_names_[id];
    }
  }

  return best_tag != "XX" &&
         best_tag != "NNP" &&
         best_tag != "NNPS" &&
        (best_tag != "PRP" || word != U"I");
}

} // namespace phonemis::tagger::en
