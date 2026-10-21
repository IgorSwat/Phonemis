#include "ipipeline.h"

#include "../utils/conversions.h"
#include "../utils/unicode.h"

namespace phonemis {

namespace {

std::size_t count_words(std::u32string_view text) {
  std::size_t count = 0;
  bool in_word = false;
  for (char32_t c : text) {
    if (!utils::unicode::isspace(c) && !in_word) {
      count++;
    }
    in_word = !utils::unicode::isspace(c);
  }
  return count;
}

} // namespace

std::u32string IPipeline::operator()(std::string_view text,
                                     bool preprocess_flag,
                                     bool postprocess_flag) {
  return operator()(utils::conversions::utf8_to_u32(text), preprocess_flag, postprocess_flag);
}

std::u32string IPipeline::operator()(std::u32string_view text,
                                     bool preprocess_flag,
                                     bool postprocess_flag) {
  std::u32string result{text};

  if (preprocess_flag) {
    result = preprocess(result);
  }

  result = process(result);

  if (postprocess_flag) {
    result = postprocess(result);
  }

  return result;
}

PhonemizedText IPipeline::phonemize_words(std::string_view text) {
  return phonemize_words(utils::conversions::utf8_to_u32(text));
}

PhonemizedText IPipeline::phonemize_words(std::u32string_view text) {
  // Preprocessing can spell a word out as several (numbers, abbreviations,
  // symbols, web addresses), so each word is preprocessed alone to count them.
  // Processing then turns each preprocessed word into one space-separated group
  // of phonemes.
  std::u32string normalized = preprocess(std::u32string{text});
  PhonemizedText result{.phonemes = postprocess(process(normalized))};

  std::vector<std::size_t> group_lengths{0};
  for (char32_t c : result.phonemes) {
    if (c == U' ') {
      group_lengths.push_back(0);
    } else {
      group_lengths.back()++;
    }
  }

  std::size_t group = 0;
  std::size_t phoneme_offset = 0;
  for (std::size_t start = 0; start < text.size();) {
    if (utils::unicode::isspace(text[start])) {
      start++;
      continue;
    }
    std::size_t end = start;
    while (end < text.size() && !utils::unicode::isspace(text[end])) {
      end++;
    }

    auto word = text.substr(start, end - start);
    std::size_t count = count_words(preprocess(std::u32string{word}));

    std::size_t phoneme_length = count > 0 ? count - 1 : 0;  // Separating spaces.
    for (std::size_t i = group; i < group + count && i < group_lengths.size(); i++) {
      phoneme_length += group_lengths[i];
    }

    result.words.push_back({start, end - start, phoneme_offset, phoneme_length});
    if (count > 0) {
      phoneme_offset += phoneme_length + 1;
    }
    group += count;
    start = end;
  }

  // A number spelled out differently on its own than in context would shift
  // every word after it, so a mismatch drops the words rather than misplace them.
  if (group != count_words(normalized) || group != group_lengths.size()) {
    result.words.clear();
  }

  return result;
}

} // namespace phonemis
