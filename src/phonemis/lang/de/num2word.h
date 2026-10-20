#pragma once

#include <phonemis/base/processor/layers/num2word/layer.h>

#include <string>

namespace phonemis::de {

using processor::num2word::Case;
using processor::num2word::Gender;

// How the ending of an ordinal follows the word before it: am dritten (weak), ein zweiter (mixed), dritter (strong)
enum class Declension { WEAK = 0, MIXED, STRONG };

// How a German number is read, from the words around it.
struct NumberReading {
  Case grammatical_case = Case::NOMINATIVE; // Given by the word before: mit einer Stimme, am dritten
  Gender gender = Gender::MASCULINE;        // Of the noun after: eine Stunde, der dritte Platz
  Declension declension = Declension::STRONG;
  bool article = false;    // "1" before a noun is said like an article: eine Stunde
  bool year = true;        // A 4-digit number reads as a year (1998 Abi), unless it counts a noun (1945 Einwohner)
  bool capitalized = true; // A decade is a noun (die Neunziger), but an adjective before Jahre (neunziger Jahre)
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
  std::u32string read_compound(const Match& match, const NumberReading& reading) const override;
  std::u32string read_money(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_scale(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_percent(const Number& number, const Match& match, const NumberReading& reading) const override;
  std::u32string read_unit(const Number& number, const Match& match, const NumberReading& reading) const override;
};

} // namespace phonemis::de
