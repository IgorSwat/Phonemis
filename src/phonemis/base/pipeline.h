#pragma once

#include "config.h"
#include "ipipeline.h"

#include <memory>

namespace phonemis {

// A main API class which provides a simple abstraction for the underlying
// language-specific pipeline implementations.
class Pipeline : public IPipeline {
public:
  Pipeline(const Config& config);

  /**
   * Phonemizes given text.
   * @param text an input text (utf-8) to be processed.
   * @returns phonemization (u32) of given input text.
   */
  std::u32string process(std::string_view text) override;

private:
  std::unique_ptr<IPipeline> impl_;
};

} // namespace phonemis