#include "test.h"
#include <phonemis/protophone/tensor.h>
#include <phonemis/protophone/tensor_view.h>
#include <phonemis/protophone/types.h>
#include <iostream>
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
		// Dim 0 is reduced by index 0. Dim 1 is preserved by Range{1, 3} (size 2). Dim 2 is carried over (size 4).
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
		// Dim 0 is preserved by All{} (size 2). Dim 1 reduced by index 1. Dim 2 carried over (size 4).
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
	TensorView<Float> view(
		const_cast<Float*>(tensor.data().data()), 
		tensor.shape(), 
		tensor.strides()
	);

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

REGISTER_TEST(tensor_repack_test) {
	// --- Case 1: Transposed 3x4 Tensor ---
	// Data: [0, 1, ..., 11]
	{
		std::vector<size_t> shape = {3, 4};
		std::vector<Float> data(12);
		std::iota(data.begin(), data.end(), 0.0f);
		Tensor<Float> tensor(shape, data.data());

		// Transpose it to 4x3. Strides will be {1, 4} (non-canonical for 4x3)
		auto view = tensor.view();
		auto transposed = view.transpose(0, 1);

		Tensor<Float> transposed_tensor(
			transposed.shape(), 
			const_cast<Float*>(transposed.data()), 
			transposed.strides()
		);
		
		utils::repack<Float>(transposed_tensor);

		std::vector<size_t> expected_shape = {4, 3};
		std::vector<size_t> expected_strides = {3, 1};
		// Logical 4x3 after transpose:
		// [[0, 4, 8],
		//  [1, 5, 9],
		//  [2, 6, 10],
		//  [3, 7, 11]]
		std::vector<Float> expected_data = {0, 4, 8, 1, 5, 9, 2, 6, 10, 3, 7, 11};

		ASSERT_EQUALS(expected_shape.size(), transposed_tensor.shape().size());
		for (size_t i = 0; i < expected_shape.size(); ++i) {
			ASSERT_EQUALS(expected_shape[i], transposed_tensor.shape()[i]);
		}

		for (size_t i = 0; i < expected_strides.size(); ++i) {
			ASSERT_EQUALS(expected_strides[i], transposed_tensor.strides()[i]);
		}

		ASSERT_EQUALS(expected_data.size(), transposed_tensor.data().size());
		for (size_t i = 0; i < expected_data.size(); ++i) {
			ASSERT_EQUALS(expected_data[i], transposed_tensor.data()[i]);
		}
	}

	// --- Case 2: Reshaped View to Tensor ---
	// Repack shouldn't change data if it started as canonical already, 
	// but let's test a reshape that preserves row-major.
	{
		std::vector<size_t> shape = {2, 6};
		std::vector<Float> data(12);
		std::iota(data.begin(), data.end(), 0.0f);
		Tensor<Float> tensor(shape, data.data());

		// Reshape 2x6 to 3x4. Row-major remains canonical.
		auto reshaped_view = tensor.view().reshape({3, 4});
		
		Tensor<Float> reshaped_tensor(
			reshaped_view.shape(),
			const_cast<Float*>(reshaped_view.data()),
			reshaped_view.strides()
		);

		utils::repack<Float>(reshaped_tensor);

		std::vector<size_t> expected_shape = {3, 4};
		std::vector<size_t> expected_strides = {4, 1};
		std::vector<Float> expected_data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

		ASSERT_EQUALS(expected_shape.size(), reshaped_tensor.shape().size());
		for (size_t i = 0; i < expected_shape.size(); ++i) {
			ASSERT_EQUALS(expected_shape[i], reshaped_tensor.shape()[i]);
		}
		ASSERT_EQUALS(expected_strides[0], reshaped_tensor.strides()[0]);
		ASSERT_EQUALS(expected_strides[1], reshaped_tensor.strides()[1]);

		for (size_t i = 0; i < expected_data.size(); ++i) {
			ASSERT_EQUALS(expected_data[i], reshaped_tensor.data()[i]);
		}
	}

	return true;
}

} // namespace phonemis::test
