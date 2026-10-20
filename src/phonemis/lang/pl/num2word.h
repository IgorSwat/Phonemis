#pragma once

#include <phonemis/base/processor/layers/num2word/layer.h>

#include <string>

namespace phonemis::pl {

using processor::num2word::Case;
using processor::num2word::Form;
using processor::num2word::Gender;

// How a Polish number is read: its form, and the case and gender its words agree with.
struct NumberReading {
  Form form = Form::CARDINAL; // An ordinal for years, days of a month and headings
  Case grammatical_case = Case::NOMINATIVE;
  Gender gender = Gender::MASCULINE;
  bool plural = false;                 // lata 70.: an ordinal in the plural
  Case amount_case = Case::NOMINATIVE; // The case of an amount (5 kg): of its number and its noun
  bool numbered = false;               // numerem 3x38: the "x" of a document number stands for a slash
};

/**
 * Polish numbers inflect for case and gender, so most readings first find the case a number takes:
 * from the preposition before it (od pięciu), the noun after it (z trzema kolegami), a governing noun or verb
 * (od kwoty trzystu tysięcy), or the year and date patterns which make it an ordinal (w 1985 roku).
 */
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
  std::u32string read_time(const Match& match, const NumberReading& reading) const override;
  std::u32string read_ordinal(const Match& match, const NumberReading& reading) const override;
  std::u32string read_compound(const Match& match, const NumberReading& reading) const override;
  std::u32string read_plain(const Match& match, const NumberReading& reading) const override;
  std::u32string read_money(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_scale(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_percent(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_unit(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_joiner(const Match& match, const NumberReading& reading) const override;
};

} // namespace phonemis::pl
