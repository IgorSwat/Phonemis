#pragma once

#include <string>

namespace phonemis::phonemizer::constants {

// IPA (International Phonetic Alphabet) related constants
namespace ipa {

// Phones categorization
inline const std::u32string kVowels = U"AIOQWYaiuæɑɒɔəɛɜɪʊʌᵻ";  // Spoken vowels
inline const std::u32string kConsonants = U"bdfhjklmnpstvwzðŋɡɹɾʃʒʤʧθ"; // Spoken consosants

// Stressing symbols
namespace stress {
  inline constexpr char32_t kPrimary = U'ˈ';
  inline constexpr char32_t kSecondary = U'ˌ';
} // namespace stress

} // namespace ipa

} // namespace phonemis::phonemizer::constants