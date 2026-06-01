#include "protophone.h"
#include <iostream>
#include <stdexcept>

namespace phonemis::protophone {

Protophone::Protophone(const std::string& model_path)
    : embedding_({0}), 
      conv3_{Tensor<float>({0}), Tensor<float>({0})},
      conv5_{Tensor<float>({0}), Tensor<float>({0})},
      conv7_{Tensor<float>({0}), Tensor<float>({0})},
      mixer_{Tensor<float>({0}), Tensor<float>({0})},
      ctc_proj_{Tensor<float>({0}), Tensor<float>({0})} {
  load_weights(model_path);
}

void Protophone::load_weights(const std::string& model_path) {
  std::ifstream f(model_path, std::ios::binary);
  if (!f.is_open()) {
    throw std::runtime_error("Failed to open model file: " + model_path);
  }

  // 1. Embedding
  embedding_ = read_tensor(f, "embedding.weight");
  
  // Add padding entry (all zeros).
  // This will be useful during the convolutional phase.
  auto current_shape = embedding_.shape();
  size_t vocab_size = current_shape[0];
  size_t embed_dim = current_shape[1];
  
  std::vector<float> padded_data(embedding_.data().begin(), embedding_.data().end());
  padded_data.resize((vocab_size + 1) * embed_dim, 0.0f);
  
  embedding_ = Tensor<float>({vocab_size + 1, embed_dim}, padded_data.data());

  // 2-4. Multi-Kernel Convolutions
  conv3_.weight = read_tensor(f, "conv3.weight");
  conv3_.bias = read_tensor(f, "conv3.bias");
  
  conv5_.weight = read_tensor(f, "conv5.weight");
  conv5_.bias = read_tensor(f, "conv5.bias");
  
  conv7_.weight = read_tensor(f, "conv7.weight");
  conv7_.bias = read_tensor(f, "conv7.bias");

  // 5. Kernel Mixer (linear)
  mixer_.weight = read_tensor(f, "kernel_mixer.weight");
  mixer_.bias = read_tensor(f, "kernel_mixer.bias");

  // 6-9. ShiftMLP layers (4 linear layers)
  for (int i = 0; i < 4; ++i) {
    auto w = read_tensor(f, "shift_mlp.layer" + std::to_string(i) + ".weight");
    auto b = read_tensor(f, "shift_mlp.layer" + std::to_string(i) + ".bias");
    shift_mlp_.push_back({std::move(w), std::move(b)});
  }

  // 10. CTC projection (linear)
  ctc_proj_.weight = read_tensor(f, "ctc_proj.weight");
  ctc_proj_.bias = read_tensor(f, "ctc_proj.bias");
}

Tensor<float> Protophone::read_tensor(std::ifstream& f, const std::string& name) {
  // The tensors inside the binary file are stored in a following format:
  // <Shape dimension>
  // [Shape values]
  // [Data]
  int64_t num_dims = 0;
  if (!f.read(reinterpret_cast<char*>(&num_dims), sizeof(int64_t))) {
    throw std::runtime_error("Failed to read num_dims for: " + name);
  }

  std::vector<size_t> shape(num_dims);
  size_t total_elements = 1;
  for (int64_t i = 0; i < num_dims; ++i) {
    int64_t dim;
    f.read(reinterpret_cast<char*>(&dim), sizeof(int64_t));
    shape[i] = static_cast<size_t>(dim);
    total_elements *= shape[i];
  }

  std::vector<float> data(total_elements);
  if (!f.read(reinterpret_cast<char*>(data.data()), total_elements * sizeof(float))) {
    throw std::runtime_error("Failed to read data for: " + name);
  }

  return Tensor<float>(shape, data.data());
}

Tensor<float> Protophone::forward(const std::vector<int64_t>& tokens) {
  // TODO: Implement forward pass using loaded weights
  (void)tokens; // Suppress unused parameter warning
  return Tensor<float>({0});
}

} // namespace phonemis::protophone
