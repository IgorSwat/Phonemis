#pragma once

#include <phonemis/base/tagger/hmm_tagger.h>
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/strings.h>
#include <limits>

namespace phonemis::en {

/**
 * English specific HMM Tagger.
 * Implements English-specific casing rules.
 */
class HMMTagger : public tagger::HMMTagger {
public:
  using tagger::HMMTagger::HMMTagger;

protected:
  /**
   * English-specific lowercase rules matching the training script create_tagger.py.
   * Words are lowercased unless they are likely proper nouns or the pronoun "I".
   */
  bool should_lowercase(std::u32string_view word) const override;
};

} // namespace phonemis::en
