#include <phonemis/phonemizer/lexicon.h>
#include <phonemis/phonemizer/constants.h>
#include <phonemis/phonemizer/stress.h>
#include <phonemis/utilities/io_utils.h>
#include <phonemis/utilities/string_utils.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace phonemis::phonemizer {

using namespace utilities;

Lexicon::Lexicon(Lang language, const std::string& dict_filepath)
  : language_(language) {
  // Load the input JSON file
	auto json_obj = io_utils::load_json(dict_filepath);

  // We assume that loaded JSON file is in plain string: string format
  for (auto& item : json_obj.items()) {
    const std::string text = item.key();
    const auto& phonemes = item.value();

    if (!phonemes.is_string())
      throw std::invalid_argument("Unexpected JSON structure in file " + dict_filepath);
    
    // Convert the value to u32string and add the entry
    auto phonemes_u32 = string_utils::utf8_to_u32string(phonemes.get<std::string>());
    dict_[text] = phonemes_u32;

    // In order to make the vocab less case-sensitive, we expand it with 
    // additional entries: lowered and capitalized one if needed.
    auto text_lowered = string_utils::to_lower(text);
    auto text_capitalized = string_utils::capitalize(text);
    if (text.size() >= 2 && text == text_lowered && text != text_capitalized)
      dict_[text_capitalized] = phonemes_u32;
    else if (text.size() >= 2 && text == string_utils::capitalize(text_lowered))
      dict_[text_lowered] = phonemes_u32;
  }
}

bool Lexicon::is_known(const std::string& word) const {
  return dict_.contains(word) || dict_.contains(string_utils::to_lower(word)) ||
         word.size() == 1 && (std::isalpha(word[0]) || constants::alphabet::kSymbols.contains(word[0]));
}

std::u32string Lexicon::lookup(const std::u32string& word,
                               const tagger::Tag& tag,
                               std::optional<float> stress) {
  
}

} // namespace phonemis::phonemizer