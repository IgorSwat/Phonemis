#include "processing.h"
#include "test.h"
#include <phonemis/base/processor/layers/num2word/layer.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace processor;
using namespace processor::num2word;

namespace {
struct NoReading {};

// Digits spelled out, so that later patterns do not read them again: 27 -> two-seven
std::u32string spelled(std::u32string_view digits) {
  static const std::u32string names[] = {U"zero", U"one", U"two",   U"three", U"four",
                                         U"five", U"six", U"seven", U"eight", U"nine"};
  std::u32string words;
  for (char32_t c : digits) {
    words += (words.empty() ? U"" : U"-") + names[c - U'0'];
  }
  return words;
}

// A language which marks the numbers its patterns find (<one-two>) and keeps the default readings of the rest.
class MarkingNum2Word : public Num2WordLayer<NoReading> {
public:
  MarkingNum2Word()
      : Num2WordLayer(Notation{.group_separators = U",",
                               .decimal_separators = U".",
                               .time_separators = U":",
                               .currencies = {U"$"},
                               .scales = {U"k"},
                               .units = {U"km"},
                               .fractions = true},
                      Joiners{.point = U"point",
                              .dot = U"dot",
                              .range = U" to ",
                              .by = U"by",
                              .score = U" to ",
                              .minus = U"minus",
                              .plus = U"plus"}) {}

protected:
  NoReading resolve(const Match& /*match*/, const Context& /*context*/) const override { return {}; }

  std::u32string read_number(const Number& number, const NoReading& /*reading*/) const override {
    std::u32string fraction = number.fraction.empty() ? U"" : U" point " + spelled(number.fraction);
    return U"<" + spelled(number.integer) + fraction + U">";
  }

  std::u32string read_code_digits(std::u32string_view digits) const override {
    return U"<" + spelled(digits) + U">";
  }
};
} // namespace

REGISTER_TEST(num2word_base_kinds_test) {
  MarkingNum2Word layer;

  // Whole expressions
  ASSERT_EQUALS(U"<two-seven> <zero-three> <two-zero-two-six>", apply(layer, U"2026-03-27"));
  ASSERT_EQUALS(U"<three> point <eight> point <one-zero>", apply(layer, U"3.8.10"));
  ASSERT_EQUALS(U"<one-zero> dot <zero> dot <zero> dot <five-one>", apply(layer, U"10.0.0.51"));

  // Symbols between numbers
  ASSERT_EQUALS(U"<five> minus <three>, <two> plus <two>", apply(layer, U"5 - 3, 2+2"));
  ASSERT_EQUALS(U"<one-zero> to <one-five>, <one-nine-two-zero> by <one-zero-eight-zero>",
                apply(layer, U"10-15, 1920x1080"));
  ASSERT_EQUALS(U"<three> <four> and minus <five>", apply(layer, U"3/4 and -5"));
  ASSERT_EQUALS(U"<one-zero> <three-zero> and <two> to <one>", apply(layer, U"10:30 and 2:1"));

  // Numbers with what they count
  ASSERT_EQUALS(U"<five> $, <five-zero-zero> k, <two-zero> %, <one-zero> to <one-five> km",
                apply(layer, U"$5, 500k, 20%, 10-15 km"));

  // Numbers joined to letters, then any other number
  ASSERT_EQUALS(U"a <two-one>-year-old MP <three>", apply(layer, U"a 21-year-old MP3"));
  ASSERT_EQUALS(U"<one-zero-zero-zero-zero-zero-zero point five> and < point five>",
                apply(layer, U"1,000,000.5 and .5"));

  return true;
}

REGISTER_TEST(num2word_base_untouched_text_test) {
  MarkingNum2Word layer;

  ASSERT_EQUALS(U"", apply(layer, U""));
  ASSERT_EQUALS(U"No numbers here.", apply(layer, U"No numbers here."));

  return true;
}

} // namespace phonemis::test
