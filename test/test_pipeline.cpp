#include <phonemis/pipeline.h>
#include <phonemis/utilities/string_utils.h>
#include <iostream>
#include <vector>
#include <string>

using namespace phonemis;
using namespace phonemis::utilities;

int main() {
  std::string TAGGER_DATA_PATH = "../data/hmm.json";
  std::string LEXICON_DATA_PATH = "../data/dictionaries/us_merged.json";

  const std::string text = "I love it! This is the best day of my entire life.";

  Pipeline pipeline(Lang::EN_US, TAGGER_DATA_PATH, LEXICON_DATA_PATH);
  auto phonemes = pipeline.process(text);

  std::cout << "Text: " << text << "\n";
  std::cout << "Phonemes: " << string_utils::u32string_to_utf8(phonemes) << "\n";

  return 0;
}