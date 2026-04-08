#include "lexicon_phonemizer.h"

namespace phonemis::en {

LexiconPhonemizer::LexiconPhonemizer(Lang lang, const std::string& lexicon_filepath)
  : phonemizer::LexiconPhonemizer(lexicon_filepath), british_(lang == Lang::EN_GB) {}

} // namespace phonemis::en