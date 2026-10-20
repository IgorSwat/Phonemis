#pragma once

#include "common/document.h"

namespace phonemis::processor {

/**
 * An interface (contract) for all preprocessing steps.
 */
class Layer {
public:
  virtual ~Layer() = default;

  // A single preprocessing step: reads the document and queues its edits.
  virtual void apply(Document& document) const = 0;
};

} // namespace phonemis::processor
