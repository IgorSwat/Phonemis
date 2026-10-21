#pragma once

#include "context.h"

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace phonemis::processor {

// A condition on the context of a written phrase, which decides between its readings.
using Condition = std::function<bool(const Context&)>;

// A way of saying a written phrase (a symbol, an abbreviation), when its condition holds.
struct Reading {
  Reading(const char32_t* words, Condition when = nullptr) : words(words), when(std::move(when)) {}

  std::u32string words; // Empty words drop the phrase
  Condition when;       // Always, if none
};

// Written phrases with their readings: the first one whose condition holds is used.
using Readings = std::unordered_map<std::u32string, std::vector<Reading>>;

/**
 * The words of a phrase in its context.
 * @returns the words of its first reading whose condition holds, or std::nullopt if there is none.
 */
std::optional<std::u32string> read(const std::vector<Reading>& readings, const Context& context);

// -----------------------
// ----- Conditions -----
// -----------------------

bool after_name(const Context& context);    // Two letters right before: Google+
bool before_number(const Context& context); // A digit right after, possibly after a space: nr 5, nr5
bool before_name(const Context& context);   // A capitalized word after: St. Louis

// The word before is one of given words: im § 5
Condition after_any(std::unordered_set<std::u32string> words);

} // namespace phonemis::processor
