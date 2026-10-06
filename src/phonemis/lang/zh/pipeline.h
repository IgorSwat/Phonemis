#pragma once

#include "lexicon_phonemizer.h"
#include "num2word.h"
#include "punctuation.h"
#include "segmenter.h"
#include "types.h"

#include <phonemis/base/config.h>
#include <phonemis/base/ipipeline.h>

#include <memory>

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
  explicit Pipeline(const Config& config);

  using IPipeline::phonemize_words;

  /**
   * Locates the words of the input in its phonemization. Chinese has no spaces between
   * words, so each phoneme group is traced back to the input characters it came from;
   * groups from the same spelled-out number or date make one word.
   */
  PhonemizedText phonemize_words(std::u32string_view text) override;

  // Performs a preprocessing stage of the pipeline.
  std::u32string preprocess(const std::u32string& input) override;

  // Performs a middle stage of the pipeline - including phonemization.
  std::u32string process(const std::u32string& input) override;

  // Performs a postprocessing stage of the pipeline.
  std::u32string postprocess(const std::u32string& input) override;

private:
  // Phonemizes preprocessed text. With `sources` (one per input character), it also
  // records in `phoneme_sources` where each phoneme came from.
  std::u32string phonemize(std::u32string_view input, const SourceSpans* sources,
                           SourceSpans* phoneme_sources) const;

  // Required submodules
  Num2Word num2word_;
  PunctuationLayer punctuation_;

  // Optional submodules
  std::unique_ptr<zh::LexiconPhonemizer> phonemizer_ = nullptr;
  std::unique_ptr<Segmenter> segmenter_ = nullptr;
};

} // namespace phonemis::zh
