#include "test.h"
#include <phonemis/lang/en/hmm_tagger.h>
#include <phonemis/utils/conversions.h>

#include <vector>

namespace phonemis::test {

using namespace tagger;

REGISTER_TEST(hmm_tagger_en_basic_test) {
  Config config;
  config.data_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/en-us/tagger.json";

  en::HMMTagger tagger(config);

  // Test sentence: "I read a book." vs "I read that book yesterday."
  // "read" can be VBD (past) or VBP (present)
  std::vector<Token> tokens = {
    {U"I", true, true},
    {U"read", false, true},
    {U"a", false, true},
    {U"book", false, false},
    {U".", false, true}
  };

  tagger.tag(tokens);

  // Basic validation that tags are assigned
  for (const auto& token : tokens) {
    ASSERT_EQUALS(true, token.tag.has_value());
  }

  // "I" should be a pronoun (PRP)
  ASSERT_EQUALS("PRP", tokens[0].tag.value());
  // "read" - VBP (Verb, present tense)
  ASSERT_EQUALS("VBP", tokens[1].tag.value());
  // "a" - DT (Determiner)
  ASSERT_EQUALS("DT", tokens[2].tag.value());
  // "book" - NN (Noun, singular or mass)
  ASSERT_EQUALS("NN", tokens[3].tag.value());
  // "." - . (Sentence-final punctuation)
  ASSERT_EQUALS(".", tokens[4].tag.value());
  
  return true;
}

REGISTER_TEST(hmm_tagger_en_case_insensitivity_test) {
  Config config;
  config.data_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/en-us/tagger.json";

  en::HMMTagger tagger(config);

  // Test lowercase vs uppercase start of sentence
  std::vector<Token> tokens_upper = {{U"The", true, true}, {U"cat", false, true}};
  std::vector<Token> tokens_lower = {{U"the", true, true}, {U"cat", false, true}};

  tagger.tag(tokens_upper);
  tagger.tag(tokens_lower);

  ASSERT_EQUALS(tokens_lower[0].tag.value(), tokens_upper[0].tag.value());
  ASSERT_EQUALS(tokens_lower[1].tag.value(), tokens_upper[1].tag.value());

  return true;
}

} // namespace phonemis::test
