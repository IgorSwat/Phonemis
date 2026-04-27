#pragma once

#include "config.h"
#include "ipipeline.h"

#include <memory>

namespace phonemis {

// A main API class which provides a simple abstraction for the underlying
// language-specific pipeline implementations.
class Pipeline : public IPipeline {
public:
  explicit Pipeline(const Config& config);

  /**
   * Performs a complete G2P (grapheme-to-phoneme) processing.
   * @param text an input text (utf-8) to be processed.
   * @param preprocess decides whether to perform preprocessing stage.
   * @param postprocess decides whether to perform postprocessing stage.
   * @returns phonemization (u32) of given input text.
   */
  std::u32string operator()(std::string_view text, 
                            bool preprocess = true,
                            bool postprocess = true) override;

  /**
   * Performs a complete G2P (grapheme-to-phoneme) processing.
   * @param text an input text (u32string_view) to be processed.
   * @param preprocess decides whether to perform preprocessing stage.
   * @param postprocess decides whether to perform postprocessing stage.
   * @returns phonemization (u32) of given input text.
   */
  std::u32string operator()(std::u32string_view text,
                            bool preprocess = true,
                            bool postprocess = true) override;

  // Performs a preprocessing stage of the pipeline.
  std::u32string preprocess(const std::u32string& input) override;

  // Performs a middle stage of the pipeline - including phonemization.
  std::u32string process(const std::u32string& input) override;

  // Performs a postprocessing stage of the pipeline.
  std::u32string postprocess(const std::u32string& input) override;

private:
  /**
   * Factory method to create a language-specific pipeline.
   */
  static std::unique_ptr<IPipeline> create_pipeline(const Config& config);

  std::unique_ptr<IPipeline> impl_;
};

} // namespace phonemis