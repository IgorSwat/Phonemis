#pragma once

#include <unordered_map>
#include <unordered_set>

namespace phonemis::tokenizer {

namespace split {

/**
 * Split rule controls how multi-phrase words, divided by special characters (ex. U.S.S.R, he's, well-being)
 * are being splitted into subphrases.
 */
enum class Rule {
  // Attach separator to the left of the right-hand phrase, e.g. "well" + "-being"
  JOIN_LEFT = 0,
  // Attach separator to the right of the left-hand phrase, e.g. "well-" + "being"
  JOIN_RIGHT,
  // Keep phrases and separator together as one token, e.g. "well-being"
  TOTAL_JOIN,

  // Split phrases and drop separator, e.g. "well" + "being"
  TOTAL_DIVIDE,
};

// Split rules are simple mappings from given (special) character to the corresponding rule.
using Rules = std::unordered_map<char32_t, Rule>;

// Split exceptions are phrases which do not subject to split rules and are not splitted at all.
using Exceptions = std::unordered_set<std::u32string_view>;

constexpr inline Rule DEFAULT_RULE = Rule::TOTAL_DIVIDE;

} // namespace split

} // namespace phonemis::tokenizer