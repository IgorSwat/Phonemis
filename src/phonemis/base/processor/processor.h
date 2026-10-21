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
   * Processes a text by applying all of its layers, one after another.
   * @details A shortcut for when only the result matters; see the overload above to keep its origins.
   * @param input an input to be processed
   * @returns processed input.
   */
  std::u32string process(std::u32string_view input) const;

  /**
   * Processes a document by applying all of its layers, one after another.
   * @details This overload exists for callers which need to know where the result came from: the document keeps
   * the origin of every change (see Document::changes()), which the text alone loses.
   * @param document a document to be processed in place.
   */
  void process(Document& document) const;

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