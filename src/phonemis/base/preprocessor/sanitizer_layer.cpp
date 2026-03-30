#include "sanitizer_layer.h"
#include <phonemis/utils/unicode.h>

#include <cctype>

namespace phonemis::preprocessor {

using namespace utils;

std::u32string SanitizerLayer::transform(std::u32string_view input) const {
  std::u32string out;
  out.reserve(input.size());

  for (char32_t c : input) {
    // Filtering
    if (filter_ && mode_ == Mode::KEEP && !filter_->contains(c) ||
        filter_ && mode_ == Mode::KEEP_ALPHABETICAL && unicode::isalpha(c) && !filter_->contains(c) ||
        filter_ && mode_ == Mode::REJECT && filter_->contains(c)) {
      continue;
    }

    // Mapping
    if (mapper_ && mapper_->contains(c)) {
      out.push_back(mapper_->at(c));
    }
    else {
      out.push_back(c);
    }
  }

  return out;
}

void SanitizerLayer::setupFilter(const std::unordered_set<char32_t>* filter, Mode mode) {
  filter_ = filter;
  mode_ = mode;
}

void SanitizerLayer::resetFilter() {
  filter_ = nullptr;
}

void SanitizerLayer::setupMapper(const std::unordered_map<char32_t, char32_t>* mapper) {
  mapper_ = mapper;
}

void SanitizerLayer::resetMapper() {
  mapper_ = nullptr;
}

} // namespace phonemis::preprocessor

