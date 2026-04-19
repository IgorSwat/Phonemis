#pragma once

#include "../types.h"

#include <optional>
#include <string>
#include <unordered_map>

namespace phonemis::phonemizer {

// A complete configuration for phonemization stage of the pipeline.
// By default it targets the hybrid phonemization method.
struct Config {
  /**
   * Language information. Some phonemizers use it to adjust the
   * phonemization for different dialects.
   */
  Lang lang;

  /**
   * Path to the lexicon file for dictionary-based phonemization (LexiconPhonemizer class).
   * @details If not provided, lexicon lookup is disabled.
   */
  std::optional<std::string> lexicon_filepath = std::nullopt;

  /**
   * Path to the model weights for neural phonemization (NeuralPhonemizer class).
   * @details If not provided, neural-based phonemization is disabled.
   */
  std::optional<std::string> nn_model_filepath = std::nullopt;

  /**
   * Optional pointers to maps for neural phonemizer tokenization.
   * If provided, they override default mappings.
   */
  const std::unordered_map<char32_t, int64_t>* nn_grapheme_mapping = nullptr;
  const std::unordered_map<char32_t, int64_t>* nn_phone_mapping = nullptr;
};

} // namespace phonemis::phonemizer