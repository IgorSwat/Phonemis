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
  // Pre-scan: determine what stress markers and vowels are present
  bool has_primary = phonemes.find(kPrimary) != std::u32string::npos;
  bool has_secondary = phonemes.find(kSecondary) != std::u32string::npos;
  bool has_vowel = std::any_of(
    phonemes.begin(), phonemes.end(),
    [](auto c) -> bool { return kVowels.find(c) != std::u32string::npos; }
  );

  if (stress < -1.F) {
    // Fully unstressed: strip all stress markers
    strings::remove__(phonemes, kPrimary);
    strings::remove__(phonemes, kSecondary);
  }
  else if (stress == -1.F || ((stress == 0.F || stress == 0.5F) && has_primary)) {
    // Reduced stress: downgrade primary → secondary (drop any existing secondary first)
    strings::remove__(phonemes, kSecondary);
    strings::replace__(phonemes, kPrimary, kSecondary);
  }
  else if ((stress == 0.F || stress == 0.5F || stress == 1.F) &&
           !has_primary && !has_secondary && has_vowel) {
    // No existing marker but vowel present: add secondary stress, then move it before vowel
    phonemes.insert(0, 1, kSecondary);
    restress(phonemes);
  }
  else if (stress >= 1.F && !has_primary && has_secondary) {
    // Promote existing secondary → primary
    strings::replace__(phonemes, kSecondary, kPrimary);
  }
  else if (stress > 1.F && !has_primary && !has_secondary && has_vowel) {
    // Forced primary: no marker present — insert primary, then move it before vowel
    phonemes.insert(0, 1, kPrimary);
    restress(phonemes);
  }
}

void restress(std::u32string& phonemes) {
  // Pair each character with a fractional sort key (initially its integer index)
  std::vector<std::pair<float, char32_t>> indexed_positions;
  indexed_positions.reserve(phonemes.size());
  for (size_t i = 0; i < phonemes.size(); i++)
    indexed_positions.emplace_back(static_cast<float>(i), phonemes[i]);

  // For each stress marker, find the next vowel and assign a key just before it
  for (size_t i = 0; i < indexed_positions.size(); i++) {
    char32_t ch = indexed_positions[i].second;
    if (ch == kPrimary || ch == kSecondary) {
      size_t j = i + 1;
      for (; j < indexed_positions.size(); ++j) {
        if (kVowels.find(indexed_positions[j].second) != std::u32string::npos) break;
      }
      if (j < indexed_positions.size()) {
        indexed_positions[i].first = static_cast<float>(j) - 0.5F; // place before vowel
      }
      // If no following vowel exists, the marker keeps its current position
    }
  }

  // Reconstruct the string in sorted order
  std::sort(indexed_positions.begin(), indexed_positions.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

  phonemes.clear();
  for (auto const& p : indexed_positions) phonemes.push_back(p.second);
}

} // namespace phonemis::en::stress