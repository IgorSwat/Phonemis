#include <phonemis/pipeline.h>
#include <phonemis/phonemizer/constants.h>
#include <phonemis/utilities/string_utils.h>

namespace phonemis {

using namespace utilities;
using phonemizer::constants::alphabet::kPunctations;
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
  // We remove numbers by replacing them with their verbalized representations.
  auto verbalized_text = preprocessor::verbalize_numbers(text);

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

    // Phonemize tokens
    // We concatenate phonemized words and add unchanged white spaces
    // and punctation characters.
    std::u32string phonemized_sentence = U"";
    for (const auto& token : tokens) {
      const auto& word = token.text;
      const auto& tag = token.tag.value();

      auto phonemes = phonemizer_->phonemize(word, tag, {}, {});
      phonemized_sentence += phonemes;

      bool is_single_char = word.size() == 1;
      if (phonemes.empty() && is_single_char && kPunctations.contains(word[0]))
        phonemized_sentence += std::u32string(1, word[0]);
      
      if (!token.whitespace.empty())
        phonemized_sentence += string_utils::utf8_to_u32string(token.whitespace);
    }

    // Expand text phonemization with the processed sentence
    phonemized_text += phonemized_sentence;
  }

  return phonemized_text;
}

} // namespace phonemis