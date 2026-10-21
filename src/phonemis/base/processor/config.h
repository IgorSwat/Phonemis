#pragma once

#include "common/conditions.h"

namespace phonemis::processor {

// Configuration of preprocessing: which layers run, and words added to a language's own tables.
struct Config {
  /**
   * Write equivalent characters in one form (NormalizeLayer): quotes as ", apostrophes as ', dashes as - or " - ".
   * The other layers expect it: without it, they miss "’" in 90’s or "–" in 10–15.
   */
  bool normalize = true;

  /**
   * Normalize whitespaces (TrimLayer): no whitespace at the edges or before punctuation, a single one between words.
   * The tokenizer expects a trimmed text, so turn it off only for an already normalized input.
   */
  bool trim = true;

  /**
   * Read abbreviations and web addresses as words (Term2WordLayer): e.g. -> for example.
   */
  bool terms = true;

  /**
   * Read numbers as words (Num2WordLayer): 25 -> twenty-five, $5 -> five dollars.
   * Without it, "$5" reads "dollars 5" if symbols stay on.
   */
  bool numbers = true;

  /**
   * Read symbols as words (Sym2WordLayer): & -> and.
   */
  bool symbols = true;

  /**
   * Replace or remove the characters a language does not expect (SanitizerLayer), if it has such rules.
   */
  bool sanitize = true;

  /**
   * Abbreviations added to the language's own, replacing those with the same key: {U"approx.", {U"approximately"}}.
   */
  Readings abbreviations;

  /**
   * Symbols added to the language's own, replacing those with the same key: {U"™", {U"trademark"}}.
   */
  Readings symbol_words;
};

} // namespace phonemis::processor
