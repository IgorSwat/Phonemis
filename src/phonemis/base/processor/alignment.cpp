#include "alignment.h"

namespace phonemis::processor {

Alignment identity(size_t size) {
  Alignment alignment(size);
  for (size_t i = 0; i < size; ++i) {
    alignment[i] = {i, i + 1};
  }
  return alignment;
}

Span source_span(const Alignment& alignment, size_t begin, size_t end) {
  if (begin < end) {
    return {alignment[begin].begin, alignment[end - 1].end};
  }

  // An empty range - anchor it right before the next character (or after the last one)
  size_t pos = begin < alignment.size() ? alignment[begin].begin
                                        : (alignment.empty() ? 0 : alignment.back().end);
  return {pos, pos};
}

Alignment compose(const Alignment& outer, const Alignment& inner) {
  Alignment result;
  result.reserve(outer.size());
  for (const auto& span : outer) {
    result.push_back(source_span(inner, span.begin, span.end));
  }
  return result;
}

AlignedWriter::AlignedWriter(Alignment* alignment, size_t reserve) : alignment_(alignment) {
  text_.reserve(reserve);
  if (alignment_) {
    alignment_->clear();
    alignment_->reserve(reserve);
  }
}

void AlignedWriter::copy(std::u32string_view input, size_t begin, size_t end) {
  text_.append(input.substr(begin, end - begin));
  if (alignment_) {
    for (size_t i = begin; i < end; ++i) {
      alignment_->push_back({i, i + 1});
    }
  }
}

void AlignedWriter::replace(std::u32string_view text, size_t begin, size_t end) {
  text_.append(text);
  if (alignment_) {
    alignment_->insert(alignment_->end(), text.size(), Span{begin, end});
  }
}

void AlignedWriter::push(char32_t c, size_t begin, size_t end) {
  text_.push_back(c);
  if (alignment_) {
    alignment_->push_back({begin, end});
  }
}

void AlignedWriter::pop() {
  text_.pop_back();
  if (alignment_) {
    alignment_->pop_back();
  }
}

} // namespace phonemis::processor
