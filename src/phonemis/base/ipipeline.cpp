#include "ipipeline.h"

#include "../utils/conversions.h"

#include <algorithm>

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

std::vector<Segment> IPipeline::phonemize_segments(std::string_view text,
                                                   bool preprocess_flag,
                                                   bool postprocess_flag) {
  return phonemize_segments(utils::conversions::utf8_to_u32(text), preprocess_flag, postprocess_flag);
}

std::vector<Segment> IPipeline::phonemize_segments(std::u32string_view text,
                                                   bool preprocess_flag,
                                                   bool postprocess_flag) {
  std::u32string input{text};
  processor::Alignment alignment;

  if (preprocess_flag) {
    input = preprocess(input, &alignment);
  } else {
    alignment = processor::identity(input.size());
  }

  // Move the segments back onto the text. Segments coming from the same part of the
  // text (e.g. "twenty" and "five" from "25") are merged into one.
  std::vector<Segment> segments;
  for (auto& segment : process_segments(input)) {
    auto source = processor::source_span(alignment, segment.begin, segment.end);

    if (!segments.empty() && source.begin < segments.back().end) {
      auto& last = segments.back();
      if (last.whitespace) {
        last.phonemes.push_back(U' ');
      }
      last.phonemes.append(segment.phonemes);
      last.whitespace = segment.whitespace;
      last.end = std::max(last.end, source.end);
    } else {
      segment.begin = source.begin;
      segment.end = source.end;
      segments.push_back(std::move(segment));
    }
  }

  if (postprocess_flag) {
    postprocess_segments(segments);
  }

  return segments;
}

std::u32string IPipeline::process(const std::u32string& input) {
  return join(process_segments(input));
}

std::u32string IPipeline::postprocess(const std::u32string& input, processor::Alignment* alignment) {
  if (alignment) {
    *alignment = processor::identity(input.size());
  }
  return input;
}

void IPipeline::postprocess_segments(std::vector<Segment>& segments) {
  // Postprocess the joined phonemes, then give each resulting character back
  // to the segment it came from.
  std::u32string joined;
  std::vector<size_t> starts, spaces;
  for (const auto& segment : segments) {
    starts.push_back(joined.size());
    joined.append(segment.phonemes);
    spaces.push_back(segment.whitespace ? joined.size() : std::u32string::npos);
    if (segment.whitespace) {
      joined.push_back(U' ');
    }
  }

  processor::Alignment alignment;
  auto result = postprocess(joined, &alignment);

  for (auto& segment : segments) {
    segment.phonemes.clear();
    segment.whitespace = false;
  }

  for (size_t i = 0; i < result.size(); ++i) {
    size_t source = alignment[i].begin;
    size_t owner = std::upper_bound(starts.begin(), starts.end(), source) - starts.begin() - 1;

    if (source == spaces[owner] && result[i] == U' ') {
      segments[owner].whitespace = true;
    } else {
      segments[owner].phonemes.push_back(result[i]);
    }
  }
}

} // namespace phonemis
