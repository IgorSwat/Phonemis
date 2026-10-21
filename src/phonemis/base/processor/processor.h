#pragma once

#include "layer.h"

#include <memory>
#include <vector>

namespace phonemis::processor {

/**
 * Main preprocessing component.
 * 
 * A composition of elementary steps - can be customized by specyfing different layers (steps).
 */
class Processor {
public:
  Processor() = default;

  /**
   * Processes the input by applying all of it's transformation layers, one after another.
   * @details Everything the layers learn about the text lives in a Document, which is dropped on return.
   * @param input an input to be processed
   * @returns processed input.
   */
  std::u32string process(std::u32string_view input) const;

  /**
   * Allows to customize Processor's behavior by expanding it with custom layers.
   * @param layer a new transformation layer to be added.
   */
  void add_layer(std::unique_ptr<Layer> layer);

private:
  // Preprocessing layers - each layer queues edits of a shared document.
  std::vector<std::unique_ptr<Layer>> layers_;
};

// Naming abstraction
using Preprocessor = Processor;
using Postprocessor = Processor;

} // namespace phonemis::processor