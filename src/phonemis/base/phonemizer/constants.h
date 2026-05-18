#pragma once

#include <string>
#include <unordered_set>

namespace phonemis::phonemizer::constants {

// IPA (International Phonetic Alphabet) related constants
namespace ipa {

// Stressing symbols
namespace stress {
  inline constexpr char32_t kPrimary = U'ˈ';
  inline constexpr char32_t kSecondary = U'ˌ';
} // namespace stress

} // namespace ipa

// Punctation character constants
namespace puncts {

inline const std::unordered_set<char32_t> kNonQuotePunctations = {
  U';', 
  U':', 
  U',', 
  U'.', 
  U'!', 
  U'?', 
  U'-', 
  U'\'',
  U'…', // Ellipsis
  U'|', // ASCII Pipe (often used as Hindi Purna Viram)
  U'।', // Hindi Purna Viram (U+0964)
  U'॥', // Hindi Deergh Viram (U+0965)
  U'¿', // Spanish Inverted Question Mark (U+00BF)
  U'¡', // Spanish Inverted Exclamation Mark (U+00A1)
  U'—', // Em Dash (U+2014)
  U'«', // Left Guillemet (U+00AB)
  U'»', // Right Guillemet (U+00BB)
};

inline const std::unordered_set<char32_t> kPunctuations = [] {
  auto s = kNonQuotePunctations;
  s.insert(U'"');
  return s;
}();

} // namespace puncts

} // namespace phonemis::phonemizer::constants
