#pragma once

#include "../../layer.h"

namespace phonemis::processor::trim {

/**
 * Normalizes whitespaces: removes them from both ends of the text and before punctuation (a , -> a,),
 * and collapses every other run into a single "\n" (if it contains a line break) or " ".
 */
class TrimLayer : public Layer {
public:
  TrimLayer() = default;
  ~TrimLayer() override = default;

  void apply(Document& document) const override;
};

} // namespace phonemis::processor::trim
