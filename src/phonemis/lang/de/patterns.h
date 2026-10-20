#pragma once

#include <phonemis/base/processor/layers/num2word/pattern.h>

namespace phonemis::de {

// 030 1234567, 0800-123 45 67: an area code starting with 0, then the number, optionally in groups.
class PhonePattern : public processor::num2word::Pattern {
public:
  PhonePattern() : Pattern(processor::num2word::Kind::PHONE) {}

  std::optional<processor::num2word::Match> match(std::u32string_view text, size_t position) const override;
};

} // namespace phonemis::de
