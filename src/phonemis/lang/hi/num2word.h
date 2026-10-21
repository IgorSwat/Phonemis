#pragma once

#include <phonemis/base/processor/layers/num2word/layer.h>

#include <string>

namespace phonemis::hi {

// Hindi numbers are read without context so far.
struct NumberReading {};

// TODO: outdated - Hindi readings predate the context-aware design and have no reference verbalizer.
// Missing: Indian digit grouping (1,00,000), Devanagari digits, units and context (gender) of the counted noun.
class Num2Word : public processor::num2word::Num2WordLayer<NumberReading> {
public:
  Num2Word();

protected:
  using Context = processor::Context;
  using Match = processor::num2word::Match;
  using Number = processor::num2word::Number;

  // Resolution
  NumberReading resolve(const Match& match, const Context& context) const override { return {}; }

  // Readings
  std::u32string read_number(const Number& number, const NumberReading& reading) const override;
  std::u32string read_code_digits(std::u32string_view digits) const override;
  std::u32string read_date(const Match& match, const NumberReading& reading) const override;
  std::u32string read_fraction(const Match& match, const NumberReading& reading) const override;
  std::u32string read_ordinal(const Match& match, const NumberReading& reading) const override;
  std::u32string read_money(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_percent(const Number& number, const Match& match, const NumberReading& reading) const override;
};

} // namespace phonemis::hi
