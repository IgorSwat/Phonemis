#include "test.h"
#include <phonemis/lang/en/pipeline.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace en;

static Pipeline g_pipeline(Config{
    .lang = Lang::EN_US,
    .tagger = {.data_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/english/hmm.json"},
    .phonemizer = {.lexicon_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/english/us_small.json"}
});

REGISTER_TEST(pipeline_en_us_integration_simple_test) {
  // Test case with a few sentences, some number, and common English words.
  std::string input = "Hello world! I have 2 cats and they are used to be here.";
  
  // The expected output assumes:
  // 1. Preprocessor converts "2" to "two"
  // 2. Tokenizer splits correctly
  // 3. Tagger assigns PoS tags for "used to" handling
  // 4. LexiconPhonemizer produces IPA phonemes
  std::u32string result = g_pipeline.process(input);

  ASSERT_EQUALS(U"həlˈO wˈɜɹld! ˌI hæv tˈu kˈæts ænd ðA ɑɹ jˈuzd tə bi hˈɪɹ.", result);

  return true;
}

} // namespace phonemis::test