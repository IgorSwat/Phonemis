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
   * Phonemizes given text.
   * @param text an input text (utf-8) to be processed.
   * @returns phonemization (u32) of given input text.
   */
  std::u32string process(std::string_view text) override;

private:
  /**
   * Factory method to create a language-specific pipeline.
   */
  static std::unique_ptr<IPipeline> create_pipeline(const Config& config);

  std::unique_ptr<IPipeline> impl_;
};

} // namespace phonemis