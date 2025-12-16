#include <phonemis/phonemizer/phonemizer.h>

namespace phonemis::phonemizer {

Phonemizer::Phonemizer(Lang language, const std::string& lexicon_filepath) {
  if (!lexicon_filepath.empty())
    lexicon_ = std::make_unique<Lexicon>(language, lexicon_filepath);
}

std::u32string 
Phonemizer::phonemize(const std::string& word,
                      const tagger::Tag& tag,
                      std::optional<float> base_stress,
                      std::optional<bool> vowel_next) const {
  if (word.empty())
    return U"";
  
  std::u32string phonemes = U"";
  
  if (lexicon_ != nullptr)
    phonemes = lexicon_->get(word, tag, base_stress, vowel_next);
  
  if (phonemes.empty())
    phonemes = fallback(word, tag);
  
  return phonemes;
}

std::u32string 
Phonemizer::fallback(const std::string& word,
                     const tagger::Tag& tag) const {
  // TODO: implement
  return U"";
}

} // namespace phonemis::phonemizer