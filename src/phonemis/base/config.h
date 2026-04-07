#pragma once

#include "types.h"

#include <optional>
#include <string>

namespace phonemis {

/**
 * @brief Complete configuration for any type of pipeline within the library.
 */
struct Config {
  /**
   * Language profile for phonemization (e.g., @ref Lang::EN_US for American English).
   * @details Defaults to @ref Lang::DEFAULT.
   */
  Lang lang = Lang::DEFAULT;

  /**
   * File path to Part-of-Speech (PoS) tagger data.
   * @details Required by some language implementations for disambiguation.
   * If not provided, tagging functionality is disabled.
   */
  std::optional<std::string> tagger_data_filepath = std::nullopt;

  /**
   * Path to the lexicon file for dictionary-based phonemization.
   * @details If not provided, lexicon lookup is disabled.
   */
  std::optional<std::string> phonemizer_lexicon_filepath = std::nullopt;

  /**
   * Path to the model weights for neural phonemization.
   * @details If not provided, neural-based phonemization is disabled.
   */
  std::optional<std::string> phonemizer_model_filepath = std::nullopt;
};

} // namespace phonemis