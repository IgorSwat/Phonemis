#pragma once

#include "../../common/conditions.h"
#include "../../layer.h"

#include <string>
#include <vector>

namespace phonemis::processor::sym2word {

// Symbols with their readings: & -> and; + -> plus after a name (Google+), más otherwise.
using Symbols = Readings;

/**
 * Reads the symbols left in a text as words, after numbers are read.
 * A symbol whose readings all have unmet conditions stays as written.
 */
class Sym2WordLayer : public Layer {
public:
  explicit Sym2WordLayer(Symbols symbols);

  void apply(Document& document) const override;

private:
  Symbols symbols_;
  Keys keys_;
};

} // namespace phonemis::processor::sym2word
