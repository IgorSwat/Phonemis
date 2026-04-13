#pragma once

#include <optional>
#include <string>

namespace phonemis::tagger {

// Configuration for token tagging/annotation.
struct Config {
  /**
   * Path to the tagger's data resource (e.g. model weights or mapping tables).
   * If empty, token annotation is disabled.
   */
  std::optional<std::string> data_filepath = std::nullopt;
};

} // namespace phonemis::tagger