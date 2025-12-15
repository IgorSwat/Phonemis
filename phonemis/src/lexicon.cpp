#include <phonemis/phonemizer/lexicon.h>
#include <phonemis/phonemizer/constants.h>
#include <phonemis/phonemizer/stress.h>
#include <phonemis/utilities/io_utils.h>
#include <phonemis/utilities/string_utils.h>
#include <filesystem>
#include <fstream>
#include <regex>
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

std::u32string Lexicon::stem_s(const std::string& word,
                               const tagger::Tag& tag,
                               std::optional<float> stress) const {
  std::string stem;

  if (word.size() < 3 || word.back() != 's')
    return U"";
  else if (!string_utils::ends_with(word, "ss") && is_known(word.substr(0, word.size() - 1)))
    stem = word.substr(0, word.size() - 1);
  else if ((string_utils::ends_with(word, "'s") || 
            word.size() > 4 && string_utils::ends_with(word, "es") && !string_utils::ends_with(word, "ies")) &&
            is_known(word.substr(0, word.size() - 2)))
    stem = word.substr(0, word.size() - 2);
  else if (word.size() > 4 && string_utils::ends_with(word, "ies") &&
           is_known(word.substr(0, word.size() - 3) + "y"))
    stem = word.substr(0, word.size() - 3) + "y";
  else
    return U"";

  auto phonemes = lookup(stem, tag, stress);

  if (phonemes.empty())
    return U"";

  // Adjust phonemization according to selected language rules.
  // https://en.wiktionary.org/wiki/-s
  static const std::u32string hard_s_suffixes = U"ptkfθ";
  static const std::u32string soft_s_suffixes = U"szʃʒʧʤ";
  if (hard_s_suffixes.find(phonemes.back()) != std::u32string::npos)
    return phonemes + U"s";
  if (soft_s_suffixes.find(phonemes.back()) != std::u32string::npos)
    return phonemes + (language_ == Lang::EN_GB ? U"ɪ" : U"ᵻ") + U"z";
  
  return phonemes + U"z";
}

std::u32string Lexicon::stem_ed(const std::string& word,
                                const tagger::Tag& tag,
                                std::optional<float> stress) const {
  std::string stem;

  if (word.size() < 4 || word.back() != 'd')
    return U"";
  else if (!string_utils::ends_with(word, "dd") && is_known(word.substr(0, word.size() - 1)))
    stem = word.substr(0, word.size() - 1);
  else if (word.size() > 4 && string_utils::ends_with(word, "ed") &&
           !string_utils::ends_with(word, "eed") && is_known(word.substr(0, word.size() - 2)))
    stem = word.substr(0, word.size() - 2);
  else
    return U"";
  
  auto phonemes = lookup(stem, tag, stress);

  if (phonemes.empty())
    return U"";

  // Adjust phonemization according to selected language rules.
  // https://en.wiktionary.org/wiki/-ed
  static const std::u32string soft_d_suffixes = U"pkfθʃsʧ";
  if (soft_d_suffixes.find(phonemes.back()) != std::u32string::npos)
    return phonemes + U"t";
  if (phonemes.back() == U'd')
    return phonemes + (language_ == Lang::EN_GB ? U"ɪ" : U"ᵻ") + U"d";
  if (phonemes.back() != U't')
    return phonemes + U"d";
  if (language_ == Lang::EN_GB || phonemes.size() < 2)
    return phonemes + U"ɪd";
  if (constants::language::kUSTaus.find(phonemes[phonemes.size() - 2]) != std::u32string::npos)
    return phonemes.substr(0, phonemes.size() - 1) + U"ɾᵻd";
  
  return phonemes + U"ᵻd";
}

std::u32string Lexicon::stem_ing(const std::string& word,
                                 const tagger::Tag& tag,
                                 std::optional<float> stress) const {
  std::string stem;

  static const std::regex ing_pattern("([bcdgklmnprstvxz])\\1ing$|cking$");

  if (word.size() < 5 || !string_utils::ends_with(word, "ing"))
    return U"";
  else if (word.size() > 5 && is_known(word.substr(0, word.size() - 3)))
    stem = word.substr(0, word.size() - 3);
  else if (is_known(word.substr(0, word.size() - 3) + "e"))
    stem = word.substr(0, word.size() - 3) + "e";
  else if (word.size() > 5 && std::regex_search(word, ing_pattern) &&
           is_known(word.substr(0, word.size() - 4)))
    stem = word.substr(0, word.size() - 4);
  else
    return U"";
  
  auto phonemes = lookup(stem, tag, stress);

  if (phonemes.empty())
    return U"";
  
  // Adjust phonemization according to selected language rules.
  // https://en.wiktionary.org/wiki/-ing
  if (language_ == Lang::EN_GB && (phonemes.back() == U'ə' || phonemes.back() == U'ː'))
    return U""; // TODO: fix this
  if (phonemes.size() > 1 && phonemes.back() == U't' &&
      constants::language::kUSTaus.find(phonemes[phonemes.size() - 2]) != std::u32string::npos)
    return phonemes.substr(0, phonemes.size() - 1) + U"ɾɪŋ";
  
  return phonemes + U"ɪŋ";
}

std::u32string Lexicon::lookup(const std::string& word,
                               const tagger::Tag& tag,
                               std::optional<float> stress) const {
  // Lookup with both exact and lower case
  std::u32string phonemes = dict_.contains(word) ? dict_.at(word) :
                            dict_.contains(string_utils::to_lower(word)) 
                              ? dict_.at(string_utils::to_lower(word)) : U"";
  
  bool is_nnp = tag == "NNP";
  bool has_primary_stress = phonemes.find(static_cast<char>(constants::stress::kPrimary)) != std::string::npos;

  // Special case - unknown words & NNP (proper nouns)
  // Since proper noun names could be very unique and not present
  // in the dict, we try to manually resolve them.
  // Note that we also treat unknown words like NNPs.
  if (phonemes.empty() || is_nnp && !has_primary_stress) {
    phonemes = lookup_nnp(word);
    if (!phonemes.empty()) return phonemes;
  }

  return stress.has_value() ? apply_stress(phonemes, stress.value()) : phonemes;
}

std::u32string Lexicon::lookup_nnp(const std::string& word) const {
  // First, filter all non-alpha characters
  std::string word_alpha = string_utils::filter(word, [](char c) -> bool { return std::isalpha(c); });
  size_t no_alphas = word_alpha.size();

  // To handle a most likely unique word, we try to phonemize it letter by letter
  std::u32string phonemes;
  phonemes.reserve(no_alphas);
  for (char c : word_alpha) {
    if (!dict_.contains(std::string(1, c)))
      return U"";
    
    phonemes += dict_.at(std::string(1, c));
  }

  phonemes = apply_stress(phonemes, 1.F);

  // Reorganize stress characters
  // We split the string according to the secondary stress character's last position.
  size_t last_ssc = phonemes.find_last_of(constants::stress::kSecondary);
  bool has_secondary = last_ssc != std::u32string::npos;
  std::u32string first_part = has_secondary ? phonemes.substr(0, last_ssc) : U"";
  std::u32string second_part = has_secondary ? phonemes.substr(last_ssc + 1) : phonemes;

  // Join and return
  return first_part + std::u32string(1, constants::stress::kPrimary) + second_part;
}

} // namespace phonemis::phonemizer