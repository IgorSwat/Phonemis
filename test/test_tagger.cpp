#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <phonemis/preprocessor/tools.h>
#include <phonemis/tokenizer/tokenize.h>
#include <phonemis/tagger/tagger.h>

using namespace phonemis;

int main() {

  std::string FILEPATH = "../data/hmm.json";

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

  return 0;
}