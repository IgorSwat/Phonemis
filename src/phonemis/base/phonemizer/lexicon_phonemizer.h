#pragma once

#include "phonemizer.h"
#include <phonemis/utils/io.h>

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace phonemis::phonemizer {

/**
 * A lexicon based phonemizer, which utilizes external data file with
 * precomputed phonemizations.
 * 
 * It is parametrized by the word context, which serves partially as a lookup key.
 */
template <typename LookupContext>
class LexiconPhonemizer : public Phonemizer {
public:
  LexiconPhonemizer(const std::string& lexicon_filepath) {
    auto json_obj = utils::io::load_json(lexicon_filepath);

    // We assume the lexicon has a strict string -> string structure.
    for (auto& item : json_obj.items()) {
      std::string key = item.key(); // `word` or `word|context`
      auto value = item.value();

      if (!value.is_string()) {
        throw std::runtime_error("Lexicon phonemizer expects a string-to-string JSON structure.");
      }

      dict_[key] = value.get<std::string>();
    }
  }

  virtual std::u32string lookup(std::u32string_view word, const LookupContext& ctx) = 0;

protected:
  // A (word | context) -> phonemes mapping.
  // We store both keys and values as utf-8 strings to save memory
  // comparing to u32string keys and/or values.
  std::unordered_map<std::string, std::string> dict_;
};

} // namespace phonemis::phonemizer