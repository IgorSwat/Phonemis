#pragma once

#include "../../common/text.h"
#include "types.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

/**
 * Number helpers
 *
 * Shared steps of the languages' readings: values of digits, digit-by-digit decisions,
 * the parts of codes and joining words.
 */
namespace phonemis::processor::num2word {

// The value of a run of digits; nullopt if it is empty or too long to hold.
std::optional<uint64_t> value_of(std::u32string_view digits);

// Whether an integer is read digit by digit: a leading zero, or a long code that is not a round amount.
bool spoken_digits(const Number& number, size_t longest);

// The parts of a code (MP3, H.264, F-16s): digits with their decimals, ".digits", runs of other characters,
// and single apostrophes, dots and hyphens.
std::vector<std::u32string_view> code_parts(std::u32string_view code);

} // namespace phonemis::processor::num2word
