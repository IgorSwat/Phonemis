#pragma once

#include "../../common/types.h"
#include "kind.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace phonemis::processor::num2word {

using processor::Keys;

// A number as written: "1 000,5" -> integer "1000", fraction "5".
struct Number {
  Span span;                 // Where it is written
  std::u32string integer;    // Digits without group separators; empty in ".5"
  std::u32string fraction;   // Digits after the decimal mark; empty if none
  char32_t decimal_mark = 0; // The decimal mark as written, if any
  bool grouped = false;      // Written with group separators: 1 000, 1,000
};

// A numeric expression found in a text: its structure only, with no decision on how to read it.
struct Match {
  Kind kind = Kind::NUMBER;
  Span span;
  std::u32string_view text;    // As written
  std::vector<Number> numbers; // In written order: one for most kinds, two for ranges, three for dates
  std::u32string_view unit;    // A unit or currency key ("km", "zł"), or a law word of a DATE ("Ley")
  std::u32string_view scale;   // A scale key ("mln", "million")
  std::u32string_view suffix;  // Marks after the number: "st", "er", "." (ordinals), "ige" (8%ige), "pm", "r."
  char32_t separator = 0;      // The symbol between numbers: '-', ':', '.', '/', 'h', 'x'
};

// Words read in place of the symbols between numbers.
struct Joiners {
  std::u32string point; // 3.8.10: three point eight point ten
  std::u32string dot;   // 10.0.0.51 (four parts): ten dot zero dot zero dot fifty-one
  std::u32string range; // 10-15, with its own spaces: " to ", " bis ", "-"
  std::u32string by;    // 1920x1080
  std::u32string score; // 2:1, with its own spaces: " to "
  std::u32string minus; // -5 and 5 - 3
  std::u32string plus;  // 2 + 2
};

// The forms of a noun after a number: one dollar, two dollars.
struct Forms {
  std::u32string one;
  std::u32string many;
};

// Vocabulary of readings: languages build their own readings out of these
enum class Form { CARDINAL = 0, ORDINAL, YEAR, DIGITS };
enum class Case { NOMINATIVE = 0, GENITIVE, DATIVE, ACCUSATIVE, INSTRUMENTAL, LOCATIVE };
enum class Gender { MASCULINE = 0, FEMININE, NEUTER };

// ---------------------------------------------
// ----- Formats of configurable patterns -----
// ---------------------------------------------

// How a numeric date is written in a language: 12.05.2024, 12/05/24, 1. 5.
struct DateFormat {
  std::u32string separators = U"/";   // Between its parts: the same one twice in a date
  std::u32string boundaries = U".,/"; // Besides letters, what may not precede it (or a digit after it)
  bool year_required = true;
  bool short_year = true;         // Two-digit years: 12/05/24
  bool closing_separator = false; // A separator also closes the month: 1.5. and 1.5.2024
  bool spaced = false;            // An optional space after the day: 12. 5.
  Keys suffixes;                  // Abbreviations after the year: 2024 r.
};

// How a phone number is written: its groups of digits and what separates them.
struct PhoneFormat {
  std::vector<std::pair<size_t, size_t>> groups; // Length range of each group of digits
  std::u32string separators = U"-";
  bool optional_separators = false; // 0612345678 as well as 06 12 34 56 78
  std::u32string prefix;            // An optional literal start: "1-"
  bool parenthesized = false;       // The first group in parentheses, then an optional space: (555) 123-4567
  bool leading_zero = false;
  std::u32string boundaries = U"-"; // Besides letters, what may not stand right before it
  std::u32string closing = U"-";    // Besides letters, what may not stand right after it
};

// How a suffix marks a number: 21st, 1 º, 90's, 2020 r.
struct SuffixFormat {
  bool space = false;                // An optional space before the suffix
  bool word_end = true;              // No word character may follow the suffix
  bool apostrophes = false;          // '90s, 90's: optional apostrophes before the number and the suffix
  std::u32string boundaries = U".,"; // Besides letters, what may not stand right before the number
  bool sentence_dot = false;         // A dot closing the suffix is left to a sentence it also closes
};

} // namespace phonemis::processor::num2word
