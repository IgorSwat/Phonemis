#include "protophone.h"

#include <third-party/xsimd/xsimd.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <thread>

namespace phonemis::protophone {

namespace {

// ---- Model architecture constants  -------------------
// Every model shape is fixed by the architecture and hardcoded here. The sole
// exception is the embedding dimension, which can vary between languages and is
// therefore read from the embedding weight at load time (see embed_dim_).
constexpr size_t F = 64;                       // features per kernel after GLU
constexpr size_t K3 = 28, K5 = 20, K7 = 10;    // number of kernels per conv
constexpr size_t TOTAL_K = K3 + K5 + K7;       // 58
constexpr size_t KF = TOTAL_K * F;             // 3712 (kernel_mixer input)
constexpr size_t L = 256;                      // hidden width
constexpr size_t NUM_PHONEMES = 116;
constexpr size_t MAX_PAD = 3;                  // padding for the largest kernel (k=7)

// Time-tile width for the register-blocked matmuls (kernel_mixer / shift_mlp).
// 2 * kTile accumulators + 2 weight registers = 10 live SIMD registers, which
// fits the x86 AVX2 register file while staying effective on NEON.
constexpr size_t kTile = 4;

// Upper bound on worker threads. Inference is a small, latency-sensitive
// workload, so a modest cap avoids oversubscription and keeps timings stable.
constexpr size_t kMaxWorkers = 4;

using batch = xsimd::batch<float>;
constexpr size_t W = batch::size;

/**
 * SIMD dot product of two contiguous float arrays of length n.
 * Uses unaligned loads so it works at any row offset.
 */
inline float simd_dot(const float* a, const float* b, size_t n) {
  batch acc(0.0f);
  size_t i = 0;
  for (; i + W <= n; i += W) {
    acc = xsimd::fma(batch::load_unaligned(a + i), batch::load_unaligned(b + i), acc);
  }
  float result = xsimd::reduce_add(acc);
  for (; i < n; ++i) {
    result += a[i] * b[i];
  }
  return result;
}

/**
 * Computes out = bias + W * x for each of the `out_dim` rows, where W is stored
 * row-major as [out_dim, in_dim] and x is contiguous of length `in_dim`.
 * Optionally applies a ReLU to the result.
 */
inline void affine(const float* w, const float* bias, const float* x,
                   size_t out_dim, size_t in_dim, float* out, bool relu) {
  for (size_t o = 0; o < out_dim; ++o) {
    float v = bias[o] + simd_dot(w + o * in_dim, x, in_dim);
    out[o] = relu ? std::max(v, 0.0f) : v;
  }
}

/**
 * Blends two source rows into a destination row using
 * dst = w0 * src0 + w1 * src1, vectorized over the channel dimension.
 */
inline void blend_rows(const float* src0, const float* src1, float w0, float w1,
                       float* dst, size_t n) {
  const batch vw0(w0), vw1(w1);
  size_t i = 0;
  for (; i + W <= n; i += W) {
    batch r = batch::load_unaligned(src0 + i) * vw0;
    r = xsimd::fma(batch::load_unaligned(src1 + i), vw1, r);
    r.store_unaligned(dst + i);
  }
  for (; i < n; ++i) {
    dst[i] = w0 * src0[i] + w1 * src1[i];
  }
}

// Grows a scratch buffer in place; never shrinks, so steady-state runs do not
// reallocate.
inline void ensure(std::vector<float, xsimd::default_allocator<float>>& buf, size_t n) {
  if (buf.size() < n) buf.resize(n);
}

// Process-wide worker budget, resolved once. Capped at kMaxWorkers.
inline size_t worker_budget() {
  static const size_t budget = [] {
    const unsigned hw = std::thread::hardware_concurrency();
    return hw <= 1 ? size_t(1) : std::min<size_t>(hw, kMaxWorkers);
  }();
  return budget;
}

/**
 * Minimal fork/join parallel-for over [0, n). Splits the range into one
 * contiguous chunk per worker and invokes fn(begin, end) for each; the first
 * chunk runs on the calling thread.
 *
 * Threads are only spawned when there are at least `min_per_worker` items per
 * worker, so small / cheap workloads stay fully serial and never pay the
 * thread-creation cost. Callers pick `min_per_worker` to reflect the per-item
 * cost (e.g. a larger value when each item is light).
 */
template <typename Fn>
inline void parallel_for(size_t n, size_t min_per_worker, Fn&& fn) {
  if (n == 0) return;
  const size_t affordable = n / std::max<size_t>(min_per_worker, 1);
  const size_t nthreads = std::min(worker_budget(), affordable);
  if (nthreads <= 1) {
    fn(size_t(0), n);
    return;
  }

  const size_t chunk = (n + nthreads - 1) / nthreads;
  std::vector<std::thread> workers;
  workers.reserve(nthreads - 1);
  for (size_t begin = chunk; begin < n; begin += chunk) {
    const size_t end = std::min(n, begin + chunk);
    workers.emplace_back([&fn, begin, end] { fn(begin, end); });
  }
  fn(size_t(0), std::min(n, chunk));  // first chunk on the calling thread
  for (std::thread& wkr : workers) wkr.join();
}

} // namespace

// =============================================================================
// Construction & weight loading
// =============================================================================

Protophone::Protophone(const std::string& model_path)
    : embedding_({0}),
      conv3_{Tensor<float>({0}), Tensor<float>({0})},
      conv5_{Tensor<float>({0}), Tensor<float>({0})},
      conv7_{Tensor<float>({0}), Tensor<float>({0})},
      mixer_{Tensor<float>({0}), Tensor<float>({0})},
      ctc_proj_{Tensor<float>({0}), Tensor<float>({0})} {
  load_weights(model_path);

  // Fixed-size scratch for one tile of ShiftLinear shifted feature vectors.
  shifted_.resize(kTile * L);
}

void Protophone::load_weights(const std::string& model_path) {
  std::ifstream f(model_path, std::ios::binary);
  if (!f.is_open()) {
    throw std::runtime_error("Failed to open model file: " + model_path);
  }

  // Repacks a conv weight from PyTorch [out_ch, in_ch, k] to [out_ch, k, in_ch].
  // After repacking, the weights of a single output channel form one contiguous
  // run that aligns with the (also contiguous) input window, so a convolution
  // output reduces to a single dot product of length k * in_ch.
  auto repack_conv = [](const Tensor<float>& src) {
    const auto& sh = src.shape();
    const size_t oc = sh[0], ic = sh[1], k = sh[2];
    std::vector<float> buf(oc * k * ic);
    const float* s = src.data().data();
    for (size_t o = 0; o < oc; ++o) {
      for (size_t c = 0; c < ic; ++c) {
        for (size_t j = 0; j < k; ++j) {
          buf[(o * k + j) * ic + c] = s[(o * ic + c) * k + j];
        }
      }
    }
    return Tensor<float>({oc, k, ic}, buf.data());
  };

  // 1. Embedding. The embedding dimension is the only shape read from weights.
  //    A trailing zero row is appended so out-of-range tokens map to a zero
  //    embedding instead of reading out of bounds.
  embedding_ = read_tensor(f, "embedding.weight");
  const size_t vocab_size = embedding_.shape()[0];
  embed_dim_ = embedding_.shape()[1];
  std::vector<float> padded(embedding_.data().begin(), embedding_.data().end());
  padded.resize((vocab_size + 1) * embed_dim_, 0.0f);
  embedding_ = Tensor<float>({vocab_size + 1, embed_dim_}, padded.data());

  // 2-4. Multi-kernel convolutions (repacked for contiguous dot products).
  conv3_.weight = repack_conv(read_tensor(f, "conv3.weight"));
  conv3_.bias = read_tensor(f, "conv3.bias");
  conv5_.weight = repack_conv(read_tensor(f, "conv5.weight"));
  conv5_.bias = read_tensor(f, "conv5.bias");
  conv7_.weight = repack_conv(read_tensor(f, "conv7.weight"));
  conv7_.bias = read_tensor(f, "conv7.bias");

  // 5. Kernel mixer.
  mixer_.weight = read_tensor(f, "kernel_mixer.weight");
  mixer_.bias = read_tensor(f, "kernel_mixer.bias");

  // 6-9. ShiftMLP layers.
  for (int i = 0; i < 4; ++i) {
    auto w = read_tensor(f, "shift_mlp.layer" + std::to_string(i) + ".weight");
    auto b = read_tensor(f, "shift_mlp.layer" + std::to_string(i) + ".bias");
    shift_mlp_.push_back({std::move(w), std::move(b)});
  }

  // 10. CTC projection.
  ctc_proj_.weight = read_tensor(f, "ctc_proj.weight");
  ctc_proj_.bias = read_tensor(f, "ctc_proj.bias");
}

Tensor<float> Protophone::read_tensor(std::ifstream& f, const std::string& name) {
  // Binary layout per tensor: <int64 num_dims> <int64 dims...> <float32 data...>
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

// =============================================================================
// Inference stages
// =============================================================================

void Protophone::embed(const std::vector<int64_t>& tokens) {
  // Build a zero-bordered embedding matrix of shape (S + 2*MAX_PAD, embed_dim_).
  // The MAX_PAD zero rows on each side let the convolutions index their
  // receptive field without any per-element boundary checks.
  const size_t padded_rows = seq_len_ + 2 * MAX_PAD;
  ensure(emb_padded_, padded_rows * embed_dim_);
  std::fill(emb_padded_.begin(), emb_padded_.begin() + padded_rows * embed_dim_, 0.0f);

  const float* table = embedding_.data().data();
  const size_t vocab = embedding_.shape()[0];
  for (size_t t = 0; t < seq_len_; ++t) {
    size_t id = static_cast<size_t>(tokens[t]);
    if (id >= vocab) id = vocab - 1;  // trailing zero row
    float* dst = emb_padded_.data() + (MAX_PAD + t) * embed_dim_;
    std::memcpy(dst, table + id * embed_dim_, embed_dim_ * sizeof(float));
  }
}

void Protophone::multi_kernel_conv_glu() {
  ensure(glu_, seq_len_ * KF);

  // One work item per output kernel (K3 + K5 + K7 = TOTAL_K of them). Each item
  // is fully independent: it reads the shared (read-only) embedding window and
  // writes a disjoint F-wide column band of glu_, so the items can run on
  // separate threads without any synchronization.
  struct KernelWork {
    const float* weight;  // weights of this kernel's first (channel 0) output
    const float* bias;    // biases of this kernel's 2*F output channels
    size_t wlen;          // receptive field length = k * embed_dim_
    size_t pad;           // convolution padding for this kernel size
    size_t glu_col;       // destination column of this kernel's F features
  };

  std::array<KernelWork, TOTAL_K> works;
  size_t idx = 0;
  auto add_conv = [&](const Weights<float>& cw, size_t k, size_t pad,
                      size_t num_kernels, size_t glu_off) {
    const float* wbase = cw.weight.data().data();
    const float* bbase = cw.bias.data().data();
    const size_t wlen = k * embed_dim_;
    for (size_t kk = 0; kk < num_kernels; ++kk) {
      works[idx++] = {wbase + kk * 2 * F * wlen, bbase + kk * 2 * F, wlen, pad,
                      glu_off + kk * F};
    }
  };
  add_conv(conv3_, 3, 1, K3, 0);
  add_conv(conv5_, 5, 2, K5, K3 * F);
  add_conv(conv7_, 7, 3, K7, (K3 + K5) * F);

  // Each kernel item is light, and the work scales with seq_len_. Only fan out
  // for non-trivial sequences; otherwise run serially (min_per_worker == TOTAL_K
  // forces a single chunk). For longer inputs, ~16 kernels per worker keeps the
  // thread count modest (a few workers out of the budget).
  const size_t min_per_worker = seq_len_ >= 4 ? 16 : TOTAL_K;

  parallel_for(TOTAL_K, min_per_worker, [&](size_t begin, size_t end) {
    float raw[2 * F];  // per-thread scratch for one kernel's 2*F conv outputs

    for (size_t wi = begin; wi < end; ++wi) {
      const KernelWork& kw = works[wi];

      for (size_t t = 0; t < seq_len_; ++t) {
        // The receptive field is a contiguous block of wlen floats starting at
        // row (MAX_PAD - pad + t). The repacked weights match this layout, so
        // each output channel is a single dot product.
        const float* window =
            emb_padded_.data() + (MAX_PAD - kw.pad + t) * embed_dim_;
        for (size_t ch = 0; ch < 2 * F; ++ch) {
          raw[ch] = kw.bias[ch] + simd_dot(kw.weight + ch * kw.wlen, window, kw.wlen);
        }

        // GLU: the 2*F channels are laid out as [F values | F gates]; the output
        // is value * sigmoid(gate), vectorized over the F features.
        float* optr = glu_.data() + t * KF + kw.glu_col;
        size_t fe = 0;
        for (; fe + W <= F; fe += W) {
          batch v = batch::load_unaligned(raw + fe);
          batch g = batch::load_unaligned(raw + F + fe);
          batch sig = 1.0f / (1.0f + xsimd::exp(-g));
          (v * sig).store_unaligned(optr + fe);
        }
        for (; fe < F; ++fe) {
          optr[fe] = raw[fe] / (1.0f + std::exp(-raw[F + fe]));
        }
      }
    }
  });
}

void Protophone::kernel_mixer() {
  ensure(mixed_, seq_len_ * L);
  const float* w = mixer_.weight.data().data();  // [L, KF]
  const float* b = mixer_.bias.data().data();
  const float* x = glu_.data();                  // [S, KF]
  const size_t S = seq_len_;

  // Register-blocked GEMM: mixed[S, L] = relu(glu[S, KF] @ W^T + b), W is [L, KF].
  //
  // The mixer is the heaviest stage (L * KF MACs per time step), so two things
  // matter most: hiding FMA latency and limiting weight-matrix memory traffic.
  //   - We tile the (small) time dimension by TILE. A streamed weight element
  //     W[o][c] is then reused across TILE time steps, roughly halving the
  //     load:FMA ratio versus computing one dot product at a time.
  //   - Each time step keeps two accumulators over interleaved chunks of the
  //     reduction dimension, giving 2 * TILE independent FMA chains so the
  //     accumulators are not serialized by the FMA latency.
  for (size_t t0 = 0; t0 < S; t0 += kTile) {
    const size_t tn = std::min(kTile, S - t0);

    for (size_t o = 0; o < L; ++o) {
      const float* wrow = w + o * KF;
      batch acc0[kTile];
      batch acc1[kTile];
      for (size_t i = 0; i < tn; ++i) {
        acc0[i] = batch(0.0f);
        acc1[i] = batch(0.0f);
      }

      size_t c = 0;
      for (; c + 2 * W <= KF; c += 2 * W) {
        const batch wb0 = batch::load_unaligned(wrow + c);
        const batch wb1 = batch::load_unaligned(wrow + c + W);
        for (size_t i = 0; i < tn; ++i) {
          const float* px = x + (t0 + i) * KF + c;
          acc0[i] = xsimd::fma(wb0, batch::load_unaligned(px), acc0[i]);
          acc1[i] = xsimd::fma(wb1, batch::load_unaligned(px + W), acc1[i]);
        }
      }
      // A single remaining full-width chunk, if KF is not a multiple of 2 * W.
      if (c + W <= KF) {
        const batch wb0 = batch::load_unaligned(wrow + c);
        for (size_t i = 0; i < tn; ++i) {
          acc0[i] = xsimd::fma(wb0, batch::load_unaligned(x + (t0 + i) * KF + c), acc0[i]);
        }
        c += W;
      }

      // Horizontal reduce + bias + scalar tail + ReLU.
      for (size_t i = 0; i < tn; ++i) {
        float v = b[o] + xsimd::reduce_add(acc0[i] + acc1[i]);
        const float* px = x + (t0 + i) * KF;
        for (size_t cc = c; cc < KF; ++cc) v += wrow[cc] * px[cc];
        mixed_[(t0 + i) * L + o] = std::max(v, 0.0f);
      }
    }
  }
}

void Protophone::upsample_linear() {
  // Mirrors F.interpolate(mode='linear', align_corners=False) along the time
  // axis. Operating directly on the (S, L) row-major layout means each output
  // step is a SIMD blend of two contiguous input rows (no transpose needed).
  ensure(shift_a_, up_len_ * L);
  const float scale = static_cast<float>(seq_len_) / static_cast<float>(up_len_);
  const float* src = mixed_.data();
  for (size_t i = 0; i < up_len_; ++i) {
    float pos = scale * (static_cast<float>(i) + 0.5f) - 0.5f;
    if (pos < 0.0f) pos = 0.0f;
    const size_t i0 = static_cast<size_t>(pos);
    const size_t i1 = std::min(i0 + 1, seq_len_ - 1);
    const float l1 = pos - static_cast<float>(i0);
    blend_rows(src + i0 * L, src + i1 * L, 1.0f - l1, l1, shift_a_.data() + i * L, L);
  }
}

void Protophone::shift_mlp() {
  // Four residual ShiftLinear layers with dilations 1, 2, 4, 8. We ping-pong
  // between two buffers because the channel shift reads neighbouring time steps,
  // which would be corrupted by an in-place update.
  ensure(shift_b_, up_len_ * L);
  constexpr size_t G = L / 4;  // 64: look-ahead / look-back / stay group size
  const size_t dilations[4] = {1, 2, 4, 8};
  const size_t T = up_len_;

  Buffer* in = &shift_a_;
  Buffer* out = &shift_b_;

  for (int layer = 0; layer < 4; ++layer) {
    const size_t d = dilations[layer];
    const float* w = shift_mlp_[layer].weight.data().data();  // [L, L]
    const float* b = shift_mlp_[layer].bias.data().data();
    const float* xin = in->data();
    float* xout = out->data();

    // Same register-blocked GEMM as kernel_mixer: tile the time dimension so a
    // streamed weight element W[o][c] is reused across the tile, and keep two
    // accumulators per time step to break the FMA latency chain. The matmul
    // operates on the per-tile shifted feature vectors assembled up front.
    for (size_t t0 = 0; t0 < T; t0 += kTile) {
      const size_t tn = std::min(kTile, T - t0);

      // Assemble the shifted feature vector for each time step in the tile:
      //   [0,   G)  <- look ahead  : xin[t + d]   (zero past the end)
      //   [G,  2G)  <- look behind : xin[t - d]   (zero before the start)
      //   [2G,  L)  <- stay        : xin[t]
      for (size_t i = 0; i < tn; ++i) {
        const size_t t = t0 + i;
        float* sh = shifted_.data() + i * L;
        if (t + d < T) {
          std::memcpy(sh, xin + (t + d) * L, G * sizeof(float));
        } else {
          std::fill(sh, sh + G, 0.0f);
        }
        if (t >= d) {
          std::memcpy(sh + G, xin + (t - d) * L + G, G * sizeof(float));
        } else {
          std::fill(sh + G, sh + 2 * G, 0.0f);
        }
        std::memcpy(sh + 2 * G, xin + t * L + 2 * G, (L - 2 * G) * sizeof(float));
      }

      for (size_t o = 0; o < L; ++o) {
        const float* wrow = w + o * L;
        batch acc0[kTile];
        batch acc1[kTile];
        for (size_t i = 0; i < tn; ++i) {
          acc0[i] = batch(0.0f);
          acc1[i] = batch(0.0f);
        }

        size_t c = 0;
        for (; c + 2 * W <= L; c += 2 * W) {
          const batch wb0 = batch::load_unaligned(wrow + c);
          const batch wb1 = batch::load_unaligned(wrow + c + W);
          for (size_t i = 0; i < tn; ++i) {
            const float* ps = shifted_.data() + i * L + c;
            acc0[i] = xsimd::fma(wb0, batch::load_unaligned(ps), acc0[i]);
            acc1[i] = xsimd::fma(wb1, batch::load_unaligned(ps + W), acc1[i]);
          }
        }
        if (c + W <= L) {
          const batch wb0 = batch::load_unaligned(wrow + c);
          for (size_t i = 0; i < tn; ++i) {
            acc0[i] = xsimd::fma(wb0, batch::load_unaligned(shifted_.data() + i * L + c), acc0[i]);
          }
          c += W;
        }

        // ReLU(W * shifted + b) + residual.
        for (size_t i = 0; i < tn; ++i) {
          float v = b[o] + xsimd::reduce_add(acc0[i] + acc1[i]);
          const float* ps = shifted_.data() + i * L;
          for (size_t cc = c; cc < L; ++cc) v += wrow[cc] * ps[cc];
          xout[(t0 + i) * L + o] = std::max(v, 0.0f) + xin[(t0 + i) * L + o];
        }
      }
    }

    std::swap(in, out);
  }

  // After four swaps the latest activations live back in shift_a_.
}

Tensor<float> Protophone::ctc_projection() {
  Tensor<float> logits({up_len_, NUM_PHONEMES});
  float* out = logits.view().data();
  const float* w = ctc_proj_.weight.data().data();  // [NUM_PHONEMES, L]
  const float* b = ctc_proj_.bias.data().data();
  const float* x = shift_a_.data();
  for (size_t t = 0; t < up_len_; ++t) {
    affine(w, b, x + t * L, NUM_PHONEMES, L, out + t * NUM_PHONEMES, /*relu=*/false);
  }
  return logits;
}

// =============================================================================
// Forward pass
// =============================================================================

Tensor<float> Protophone::forward(const std::vector<int64_t>& tokens) {
  seq_len_ = tokens.size();
  if (seq_len_ == 0) {
    return Tensor<float>({0, NUM_PHONEMES});
  }
  up_len_ = 2 * seq_len_;

  embed(tokens);
  multi_kernel_conv_glu();
  kernel_mixer();
  upsample_linear();
  shift_mlp();
  return ctc_projection();
}

} // namespace phonemis::protophone
