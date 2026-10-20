#pragma once

#include <phonemis/base/processor/layers/num2word/layer.h>

#include <string>

namespace phonemis::es {

// How "uno" ends a number: alone (veintiuno), before a masculine noun (veintiún años) or a feminine one (veintiuna)
enum class One { UNO = 0, UN, UNA };

// How a Spanish number is read, from the words around it.
struct NumberReading {
  One one = One::UNO; // From the noun after the number
  bool named = false; // $5.000 dólares: the currency is already named after the amount
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

private:
  // A number ending with a given form of "uno", as before a currency or a unit
  std::u32string read_with(const Number& number, One one) const;
};

} // namespace phonemis::es
