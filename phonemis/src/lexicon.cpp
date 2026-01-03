#include <phonemis/phonemizer/lexicon.h>
#include <phonemis/phonemizer/constants.h>
#include <phonemis/phonemizer/stress.h>
#include <phonemis/utilities/io_utils.h>
#include <phonemis/utilities/string_utils.h>
#include <filesystem>
#include <fstream>
#include <numeric>
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

std::u32string Lexicon::get(const std::string& word, 
                            const tagger::Tag& tag,
                            std::optional<float> base_stress,
                            std::optional<bool> vowel_next) {
  std::optional<float> stress = word == string_utils::to_lower(word) ? std::nullopt :
                                word == string_utils::to_upper(word) ? 
                                  std::make_optional(2.F) : std::make_optional(0.5F);
  
  // Phonemize
  std::u32string phonemes = get_word(word, tag, stress, vowel_next);

  // Apply base stress
  // TODO: consider dealing with some trailing currency characters here
  if (!phonemes.empty() && base_stress.has_value())
    return apply_stress(phonemes, base_stress.value());
  
  return phonemes;
}

std::u32string Lexicon::get_word(const std::string& word,
                                 const tagger::Tag& tag,
                                 std::optional<float> stress,
                                 std::optional<bool> vowel_next) const {
  // Lookup for special words
  std::u32string phonemes = lookup_special(word, tag, stress, vowel_next);
  if (!phonemes.empty())
    return phonemes;
  
  // TODO: add unicode normalization
  std::string used_word = word;
  std::string lower = string_utils::to_lower(word);
  if (word.size() > 1 &&
      string_utils::is_alpha(string_utils::filter(word, [](char c) -> bool { return c != '\''; })) &&
      word != lower &&
      (tag != "NNP" || word.size() > 7) &&
      !dict_.contains(word) &&
      (word == string_utils::to_upper(word) || word.substr(1) == string_utils::to_lower(word.substr(1))) &&
      (dict_.contains(lower) || stem_s(word, tag, stress) != U"" ||
        stem_ed(word, tag, stress) != U"" || stem_ing(word, tag, stress) != U""))
    used_word = lower;
  
  if (is_known(used_word))
    return lookup(word, tag, stress);
  if (string_utils::ends_with(used_word, "s'") && is_known(used_word.substr(0, used_word.size() - 2) + "'s"))
    return lookup(used_word.substr(0, used_word.size() - 2) + "'s", tag, stress);
  if (string_utils::ends_with(used_word, "'") && is_known(used_word.substr(0, used_word.size() - 1)))
    return lookup(used_word.substr(0, used_word.size() - 1), tag, stress);
  
  for (auto stem_f : {&Lexicon::stem_s, &Lexicon::stem_ed}) {
    phonemes = (this->*stem_f)(used_word, tag, stress);
    if (!phonemes.empty()) 
      return phonemes;
  }

  phonemes = stem_ing(used_word, tag, stress.has_value() ? stress.value() : 0.5F);
  if (!phonemes.empty())
    return phonemes;
  
  if (used_word != lower && 
      dict_.contains(lower))
    return dict_.at(lower);
  
  return U"";
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
  bool has_primary_stress = phonemes.find(constants::stress::kPrimary) != std::u32string::npos;

  // Special case - unknown words & NNP (proper nouns)
  // Since proper noun names could be very unique and not present
  // in the dict, we try to manually resolve them.
  // Note that we also treat unknown words like NNPs.
  if (phonemes.empty() || is_nnp && !has_primary_stress) {
    auto phonemes_nnp = lookup_nnp(word);
    if (!phonemes_nnp.empty()) return phonemes_nnp;
    else return phonemes;
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

  if (first_part.empty() && second_part.empty())
    return U"";

  // Join and return
  return first_part + std::u32string(1, constants::stress::kPrimary) + second_part;
}

std::u32string 
Lexicon::lookup_special(const std::string& word,
                        const tagger::Tag& tag,
                        std::optional<float> stress,
                        std::optional<bool> vowel_next) const {
  bool is_single_char = word.size() == 1;
  bool is_add_symbol = is_single_char && constants::alphabet::kAddSymbols.contains(word[0]);
  bool is_other_symbol = is_single_char && constants::alphabet::kSymbols.contains(word[0]);

  std::string word_stripped = string_utils::strip(word, std::make_optional('.'));
  std::string word_without_dots = string_utils::filter(word, [](char c) -> bool { return c != '.'; });
  std::vector<std::string> word_splitted = string_utils::split(word_stripped, '.');
  size_t max_subword_size = 
    std::accumulate(word_splitted.begin(), word_splitted.end(), 0LL, 
      [](size_t m, const auto& str) { return std::max(m, str.size()); });
  
  
  if (tag == "ADD" && is_add_symbol)
    return lookup(constants::alphabet::kAddSymbols.at(word[0]), {""}, {-0.5F});
  else if (is_other_symbol)
    return lookup(constants::alphabet::kSymbols.at(word[0]), {""}, {});
  else if (word_stripped.find('.') != std::string::npos &&
           string_utils::is_alpha(word_without_dots) && 
           max_subword_size < 3)
    return lookup_nnp(word);
  else if (is_single_char && (word[0] == 'a' || word[0] == 'A'))
    return tag == "DT" ? U"ɐ" : U"ˈA";
  else if (word == "am" || word == "Am" || word == "AM") {
    if (string_utils::starts_with(tag, "NN"))
      return lookup_nnp(word);
    if (!vowel_next.has_value() || word != "am" || stress.has_value() && stress.value() > 0)
      return dict_.at("am");
    else
      return U"ɐm";
  }
  else if (word == "an" || word == "An" || word == "AN")
    return word == "AN" && string_utils::starts_with(tag, "NN") ? lookup_nnp(word) : U"ɐn";
  else if (is_single_char && word[0] == 'I' && tag == "PRP")
    return std::u32string(1, constants::stress::kSecondary) + U"I";
  else if ((word == "by" || word == "By" || word == "BY") && tag.parent_tag() == "ADV")
    return U"bˈI";
  else if (word == "to" || word == "To" || word == "TO" && (tag == "TO" || tag == "IN"))
    return !vowel_next.has_value() ? dict_.at("to") :
           vowel_next.value() ? U"tʊ" : U"tə";
  else if (word == "in" || word == "In" || word == "IN" && tag != "NNP")
    return (!vowel_next.has_value() || tag != "IN" ? std::u32string(1, constants::stress::kPrimary) : U"") + U"ɪn";
  else if (word == "the" || word == "The" || word == "THE" && tag == "DT")
    return vowel_next.has_value() && vowel_next.value() ? U"ði" : U"ðə";
  else if (std::regex_match(word, std::regex(R"(vs\.?$)", std::regex_constants::icase)))
    return lookup("versus", {""}, {});
  else if (word == "used" || word == "Used" || word == "USED")
    return dict_.at("word");
  
  // If the word is not a special case, return no phonemes
  return U"";
}

} // namespace phonemis::phonemizer