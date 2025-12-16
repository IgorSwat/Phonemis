#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <phonemis/phonemizer/lexicon.h>
#include <phonemis/preprocessor/tools.h>
#include <phonemis/tokenizer/tokenize.h>
#include <phonemis/tagger/tagger.h>
#include <phonemis/utilities/string_utils.h>

using namespace phonemis;

int main() {

  std::string FILEPATH = "../data/hmm.json";
  std::string LEXICON_PATH = "../data/dictionaries/us_merged.json";

  std::string text = "It's a great feeling to be a part of the 12th company! I am so proud of me.";

  // Preprocess the text
  auto verbalized_text = preprocessor::verbalize_numbers(text);
  std::cout << "[After verbalize_numbers] " << verbalized_text << "\n";
  auto sentences = preprocessor::split_sentences(verbalized_text);
  std::cout << "[After split_sentences]\n";
  for (const auto& sentence : sentences) {
    std::cout << "Sentence: " << sentence << "\n";
  }

  std::cout << "\nNow continouing with the first sentence...\n\n";

  auto tokens = tokenizer::tokenize(sentences[0]);
  std::cout << "[After tokenize]\n";
  for (const auto& token : tokens) {
    std::cout << "Token: " << token.text << ", is_first? " << token.is_first << ", whitespace size: " << token.whitespace.size() << "\n";
  }
  
  auto tagger = std::make_unique<tagger::Tagger>(FILEPATH);
  tagger->tag(tokens);
  std::cout << "[After tag]\n";
  for (const auto& token : tokens) {
    std::cout << "Token: " << token.text << ", tag: " << token.tag.value() << "\n";
  }

  auto lexicon = std::make_unique<phonemizer::Lexicon>(phonemizer::Lang::EN_US, LEXICON_PATH);
  std::cout << "\nPhonemizing...\n\n";
  for (const auto& token : tokens) {
    auto phonemes = lexicon->get(token.text, token.tag.value());
    std::cout << "Word: " << token.text << ", phonemes: " << utilities::string_utils::u32string_to_utf8(phonemes) << "\n";
  }

  return 0;
}