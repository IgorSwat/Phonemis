#include "constants.h"
#include "lexicon_phonemizer.h"
#include "stress.h"
#include <phonemis/base/phonemizer/constants.h>
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/strings.h>

#include <algorithm>
#include <numeric>
#include <string>
#include <string_view>

// WARNING: This file is called 'big szpont'
//          If you are allergic to a lot of if statements, then close it as soon as possible!

namespace phonemis::en {

using namespace utils;

// Helper functions
namespace {
std::string get_parent_tag(const std::string& tag) {
  if (tag.size() >= 2 && tag.substr(0, 2) == "VB") return "VERB";
  if (tag.size() >= 2 && tag.substr(0, 2) == "NN") return "NOUN";
  if (tag.size() >= 3 && tag.substr(0, 3) == "ADV") return "ADV";
  if (tag.size() >= 2 && tag.substr(0, 2) == "RB") return "ADV";
  if (tag.size() >= 3 && tag.substr(0, 3) == "ADJ") return "ADJ";
  if (tag.size() >= 2 && tag.substr(0, 2) == "JJ") return "ADJ";
  return tag;
}
} // namespace

LexiconPhonemizer::LexiconPhonemizer(const phonemizer::Config& config)
    : phonemizer::LexiconPhonemizer(config), british_(config.lang == Lang::EN_GB) {}

std::optional<std::u32string> LexiconPhonemizer::phonemize(const Token& token) const {
  // Set stress level.
  // Words with upper case characters are stressed more.
  std::optional<float> stress = token.text == strings::to_lower(token.text) ?
                                std::nullopt : std::make_optional(0.5F);

  // Start with lookup_special(), as it is the most concrete and some of the
  // special words might require different phonemizations than present in the lexicon.
  std::u32string phonemes = lookup_special(
    token.text, token.tag.value_or(""), stress,
    global_context_.vowel_next, global_context_.future_to
  );
  if (!phonemes.empty()) {
    return std::make_optional(phonemes);
  }

  // NNP words (acronyms, initialysms) are sort of out of the main pipeline, so they go next.
  std::u32string text_uppercase = strings::to_upper(token.text);
  if (text_uppercase == token.text) {
    phonemes = lookup_nnp(token.text);
    return !phonemes.empty() ? std::make_optional(phonemes) : std::nullopt;
  }

  // Standard (generic) lookup
  if (is_known(token.text))
    return lookup_and_restress(token.text, token.tag.value_or(""), stress);
  if (strings::ends_with(token.text, U"s'") && is_known(token.text.substr(0, token.text.size() - 2) + U"'s"))
    return lookup_and_restress(token.text.substr(0, token.text.size() - 2) + U"'s", token.tag.value_or(""), stress);
  if (strings::ends_with(token.text, U"'") && is_known(token.text.substr(0, token.text.size() - 1)))
    return lookup_and_restress(token.text.substr(0, token.text.size() - 1), token.tag.value_or(""), stress);

  // If the generic lookup failed, try to divide the word for a well known stem and suffix.
  phonemes = lookup_stem_s(token.text, token.tag.value_or(""), stress);
  if (!phonemes.empty()) {
    return std::make_optional(phonemes);
  }

  phonemes = lookup_stem_ed(token.text, token.tag.value_or(""), stress);
  if (!phonemes.empty()) {
    return std::make_optional(phonemes);
  }

  phonemes = lookup_stem_ing(token.text, token.tag.value_or(""), 0.5F);
  if (!phonemes.empty()) {
    return std::make_optional(phonemes);
  }

  return std::nullopt;
}

void LexiconPhonemizer::update_context(size_t curr_token_id, std::span<const Token> tokens) {
  using namespace phonemizer::constants;

  // Update vowel_next
  global_context_.vowel_next = std::nullopt;
  if (curr_token_id + 1 < tokens.size()) {
    const auto& next_token = tokens[curr_token_id + 1];
    if (!next_token.text.empty()) {
      char32_t first_char = strings::to_lower(next_token.text)[0];
      if (constants::alphabet::kVowels.find(first_char) != std::u32string::npos) {
        global_context_.vowel_next = {true};
      } else if (constants::alphabet::kConsosants.find(first_char) != std::u32string::npos) {
        global_context_.vowel_next = {false};
      } else {
        global_context_.vowel_next = std::nullopt;
      }
    }
  }

  // Update future_to
  // Start from the next token and check the 2 nearest, non-punctation words.
  global_context_.future_to = false;
  for (size_t j = curr_token_id + 1, seen = 0; j < tokens.size() && seen < 2; j++) {
    if (tokens[j].text.size() == 1 && puncts::kPunctations.contains(static_cast<char>(tokens[j].text[0])))
      continue;
    seen++;
    if (tokens[j].text == U"to") {
      global_context_.future_to = true;
      break;
    }
  }
}

std::u32string LexiconPhonemizer::lookup(const std::u32string& word, const Tag& tag) const {
  std::u32string_view used_word = word;
  std::u32string lower_word;

  if (!Base::is_known(word, false)) {
    lower_word = strings::to_lower(word);
    used_word = lower_word;
  }

  std::string utf8_word = conversions::u32_to_utf8(used_word);

  if (!tag.empty()) {
    // Try original tag
    auto result = Base::lookup(utf8_word, tag);
    if (!result.empty()) {
      return result;
    }

    // Try parent tag
    std::string parent_tag = get_parent_tag(tag);
    result = Base::lookup(utf8_word, parent_tag);
    if (!result.empty()) {
      return result;
    }
  }

  // Fallback to no context
  return Base::lookup(utf8_word, Tag(""));
}

std::u32string LexiconPhonemizer::lookup_and_restress(const std::u32string& word, const Tag& tag,
                                                      std::optional<float> stress) const {
  std::u32string phonemes = lookup(word, tag);

  if (!phonemes.empty() && stress.has_value()) {
    stress::apply(phonemes, *stress);
  }

  return phonemes;
}

std::u32string LexiconPhonemizer::lookup_stem_ed(const std::u32string& word, 
                                                 const Tag& tag,
                                                 std::optional<float> stress) const {
  std::u32string stem;

  // Find the stem - a main part of the word without the "d" / "ed" suffix
  if (word.size() < 4 || word.back() != U'd') {
    return U"";
  }
  else if (!strings::ends_with(word, U"dd") && is_known(word.substr(0, word.size() - 1))) {
    stem =  word.substr(0, word.size() - 1);
  }
  else if (strings::ends_with(word, U"ed") && !strings::ends_with(word, U"eed") && 
           is_known(word.substr(0, word.size() - 2))) {
    stem = word.substr(0, word.size() - 2);
  }
  else {
    return U"";
  }

  // Phonemize the stem
  std::u32string phonemes = lookup_and_restress(stem, tag, stress);
  if (phonemes.empty()) {
    return U"";
  }
  
  // Adjust phonemization according to the selected language rules.
  // https://en.wiktionary.org/wiki/-ed
  static const std::u32string soft_d_suffixes = U"pkfθʃsʧ";
  if (soft_d_suffixes.find(phonemes.back()) != std::u32string::npos) {
    return phonemes + U"t";
  }
  if (phonemes.back() == U'd') {
    return phonemes + (british_ ? U"ɪ" : U"ᵻ") + U"d";
  }
  if (phonemes.back() != U't') {
    return phonemes + U"d";
  }
  if (british_ || phonemes.size() < 2) {
    return phonemes + U"ɪd";
  }
  if (constants::kUSTaus.find(phonemes[phonemes.size() - 2]) != std::u32string::npos) {
    return phonemes.substr(0, phonemes.size() - 1) + U"ɾᵻd";
  }
  
  return phonemes + U"ᵻd";
}

std::u32string LexiconPhonemizer::lookup_stem_s(const std::u32string& word, 
                                                const Tag& tag,
                                                std::optional<float> stress) const {
  std::u32string stem;

  if (word.size() < 3 || word.back() != U's') {
    return U"";
  } else if (!strings::ends_with(word, U"ss") && is_known(word.substr(0, word.size() - 1))) {
    stem = word.substr(0, word.size() - 1);
  } else if ((strings::ends_with(word, U"'s") || 
            (word.size() > 4 && strings::ends_with(word, U"es") && !strings::ends_with(word, U"ies"))) &&
            is_known(word.substr(0, word.size() - 2))) {
    stem = word.substr(0, word.size() - 2);
  } else if (word.size() > 4 && strings::ends_with(word, U"ies") &&
           is_known(word.substr(0, word.size() - 3) + U"y")) {
    stem = word.substr(0, word.size() - 3) + U"y";
  } else {
    return U"";
  }

  // Phonemize the stem
  std::u32string phonemes = lookup_and_restress(stem, tag, stress);
  if (phonemes.empty()) {
    return U"";
  }

  // Adjust phonemization according to the selected language rules.
  // https://en.wiktionary.org/wiki/-s
  static const std::u32string hard_s_suffixes = U"ptkfθ";
  static const std::u32string soft_s_suffixes = U"szʃʒʧʤ";
  if (hard_s_suffixes.find(phonemes.back()) != std::u32string::npos) {
    return phonemes + U"s";
  }
  if (soft_s_suffixes.find(phonemes.back()) != std::u32string::npos) {
    return phonemes + (british_ ? U"ɪ" : U"ᵻ") + U"z";
  }
  
  return phonemes + U"z";
}

std::u32string LexiconPhonemizer::lookup_stem_ing(const std::u32string& word, 
                                                  const Tag& tag,
                                                  std::optional<float> stress) const {
  std::u32string stem;

  static const std::u32string double_consonants = U"bcdgklmnprstvxz";

  if (word.size() < 5 || !strings::ends_with(word, U"ing")) {
    return U"";
  } else if (is_known(word.substr(0, word.size() - 3))) {
    stem = word.substr(0, word.size() - 3);
  } else if (is_known(word.substr(0, word.size() - 3) + U"e")) {
    stem = word.substr(0, word.size() - 3) + U"e";
  } else if ((word.size() >= 7 && strings::ends_with(word, U"cking") || 
              (word[word.size() - 4] == word[word.size() - 5] && 
               double_consonants.find(word[word.size() - 4]) != std::u32string::npos)) &&
             is_known(word.substr(0, word.size() - 4))) {
    stem = word.substr(0, word.size() - 4);
  } else {
    return U"";
  }
  
  // Phonemize the stem
  std::u32string phonemes = lookup_and_restress(stem, tag, stress);
  if (phonemes.empty()) {
    return U"";
  }

  // Adjust phonemization according to selected language rules.
  // https://en.wiktionary.org/wiki/-ing
  // In GB English, stems ending in schwa get linking-r before -ɪŋ
  // (e.g. "enter" /ˈɛntə/ → "entering" /ˈɛntəɹɪŋ/).
  if (british_ && phonemes.back() == U'ə') {
    return phonemes + U"ɹɪŋ";
  }
  // For stems ending in ː, linking-r only applies after historically rhotic
  // vowels (ɔː, ɑː, ɜː). Not after iː or uː (e.g. "seeing" = /siːɪŋ/).
  if (british_ && phonemes.back() == U'ː' && phonemes.size() >= 2) {
    char32_t vowel = phonemes[phonemes.size() - 2];
    if (vowel == U'ɔ' || vowel == U'ɑ' || vowel == U'ɜ') {
      return phonemes + U"ɹɪŋ";
    }
    return phonemes + U"ɪŋ";
  }
  if (phonemes.size() > 1 && phonemes.back() == U't' &&
      constants::kUSTaus.find(phonemes[phonemes.size() - 2]) != std::u32string::npos) {
    return phonemes.substr(0, phonemes.size() - 1) + U"ɾɪŋ";
  }
  
  return phonemes + U"ɪŋ";
}

std::u32string LexiconPhonemizer::lookup_nnp(const std::u32string& word) const {
  using phonemizer::constants::ipa::stress::kPrimary;
  using phonemizer::constants::ipa::stress::kSecondary;

  std::u32string word_alpha = strings::filter(word, [](char32_t c) { return !unicode::isalpha(c); });

  std::u32string phonemes;
  for (char32_t c : word_alpha) {
    std::u32string char_phonemes = lookup(std::u32string(1, c), Tag(""));
    if (char_phonemes.empty()) {
      return U"";
    }
    phonemes += char_phonemes;
  }

  if (phonemes.empty()) {
    return U"";
  }

  stress::apply(phonemes, 0.F);

  size_t last_secondary = phonemes.find_last_of(kSecondary);
  if (last_secondary != std::u32string::npos) {
    phonemes[last_secondary] = kPrimary;
  }

  return phonemes;
}

std::u32string LexiconPhonemizer::lookup_special(const std::u32string& word, const Tag& tag,
                                                 std::optional<float> stress,
                                                 std::optional<bool> vowel_next, bool future_to) const {
  bool is_single_char = word.size() == 1;
  bool is_add_symbol = is_single_char && constants::kAddSymbols.contains(word[0]);
  bool is_other_symbol = is_single_char && constants::kSymbols.contains(word[0]);

  std::u32string word_stripped = strings::strip(word, std::make_optional(U'.'));
  std::u32string word_without_dots = strings::filter(word, [](char32_t c) -> bool { return c != U'.'; });
  std::vector<std::u32string> word_splitted = strings::split(word_stripped, U'.');
  
  size_t max_subword_size = std::accumulate(
    word_splitted.begin(), word_splitted.end(), 0LL,
    [](size_t m, const auto& str) { return std::max(m, str.size()); }
  );
  
  std::u32string lower_word = strings::to_lower(word);
  
  if (tag == "ADD" && is_add_symbol) {
    return lookup_and_restress(constants::kAddSymbols.at(word[0]), Tag(""), -0.5F);
  } else if (is_other_symbol) {
    return lookup_and_restress(constants::kSymbols.at(word[0]), Tag(""), stress);
  } else if (word_stripped.find(U'.') != std::u32string::npos &&
             strings::is_alpha(word_without_dots) && 
             max_subword_size < 3) {
    return lookup_nnp(word);
  } else if (is_single_char && (word[0] == U'a' || word[0] == U'A')) {
    return tag == "DT" ? U"ɐ" : U"ˈA";
  } else if (lower_word == U"am") {
    if (strings::starts_with(tag, "NN")) {
      return lookup_nnp(word);
    }
    if (!vowel_next.has_value() || word != U"am" || (stress.has_value() && stress.value() > 0)) {
      return lookup_and_restress(U"am", Tag(""), stress);
    } else {
      return U"ɐm";
    }
  } else if (lower_word == U"an") {
    if (word == U"AN" && strings::starts_with(tag, "NN")) {
      return lookup_nnp(word);
    }
    return U"ɐn";
  } else if (is_single_char && word[0] == U'I' && tag == "PRP") {
    return U"ˌI";
  } else if (lower_word == U"by" && get_parent_tag(tag) == "ADV") {
    return U"bˈI";
  } else if (lower_word == U"to" && (tag == "TO" || tag == "IN")) {
    if (!vowel_next.has_value()) {
      return lookup(U"to", Tag(""));
    }
    return vowel_next.value() ? U"tʊ" : U"tə";
  } else if (lower_word == U"in" && tag != "NNP") {
    std::u32string prefix = (!vowel_next.has_value() || tag != "IN") ? U"ˈ" : U"";
    return prefix + U"ɪn";
  } else if (lower_word == U"the" && tag == "DT") {
    return (vowel_next.has_value() && vowel_next.value()) ? U"ði" : U"ðə";
  } else if (lower_word == U"vs" || lower_word == U"vs.") {
    return lookup_and_restress(U"versus", Tag(""), stress);
  } else if (lower_word == U"used") {
    if ((tag == "VBD" || tag == "JJ") && future_to) {
      return lookup_and_restress(U"used", Tag("VBD"), stress);
    }
    return lookup_and_restress(U"used", Tag(""), stress);
  } else if (lower_word == U"supposed") {
    if ((tag == "VBD" || tag == "JJ") && future_to) {
      return lookup_and_restress(U"supposed", Tag("VBD"), stress);
    }
    return lookup_and_restress(U"supposed", Tag(""), stress);
  } else if (lower_word == U"src") {
    return lookup(U"source", Tag(""));
  }

  return U"";
}

} // namespace phonemis::en