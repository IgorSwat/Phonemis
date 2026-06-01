#pragma once

#include "types.h"
#include <third-party/xsimd/xsimd.hpp>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// Forward declarations
namespace phonemis::protophone {
  template <typename T> class Tensor;
  template <typename T> class TensorView;
}

namespace phonemis::protophone::utils {

/**
 * Calculates strides (from scratch) for given tensor shape.
 * @param shape a tensor shape to calculate stride for
 */
inline std::vector<size_t> compute_strides(const std::vector<size_t>& shape) {
  if (shape.empty()) return {};

  std::vector<size_t> strides(shape.size());

  size_t stride = 1;
  for (int i = static_cast<int>(shape.size()) - 1; i >= 0; --i) {
    strides[i] = stride;
    stride *= shape[i];
  }

  return strides;
}

/**
 * Reorganizes the data pointed to by a TensorView in-place to match canonical row-major strides.
 * Generic implementation for any rank R that uses a temporary buffer.
 * 
 * @tparam T The data type of the tensor elements.
 * @param view The view to repack in-place.
 */
template <typename T>
inline void repack(TensorView<T>& view) {
  const auto& shape = view.shape();
  const auto& old_strides = view.strides();
  const T* view_data = view.data() + view.offset();

  std::vector<size_t> new_strides = compute_strides(shape);
  size_t total_size = view.size();
  
  std::vector<T, xsimd::default_allocator<T>> buffer(total_size);
  
  for (size_t i = 0; i < total_size; ++i) {
    size_t offset = 0;
    size_t remaining = i;
    for (size_t d = 0; d < shape.size(); ++d) {
      size_t idx = remaining / new_strides[d];
      remaining %= new_strides[d];
      offset += idx * old_strides[d];
    }
    buffer[i] = view_data[offset];
  }

  T* dest = view.data() + view.offset();
  std::copy(buffer.begin(), buffer.end(), dest);

  // Update view metadata to reflect canonical strides
  view.strides_ = std::move(new_strides);
}

/**
 * Reorganizes the data of the input Tensor in-place to match canonical row-major strides.
 * After this operation, the physical memory layout will be contiguous.
 * 
 * @tparam T The data type of the tensor elements.
 * @param tensor The tensor to be repacked in-place.
 */
template <typename T>
inline void repack(Tensor<T>& tensor) {
  auto v = tensor.view();
  repack<T>(v);
  
  // Update tensor metadata to reflect canonical strides
  tensor.strides_ = v.strides();
}

} // namespace phonemis::protophone