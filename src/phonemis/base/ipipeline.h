#pragma once

#include <string>
#include <string_view>

namespace phonemis {

// An interface which allows to dynamically resolve pipelines for various languages.
class IPipeline {
public:
  virtual ~IPipeline() = default;

  virtual std::u32string operator()(std::string_view text, 
                                    bool preprocess = true,
                                    bool postprocess = true);

  virtual std::u32string preprocess(const std::u32string& input) = 0;

  virtual std::u32string process(const std::u32string& input) = 0;

  virtual std::u32string postprocess(const std::u32string& input) = 0;
};

} // namespace phonemis