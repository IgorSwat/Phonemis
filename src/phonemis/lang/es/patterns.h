#pragma once

#include <phonemis/base/processor/layers/num2word/pattern.h>

namespace phonemis::es {

// Ley 2/2018: the number of a law with its year. The law word stays, its numbers are the number and the year.
class LawPattern : public processor::num2word::Pattern {
public:
  LawPattern() : Pattern(processor::num2word::Kind::DATE) {}

  std::optional<processor::num2word::Match> match(std::u32string_view text, size_t position) const override;
};

} // namespace phonemis::es
