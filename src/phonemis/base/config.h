#pragma once

#include "phonemizer/config.h"
#include "tagger/config.h"
#include "types.h"

#include <optional>
#include <string>

namespace phonemis {

// A general and complete configuration for any type of pipeline within the library.
struct Config {
  /**
   * Language profile for phonemization (e.g., @ref 'en-us' for American English).
   */
  Lang lang;

  /**
   * Tagger subconfiguration - optional (unused by some languages).
   */
  std::optional<tagger::Config> tagger;

  /**
   * Phonemizer subconfiguration - required.
   */
  phonemizer::Config phonemizer;
};

} // namespace phonemis