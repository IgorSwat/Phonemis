#pragma once

#include <phonemis/base/processor/layers/num2word/layer.h>

#include <string>

namespace phonemis::fr {

// How a French number is read, from the words around it.
struct NumberReading {
  bool feminine = false; // vingt et une heures: before a feminine noun
  bool plural_s = true;  // quatre-vingts, deux cents; but les années quatre-vingt
  bool equation = false; // 12 x 4 = 48: "x" is "fois" rather than "par"
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
  std::u32string read_money(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_scale(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_percent(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_unit(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_joiner(const Match& match, const NumberReading& reading) const override;

private:
  // A number with an explicit gender, as before a currency or a unit
  std::u32string read_gendered(const Number& number, const NumberReading& reading, bool feminine) const;
};

} // namespace phonemis::fr
