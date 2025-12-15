#include <phonemis/phonemizer/stress.h>
#include <phonemis/utilities/string_utils.h>
#include <vector>

namespace phonemis::phonemizer {

using namespace utilities;
using constants::stress::kVowels;

std::u32string apply_stress(const std::u32string& phonemes, float stress) {
  std::u32string result = phonemes;

  bool has_primary = phonemes.find(constants::stress::kPrimary) != std::u32string::npos;
  bool has_secondary = phonemes.find(constants::stress::kSecondary) != std::u32string::npos;
  bool has_vowel = std::any_of(
    phonemes.begin(), phonemes.end(),                           
    [](auto c) -> bool { return kVowels.find(c) != std::u32string::npos; }
  );

  if (stress < -1.F) {
    string_utils::replace__(result, constants::stress::kPrimary, {});
    string_utils::replace__(result, constants::stress::kSecondary, {});
  }
  else if (stress == -1.F || (stress == 0.F || stress == 0.5F) && has_primary) {
    string_utils::replace__(result, constants::stress::kSecondary, {});
    string_utils::replace__(result, constants::stress::kPrimary,
                                  {constants::stress::kSecondary});    
  }
  else if ((stress == 0.F || stress == 0.5F || stress == 1.F ) &&
           !has_primary && !has_secondary && has_vowel) {
    return restress(std::u32string(1, constants::stress::kSecondary) + result);
  }
  else if (stress >= 1.F && !has_primary && has_secondary) {
    string_utils::replace__(result, constants::stress::kSecondary,
                                  {constants::stress::kPrimary});  
  }
  else if (stress > 1.F && !has_primary && !has_secondary && has_vowel) {
    return restress(std::u32string(1, constants::stress::kPrimary) + result);
  }

  return result;
}

std::u32string restress(const std::u32string& phonemes) {
  // Build the index - value table for sorting
  std::vector<std::pair<float, char32_t>> indexed_positions;
  indexed_positions.reserve(phonemes.size());
  for (size_t i = 0; i < phonemes.size(); i++)
    indexed_positions.emplace_back(static_cast<float>(i), phonemes[i]);
  
  // Move the stress markers according to the next vowel
  for (size_t i = 0; i < indexed_positions.size(); i++) {
    char32_t ch = indexed_positions[i].second;
    if (ch == constants::stress::kPrimary || ch == constants::stress::kSecondary) {
      std::size_t j = i + 1;
      for (; j < indexed_positions.size(); ++j) {
        if (kVowels.find(indexed_positions[j].second) != std::u32string::npos) break;
      }
      if (j < indexed_positions.size()) {
        indexed_positions[i].first = static_cast<float>(j) - 0.5F; // place before vowel
      }
    }
  }

  // Sort by position and rebuild string
  std::sort(indexed_positions.begin(), indexed_positions.end(),
            [](auto const& a, auto const& b) { return a.first < b.first; });

  std::u32string result;
  result.reserve(indexed_positions.size());
  for (auto const& p : indexed_positions) result.push_back(p.second);

  return result;
}

} // namespace phonemis::phonemizer