#include "document.h"

#include <algorithm>
#include <stdexcept>

namespace phonemis::processor {

Document::Document(std::u32string_view text) : text_(text), original_size_(text.size()) {
  // At first, the whole text is a single copied piece
  if (!text.empty()) {
    pieces_.push_back({.original = {0, text.size()}, .size = text.size()});
  }
}

void Document::replace(Span span, std::u32string replacement) {
  if (span.begin > span.end || span.end > text_.size()) {
    throw std::invalid_argument("Document: edit span out of range");
  }

  // Keep the edits sorted, so that commit() applies them in a single pass
  auto it = std::ranges::upper_bound(edits_, span.begin, {}, [](const Edit& edit) { return edit.span.begin; });
  bool overlaps_previous = it != edits_.begin() && std::prev(it)->span.end > span.begin;
  bool overlaps_next = it != edits_.end() && it->span.begin < span.end;
  if (overlaps_previous || overlaps_next) {
    throw std::invalid_argument("Document: overlapping edits");
  }

  edits_.insert(it, {span, std::move(replacement)});
}

void Document::commit() {
  if (edits_.empty()) {
    return;
  }

  // Where each piece starts in the current text
  std::vector<size_t> starts;
  for (size_t start = 0; const auto& piece : pieces_) {
    starts.push_back(start);
    start += piece.size;
  }

  // 1. Group the edits into the spans they really change.
  // An edit inside an earlier replacement replaces all of it ("ten" -> "Ten" changes the whole "ten" of "10"),
  // and edits whose widened spans meet are merged: each group (an edit itself) becomes one replaced piece.
  std::vector<Edit> groups;
  for (size_t i = 0; i < edits_.size();) {
    // Extend the group while the next widened edit starts inside it
    Edit group{.span = widen(edits_[i].span, starts)};
    size_t last = i;
    while (last + 1 < edits_.size() && widen(edits_[last + 1].span, starts).begin < group.span.end) {
      group.span.end = std::max(group.span.end, widen(edits_[++last].span, starts).end);
    }

    // Its text: the current text of the span, with each edit's text in place of its range
    size_t position = group.span.begin;
    for (; i <= last; ++i) {
      group.text.append(text_, position, edits_[i].span.begin - position);
      group.text.append(edits_[i].text);
      position = edits_[i].span.end;
    }
    group.text.append(text_, position, group.span.end - position);
    groups.push_back(std::move(group));
  }

  // 2. Rebuild the text and its pieces, walking the groups and the old pieces side by side.
  // Pieces before a group are kept, pieces it touches are merged into one replaced piece.
  std::u32string text;
  std::vector<Piece> pieces;
  std::vector<Piece> old = pieces_;
  size_t next = 0;     // The first old piece not handled yet
  size_t position = 0; // The end of the current text copied so far

  for (const auto& [span, replacement] : groups) {
    // An empty piece (a deletion) counts as touched when it lies at the group's edge, so it merges into the group
    auto end_of = [&](size_t i) { return starts[i] + old[i].size; };
    auto before = [&](size_t i) { return old[i].size > 0 ? end_of(i) <= span.begin : starts[i] < span.begin; };
    auto touched = [&](size_t i) {
      return old[i].size > 0 ? starts[i] < span.end && span.begin < end_of(i) :
                               span.begin <= starts[i] && starts[i] <= span.end;
    };

    // 2.1. The pieces before the group stay as they are
    while (next < old.size() && before(next)) {
      pieces.push_back(old[next++]);
    }

    // 2.2. The original span behind the group: from its first touched piece to its last one.
    // An insertion between pieces touches none, and has an empty original span.
    size_t original_begin = next < old.size() ? old[next].original.begin : original_size_;
    size_t original_end = original_begin;
    if (next < old.size() && touched(next)) {
      // A copied piece starting before the group keeps its head: "I have 2 cats" -> "I have " stays copied
      if (!old[next].replaced && starts[next] < span.begin) {
        size_t cut = span.begin - starts[next];
        pieces.push_back({.original = {original_begin, original_begin + cut}, .size = cut});
        original_begin += cut;
      }

      size_t last = next;
      while (next < old.size() && touched(next)) {
        last = next++;
      }
      original_end = old[last].original.end;

      // A copied piece ending after the group keeps its tail as an old piece: the next group may cut it again
      if (!old[last].replaced && end_of(last) > span.end) {
        size_t cut = span.end - starts[last];
        original_end = old[last].original.begin + cut;
        old[last].original.begin += cut;
        old[last].size -= cut;
        starts[last] = span.end;
        next = last;
      }
    }

    // 2.3. The text up to the group, then the group's replacement as one replaced piece
    text.append(text_, position, span.begin - position);
    text.append(replacement);
    pieces.push_back({.original = {original_begin, original_end}, .size = replacement.size(), .replaced = true});
    position = span.end;
  }

  // 3. The rest of the text after the last group, and its pieces
  text.append(text_, position);
  pieces.insert(pieces.end(), old.begin() + next, old.end());

  text_ = std::move(text);
  pieces_ = std::move(pieces);
  edits_.clear();
}

std::vector<Document::Edit> Document::changes() const {
  std::vector<Edit> changes;

  // Walk the pieces with their position in the current text; only the replaced ones are changes
  size_t position = 0;
  bool previous_replaced = false;
  for (const auto& piece : pieces_) {
    if (piece.replaced) {
      // Adjacent replaced pieces form a single change
      if (previous_replaced) {
        changes.back().span.end = piece.original.end;
        changes.back().text.append(text_, position, piece.size);
      } else {
        changes.push_back({piece.original, text_.substr(position, piece.size)});
      }
    }
    previous_replaced = piece.replaced;
    position += piece.size;
  }
  return changes;
}

Span Document::widen(Span span, std::span<const size_t> starts) const {
  // An edge falling strictly inside a replaced piece moves to that piece's edge
  for (size_t i = 0; i < pieces_.size(); ++i) {
    size_t begin = starts[i], end = begin + pieces_[i].size;
    if (!pieces_[i].replaced) {
      continue;
    }
    if (begin < span.begin && span.begin < end) {
      span.begin = begin;
    }
    if (begin < span.end && span.end < end) {
      span.end = end;
    }
  }

  return span;
}

} // namespace phonemis::processor
