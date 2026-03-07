#include <phonemis/pipeline.h>
#include <phonemis/utilities/string_utils.h>
#include <iostream>
#include <vector>
#include <string>

using namespace phonemis;
using namespace phonemis::utilities;

int main() {
  std::string TAGGER_DATA_PATH = "../data/hmm.json";
  std::string US_LEXICON_PATH = "../data/dictionaries/us_merged.json";
  std::string GB_LEXICON_PATH = "../data/dictionaries/gb_merged.json";

  Pipeline us_pipeline(Lang::EN_US, TAGGER_DATA_PATH, US_LEXICON_PATH);
  Pipeline gb_pipeline(Lang::EN_GB, TAGGER_DATA_PATH, GB_LEXICON_PATH);

  // Original test
  const std::string text = "Damian cloud is a real beast! He is the 66th of the raiders!";
  auto phonemes = us_pipeline.process(text);
  std::cout << "Text: " << text << "\n";
  std::cout << "Phonemes: " << string_utils::u32string_to_utf8(phonemes) << "\n\n";

  // Compound word fallback tests
  std::vector<std::string> compound_tests = {
    "Holloway", "Galloway", "Calloway", "Stanfield", "Ridgeway"
  };

  std::cout << "=== Compound word fallback (US) ===\n";
  for (const auto& word : compound_tests) {
    auto p = us_pipeline.process(word);
    std::cout << word << ": " << string_utils::u32string_to_utf8(p) << "\n";
  }

  std::cout << "\n=== Compound word fallback (GB) ===\n";
  for (const auto& word : compound_tests) {
    auto p = gb_pipeline.process(word);
    std::cout << word << ": " << string_utils::u32string_to_utf8(p) << "\n";
  }

  return 0;
}
