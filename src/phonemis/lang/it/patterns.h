#pragma once

#include <phonemis/base/processor/layers/num2word/pattern.h>

namespace phonemis::it {

// '80: a decade written as its two digits after an apostrophe (gli anni '80).
class DecadePattern : public processor::num2word::Pattern {
public:
  DecadePattern() : Pattern(processor::num2word::Kind::DECADE) {}

  std::optional<processor::num2word::Match> match(std::u32string_view text, size_t position) const override;
};

} // namespace phonemis::it
