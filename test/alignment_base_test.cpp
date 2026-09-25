#include "test.h"
#include <phonemis/base/processor/processor.h>
#include <phonemis/base/processor/sanitizer_layer.h>
#include <phonemis/base/processor/trim_layer.h>
#include <phonemis/lang/en/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace processor;

// Renders an alignment as space-separated "begin-end" spans.
static std::string show(const Alignment& alignment) {
  std::string result;
  for (const auto& span : alignment) {
    if (!result.empty()) result += " ";
    result += std::to_string(span.begin) + "-" + std::to_string(span.end);
  }
  return result;
}

// Renders `count` copies of the same span.
static std::string repeat(const std::string& span, size_t count) {
  std::string result;
  for (size_t i = 0; i < count; ++i) {
    if (!result.empty()) result += " ";
    result += span;
  }
  return result;
}

REGISTER_TEST(alignment_trim_layer_test)
{
  TrimLayer layer;
  Alignment alignment;

  ASSERT_EQUALS(U"a bc", layer.transform(U"  a  bc ", &alignment));
  ASSERT_EQUALS("2-3 3-4 5-6 6-7", show(alignment));

  return true;
}

REGISTER_TEST(alignment_sanitizer_layer_test)
{
  SanitizerLayer layer([](char32_t c) { return c != U'x'; },
                       [](char32_t c) { return c == U'b' ? U'B' : c; });
  Alignment alignment;

  ASSERT_EQUALS(U"aBc", layer.transform(U"axbc", &alignment));
  ASSERT_EQUALS("0-1 2-3 3-4", show(alignment));

  return true;
}

REGISTER_TEST(alignment_num2word_layer_test)
{
  en::Num2Word layer;
  Alignment alignment;

  // Every character of a verbalized number points at the whole number
  ASSERT_EQUALS(U"a twenty five b", layer.transform(U"a 25 b", &alignment));
  ASSERT_EQUALS("0-1 1-2 " + repeat("2-4", 11) + " 4-5 5-6", show(alignment));

  return true;
}

REGISTER_TEST(alignment_processor_compose_test)
{
  Processor processor;
  processor.add_layer(std::make_unique<TrimLayer>());
  processor.add_layer(std::make_unique<en::Num2Word>());
  Alignment alignment;

  // Spans refer to the original input, not to the output of the previous layer
  ASSERT_EQUALS(U"x twenty five", processor.process(U"  x   25", &alignment));
  ASSERT_EQUALS("2-3 3-4 " + repeat("6-8", 11), show(alignment));

  return true;
}

REGISTER_TEST(alignment_source_span_test)
{
  Alignment alignment = {{0, 1}, {2, 4}, {2, 4}, {5, 6}};

  auto span = source_span(alignment, 1, 3);
  ASSERT_EQUALS(2, span.begin);
  ASSERT_EQUALS(4, span.end);

  // Empty ranges anchor before the next character, or after the last one
  span = source_span(alignment, 3, 3);
  ASSERT_EQUALS(5, span.begin);
  ASSERT_EQUALS(5, span.end);
  span = source_span(alignment, 4, 4);
  ASSERT_EQUALS(6, span.begin);
  ASSERT_EQUALS(6, span.end);

  return true;
}

} // namespace phonemis::test
