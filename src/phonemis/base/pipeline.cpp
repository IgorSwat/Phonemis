#include "pipeline.h"
#include <phonemis/lang/en/pipeline.h>

#include <stdexcept>

namespace phonemis {

Pipeline::Pipeline(const Config& config) {
  impl_ = create_pipeline(config);
}

std::unique_ptr<IPipeline> Pipeline::create_pipeline(const Config& config) {
  if (config.lang.empty()) {
    throw std::invalid_argument("Language profile cannot be empty");
  }

  if (config.lang == "en-us" || config.lang == "en-gb") {
    return std::make_unique<en::Pipeline>(config);
  }

  throw std::invalid_argument("Unsupported language profile: " + config.lang);
}

std::u32string Pipeline::process(std::string_view text) {
  return impl_->process(text);
}

} // namespace phonemis