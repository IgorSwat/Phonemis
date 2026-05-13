#include "test.h"
#include <phonemis/protophone/tensor.h>
#include <phonemis/protophone/tensor_view.h>
#include <phonemis/protophone/types.h>
#include <vector>

namespace phonemis::test {

using namespace protophone;

REGISTER_TEST(tensor_view_reshape_test) {
    // Input: 2x3 tensor
    std::vector<size_t> shape = {2, 3};
    Tensor<Float> tensor(shape);
    TensorView<Float> view(const_cast<Float*>(tensor.data().data()), tensor.shape(), tensor.strides());

    // Operation: reshape to 1x6
    {
        auto reshaped = view.reshape({1, 6});

        // Expected: shape {1, 6}, same offset 0
        std::vector<size_t> expected_shape = {1, 6};
        ASSERT_EQUALS(expected_shape.size(), reshaped.shape().size());
        for (size_t i = 0; i < expected_shape.size(); ++i) {
            ASSERT_EQUALS(expected_shape[i], reshaped.shape()[i]);
        }
        ASSERT_EQUALS(size_t(0), reshaped.offset());
    }

    return true;
}

REGISTER_TEST(tensor_view_slice_test) {
    // Input: 2x3x4 tensor with linearized data [0, 1, 2, ..., 23]
    std::vector<size_t> shape = {2, 3, 4};
    std::vector<Float> data(24);
    std::iota(data.begin(), data.end(), 0.0f);

    Tensor<Float> tensor(shape, data.data());
    TensorView<Float> view(
      const_cast<Float*>(tensor.data().data()), 
      tensor.shape(), 
      tensor.strides()
    );

    // --- Case 1: Single index slice ---
    // Operation: slice(1) on dimension 0
    {
        auto sliced = view.slice(1);
        
        // Expected: shape {3, 4}, offset 12 (stride[0]=12 * index=1)
        std::vector<size_t> expected_shape = {3, 4};
        size_t expected_offset = 12;

        ASSERT_EQUALS(expected_shape.size(), sliced.shape().size());
        for (size_t i = 0; i < expected_shape.size(); ++i) {
            ASSERT_EQUALS(expected_shape[i], sliced.shape()[i]);
        }
        ASSERT_EQUALS(expected_offset, sliced.offset());
    }

    // --- Case 2: Multidimensional slice (Index + Range) ---
    // Operation: slice(0, [1, 3)) on dimensions 0 and 1
    {
        auto sliced = view.slice(0, Range<size_t>{1, 3});
        
        // Expected: shape {2, 4}, offset 4 (stride[1]=4 * range.begin=1)
        std::vector<size_t> expected_shape = {2, 4};
        size_t expected_offset = 4;

        ASSERT_EQUALS(expected_shape.size(), sliced.shape().size());
        for (size_t i = 0; i < expected_shape.size(); ++i) {
            ASSERT_EQUALS(expected_shape[i], sliced.shape()[i]);
        }
        ASSERT_EQUALS(expected_offset, sliced.offset());
    }

    // --- Case 3: All semantic slice ---
    // Operation: slice(All{}, 1) on dimensions 0 and 1
    {
        auto sliced = view.slice(All{}, 1);
        
        // Expected: shape {2, 4}, offset 4 (stride[1]=4 * index=1)
        // Dim 0 is preserved by All{}, Dim 1 reduced by index 1.
        std::vector<size_t> expected_shape = {2, 4};
        size_t expected_offset = 4;

        ASSERT_EQUALS(expected_shape.size(), sliced.shape().size());
        for (size_t i = 0; i < expected_shape.size(); ++i) {
            ASSERT_EQUALS(expected_shape[i], sliced.shape()[i]);
        }
        ASSERT_EQUALS(expected_offset, sliced.offset());
    }

    return true;
}

REGISTER_TEST(tensor_view_transpose_test) {
    // Input: 2x3 tensor
    std::vector<size_t> shape = {2, 3};
    Tensor<Float> tensor(shape);
    TensorView<Float> view(const_cast<Float*>(tensor.data().data()), tensor.shape(), tensor.strides());

    // Operation: transpose dimensions 0 and 1
    {
        auto transposed = view.transpose(0, 1);
        
        // Expected: shape {3, 2}, strides {1, 3} (swapped from {3, 1})
        std::vector<size_t> expected_shape = {3, 2};
        std::vector<size_t> expected_strides = {1, 3};

        ASSERT_EQUALS(expected_shape.size(), transposed.shape().size());
        for (size_t i = 0; i < expected_shape.size(); ++i) {
            ASSERT_EQUALS(expected_shape[i], transposed.shape()[i]);
        }
        
        for (size_t i = 0; i < expected_strides.size(); ++i) {
            ASSERT_EQUALS(expected_strides[i], transposed.strides()[i]);
        }
    }

    return true;
}

} // namespace phonemis::test
