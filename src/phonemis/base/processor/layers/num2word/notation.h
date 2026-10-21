#pragma once

#include "types.h"

namespace phonemis::processor::num2word {

// How numbers are written in a language, read by the generic patterns.
struct Notation {
  std::u32string group_separators;   // 1.000, 1 000, 1,000
  std::u32string decimal_separators; // 1,5, 1.5
  std::u32string time_separators;    // 18:30, 18.30, 18h30
  Keys time_suffixes;                // pm, Uhr, h
  Keys currencies;                   // $, zł, EUR
  Keys scales;                       // million, mln, k
  Keys units;                        // km, kg, °C
  bool fractions = false;            // Whether 3/4 reads as a fraction
};

// The keys of a table, for a Notation built out of a language's constants.
template <typename Table>
Keys keys_of(const Table& table) {
  Keys keys;
  keys.reserve(table.size());
  for (const auto& [key, value] : table) {
    keys.emplace_back(key);
  }
  return keys;
}

} // namespace phonemis::processor::num2word
