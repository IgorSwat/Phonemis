#pragma once

#include "kind.h"
#include "types.h"

#include <optional>
#include <string_view>

namespace phonemis::processor::num2word {

// Recognizes one kind of numeric expression at a given position of a text.
class Pattern {
public:
  explicit Pattern(Kind kind) : kind_(kind) {}
  virtual ~Pattern() = default;

  Kind kind() const { return kind_; }

  /**
   * Matches the expression starting at given position, if there is one.
   * @param text the text, as rewritten by the patterns before.
   * @param position where the expression has to start.
   */
  virtual std::optional<Match> match(std::u32string_view text, size_t position) const = 0;

private:
  Kind kind_;
};

} // namespace phonemis::processor::num2word
