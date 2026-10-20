#pragma once

#include <phonemis/base/processor/layers/num2word/layer.h>

#include <optional>
#include <string>

namespace phonemis::en {

// How an English number is read, from the words around it.
struct NumberReading {
  bool after_month = false; // March 3: a day of the month, read as an ordinal
  bool counted = false;     // 1999 people: a count rather than a year
  bool attributive = false; // a $5 bill: before a noun, the unit stays singular
};

class Num2Word : public processor::num2word::Num2WordLayer<NumberReading> {
public:
  Num2Word();

protected:
  using Context = processor::Context;
  using Match = processor::num2word::Match;
  using Number = processor::num2word::Number;

  // Resolution
  bool accept(const Match& match, const Context& context) const override;
  NumberReading resolve(const Match& match, const Context& context) const override;

  // Readings
  std::u32string read_number(const Number& number, const NumberReading& reading) const override;
  std::u32string read_code_digits(std::u32string_view digits) const override;
  std::u32string read_date(const Match& match, const NumberReading& reading) const override;
  std::u32string read_phone(const Match& match, const NumberReading& reading) const override;
  std::u32string read_fraction(const Match& match, const NumberReading& reading) const override;
  std::u32string read_time(const Match& match, const NumberReading& reading) const override;
  std::u32string read_ordinal(const Match& match, const NumberReading& reading) const override;
  std::u32string read_decade(const Match& match, const NumberReading& reading) const override;
  std::u32string read_money(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_scale(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_percent(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_unit(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_range(const Number& from, const Number& to, const NumberReading& reading) const override;
  std::optional<std::u32string> plural(std::u32string_view words) const override;

private:
  // An integer as a year, a day of a month or a count, digit by digit if it is a code
  std::u32string read_integer(const Number& number, const NumberReading& reading) const;
};

} // namespace phonemis::en
