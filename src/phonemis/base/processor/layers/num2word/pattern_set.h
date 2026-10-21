#pragma once

#include "pattern.h"

#include <memory>
#include <vector>

namespace phonemis::processor::num2word {

// Where a pattern comes from: within a kind, a language's own patterns go before the generic ones.
enum class Origin { LANGUAGE = 0, GENERIC };

// Patterns in priority order: by kind (see kind.h), then by origin, then by addition.
class PatternSet {
public:
  struct Entry {
    std::unique_ptr<Pattern> pattern;
    Origin origin;
  };

  void add(std::unique_ptr<Pattern> pattern, Origin origin);

  // Iteration in priority order
  auto begin() const { return entries_.begin(); }
  auto end() const { return entries_.end(); }

private:
  std::vector<Entry> entries_;
};

} // namespace phonemis::processor::num2word
