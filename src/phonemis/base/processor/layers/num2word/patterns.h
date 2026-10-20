#pragma once

#include "notation.h"
#include "pattern.h"

#include <memory>
#include <utility>
#include <vector>

/**
 * Patterns of numeric expressions
 *
 * Generic patterns are added for every language and read its Notation.
 * Configurable patterns (dates, phones, suffixes) are added by each language with its own format.
 */
namespace phonemis::processor::num2word {

// ----------------------------
// ----- Generic patterns -----
// ----------------------------

// The generic patterns of a language.
std::vector<std::unique_ptr<Pattern>> generic_patterns(const Notation& notation);

// 2024-05-12: an ISO date. Its numbers are ordered as day, month, year.
class IsoDatePattern : public Pattern {
public:
  IsoDatePattern() : Pattern(Kind::DATE) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// 3.8.10, 10.0.0.51; a chain of thousands groups (1.000.000) is left to numbers.
class VersionPattern : public Pattern {
public:
  explicit VersionPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  std::u32string group_;
};

// The " - " of 5 - 3.
class DifferencePattern : public Pattern {
public:
  DifferencePattern() : Pattern(Kind::DIFFERENCE) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// The "+" of 2 + 2 or 90+6, with the spaces around it.
class SumPattern : public Pattern {
public:
  SumPattern() : Pattern(Kind::SUM) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// The "-" of 10-15 and 2020-2022. Its numbers are the digits on both sides.
class RangePattern : public Pattern {
public:
  RangePattern() : Pattern(Kind::RANGE) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// The "x" of 1920x1080, or of 3 m x 4 m (the units are left to UnitPattern).
class DimensionPattern : public Pattern {
public:
  explicit DimensionPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Keys units_;
};

// 3/4
class FractionPattern : public Pattern {
public:
  FractionPattern() : Pattern(Kind::FRACTION) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// The minus of -5.
class SignPattern : public Pattern {
public:
  SignPattern() : Pattern(Kind::SIGN) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// 18:30, 10.30 pm, 21h30: hours and minutes, with an optional suffix.
class TimePattern : public Pattern {
public:
  explicit TimePattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  // Whether a time may end at a position: not a longer number, nor a measure (1.30 min)
  bool ends_at(std::u32string_view text, size_t position) const;

  std::u32string separators_;
  Keys suffixes_;
  Keys measures_;
};

// 7pm, 18 Uhr, 21h: an hour marked by its suffix.
class HourPattern : public Pattern {
public:
  explicit HourPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Keys suffixes_;
};

// The ":" of 2:1.
class ScorePattern : public Pattern {
public:
  ScorePattern() : Pattern(Kind::SCORE) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// $5, € 1,5 million: a currency symbol before an amount.
class PrefixedMoneyPattern : public Pattern {
public:
  explicit PrefixedMoneyPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Notation notation_;
  Keys currencies_; // Only the ones which are not words
};

// 50$, 12,99 zł, 10-15 €, 3 mln zł: an amount, possibly a range, followed by its currency.
class MoneyPattern : public Pattern {
public:
  explicit MoneyPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Notation notation_;
};

// 3 mln, 500k, 1.5 million
class ScalePattern : public Pattern {
public:
  explicit ScalePattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Notation notation_;
};

// 20%, 10-15 %, 8%ige
class PercentPattern : public Pattern {
public:
  explicit PercentPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Notation notation_;
};

// 5 kg, 10-15 km, 230 V; a unit of a single capital letter only after a space.
class UnitPattern : public Pattern {
public:
  explicit UnitPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Notation notation_;
  Keys letters_; // Units of a single capital letter (W, V)
};

// 21-year-old, 45-metrowe: a number joined to a word by a hyphen.
class CompoundPattern : public Pattern {
public:
  explicit CompoundPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Notation notation_;
};

// MP3, A380, COVID-19, H.264: a token mixing letters and digits.
class CodePattern : public Pattern {
public:
  CodePattern() : Pattern(Kind::CODE) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// 42, 1 000,5, .5: any other number.
class NumberPattern : public Pattern {
public:
  explicit NumberPattern(const Notation& notation);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Notation notation_;
};

// ---------------------------------
// ----- Configurable patterns -----
// ---------------------------------

// A numeric date. Its numbers are ordered as day, month and year (if written).
class DatePattern : public Pattern {
public:
  explicit DatePattern(DateFormat format);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  // The date without (zero) or with its year, if it ends at a valid position
  std::optional<Match> complete(std::u32string_view text, Match date, size_t position) const;

  DateFormat format_;
};

// +48 600 123 456, +1 (555) 123-4567: a phone number with a country code. Its numbers are the code, then the groups.
class InternationalPhonePattern : public Pattern {
public:
  InternationalPhonePattern() : Pattern(Kind::INTERNATIONAL_PHONE) {}

  std::optional<Match> match(std::u32string_view text, size_t position) const override;
};

// A phone number. Its numbers are its groups of digits.
class PhonePattern : public Pattern {
public:
  explicit PhonePattern(PhoneFormat format);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  // The phone number starting with its first group at a position
  std::optional<Match> groups_at(std::u32string_view text, size_t start, size_t position) const;

  PhoneFormat format_;
};

// Digits followed by a suffix from a table.
class SuffixPattern : public Pattern {
public:
  SuffixPattern(Kind kind, Keys suffixes, SuffixFormat format = {});

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  Keys suffixes_;
  SuffixFormat format_;
};

// 3. Platz, 55. sesję: an ordinal written as digits with a dot, followed by a word.
class DotOrdinalPattern : public Pattern {
public:
  /**
   * @param before_punctuation also before a comma or at the end of the text: lat 70., Punkt 3.
   * @param sentence_dot leaves the dot to a sentence it also closes.
   */
  explicit DotOrdinalPattern(bool before_punctuation = false, bool sentence_dot = false);

  std::optional<Match> match(std::u32string_view text, size_t position) const override;

private:
  bool before_punctuation_;
  bool sentence_dot_;
};

} // namespace phonemis::processor::num2word
