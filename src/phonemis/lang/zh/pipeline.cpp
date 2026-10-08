#include "pipeline.h"
#include "constants.h"

#include <vector>

namespace phonemis::zh {

Pipeline::Pipeline(const Config& config) {
  if (config.phonemizer.lexicon_filepath.has_value()) {
    phonemizer_ = std::make_unique<zh::LexiconPhonemizer>(config.phonemizer);
  }
  if (config.tagger.has_value()) {
    segmenter_ = std::make_unique<Segmenter>(*config.tagger);
  }
}

std::u32string Pipeline::preprocess(const std::u32string& input) {
  return punctuation_.transform(num2word_.transform(input));
}

std::u32string Pipeline::process(const std::u32string& input) {
  return phonemize(input, nullptr, nullptr);
}

std::u32string Pipeline::postprocess(const std::u32string& input) {
  // No postprocessing
  return input;
}

std::u32string Pipeline::phonemize(std::u32string_view input, const SourceSpans* sources,
                                   SourceSpans* phoneme_sources) const {
  using constants::han::is_han;

  std::u32string result;
  auto append = [&](std::u32string_view text, SourceSpan source) {
    result += text;
    if (phoneme_sources) {
      phoneme_sources->resize(result.size(), source);
    }
  };

  for (size_t start = 0; start < input.size();) {
    bool han = is_han(input[start]);
    size_t end = start;
    while (end < input.size() && is_han(input[end]) == han) {
      end++;
    }

    auto run = input.substr(start, end - start);
    if (!han) {
      for (size_t i = start; i < end; i++) {
        append(input.substr(i, 1), sources ? (*sources)[i] : kNoSource);
      }
    } else if (phonemizer_) {
      auto words = segmenter_ ? segmenter_->segment(run) : std::vector{run};
      bool first = true;
      for (auto word : words) {
        auto phonemes = phonemizer_->phonemize_word(word);
        if (phonemes.empty()) {
          continue;
        }
        if (!first) {
          append(U" ", kNoSource);
        }

        SourceSpan source = kNoSource;
        if (sources) {
          size_t offset = word.data() - input.data();
          for (size_t i = offset; i < offset + word.size(); i++) {
            source = merge(source, (*sources)[i]);
          }
        }
        append(phonemes, source);
        first = false;
      }
    }
    start = end;
  }
  return result;
}

PhonemizedText Pipeline::phonemize_words(std::u32string_view text) {
  SourceSpans sources(text.size());
  for (size_t i = 0; i < text.size(); i++) {
    sources[i] = {i, i + 1};
  }
  std::u32string normalized = num2word_.transform(text, &sources);
  normalized = punctuation_.transform(normalized, &sources);

  SourceSpans phoneme_sources;
  PhonemizedText result{.phonemes = postprocess(phonemize(normalized, &sources, &phoneme_sources))};

  // Each space-separated group of phonemes becomes a word, spanning the input characters
  // its phonemes came from. Groups that share input characters (a number spelled out as
  // several words) make one word.
  SourceSpan word_source = kNoSource;
  const auto& phonemes = result.phonemes;
  for (size_t start = 0; start < phonemes.size();) {
    if (phonemes[start] == U' ') {
      start++;
      continue;
    }
    size_t end = start;
    SourceSpan source = kNoSource;
    while (end < phonemes.size() && phonemes[end] != U' ') {
      source = merge(source, phoneme_sources[end++]);
    }

    if (source != kNoSource) {
      if (!result.words.empty() && source.first < word_source.second) {
        auto& word = result.words.back();
        word_source = merge(word_source, source);
        word.length = word_source.second - word_source.first;
        word.phoneme_length = end - word.phoneme_offset;
      } else {
        word_source = source;
        result.words.push_back({source.first, source.second - source.first, start, end - start});
      }
    }
    start = end;
  }

  return result;
}

} // namespace phonemis::zh
