#pragma once

#include <string>

namespace phonemis::en::stress {

/**
 * Characteristic stress levels for English phonemization.
 */
enum class Level : int32_t {
  /** Remove all stress markers. */
  UNSTRESSED = -3,
  /** Primary becomes secondary, existing secondary is dropped. */
  REDUCED = -2,
  /** Default stress levels. Adds secondary if missing; downgrades primary. */
  NORMAL = 0,
  /** Promote secondary to primary. */
  EMPHASIZED = 1,
  /** Insert primary if none exist. */
  FORCED = 3
};

/**
 * Adjusts stress markers (ˈ, ˌ) based on a target stress level.
 * 
 * @param phonemes The UTF-32 phoneme string to modify in-place.
 * @param level    The target stress level.
 */
void apply(std::u32string& phonemes, Level level);

/**
 * Repositions stress markers so they immediately precede the nearest vowel.
 * If no vowel follows a marker, it remains in its original position.
 * 
 * @param phonemes The UTF-32 phoneme string to modify in-place.
 */
void restress(std::u32string& phonemes);

} // namespace phonemis::en::stress