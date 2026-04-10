#pragma once

#include <string>
#include <unordered_set>

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

// Punctation character constants
namespace puncts {

inline const std::unordered_set<char32_t> kPunctations = {
  U';', U':', U',', U'.', U'!', U'?', U'-', U'"', U'\''
};

inline const std::unordered_set<char32_t> kNonQuotePunctations = {
  U';', U':', U',', U'.', U'!', U'?', U'-', U'\''
};

} // namespace puncts

} // namespace phonemis::phonemizer::constants