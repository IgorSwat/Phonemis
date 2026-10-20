#pragma once

#include "../../common/context.h"
#include "../../layer.h"
#include "notation.h"
#include "numerals.h"
#include "pattern_set.h"
#include "patterns.h"
#include "primitives.h"
#include <phonemis/utils/unicode.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace phonemis::processor::num2word {

/**
 * A skeleton of numbers verbalization, specialized by every language.
 *
 * Each pattern, in priority order (see kind.h), rewrites the text in turn: it finds its expressions (scan),
 * decides how to read each of them from its context (resolve) and says it (verbalize).
 * A language provides the patterns specific to it, the resolution and the readings.
 *
 * @tparam Reading a language's decision on how to read a match: the features its words agree with.
 */
template <typename Reading>
class Num2WordLayer : public Layer {
public:
  void apply(Document& document) const override;

protected:
  Num2WordLayer(const Notation& notation, Joiners joiners);

  // Adds a language's own pattern; it goes before the generic patterns of its kind.
  void add_pattern(std::unique_ptr<Pattern> pattern);

  // ---------------------
  // ----- Resolution -----
  // ---------------------

  /**
   * Whether a match really is what its pattern found: a time can turn out to be a decimal (1.13).
   * A rejected match is left to the patterns of later kinds.
   */
  virtual bool accept(const Match& match, const Context& context) const { return true; }

  // Decides how to read a match from its context.
  virtual Reading resolve(const Match& match, const Context& context) const = 0;

  // ---------------------------------
  // ----- Readings of a language -----
  // ---------------------------------

  virtual std::u32string read_number(const Number& number, const Reading& reading) const = 0;
  virtual std::u32string read_code_digits(std::u32string_view digits) const = 0; // The 380 of A380

  virtual std::u32string read_date(const Match& match, const Reading& reading) const;
  virtual std::u32string read_phone(const Match& match, const Reading& reading) const;
  virtual std::u32string read_fraction(const Match& match, const Reading& reading) const;
  virtual std::u32string read_time(const Match& match, const Reading& reading) const;
  virtual std::u32string read_ordinal(const Match& match, const Reading& reading) const;
  virtual std::u32string read_decade(const Match& match, const Reading& reading) const;
  virtual std::u32string read_compound(const Match& match, const Reading& reading) const;
  virtual std::u32string read_plain(const Match& match, const Reading& reading) const; // A NUMBER match

  // Amounts: each reads the last number of a match, a range (10-15 km) gets its start prepended
  virtual std::u32string read_money(const Number& number, const Match& match, const Reading& reading) const;
  virtual std::u32string read_scale(const Number& number, const Match& match, const Reading& reading) const;
  virtual std::u32string read_percent(const Number& number, const Match& match, const Reading& reading) const;
  virtual std::u32string read_unit(const Number& number, const Match& match, const Reading& reading) const;

  // Symbols between numbers (DIFFERENCE, SUM, RANGE, DIMENSION, SIGN, SCORE), from the joiners by default
  virtual std::u32string read_joiner(const Match& match, const Reading& reading) const;
  virtual std::u32string read_range(const Number& from, const Number& to, const Reading& reading) const;

  // The plural of a reading, for codes like F-16s; none by default.
  virtual std::optional<std::u32string> plural(std::u32string_view words) const { return std::nullopt; }

  Joiners joiners_;

private:
  using AmountReading = std::u32string (Num2WordLayer::*)(const Number&, const Match&, const Reading&) const;

  std::u32string verbalize(const Match& match, const Reading& reading) const;

  // Shared readings
  std::u32string read_each(const Match& match, const Reading& reading) const; // Its numbers one by one
  std::u32string read_version(const Match& match, const Reading& reading) const;
  std::u32string read_international_phone(const Match& match, const Reading& reading) const;
  std::u32string read_code(const Match& match, const Reading& reading) const;
  std::u32string read_amount(const Match& match, const Reading& reading, AmountReading read) const;

  PatternSet patterns_;
};

// ------------------------
// ----- Implementation -----
// ------------------------

