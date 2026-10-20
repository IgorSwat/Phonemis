#pragma once

namespace phonemis::processor::num2word {

// Types of numeric expressions, in priority order: at a given position of a text,
// patterns of earlier kinds are tried first. Languages add patterns, but never kinds.
enum class Kind {
  // Whole expressions whose parts are numbers
  DATE = 0,            // 12.05.2024, 2024-05-12, 1. 5.
  INTERNATIONAL_PHONE, // +48 600 123 456, +1 (555) 123-4567: before a language's own formats of its groups
  PHONE,               // 555-123-4567, 06 12 34 56 78
  VERSION,             // 3.8.10, 10.0.0.51

  // Symbols between numbers (only the symbol is matched)
  DIFFERENCE, // the " - " of 5 - 3
  SUM,        // the "+" of 2 + 2 and 90+6
  RANGE,      // the "-" of 10-15 and 2020-2022
  DIMENSION,  // the "x" of 1920x1080 and 3 m x 4 m
  FRACTION,   // 3/4
  SIGN,       // the minus of -5
  TIME,       // 18:30, 7pm, 18 Uhr, 21h30
  SCORE,      // the ":" of 2:1

  // Numbers with what they count
  MONEY,   // $5, 12,99 zł, 10-15 €, 3 mln zł
  SCALE,   // 3 mln, 500k, 1.5 million
  PERCENT, // 20%, 10-15%, 8%ige
  UNIT,    // 5 kg, 10-15 km

  // Numbers marked by a suffix or joined to letters
  ORDINAL,  // 21st, 3., 1er, 1º
  DECADE,   // 1990s, 90er, '80
  COMPOUND, // 21-year-old, 45-metrowe
  CODE,     // MP3, A380, COVID-19

  // Any other number
  NUMBER, // 42, 1 000,5, .5, 2020 r.
};

} // namespace phonemis::processor::num2word
