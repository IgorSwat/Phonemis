#pragma once

#include "tensor.h"
#include "types.h"
#include <fstream>
#include <string>
#include <vector>

namespace phonemis::protophone {

/**
 * High-performance C++ implementation of the Protophone model.
 * Handles weight loading from binary format and inference.
 */
class Protophone {
public:
  explicit Protophone(const std::string& model_path);

  /**
   * Implements forward pass of the Protophone model.
   * @param tokens input tokens
   */
  Tensor<float> forward(const std::vector<int64_t>& tokens);

protected:
  // Helper functions - reading model weights.
  void load_weights(const std::string& model_path);
  Tensor<float> read_tensor(std::ifstream& f, const std::string& name);

  // Model Weights
  Tensor<float> embedding_;

  // Multi-Kernel Convolutions
  Weights<float> conv3_;
  Weights<float> conv5_;
  Weights<float> conv7_;

  // Kernel Mixer
  Weights<float> mixer_;

  // ShiftMLP
  std::vector<Weights<float>> shift_mlp_;

  // CTC Projection
  Weights<float> ctc_proj_;
};

} // namespace phonemis::protophone
