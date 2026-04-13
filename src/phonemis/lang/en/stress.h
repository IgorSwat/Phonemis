#pragma once

#include <string>

namespace phonemis::en::stress {

/**
 * Adjusts stress markers (ˈ, ˌ) based on a target stress level.
 * 
 * Stress levels:
 * < -1  : Remove all stress markers.
 * = -1  : Reduced (primary becomes secondary, secondary is dropped).
 * 0..1  : Normal (add secondary if missing; downgrade primary).
 * >= 1  : Emphasized (promote secondary to primary).
 * > 1   : Forced primary (insert primary if none exist).
 * 
 * @param phonemes The UTF-32 phoneme string to modify in-place.
 * @param stress   The target stress intensity.
 */
void apply(std::u32string& phonemes, float stress);

/**
 * Repositions stress markers so they immediately precede the nearest vowel.
 * If no vowel follows a marker, it remains in its original position.
 * 
 * @param phonemes The UTF-32 phoneme string to modify in-place.
 */
void restress(std::u32string& phonemes);

} // namespace phonemis::en::stress