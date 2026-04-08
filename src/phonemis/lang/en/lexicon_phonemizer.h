#pragma once

#include <phonemis/base/phonemizer/lexicon_phonemizer.h>
#include <phonemis/base/tagger/types.h>
#include <phonemis/base/types.h>

#include <optional>
#include <string>

namespace phonemis::en {

using tagger::Tag;

/**
 * Word context definition for english (en-us & en-gb).
 * This is a word-level context which helps in lookups & postprocessing.
 */
struct LocalContext {
  std::optional<Tag> tag = std::nullopt;
  std::optional<float> stress = std::nullopt;
};

/**
 * A global context - affected by the surrounding words and stored as a class member.
 */
struct GlobalContext {
  std::optional<bool> vowel_next = std::nullopt;
  bool future_to = false;
};

class LexiconPhonemizer : public phonemizer::LexiconPhonemizer<LocalContext> {
public:
  LexiconPhonemizer(Lang lang, const std::string& lexicon_filepath);

private:
  // Configuration fields - set up during initialization.
  bool british_;

  GlobalContext gcontext_;
};


} // phonemis::en