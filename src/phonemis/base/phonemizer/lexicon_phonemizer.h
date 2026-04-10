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
  // A shortcut for base class calls
  using Base = phonemizer::Phonemizer;
  using Base::phonemize;

  LexiconPhonemizer(const std::string& lexicon_filepath);

  // Base class overrides
  std::optional<std::u32string> phonemize(const Token& token) const override;
  void update_context(size_t curr_token_id, std::span<const Token> tokens) override;

  /**
   * Perform a lexicon lookup for the given word and optional context.
   * If a context is provided, the key "word|context" is used; otherwise the key is just "word".
   * Note that it requires the input to already be in UTF-8 format.
   *
   * @param word The word to look up.
   * @param context Optional stringified context appended to the word to form the lookup key.
   * @returns The found phonemization as a UTF-32 string, or an empty string if no entry was found.
   */
  std::u32string lookup(const std::string& word, std::string_view context = "") const;

  /**
   * A lookup() method variant for UTF-32 string input instead of UTF-8.
   * Compared to the other lookup() overload, it applies one additional conversion to UTF-8.
   * @param word The word to look up.
   * @param context Optional stringified context appended to the word to form the lookup key.
   * @returns The found phonemization as a UTF-32 string, or an empty string if no entry was found.
   */
  std::u32string lookup(const std::u32string& word, std::string_view context = "") const;

  /**
   * Checks if the entry for given word exists in a dict, regardless of the context.
   * Matches the exact given key (word).
   *
   * @param word The word to check in UTF-8.
   * @returns True if the word is known, false otherwise.
   */
  bool is_known(const std::string& word) const;

  /**
   * UTF-32 variant of the is_known() method.
   * 
   * @param word The word to check in UTF-32.
   * @param try_lowercase If set to true, it will check the lowercase word as a fallback.
   * @returns True if the word is known, false otherwise.
   */
  bool is_known(const std::u32string& word, bool try_lowercase = true) const;

protected:
  // A (word | context) -> phonemes mapping.
  // We store both keys and values as utf-8 strings to save memory
  // comparing to u32string keys and/or values.
  std::unordered_map<std::string, std::string> dict_;
};

} // namespace phonemis::phonemizer