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
   * Language profile for phonemization (e.g., @ref Lang::EN_US for American English).
   * @details Defaults to @ref Lang::DEFAULT.
   */
  Lang lang = Lang::DEFAULT;

  /**
   * Tagger subconfiguration.
   */
  tagger::Config tagger;

  /**
   * Phonemizer subconfiguration.
   */
  phonemizer::Config phonemizer;
};

} // namespace phonemis