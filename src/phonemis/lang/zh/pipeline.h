#pragma once

#include "constants.h"
#include "lexicon_phonemizer.h"
#include "num2word.h"
#include "punctuation.h"
#include "segmenter.h"

#include <phonemis/base/config.h>
#include <phonemis/base/ipipeline.h>
#include <phonemis/base/processor/processor.h>

#include <memory>
#include <vector>

namespace phonemis::zh {

/**
 * Mandarin pipeline, matching misaki's G2P for Kokoro v1.0: numbers are spelled out,
 * Han text is segmented into words, and each word becomes one space-separated group of
 * phonemes, with tones marked by arrows. Other text passes through unchanged.
 * The segmenter's data goes in the tagger configuration; without it, every run of Han
 * characters is read as one word. Without a lexicon, Han characters are dropped.
 */
class Pipeline : public IPipeline {
public:
  explicit Pipeline(const Config& config) {
    // 1. Setup Preprocessing layers
    preprocessor_.add_layer(std::make_unique<Num2Word>());
    preprocessor_.add_layer(std::make_unique<PunctuationLayer>());

    // 2. Setup phonemization and word segmentation if specified
    if (config.phonemizer.lexicon_filepath.has_value()) {
      phonemizer_ = std::make_unique<zh::LexiconPhonemizer>(config.phonemizer);
    }
    if (config.tagger.has_value()) {
      segmenter_ = std::make_unique<Segmenter>(*config.tagger);
    }
  }

  // Performs a preprocessing stage of the pipeline.
  std::u32string preprocess(const std::u32string& input) override {
    return preprocessor_.process(input);
  }

  // Performs a middle stage of the pipeline - including phonemization.
  std::u32string process(const std::u32string& input) override {
    using constants::han::is_han;

    std::u32string result;
    for (size_t start = 0; start < input.size();) {
      bool han = is_han(input[start]);
      size_t end = start;
      while (end < input.size() && is_han(input[end]) == han) {
        end++;
      }

      std::u32string_view run = std::u32string_view{input}.substr(start, end - start);
      if (!han) {
        result += run;
      } else if (phonemizer_) {
        auto words = segmenter_ ? segmenter_->segment(run) : std::vector{run};
        bool first = true;
        for (auto word : words) {
          auto phonemes = phonemizer_->phonemize_word(word);
          if (phonemes.empty()) {
            continue;
          }
          if (!first) {
            result += U' ';
          }
          result += phonemes;
          first = false;
        }
      }
      start = end;
    }
    return result;
  }

  // Performs a postprocessing stage of the pipeline.
  std::u32string postprocess(const std::u32string& input) override {
    // No postprocessing
    return input;
  }

private:
  // Required submodules
  processor::Preprocessor preprocessor_;

  // Optional submodules
  std::unique_ptr<zh::LexiconPhonemizer> phonemizer_ = nullptr;
  std::unique_ptr<Segmenter> segmenter_ = nullptr;
};

} // namespace phonemis::zh
