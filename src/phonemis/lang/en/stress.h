#pragma once

#include <string>

namespace phonemis::en::stress {

// Applies stress to a phonemized string based on a continuous stress level.
//
// The `stress` parameter encodes the desired stress level:
//   < -1  — unstressed: remove all stress markers
//   = -1  — reduced: downgrade primary (ˈ) to secondary (ˌ), drop existing secondary
//   0–1   — normal: add secondary stress if none present; downgrade primary if present
//   >= 1  — emphasized: promote secondary (ˌ) to primary (ˈ); add primary if none present
//   > 1   — forced primary: insert primary stress even when no markers exist
//
// When a stress marker is inserted at position 0, `restress` is called to move
// it directly before the nearest vowel.
void apply(std::u32string& phonemes, float stress);

// Repositions each stress marker (ˈ, ˌ) so that it immediately precedes
// the nearest following vowel. If no following vowel exists, the marker
// stays in place. The relative order of all other phonemes is preserved.
void restress(std::u32string& phonemes);

} // namespace phonemis::en::stress