#pragma once

#include "tensor.h"
#include "types.h"
#include <fstream>
#include <string>
#include <vector>

namespace phonemis::protophone {

/**
 * High-performance C++ implementation of the Protophone CTC model.
 *
 * The model is always evaluated for a single sequence (batch size 1), matching
 * the runner which phonemizes one word at a time. The forward pass is split
 * into a handful of fused, SIMD-accelerated stages (see protophone.cpp).
 *
 * Large scratch buffers are kept as members and only ever grow, so repeated
 * calls to forward() do not reallocate once a steady-state sequence length is
 * reached.
 */
class Protophone {
public:
  explicit Protophone(const std::string& model_path);

  /**
   * Implements forward pass of the Protophone model.
   * @param tokens input tokens for a single word.
   * @return logits of shape (2 * tokens.size(), NUM_PHONEMES).
   */
  Tensor<float> forward(const std::vector<int64_t>& tokens);

protected:
  // ---- Weight loading -------------------------------------------------------
  void load_weights(const std::string& model_path);
  Tensor<float> read_tensor(std::ifstream& f, const std::string& name);

  // ---- Inference stages (operate on the member scratch buffers) -------------
  void embed(const std::vector<int64_t>& tokens);  // -> emb_padded_
  void multi_kernel_conv_glu();                     // emb_padded_ -> glu_
  void kernel_mixer();                              // glu_ -> mixed_
  void upsample_linear();                           // mixed_ -> shift_a_
  void shift_mlp();                                 // shift_a_/shift_b_ ping-pong
  Tensor<float> ctc_projection();                   // shift_a_ -> logits

  // ---- Model weights --------------------------------------------------------
  Tensor<float> embedding_;

  // Multi-kernel convolutions. Weights are repacked at load time from the
  // PyTorch [out_ch, in_ch, k] layout to [out_ch, k, in_ch] so that a single
  // convolution output is a contiguous dot product over k * EMBEDDING_DIM.
  Weights<float> conv3_;
  Weights<float> conv5_;
  Weights<float> conv7_;

  // Kernel mixer (linear 3712 -> 256).
  Weights<float> mixer_;

  // ShiftMLP (four linear 256 -> 256 layers, dilations 1/2/4/8).
  std::vector<Weights<float>> shift_mlp_;

  // CTC projection (linear 256 -> 116).
  Weights<float> ctc_proj_;

  // ---- Scratch buffers (grow-only, reused across forward() calls) -----------
  using Buffer = std::vector<float, xsimd::default_allocator<float>>;

  size_t embed_dim_ = 0;  // EMBEDDING_DIM: read from the embedding weight shape
  size_t seq_len_ = 0;    // S: number of input tokens
  size_t up_len_ = 0;     // T = 2 * S: upsampled length

  Buffer emb_padded_;   // (S + 2*MAX_PAD) x embed_dim_, zero-padded borders
  Buffer glu_;          // S x (total_K * F)
  Buffer mixed_;        // S x L
  Buffer shift_a_;      // T x L  (ping)
  Buffer shift_b_;      // T x L  (pong)
  Buffer shifted_;      // L: shifted feature vector for one ShiftLinear step
};

} // namespace phonemis::protophone
