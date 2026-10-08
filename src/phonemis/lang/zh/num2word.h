#pragma once

#include "types.h"

#include <phonemis/base/processor/layer.h>

#include <optional>
#include <string>
#include <string_view>

namespace phonemis::zh {

/**
 * Spells out Arabic numbers in Chinese, as cn2an.transform(text, "an2cn") does.
 * In order, it reads ranges before a measure word ("3-5个" -> "三到五个"), dates
 * (years digit by digit), fractions, percentages, degrees Celsius and plain numbers.
 * Full-width digits count as digits.
 */
class Num2Word : public processor::Layer {
public:
  std::u32string transform(std::u32string_view input) const override;

  // The same, also tracking where each character of the result came from.
  std::u32string transform(std::u32string_view input, SourceSpans* sources) const;

  /**
   * Reads a number such as "-12.5" ("负十二点五").
   * @returns nullopt if its integer part has more than 16 digits.
   */
  static std::optional<std::u32string> to_cardinal(std::u32string_view number);

  // Reads a number digit by digit, such as a year ("2024" -> "二零二四").
  static std::u32string to_digits(std::u32string_view number);
};

} // namespace phonemis::zh
