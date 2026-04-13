#pragma once

#include <unordered_map>
#include <unordered_set>

namespace phonemis::tokenizer {

namespace split {

/**
 * Defines how to handle words containing special separators (e.g., "well-being", "it's").
 */
enum class Rule {
  // Separator attached to the start of the following word (e.g., "well", "-being")
  JOIN_LEFT = 0,
  // Separator attached to the end of the preceding word (e.g., "it'", "s")
  JOIN_RIGHT,
  // Keep the word and separator as a single token (e.g., "well-being")
  TOTAL_JOIN,
  // Split into words and discard the separator entirely (e.g., "well", "being")
  TOTAL_DIVIDE,
};

// Maps specific characters to their respective split rules.
using Rules = std::unordered_map<char32_t, Rule>;

// Set of full phrases that should never be split, overriding any rules.
using Exceptions = std::unordered_set<std::u32string_view>;

constexpr inline Rule DEFAULT_RULE = Rule::TOTAL_DIVIDE;

} // namespace split

} // namespace phonemis::tokenizer