#include "test.h"
#include <phonemis/base/tagger/tagger.h>

#include <vector>

namespace phonemis::test {

using namespace tagger;

// A simple implementation of Tagger for testing purposes.
// It assigns a sequential Tag to each token in a sentence starting from 1
// (e.g., first token gets Tag(1), second gets Tag(2), etc.).
class DumbTagger : public Tagger {
public:
  void tag_sentence(std::span<Token> sentence) const override {
    for (size_t i = 0; i < sentence.size(); ++i) {
      sentence[i].tag = static_cast<Tag>(std::to_string(i + 1));
    }
  }
};

REGISTER_TEST(tagger_base_sentence_splitting_test) {
  DumbTagger tagger;

  // Case 1: Two sentences: "Hello world!" and "How are you?"
  std::vector<Token> tokens = {
    {U"Hello", true, true},   // Sentence 1, Token 1
    {U"world", false, false}, // Sentence 1, Token 2
    {U"!", false, true},      // Sentence 1, Token 3 (EOS)
    {U"How", true, true},     // Sentence 2, Token 1
    {U"are", false, true},    // Sentence 2, Token 2
    {U"you", false, false},   // Sentence 2, Token 3
    {U"?", false, true}       // Sentence 2, Token 4 (EOS)
  };

  tagger.tag(tokens);

  // Check Sentence 1
  ASSERT_EQUALS(tokens[0].tag.value(), "1");
  ASSERT_EQUALS(tokens[1].tag.value(), "2");
  ASSERT_EQUALS(tokens[2].tag.value(), "3");

  // Check Sentence 2
  ASSERT_EQUALS(tokens[3].tag.value(), "1");
  ASSERT_EQUALS(tokens[4].tag.value(), "2");
  ASSERT_EQUALS(tokens[5].tag.value(), "3");
  ASSERT_EQUALS(tokens[6].tag.value(), "4");

  // Case 2: One sentence with EOS, second one without.
  std::vector<Token> tokens_trailing = {
    {U"Exit", true, false},   // Sentence 1, Token 1
    {U";", false, true},      // Sentence 1, Token 2 (EOS)
    {U"Run", true, true}      // Sentence 2, Token 1 (Trailing sentence)
  };

  tagger.tag(tokens_trailing);

  // Check Sentence 1
  ASSERT_EQUALS(tokens_trailing[0].tag.value(), "1");
  ASSERT_EQUALS(tokens_trailing[1].tag.value(), "2");

  // Check Sentence 2 (should be processed even without final EOS)
  ASSERT_EQUALS(tokens_trailing[2].tag.value(), "1");

  // Case 3: Sentence with special character but no trailing whitespace (should be ONE sentence)
  std::vector<Token> tokens_no_whitespace = {
    {U"End", true, false},    // Token 1
    {U".", false, false},     // Token 2 (EOS char, but whitespace=false)
    {U"Now", false, true}      // Token 3
  };

  tagger.tag(tokens_no_whitespace);

  // Check that it's treated as one single sentence
  ASSERT_EQUALS(tokens_no_whitespace[0].tag.value(), "1");
  ASSERT_EQUALS(tokens_no_whitespace[1].tag.value(), "2");
  ASSERT_EQUALS(tokens_no_whitespace[2].tag.value(), "3");

  return true;
}

} // namespace phonemis::tagger