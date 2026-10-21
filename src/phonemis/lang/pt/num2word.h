#pragma once

#include <phonemis/base/processor/layers/num2word/layer.h>

#include <string>

namespace phonemis::pt {

using processor::num2word::Gender;

// How a Portuguese number is read, from the words around it.
struct NumberReading {
  Gender gender = Gender::MASCULINE; // Of the noun after: duas pessoas; an hour is feminine (às duas)
  bool day = false;                  // 1 de maio: "1" before "de" and a month is "primeiro"
  bool heading = false;              // artigo 80º: an ordinal after a legal heading reads as a cardinal
  bool lettered = false;             // A320-200, 3-4x: a range touching letters is a plain hyphen
  bool named = false;                // R$ 50 reais: the currency is already named after the amount
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
  std::u32string read_compound(const Match& match, const NumberReading& reading) const override;
  std::u32string read_money(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_scale(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_percent(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_unit(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_range(const Number& from, const Number& to, const NumberReading& reading) const override;

private:
  // A number of a given gender, as before a currency or a unit
  std::u32string read_with(const Number& number, Gender gender) const;
};

} // namespace phonemis::pt
