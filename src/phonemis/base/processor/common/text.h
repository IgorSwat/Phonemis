#pragma once

#include "types.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

/**
 * Scanning primitives shared by the layers: lookarounds and table keys at a position of a text.
 * Character classes come from utils/unicode.h.
 */
namespace phonemis::processor {

// -----------------------
// ----- Lookarounds -----
// -----------------------

/**
 * Checks if a position starts a word: no word character stands right before it.
 * @param also other characters which may not stand there, e.g. "." to reject the "g" of "e.g".
 */
bool is_clear_before(std::u32string_view text, size_t position, std::u32string_view also = U"");

/**
 * Checks if a position ends a word: no word character stands at it, or the text ends there.
 * @param also other characters which may not stand there.
 */
bool is_clear_after(std::u32string_view text, size_t position, std::u32string_view also = U"");

/**
 * Checks if a digit stands at a position, possibly after one of given marks: "5", ".5".
 * @param marks characters which may stand between the position and the digit.
 */
bool is_digit_after(std::u32string_view text, size_t position, std::u32string_view marks = U"");

/**
 * Checks if a sentence ends at a position: only whitespaces follow it.
 * @param capital whether a capital letter after the whitespaces also starts a new sentence: "2020 r. Potem".
 */
bool is_sentence_end(std::u32string_view text, size_t position, bool capital = false);

// ---------------------------
// ----- Runs and keys -----
// ---------------------------

// End of the run of digits starting at a position.
size_t digits_end(std::u32string_view text, size_t position);

// Position after an optional whitespace.
size_t skip_space(std::u32string_view text, size_t position);

// The keys starting at a position, longest first.
std::vector<std::u32string_view> keys_at(std::u32string_view text, size_t position, const Keys& keys);

// The non-empty words, joined with a separator.
std::u32string join(std::span<const std::u32string> words, std::u32string_view separator = U" ");

} // namespace phonemis::processor
