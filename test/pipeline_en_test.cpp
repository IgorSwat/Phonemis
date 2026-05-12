#include "test.h"
#include <phonemis/lang/en/pipeline.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace en;

static Pipeline g_pipeline(Config{
    .lang = "en-us",
    .tagger = tagger::Config{
      .data_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/en-us/tagger.json"
    },
    .phonemizer = phonemizer::Config{
      .lexicon_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/en-us/lexicon_small.json"
    }
});

REGISTER_TEST(pipeline_en_us_integration_test) {
  // Test case with a few sentences, some number, and common English words.
  std::string input = "Hello world! I have 2 cats and they are used to be here.";
  
  // The expected output assumes:
  // 1. Preprocessor converts "2" to "two"
  // 2. Tokenizer splits correctly
  // 3. Tagger assigns PoS tags for "used to" handling
  // 4. LexiconPhonemizer produces IPA phonemes
  std::u32string result = g_pipeline(input);

  ASSERT_EQUALS(U"həlˈO wˈɜɹld! ˌI hæv tˈu kˈæts ænd ðA ɑɹ jˈust tə bi hˈɪɹ.", result);

  return true;
}

REGISTER_TEST(pipeline_en_us_contraction_test) {
  // Increase complexity: Contractions and multiple numbers
  std::string input = "He's 25 years old. It's 10:30 PM.";
  std::u32string result = g_pipeline(input);

  // Expected: "He's" split to "He" + "'s", "25" to "twenty-five", "10:30" to "ten thirty"
  ASSERT_EQUALS(U"hˌiz twˈɛnti fˈIv jˈɪɹz ˈOld. ˌɪts tˈɛn:θˈɜɹɾi pˌiˈɛm.", result);
  return true;
}

REGISTER_TEST(pipeline_en_us_special_suffixes_test) {
  // Increase complexity: morphological suffixes (-s, -ed, -ing)
  std::string input = "She looked at the running dogs.";
  std::u32string result = g_pipeline(input);

  // Checks stem lookups (look+ed, run+ing, dog+s)
  ASSERT_EQUALS(U"ʃˌi lˈʊkt æt ðə ɹˈʌnɪŋ dˈɔɡz.", result);
  return true;
}

REGISTER_TEST(pipeline_en_us_ambiguity_test) {
  std::string input = "I read the new book. I have read the old book.";
  std::u32string result = g_pipeline(input);

  // Sentence 1: "read" (VBP/VBD) -> ɹˈid or ɹˈɛd (HMM context)
  // Sentence 2: "read" (VBN) -> ɹˈɛd
  // This test validates that the tagger affects the phonemizer lookup.
  ASSERT_EQUALS(U"ˌI ɹˈid ðə nˈu bˈʊk. ˌI hæv ɹˈɛd ði ˈOld bˈʊk.", result);
  return true;
}

REGISTER_TEST(pipeline_en_us_acronym_test) {
  // Complex case: Mixed case, acronyms, and punctuation
  std::string input = "The FBI and CIA are in the USA.";
  std::u32string result = g_pipeline(input);

  // Validates NNP lookup logic and vowel-next context for "the"
  ASSERT_EQUALS(U"ði ˌɛfbˌiˈI ænd sˌiˌIˈA ɑɹ ɪn ðə jˌuˌɛsˈA.", result);
  return true;
}

} // namespace phonemis::test