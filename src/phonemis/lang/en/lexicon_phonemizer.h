#pragma once

#include <phonemis/base/phonemizer/lexicon_phonemizer.h>
#include <phonemis/base/tagger/types.h>
#include <phonemis/base/tokenizer/token.h>
#include <phonemis/base/types.h>

#include <optional>
#include <string>

namespace phonemis::en {

using tagger::Tag;
using tokenizer::Token;

/**
 * A global context - affected by the surrounding words and stored as a class member.
 */
struct GlobalContext {
  std::optional<bool> vowel_next = std::nullopt;
  bool future_to = false;
};

class LexiconPhonemizer : public phonemizer::LexiconPhonemizer {
public:
  // A shortcut for base class calls
  using Base = phonemizer::LexiconPhonemizer;
  using Base::phonemize;

  explicit LexiconPhonemizer(const phonemizer::Config& config);

  std::optional<std::u32string> phonemize(const Token& token) const override;
  void update_context(size_t curr_token_id, std::span<const Token> tokens) override;

  // Lookup specializations
  std::u32string lookup(const std::u32string& word, const Tag& tag) const; // A modified standard lookup with tag as a context.
  std::u32string lookup_and_restress(const std::u32string& word, const Tag& tag,  // Lookup + stress application in one method.
                                     std::optional<float> stress) const;
  std::u32string lookup_stem_ed(const std::u32string& word, const Tag& tag,  // Lookup with an extended capability to handle custom 'ed' suffix applications.
                                std::optional<float> stress) const;
  std::u32string lookup_stem_s(const std::u32string& word, const Tag& tag,  // Lookup with an extended capability to handle custom 's' suffix applications.
                                std::optional<float> stress) const;
  std::u32string lookup_stem_ing(const std::u32string& word, const Tag& tag,  // Lookup with an extended capability to handle custom 'ing' suffix applications.
                                std::optional<float> stress) const;
  std::u32string lookup_nnp(const std::u32string& word) const; // Specialized lookup for acronyms and initialisms
  std::u32string lookup_special(const std::u32string& word, const Tag& tag,  // A lookup for special words, with non-standard phonemization rules.
                                std::optional<float> stress,
                                std::optional<bool> vowel_next, bool future_to = false) const;

private:
  // Configuration fields - set up during initialization.
  bool british_;

  GlobalContext global_context_;
};


} // phonemis::en