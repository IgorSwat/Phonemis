#pragma once

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
  /**
   * Constructs a tensor with a given shape. Data is zero-initialized.
   * @param shape The dimensions of the tensor.
   */
  explicit Tensor(std::vector<size_t> shape)
    : shape_(std::move(shape)) {
    build_strides();
    data_.resize(size(), static_cast<T>(0));
  }

  /**
   * Constructs a tensor with a given shape and initializes it with external data.
   * @param shape The dimensions of the tensor.
   * @param data_ptr Pointer to the data to copy into the tensor.
   */
  Tensor(std::vector<size_t> shape, const T* data_ptr)
    : shape_(std::move(shape)) {
    build_strides();
    data_.assign(data_ptr, data_ptr + size());
  }

  /**
   * Calculates and returns number of elements in it's data storage.
   */
  size_t size() const { 
    return !shape_.empty() ? std::accumulate(shape_.begin(), shape_.end(), 1ULL, std::multiplies<size_t>()) : 0; 
  }

  // Accessors
  const std::vector<size_t>& shape() const { return shape_; }
  const std::vector<size_t>& strides() const { return strides_; }
  const std::vector<T, xsimd::default_allocator<T>>& data() const { return data_; }

private:
  /**
   * Calculates strides based on the current shape for row-major layout.
   */
  void build_strides() {
    strides_.resize(shape_.size());
    if (shape_.empty()) return;

    size_t stride = 1;
    for (int i = static_cast<int>(shape_.size()) - 1; i >= 0; --i) {
      strides_[i] = stride;
      stride *= shape_[i];
    }
  }

  // Tensor data
  std::vector<T, xsimd::default_allocator<T>> data_;

  // Tensor metadata
  std::vector<size_t> shape_;
  std::vector<size_t> strides_;
};

} // namespace phonemis::protophone
