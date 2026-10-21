#include "patterns.h"
#include <phonemis/base/processor/layers/num2word/primitives.h>

#include <optional>

namespace phonemis::it {

using namespace processor;
using namespace processor::num2word;

std::optional<Match> DecadePattern::match(std::u32string_view text, size_t position) const {
  if (!is_clear_before(text, position) || text[position] != U'\'') {
    return std::nullopt;
  }

  // Two digits: a ten other than 00
  Number decade = digits_at(text, position + 1);
  if (decade.integer.size() != 2 || decade.integer[0] == U'0' || decade.integer[1] != U'0' ||
      !is_clear_after(text, decade.span.end)) {
    return std::nullopt;
  }
  return Match{.kind = Kind::DECADE, .span = {position, decade.span.end}, .numbers = {decade}};
}

} // namespace phonemis::it