template <typename Reading>
Num2WordLayer<Reading>::Num2WordLayer(const Notation& notation, Joiners joiners) : joiners_(std::move(joiners)) {
  for (auto& pattern : generic_patterns(notation)) {
    patterns_.add(std::move(pattern), Origin::GENERIC);
  }
}

template <typename Reading>
void Num2WordLayer<Reading>::add_pattern(std::unique_ptr<Pattern> pattern) {
  patterns_.add(std::move(pattern), Origin::LANGUAGE);
}

template <typename Reading>
void Num2WordLayer<Reading>::apply(Document& document) const {
  // A draft rewritten by each pattern in turn: later patterns see the words of earlier ones
  Document draft(document.text());

  for (const auto& [pattern, origin] : patterns_) {
    std::u32string_view text = draft.text();

    for (size_t position = 0; position < text.size();) {
      // 1. Scan
      auto match = pattern->match(text, position);
      if (!match) {
        position++;
        continue;
      }
      match->text = text.substr(match->span.begin, match->span.end - match->span.begin);

      // 2. Resolve and verbalize
      Context context(text, match->span);
      if (accept(*match, context)) {
        std::u32string words = verbalize(*match, resolve(*match, context));
        if (words != match->text) {
          draft.replace(match->span, std::move(words));
        }
      }
      position = match->span.end;
    }
    draft.commit();
  }

  // Each change of the draft is an edit of the document
  for (auto& [span, words] : draft.changes()) {
    document.replace(span, std::move(words));
  }
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::verbalize(const Match& match, const Reading& reading) const {
  switch (match.kind) {
    case Kind::DATE:
      return read_date(match, reading);
    case Kind::INTERNATIONAL_PHONE:
      return read_international_phone(match, reading);
    case Kind::PHONE:
      return read_phone(match, reading);
    case Kind::VERSION:
      return read_version(match, reading);
    case Kind::DIFFERENCE:
    case Kind::SUM:
    case Kind::RANGE:
    case Kind::DIMENSION:
    case Kind::SIGN:
    case Kind::SCORE:
      return read_joiner(match, reading);
    case Kind::FRACTION:
      return read_fraction(match, reading);
    case Kind::TIME:
      return read_time(match, reading);
    case Kind::MONEY:
      return read_amount(match, reading, &Num2WordLayer::read_money);
    case Kind::SCALE:
      return read_amount(match, reading, &Num2WordLayer::read_scale);
    case Kind::PERCENT:
      return read_amount(match, reading, &Num2WordLayer::read_percent);
    case Kind::UNIT:
      return read_amount(match, reading, &Num2WordLayer::read_unit);
    case Kind::ORDINAL:
      return read_ordinal(match, reading);
    case Kind::DECADE:
      return read_decade(match, reading);
    case Kind::COMPOUND:
      return read_compound(match, reading);
    case Kind::CODE:
      return read_code(match, reading);
    case Kind::NUMBER:
      return read_plain(match, reading);
  }
  return std::u32string(match.text);
}

// Default readings - all numbers read one by one

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_date(const Match& match, const Reading& reading) const {
  return read_each(match, reading);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_phone(const Match& match, const Reading& reading) const {
  std::vector<std::u32string> words;
  for (const auto& number : match.numbers) {
    words.push_back(read_code_digits(number.integer));
  }
  return join(words);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_fraction(const Match& match, const Reading& reading) const {
  return read_each(match, reading);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_time(const Match& match, const Reading& reading) const {
  return read_each(match, reading);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_ordinal(const Match& match, const Reading& reading) const {
  return read_number(match.numbers[0], reading);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_decade(const Match& match, const Reading& reading) const {
  return read_number(match.numbers[0], reading);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_compound(const Match& match, const Reading& reading) const {
  return read_number(match.numbers[0], reading) + U"-" + std::u32string(match.suffix);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_plain(const Match& match, const Reading& reading) const {
  return read_number(match.numbers[0], reading);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_money(const Number& number, const Match& match,
                                                  const Reading& reading) const {
  return read_number(number, reading) + U" " + std::u32string(match.unit);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_scale(const Number& number, const Match& match,
                                                  const Reading& reading) const {
  return read_number(number, reading) + U" " + std::u32string(match.scale);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_percent(const Number& number, const Match& match,
                                                    const Reading& reading) const {
  return read_number(number, reading) + U" %";
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_unit(const Number& number, const Match& match,
                                                 const Reading& reading) const {
  return read_number(number, reading) + U" " + std::u32string(match.unit);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_joiner(const Match& match, const Reading& reading) const {
  switch (match.kind) {
    case Kind::DIFFERENCE:
      return U" " + joiners_.minus + U" ";
    case Kind::SUM:
      return U" " + joiners_.plus + U" ";
    case Kind::RANGE:
      return read_range(match.numbers[0], match.numbers[1], reading);
    case Kind::DIMENSION:
      return U" " + joiners_.by + U" ";
    case Kind::SIGN:
      return joiners_.minus + U" ";
    default:
      return joiners_.score;
  }
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_range(const Number& from, const Number& to, const Reading& reading) const {
  return joiners_.range;
}

// Shared readings

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_each(const Match& match, const Reading& reading) const {
  std::vector<std::u32string> words;
  for (const auto& number : match.numbers) {
    words.push_back(read_number(number, reading));
  }
  return join(words);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_international_phone(const Match& match, const Reading& reading) const {
  // plus forty-eight, then the groups as the language reads a phone number
  Match groups = match;
  groups.kind = Kind::PHONE;
  groups.numbers.erase(groups.numbers.begin());
  groups.text = match.text.substr(groups.numbers.front().span.begin - match.span.begin);
  return joiners_.plus + U" " + read_code_digits(match.numbers[0].integer) + U" " + read_phone(groups, reading);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_version(const Match& match, const Reading& reading) const {
  std::vector<std::u32string> words;
  for (const auto& part : match.numbers) {
    words.push_back(read_number(part, reading));
  }
  return join(words, U" " + (match.numbers.size() == 4 ? joiners_.dot : joiners_.point) + U" ");
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_code(const Match& match, const Reading& reading) const {
  // Letters stay, digit runs read as numbers: MP3, COVID-19, L'A380, F-16s
  std::vector<std::u32string> out;
  bool glue = false;
  std::u32string_view previous;

  for (auto part : code_parts(match.text)) {
    if (part == U"'" && !out.empty()) {
      out.back() += part; // L'A380
      glue = true;
      continue;
    }

    std::optional<std::u32string> word;
    if (part == U"-" || part == U".") {
      word = std::nullopt;
    } else if (utils::unicode::isdigit(part.front()) && part.find(U'.') == std::u32string_view::npos) {
      word = read_code_digits(part);
    } else if (part.front() == U'.' && !out.empty()) {
      word = read_code_digits(part.substr(1)); // H.264
    } else if (utils::unicode::isdigit(part.back())) {
      // 2.5 inside a code, or .5 starting it
      size_t dot = part.find(U'.');
      Number number{.integer = std::u32string(part.substr(0, dot)),
                    .fraction = std::u32string(part.substr(dot + 1)),
                    .decimal_mark = U'.'};
      word = read_number(number, reading);
    } else if (bool after_digits = !previous.empty() && utils::unicode::isdigit(previous.back());
               auto words = part == U"s" && after_digits ? plural(out.back()) : std::nullopt) {
      out.back() = *words; // F-16s
    } else {
      word = std::u32string(part);
    }

    if (word) {
      if (glue && !out.empty()) {
        out.back() += *word;
      } else {
        out.push_back(*word);
      }
    }
    glue = false;
    previous = part;
  }
  return join(out);
}

template <typename Reading>
std::u32string Num2WordLayer<Reading>::read_amount(const Match& match, const Reading& reading,
                                                   AmountReading read) const {
  Number last = match.numbers.back();

  // A unit reads its number without leading zeros: 05 km
  while (match.kind == Kind::UNIT && last.integer.size() > 1 && last.integer[0] == U'0') {
    last.integer.erase(0, 1);
  }

  std::u32string words = (this->*read)(last, match, reading);
  if (match.numbers.size() == 1) {
    return words;
  }
  return read_number(match.numbers[0], reading) + read_range(match.numbers[0], last, reading) + words;
}

} // namespace phonemis::processor::num2word
