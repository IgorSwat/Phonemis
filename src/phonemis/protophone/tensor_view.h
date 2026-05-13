#pragma once

#include "meta.h"
#include "numeric.h"
#include "types.h"
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <variant>
#include <vector>

namespace phonemis::protophone {

/**
 * A non-owning view into a Tensor or a subset of its data.
 * Facilitates slicing and zero-copy operations.
 */
template <typename T>
class TensorView {
public:
  TensorView(T* data, std::vector<size_t> shape, std::vector<size_t> strides, size_t offset = 0)
      : data_(data), shape_(std::move(shape)), strides_(std::move(strides)), offset_(offset) {}

  // Accessors
  T* data() { return data_; }
  const T* data() const { return data_; }
  const std::vector<size_t>& shape() const { return shape_; }
  const std::vector<size_t>& strides() const { return strides_; }
  size_t offset() const { return offset_; }

  /**
   * Returns a reshaped view with the same data.
   * @param new_shape The desired dimensions.
   * @return A new TensorView with the updated shape.
   * @throws std::invalid_argument if the new shape is incompatible with the total size.
   */
  TensorView reshape(std::vector<size_t> new_shape) const {
    if (numeric::product(shape_) != numeric::product(new_shape)) {
      throw std::invalid_argument("New shape must have the same total size as the original.");
    }

    // Logic for stride calculation in a reshaped view (assuming row-major linearized data)
    std::vector<size_t> new_strides(new_shape.size());
    if (!new_shape.empty()) {
      size_t stride = 1;
      for (int i = static_cast<int>(new_shape.size()) - 1; i >= 0; --i) {
        new_strides[i] = stride;
        stride *= new_shape[i];
      }
    }

    return TensorView(data_, std::move(new_shape), std::move(new_strides), offset_);
  }

  /**
   * Returns a sliced view along multiple dimensions.
   * Supports both single index and Range<size_t> slicing.
   * @param args Pack of variants (size_t or Range<size_t>) for each dimension.
   *             A single size_t value corresponds to an index lookup (reduces dimensionality),
   *             while a Range<size_t> corresponds to range slicing (preserves dimensionality).
   * @return A new TensorView representing the sub-region.
   */
  template <typename... Args>
  TensorView slice(Args... args) const {
    // Aggregate all slices in iterable structure
    std::vector<std::variant<size_t, Range<size_t>, All>> slices = {
      meta::to_slice_variant(args)...
    };
    
    std::vector<size_t> new_shape;
    std::vector<size_t> new_strides;
    size_t new_offset = offset_;

    for (size_t i = 0; i < slices.size(); ++i) {
      if (std::holds_alternative<size_t>(slices[i])) {
        size_t idx = std::get<size_t>(slices[i]);
        new_offset += strides_[i] * idx;
      } else if (std::holds_alternative<All>(slices[i])) {
        new_shape.push_back(shape_[i]);
        new_strides.push_back(strides_[i]);
      } else {
        Range<size_t> range = std::get<Range<size_t>>(slices[i]);
        new_offset += strides_[i] * range.begin;
        new_shape.push_back(range.end - range.begin);
        new_strides.push_back(strides_[i]);
      }
    }

    // Add remaining dimensions that weren't sliced
    for (size_t i = slices.size(); i < shape_.size(); ++i) {
      new_shape.push_back(shape_[i]);
      new_strides.push_back(strides_[i]);
    }

    return TensorView(data_, std::move(new_shape), std::move(new_strides), new_offset);
  }

  /**
   * Returns a transposed view by swapping two dimensions.
   * @param dim0 First dimension to swap.
   * @param dim1 Second dimension to swap.
   * @return A new TensorView with swapped shape and strides.
   */
  TensorView transpose(size_t dim0 = 0, size_t dim1 = 1) const {
    std::vector<size_t> new_shape = shape_;
    std::vector<size_t> new_strides = strides_;
    
    std::swap(new_shape[dim0], new_shape[dim1]);
    std::swap(new_strides[dim0], new_strides[dim1]);

    return TensorView(data_, std::move(new_shape), std::move(new_strides), offset_);
  }

private:
  // Pointer to the original data buffer (non-owning)
  T* data_;

  // View metadata
  std::vector<size_t> shape_;
  std::vector<size_t> strides_;
  size_t offset_;
};

} // namespace phonemis::protophone
