#pragma once

#include <cstdint>

namespace phonemis::protophone {

/**
 * General type definitions for custom neural inference.
 */
using Float = float;
using Long = int64_t;

/**
 * Placeholder type for 'Entire range' semantic.
 */
struct All {};

/**
 * Defines a pair of indices (i, j), which logicaly corresponds to a range
 * i, i + 1, i + 2, ..., j - 2, j - 1.
 */
template <typename T>
struct Range {
  T begin;
  T end;

  // Convenient alias for the global All type.
  using ALL = All;
};

/**
 * A simple abstraction for possible tensor dimensionalities.
 * For example, D2 corresponds to 2-dimensional tensor (or, in simple terms - a matrix).
 */
enum Rank : uint8_t {
  D1 = 1,
  D2 = 2,
  D3 = 3,
  D4 = 4
};

} // namespace phonemis::protophone
