#include "test.h"
#include <phonemis/base/processor/preprocessor.h>
#include <phonemis/lang/en/constants.h>
#include <phonemis/lang/en/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace processor;

namespace {
const Language kEnglish = {.terms = &en::constants::term2word::kTerms, .symbols = &en::constants::sym2word::kSymbols};

// Preprocesses a text in English with a configuration
std::u32string preprocess(const Config& config, std::u32string_view text) {
  return make_preprocessor<en::Num2Word>(config, kEnglish).process(text);
}
} // namespace

REGISTER_TEST(preprocessor_config_layers_test) {
  std::u32string_view text = U"  Dr. Smith’s 2 cats & e.g. example.com  ";

  ASSERT_EQUALS(U"Doctor Smith's two cats and for example example dot com", preprocess({}, text));
  ASSERT_EQUALS(U"  Doctor Smith's two cats and for example example dot com  ", preprocess({.trim = false}, text));
  ASSERT_EQUALS(U"Dr. Smith's two cats and e.g. example.com", preprocess({.terms = false}, text));
  ASSERT_EQUALS(U"Doctor Smith's 2 cats and for example example dot com", preprocess({.numbers = false}, text));
  ASSERT_EQUALS(U"Doctor Smith's two cats & for example example dot com", preprocess({.symbols = false}, text));
  ASSERT_EQUALS(U"Doctor Smith’s two cats and for example example dot com", preprocess({.normalize = false}, text));

  // Nothing at all
  Config none = {.normalize = false, .trim = false, .terms = false, .numbers = false, .symbols = false};
  ASSERT_EQUALS(std::u32string(text), preprocess(none, text));

  // A language's character replacements, last
  Language replacing = kEnglish;
  Replacements replacements = {{U'ß', U's'}};
  replacing.replacements = &replacements;
  ASSERT_EQUALS(U"Strase two", make_preprocessor<en::Num2Word>({}, replacing).process(U"Straße 2"));
  ASSERT_EQUALS(U"Straße two", make_preprocessor<en::Num2Word>({.sanitize = false}, replacing).process(U"Straße 2"));

  return true;
}

REGISTER_TEST(preprocessor_config_tables_test) {
  // Words added to the language's own, or replacing them
  Config config = {.abbreviations = {{U"approx.", {U"approximately"}}, {U"Dr.", {U"Drive"}}},
                   .symbol_words = {{U"™", {U"trademark"}}, {U"&", {U"plus"}}}};
  ASSERT_EQUALS(U"approximately five kilometers on Main Drive, Brand trademark plus more",
                preprocess(config, U"approx. 5 km on Main Dr., Brand™ & more"));

  return true;
}

} // namespace phonemis::test
