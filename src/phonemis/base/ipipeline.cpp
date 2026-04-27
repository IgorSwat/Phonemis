#include "ipipeline.h"

#include "../utils/conversions.h"

namespace phonemis {

std::u32string IPipeline::operator()(std::string_view text,
                                     bool preprocess_flag,
                                     bool postprocess_flag) {
  return operator()(utils::conversions::utf8_to_u32(text), preprocess_flag, postprocess_flag);
}

std::u32string IPipeline::operator()(std::u32string_view text,
                                     bool preprocess_flag,
                                     bool postprocess_flag) {
  std::u32string result{text};

  if (preprocess_flag) {
    result = preprocess(result);
  }

  result = process(result);

  if (postprocess_flag) {
    result = postprocess(result);
  }

  return result;
}

} // namespace phonemis
