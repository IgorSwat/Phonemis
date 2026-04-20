#pragma once

#include <unordered_set>

namespace phonemis::preprocessor::num2word {

/**
 * A basic num2word configuration.
 *
 * @param allow_general_ord_notation when set to true, considers notations such as `2.` as ordinal number notations.
 */
struct Config {
    bool allow_general_ord_notation = true;
};

} // namespace phonemis::preprocessor::num2word