#pragma once

#include "layer.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace phonemis::preprocessor {

/**
 * Sanitizer layer is responsible for filtering out (and/or replacing) unrecognizable
 * or undesirable characters in given language.
 * 
 * It is controlled by keep/reject set of characters and an optional mapping.
 */
class SanitizerLayer : public Layer {
public:
  // Sanitizer's filtering mode
  enum class Mode {
    // KEEP: Only characters in the set are kept; all others are filtered out.
    KEEP = 0,
    // KEEP_ALPHABETICAL: Similar to keep, but only checks the alphabetical characters.
    KEEP_ALPHABETICAL,
    // REJECT: Only characters in the set are filtered out; all others are kept.
    REJECT
  };

  SanitizerLayer() = default;

  std::u32string transform(std::u32string_view input) const override;

  // Sanitizer's mode controllers
  void setupFilter(const std::unordered_set<char32_t>* filter, Mode mode);
  void resetFilter();
  void setupMapper(const std::unordered_map<char32_t, char32_t>* mapper);
  void resetMapper();

private:
  const std::unordered_set<char32_t>* filter_ = nullptr;
  const std::unordered_map<char32_t, char32_t>* mapper_ = nullptr;

  Mode mode_ = Mode::KEEP;
};

} // namespace phonemis::preprocessor