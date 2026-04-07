#include "pipeline.h"

#include <stdexcept>

namespace phonemis {

Pipeline::Pipeline(const Config& config) {
  switch (config.lang) {
  case Lang::EN_US:
  case Lang::EN_GB:
    // TODO: Initialize English-specific pipeline implementation
    // impl_ = std::make_unique<EnglishPipeline>(config);
    break;
  default:
    throw std::invalid_argument("Unsupported language profile");
  }
}

std::u32string Pipeline::process(std::string_view text) {
  return impl_->process(text);
}

} // namespace phonemis