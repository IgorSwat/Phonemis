#pragma once

#include <string>
#include <string_view>

namespace phonemis {

// An interface which allows to dynamically resolve pipelines for various languages.
class IPipeline {
public:
  virtual ~IPipeline() = default;

  virtual std::u32string process(std::string_view text) = 0;
};

} // namespace phonemis