#pragma once

#include "phonemizer.h"

#include <string>
#include <string_view>
#include <unordered_map>

namespace phonemis::phonemizer {

/**
 * A lexicon based phonemizer, which utilizes external data file with
 * precomputed phonemizations.
 */
class LexiconPhonemizer : public Phonemizer {
public:
  LexiconPhonemizer(const std::string& lexicon_filepath);

  // Base class overrides
  std::optional<std::u32string> phonemize(const Token& token) const override;
  void update_context(std::span<const Token> tokens, size_t next_token_id) override;

  /**
   * Perform a lexicon lookup for the given word and optional context.
   * If a context is provided, the key "word|context" is used; otherwise the key is just "word".
   * Note that it requires the input to be already in UTF-8 format.
   *
   * @param word The word to look up.
   * @param context Optional stringified context appended to the word to form the lookup key.
   * @returns The found phonemization as a UTF-32 string, or an empty string if no entry was found.
   */
  std::u32string lookup(std::string_view word, std::string_view context = "") const;

protected:
  // A (word | context) -> phonemes mapping.
  // We store both keys and values as utf-8 strings to save memory
  // comparing to u32string keys and/or values.
  std::unordered_map<std::string, std::string> dict_;
};

} // namespace phonemis::phonemizer