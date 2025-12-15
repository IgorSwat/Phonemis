#pragma once

#include "types.h"
#include <optional>
#include <phonemis/tagger/tag.h>
#include <string>
#include <unordered_map>

namespace phonemis::phonemizer {

// Lexicon class
// Provides phonemization of extracted tokens.
// Wrapps a dictionary lookup for given word with additional pre/post-processing.
class Lexicon {
public:
  Lexicon(Lang language, const std::string& dict_filepath);

  // Checks if given world exists in the lexicon in any form
  bool is_known(const std::string& word) const;

private:
  // Helper functions - dictionary lookup with stressing
  std::u32string lookup(const std::u32string& word,
                        const tagger::Tag& tag,
                        std::optional<float> stress = std::nullopt);

  // Resolved language
  Lang language_;

  // Lookup dictionary: text -> phonemes
  // Provide quick and direct phonemization for popular words.
  std::unordered_map<std::string, std::u32string> dict_ = {};
};

} // namespace phonemis::phonemizer