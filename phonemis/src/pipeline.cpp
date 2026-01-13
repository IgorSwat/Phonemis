#include <phonemis/pipeline.h>
#include <phonemis/phonemizer/constants.h>
#include <phonemis/utilities/string_utils.h>

namespace phonemis {

using namespace utilities;
using phonemizer::constants::alphabet::kPunctations;
using phonemizer::constants::alphabet::kNonQuotePunctations;
using phonemizer::constants::language::kVowels;
using phonemizer::constants::language::kConsonants;
using tagger::Tag;

Pipeline::Pipeline(Lang language,
                   const std::string& tagger_data_filepath,
                   const std::string& lexicon_data_filepath)
  : language_(language) {
  if (!tagger_data_filepath.empty())
    tagger_ = std::make_unique<Tagger>(tagger_data_filepath);
  
  phonemizer_ = std::make_unique<Phonemizer>(language, lexicon_data_filepath);
}

// TODO: It works fine, but there are still some missing parts
// of the solution
std::u32string Pipeline::process(const std::string& text) {
  // Start by preprocessing the text
  // Normalize the text to replace any foreign characters.
  auto normalized_text = preprocessor::normalize_unicode(text);

  // We remove numbers by replacing them with their verbalized representations.
  auto verbalized_text = preprocessor::verbalize_numbers(normalized_text);

  // Since HMM tagger processes entire sentences, we divide
  // the text for separate sentences to be processed.
  auto sentences = preprocessor::split_sentences(verbalized_text);

  // Each sentence is processed in similar way, and the results
  // are concatenated at the end.
  std::u32string phonemized_text = U"";
  for (const auto& sentence : sentences) {
    // Tokenize the sentence with tokenizer
    auto tokens = tokenizer::tokenize(sentence);

    // Apply tagging
    // If tagger is not defined (that is, if user has not passed the tagger data file)
    // we simply mark tokens with 'unknown' tag.
    if (tagger_)
      tagger_->tag(tokens);
    else {
      for (auto& token : tokens)
        token.tag = std::make_optional(Tag("XX"));
    }

    // TODO: intermediate part of preprocessing
    std::optional<bool> vowel_next = {};

    // Phonemize tokens
    // We concatenate phonemized words and add unchanged white spaces
    // and punctation characters.
    std::u32string phonemized_sentence = U"";
    for (size_t i = 0; i < tokens.size(); i++) {
      const auto& token = tokens[i];
      bool is_last_token = i == tokens.size() - 1;

      const auto& word = token.text;
      const auto& tag = token.tag.value();

      auto phonemes = phonemizer_->phonemize(word, tag, {}, vowel_next);
      phonemized_sentence += phonemes;

      // Handle reimaining punctation characters
      // (which tend to not have it's own phonemization, but affect the local phonemes
      //  from surrounding characters).
      bool is_single_char = word.size() == 1;
      bool is_punctation = is_single_char && kPunctations.contains(word[0]);
      bool is_dot_or_hyphen = is_punctation && (word[0] == '.' || word[0] == '-');
      if (phonemes.empty() && (!is_dot_or_hyphen || !token.whitespace.empty() || is_last_token))
        phonemized_sentence += std::u32string(1, word[0]);
      
      if (!token.whitespace.empty())
        phonemized_sentence += string_utils::utf8_to_u32string(token.whitespace);

      // Check if the latest phonemization contains any vowels
      // This will affect the following phonemization (of the next token).
      // TODO: should be moved into Phonemizer's territory
      for (char32_t p : phonemes) {
        if (p <= 127 && kNonQuotePunctations.contains(static_cast<char>(p))) {
          vowel_next = {};
          break;
        }
        else if (kVowels.find(p) != std::u32string::npos) {
          vowel_next = {true};
          break;
        }
        else if (kConsonants.find(p) != std::u32string::npos) {
          vowel_next = {false};
          break;
        }
      }
    }

    // Expand text phonemization with the processed sentence
    phonemized_text += phonemized_sentence;
  }

  return phonemized_text;
}

} // namespace phonemis