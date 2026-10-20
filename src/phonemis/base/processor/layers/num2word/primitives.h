#pragma once

#include "../../common/text.h"
#include "types.h"

#include <string_view>
#include <vector>

// Scanning primitives of written numbers, on top of the shared ones (../text.h).
namespace phonemis::processor::num2word {

// The digits starting at a position, as a number.
Number digits_at(std::u32string_view text, size_t position);

/**
 * A written number at a position: 1,000.5 or 1 000,5 or 15.
 * @returns its possible readings, in the order a regex would try them: grouped first,
 *          each with and then without its fraction. Empty if no number starts there.
 */
std::vector<Number> numbers_at(std::u32string_view text, size_t position, std::u32string_view group,
                               std::u32string_view decimal);

} // namespace phonemis::processor::num2word
