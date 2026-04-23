#pragma once

#include "layer.h"

#include <memory>
#include <vector>

namespace phonemis::processor {

// Can be customized in derived classes by adding different preprocessing layers.
class Processor {
public:
  Processor() = default;

  /**
   * Allows to customize Processor's behavior by expanding it with custom layers.
   * @param layer a new transformation layer to be added.
   */
  void add_layer(std::unique_ptr<Layer> layer);

  /**
   * Processes the input by applying all of it's transformation layers.
   * @param input an input to be processed
   * @returns processed input.
   */
  std::u32string process(std::u32string_view input) const;

private:
  // Preprocessing layers - each layer performs some sort of
  // text -> text transformation on its input.
  std::vector<std::unique_ptr<Layer>> layers_;
};

// Naming abstraction
using Preprocessor = Processor;
using Postprocessor = Processor;

} // namespace phonemis::processor