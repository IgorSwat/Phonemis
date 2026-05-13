#pragma once

#include "numeric.h"
#include "tensor_utils.h"
#include "tensor_view.h"
#include <third-party/xsimd/xsimd.hpp>

#include <algorithm>
#include <numeric>
#include <vector>

namespace phonemis::protophone {

/**
 * A multidimensional tensor class template.
 * Stores data in a linearized std::vector with XSIMD-compatible alignment.
 */
template <typename T>
class Tensor {
public:
  explicit Tensor(std::vector<size_t> shape)
    : shape_(std::move(shape)) {
    strides_ = utils::compute_strides(shape_);
    data_.resize(size(), static_cast<T>(0));
  }

  explicit Tensor(std::vector<size_t> shape, const T* data_ptr)
    : shape_(std::move(shape)) {
    strides_ = utils::compute_strides(shape_);
    data_.assign(data_ptr, data_ptr + size());
  }

  explicit Tensor(std::vector<size_t> shape, const T* data_ptr, std::vector<size_t> strides)
    : shape_(std::move(shape)), strides_(std::move(strides)) {
    data_.assign(data_ptr, data_ptr + size());
  }

  explicit Tensor(const TensorView<T>& view)
      : shape_(view.shape()), strides_(view.strides()) {
    data_.assign(view.data() + view.offset(), view.data() + view.offset() + size());
  }

  /**
   * Returns a non-owning view of the tensor.
   */
  TensorView<T> view() {
    return TensorView<T>(data_.data(), shape_, strides_);
  }

  /**
   * Returns a non-owning view of the tensor (const version).
   */
  TensorView<const T> view() const {
    return TensorView<const T>(data_.data(), shape_, strides_);
  }

  // Simple accessors
  const std::vector<size_t>& shape() const { return shape_; }
  const std::vector<size_t>& strides() const { return strides_; }
  size_t size() const { return !shape_.empty() ? numeric::product(shape_) : 0; }
  const std::vector<T, xsimd::default_allocator<T>>& data() const { return data_; }

private:
  // Friends declarations
  template <typename U>
  friend void utils::repack(Tensor<U>& tensor);

  // Tensor data
  std::vector<T, xsimd::default_allocator<T>> data_;

  // Tensor metadata
  std::vector<size_t> shape_;
  std::vector<size_t> strides_;
};

} // namespace phonemis::protophone
