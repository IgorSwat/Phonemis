#include <phonemis/phonemizer/phonemizer.h>
#include <phonemis/phonemizer/constants.h>
#include <phonemis/utilities/string_utils.h>
#include <vector>
#include <iostream>

namespace phonemis::phonemizer {

using namespace utilities;

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
  
  if (phonemes.empty() && string_utils::is_alpha(word))
    phonemes = fallback(word, tag);
  
  return phonemes;
}

std::u32string 
Phonemizer::fallback(const std::string& word,
                     const tagger::Tag& tag) const {
  // The main idea behind the fallback algorithm is to syllabify the words,
  // and then perform lookup phonemization for well known, short syllabes.
  // We make an assumption, that the best phonemization is the shortest one 
  // (in terms of number of output phonemes).
  // This is a heuristic, but quite a good one, which performs well on typical
  // english words, as well as some standard foreign words.
  // To quickly calculate the shortest phonemization possible, we utilize 
  // dynamic programming methods.

  // TODO: some preprocessing, like eliminating special characters?
  // ...
  auto lword = string_utils::to_lower(word);
  int32_t length = word.size();

  if (lword.empty())
    return U"";

  // Define DP table with both DP function values & corresponding phonemizations.
  // TODO: can be done in leaner time, by storing indices instead of phonemes
  constexpr int32_t INF = 1e5;
  std::vector<std::pair<int32_t, std::u32string>> dpTable(length, {INF, U""});

  // Iterate through each character of the input word
  for (int32_t i = 0; i < length; i++) {
    // Now check all possible syllabes ending with the word[i]
    // By starting with the longest syllable, we ensure that 
    // the solution with longer syllables will be preferred if there is a tie in phonemization length.
    // `d` stands for number of characters in syllabe other than word[i].
    for (int32_t d = std::min(i, constants::kMaxSyllabeLength - 1); d >= 0; d--) {
      auto syllabe = lword.substr(i - d, d + 1);
      
      // A syllabe must contain at least one vowel (or be degenerated to a single consonant)
      auto hasVowel = !string_utils::filter(lword, [](char c) -> bool {
        return constants::alphabet::kVowels.find(c) != std::string::npos;
      }).empty();
      if (syllabe.size() > 1 && !hasVowel)
        continue;
      
      if (lexicon_->is_known(syllabe)) {
        // Simple lookup
        auto phonemes = lexicon_->get(syllabe);
        int32_t plength = phonemes.size();

        if (phonemes.empty())
          continue;

        // We do in fact apply some very minimalistic postprocessing
        // For example, handle special cases of syllabes with 'e' at the end.
        if (i < length - 1 &&
            syllabe.back() == 'e' && 
            constants::language::kConsonants.find(phonemes.back()) != std::u32string::npos)
          phonemes += U"ɜ";

        // Or replace the primary stress with the secondary stress in case of
        // trailing syllabes.
        auto primary_stress_pos = phonemes.find(constants::stress::kPrimary);
        if (i > d && primary_stress_pos != std::u32string::npos)
          phonemes[primary_stress_pos] = constants::stress::kSecondary;
        
        // Apply penalty for using syllabes starting with vowels
        if (i > d && 
            constants::alphabet::kVowels.find(syllabe.front()) != std::string::npos)
          plength += constants::kVowelSyllabePenalty;
        
        int32_t totalPLength = plength;
        auto totalPhonemes = phonemes;
        if (i > d) {
          totalPLength += dpTable[i - d - 1].first;
          totalPhonemes = dpTable[i - d - 1].second + phonemes;
        }
        
        // Update the DP table
        if (totalPLength < dpTable[i].first) {
          dpTable[i].first = totalPLength;
          dpTable[i].second = totalPhonemes;
        }
      }
    }
  }

  // If the resulting length is not infinite (not equal to INF constant),
  // then we were able to phonemize the word.
  auto [bestLength, bestPhonemization] = dpTable[length - 1];
  
  return bestLength != INF ? bestPhonemization : U"";
}

} // namespace phonemis::phonemizer