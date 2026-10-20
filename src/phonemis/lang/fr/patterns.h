#pragma once

#include <phonemis/base/processor/layers/num2word/pattern.h>

namespace phonemis::fr {

// XXe, XIXème, Ier: an ordinal written in Roman numerals. Its number is the value of the numeral.
class RomanOrdinalPattern : public processor::num2word::Pattern {
public:
  RomanOrdinalPattern() : Pattern(processor::num2word::Kind::ORDINAL) {}

  std::optional<processor::num2word::Match> match(std::u32string_view text, size_t position) const override;
};

} // namespace phonemis::fr
