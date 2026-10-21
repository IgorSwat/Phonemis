#include "processing.h"
#include "test.h"
#include <phonemis/lang/en/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

// English readings follow the Echo verbalizer; ISO dates are read by phonemis only.
REGISTER_TEST(num2word_en_test) {
  en::Num2Word layer;

  ASSERT_EQUALS(U"The meeting is at ten thirty, about twenty to twenty-five minutes.",
                apply(layer, U"The meeting is at 10:30, about 20-25 minutes."));
  ASSERT_EQUALS(U"He was born on March third, nineteen ninety, and paid one point five million dollars.",
                apply(layer, U"He was born on March 3, 1990, and paid $1.5 million."));
  ASSERT_EQUALS(U"About one thousand nine hundred ninety-nine people, a five dollar bill and SCP six oh six's.",
                apply(layer, U"About 1999 people, a $5 bill and SCP-606's."));
  ASSERT_EQUALS(U"It costs four ninety-nine or ninety-nine cents, maybe fifteen euros or fifty dollars.",
                apply(layer, U"It costs $4.99 or 99 cents, maybe €15 or 50$."));
  ASSERT_EQUALS(U"The twenty-first century began in two thousand one; the nineteen nineties and eighties were fun.",
                apply(layer, U"The 21st century began in 2001; the 1990s and '80s were fun."));
  ASSERT_EQUALS(U"A seventy-five millimeter gun fired three rounds at seven p.m., ten to fifteen kilometers away.",
                apply(layer, U"A 75mm gun fired 3 rounds at 7pm, 10-15 km away."));
  ASSERT_EQUALS(U"Version three point eight point ten runs on ten dot zero dot zero dot fifty-one with nineteen twenty by ten eighty screens.",
                apply(layer, U"Version 3.8.10 runs on 10.0.0.51 with 1920x1080 screens."));
  ASSERT_EQUALS(U"Call one five five five, one two three, four five six seven or five five five, one two three four now.",
                apply(layer, U"Call 1-555-123-4567 or 555-1234 now."));
  ASSERT_EQUALS(U"Temperatures of minus five degrees Celsius, twenty percent humidity and one half of the three/seven votes.",
                apply(layer, U"Temperatures of -5 °C, 20% humidity and 1/2 of the 3/7 votes."));
  ASSERT_EQUALS(U"The score was two to one and five minus three equals two.",
                apply(layer, U"The score was 2:1 and 5 - 3 equals 2."));
  ASSERT_EQUALS(U"MP three, COVID nineteen, F sixteens and H two sixty-four are codes; a twenty-one-year-old won.",
                apply(layer, U"MP3, COVID-19, F-16s and H.264 are codes; a 21-year-old won."));
  ASSERT_EQUALS(U"It's one thirteen not one thirteen, and two plus two.",
                apply(layer, U"It's 1.13 not 1:13, and 2 + 2."));
  ASSERT_EQUALS(U"On twenty-seventh March twenty twenty-six and twenty-seventh March twenty twenty-six we met.",
                apply(layer, U"On 2026-03-27 and 27.03.2026 we met."));
  ASSERT_EQUALS(U"We counted one million people and point five of them.",
                apply(layer, U"We counted 1,000,000 people and .5 of them."));
  ASSERT_EQUALS(U"The four K screen and five hundred thousand fans, three billion dollars.",
                apply(layer, U"The 4K screen and 500k fans, 3 bn dollars."));

  return true;
}

} // namespace phonemis::test
