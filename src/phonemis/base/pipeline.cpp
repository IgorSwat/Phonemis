#include "pipeline.h"
#include <phonemis/lang/en/pipeline.h>

#include <stdexcept>

namespace phonemis {

Pipeline::Pipeline(const Config& config) {
  impl_ = create_pipeline(config);
}

std::unique_ptr<IPipeline> Pipeline::create_pipeline(const Config& config) {
  switch (config.lang) {
  case Lang::EN_US:
  case Lang::EN_GB:
    return std::make_unique<en::Pipeline>(config);
  default:
    throw std::invalid_argument("Unsupported language profile");
  }
}

std::u32string Pipeline::process(std::string_view text) {
  if (!impl_) {
    throw std::runtime_error("Pipeline implementation not initialized.");
  }
  
  return impl_->process(text);
}

} // namespace phonemis