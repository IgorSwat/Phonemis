#pragma once

#include <cstdint>

namespace phonemis::protophone {

// Forward declarations
template <typename T> class Tensor;

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
 * Defines weights of an affine transormation Wx + b.
 */
template <typename T>
struct Weights {
  Tensor<T> weight;
  Tensor<T> bias;
};

} // namespace phonemis::protophone
