#pragma once

#include "processor/alignment.h"
#include "segment.h"

#include <string>
#include <string_view>
#include <vector>

namespace phonemis {

// An interface which allows to dynamically resolve pipelines for various languages.
class IPipeline {
public:
  virtual ~IPipeline() = default;

  virtual std::u32string operator()(std::string_view text,
                                    bool preprocess = true,
                                    bool postprocess = true);

  virtual std::u32string operator()(std::u32string_view text,
                                    bool preprocess = true,
                                    bool postprocess = true);

  // Performs the operator() processing, aligning its result with the input text.
  // Segment positions count characters (code points) of the text.
  virtual std::vector<Segment> phonemize_segments(std::string_view text,
                                                  bool preprocess = true,
                                                  bool postprocess = true);

  virtual std::vector<Segment> phonemize_segments(std::u32string_view text,
                                                  bool preprocess = true,
                                                  bool postprocess = true);

  // A processing parts to be implemented by derived classes.
  virtual std::u32string preprocess(const std::u32string& input,
                                    processor::Alignment* alignment = nullptr) = 0;
  virtual std::vector<Segment> process_segments(const std::u32string& input) = 0;

  // Joins the process_segments() result.
  virtual std::u32string process(const std::u32string& input);

  // No postprocessing by default.
  virtual std::u32string postprocess(const std::u32string& input,
                                     processor::Alignment* alignment = nullptr);

private:
  // Applies the postprocessing stage to already phonemized segments.
  void postprocess_segments(std::vector<Segment>& segments);
};

} // namespace phonemis
