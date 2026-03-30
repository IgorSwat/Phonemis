#pragma once

#include <unordered_set>

namespace phonemis::preprocessor::num2word {

/**
 * A basic num2word configuration.
 *
 * @param allowGeneralOrdNotation when set to true, considers notations such as `2.` as ordinal number notations.
 */
struct Config {
    bool allowGeneralOrdNotation = true;
};

} // namespace phonemis::preprocessor::num2word