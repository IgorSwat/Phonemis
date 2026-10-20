#include "pattern_set.h"

#include <algorithm>

namespace phonemis::processor::num2word {

void PatternSet::add(std::unique_ptr<Pattern> pattern, Origin origin) {
  auto priority = [](Kind kind, Origin origin) { return std::pair(kind, origin); };
  auto key = priority(pattern->kind(), origin);

  // After every pattern of the same or a higher priority
  auto it = std::ranges::upper_bound(entries_, key, {},
                                     [&](const Entry& entry) { return priority(entry.pattern->kind(), entry.origin); });
  entries_.insert(it, {std::move(pattern), origin});
}

} // namespace phonemis::processor::num2word
