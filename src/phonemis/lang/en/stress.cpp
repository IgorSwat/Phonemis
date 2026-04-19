#include "constants.h"
#include "stress.h"
#include <phonemis/base/phonemizer/constants.h>
#include <phonemis/utils/strings.h>

#include <algorithm>
#include <vector>

namespace phonemis::en::stress {

using namespace phonemis::en::constants;
using namespace phonemis::phonemizer::constants::ipa::stress;
using namespace phonemis::utils;

void apply(std::u32string& phonemes, Level stress) {
  // 1. Pre-scan for existing features
  bool has_primary = phonemes.find(kPrimary) != std::u32string::npos;
  bool has_secondary = phonemes.find(kSecondary) != std::u32string::npos;
  bool has_vowel = std::any_of(
    phonemes.begin(), phonemes.end(),
    [](auto c) -> bool { return ipa::kVowels.contains(c); }
  );

  // 2. Apply stress transformation
  if (stress == Level::UNSTRESSED) {
    // Unstressed: strip all markers
    strings::remove__(phonemes, kPrimary);
    strings::remove__(phonemes, kSecondary);
  }
  else if (stress == Level::REDUCED || (stress == Level::NORMAL && has_primary)) {
    // Reduced: primary -> secondary; remove existing secondary
    strings::remove__(phonemes, kSecondary);
    strings::replace__(phonemes, kPrimary, kSecondary);
  }
  else if ((stress == Level::NORMAL || stress == Level::EMPHASIZED) &&
           !has_primary && !has_secondary && has_vowel) {
    // Missing stress: Add secondary and align to first vowel
    phonemes.insert(0, 1, kSecondary);
    restress(phonemes);
  }
  else if (stress == Level::EMPHASIZED && !has_primary && has_secondary) {
    // Emphasized: secondary -> primary
    strings::replace__(phonemes, kSecondary, kPrimary);
  }
  else if (stress == Level::FORCED && !has_primary && !has_secondary && has_vowel) {
    // Forced: Add primary and align to first vowel
    phonemes.insert(0, 1, kPrimary);
    restress(phonemes);
  }
}

void restress(std::u32string& phonemes) {
  // Move stress markers immediately before the next vowel
  for (size_t i = 0; i < phonemes.size(); ++i) {
    if (phonemes[i] == kPrimary || phonemes[i] == kSecondary) {
      // Find following vowel
      size_t j = i + 1;
      while (j < phonemes.size() && !ipa::kVowels.contains(phonemes[j])) {
        // Stop if we hit another stress marker
        if (phonemes[j] == kPrimary || phonemes[j] == kSecondary) break;
        ++j;
      }

      // If a vowel was found, shift characters to place the marker before it
      if (j < phonemes.size() && ipa::kVowels.contains(phonemes[j])) {
        char32_t marker = phonemes[i];
        for (size_t k = i; k < j - 1; ++k) {
          phonemes[k] = phonemes[k + 1];
        }
        phonemes[j - 1] = marker;
      }
    }
  }
}

} // namespace phonemis::en::stress