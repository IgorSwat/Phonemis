#include "stress.h"
#include <phonemis/base/phonemizer/constants.h>
#include <phonemis/utils/strings.h>

#include <algorithm>
#include <vector>

namespace phonemis::en::stress {

using namespace phonemis::phonemizer::constants::ipa;
using namespace phonemis::phonemizer::constants::ipa::stress;
using namespace phonemis::utils;

void apply(std::u32string& phonemes, float stress) {
  // 1. Pre-scan for existing features
  bool has_primary = phonemes.find(kPrimary) != std::u32string::npos;
  bool has_secondary = phonemes.find(kSecondary) != std::u32string::npos;
  bool has_vowel = std::any_of(
    phonemes.begin(), phonemes.end(),
    [](auto c) -> bool { return kVowels.find(c) != std::u32string::npos; }
  );

  // 2. Apply stress transformation
  if (stress < -1.F) {
    // Unstressed: strip all markers
    strings::remove__(phonemes, kPrimary);
    strings::remove__(phonemes, kSecondary);
  }
  else if (stress == -1.F || ((stress == 0.F || stress == 0.5F) && has_primary)) {
    // Reduced: primary -> secondary; remove existing secondary
    strings::remove__(phonemes, kSecondary);
    strings::replace__(phonemes, kPrimary, kSecondary);
  }
  else if ((stress == 0.F || stress == 0.5F || stress == 1.F) &&
           !has_primary && !has_secondary && has_vowel) {
    // Missing stress: Add secondary and align to first vowel
    phonemes.insert(0, 1, kSecondary);
    restress(phonemes);
  }
  else if (stress >= 1.F && !has_primary && has_secondary) {
    // Emphasized: secondary -> primary
    strings::replace__(phonemes, kSecondary, kPrimary);
  }
  else if (stress > 1.F && !has_primary && !has_secondary && has_vowel) {
    // Forced: Add primary and align to first vowel
    phonemes.insert(0, 1, kPrimary);
    restress(phonemes);
  }
}

void restress(std::u32string& phonemes) {
  // Use Fractional Sorting to move markers before vowels without complex manual shifts
  std::vector<std::pair<float, char32_t>> indexed_positions;
  indexed_positions.reserve(phonemes.size());
  for (size_t i = 0; i < phonemes.size(); i++)
    indexed_positions.emplace_back(static_cast<float>(i), phonemes[i]);

  for (size_t i = 0; i < indexed_positions.size(); i++) {
    char32_t ch = indexed_positions[i].second;
    if (ch == kPrimary || ch == kSecondary) {
      // Find following vowel
      size_t j = i + 1;
      for (; j < indexed_positions.size(); ++j) {
        if (kVowels.find(indexed_positions[j].second) != std::u32string::npos) break;
      }
      // Assign sort key to place marker immediately before the vowel
      if (j < indexed_positions.size()) {
        indexed_positions[i].first = static_cast<float>(j) - 0.5F;
      }
    }
  }

  // Finalize positions
  std::sort(indexed_positions.begin(), indexed_positions.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

  phonemes.clear();
  for (auto const& p : indexed_positions) phonemes.push_back(p.second);
}

} // namespace phonemis::en::stress