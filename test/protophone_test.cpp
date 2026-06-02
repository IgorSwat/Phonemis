#include "test.h"
#include <phonemis/protophone/protophone.h>
#include <phonemis/protophone/tensor.h>
#include <phonemis/protophone/types.h>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

namespace phonemis::test {

using namespace protophone;

/**
 * Derived class to expose Protophone's protected members for testing.
 */
class TestableProtophone : public Protophone {
public:
  using Protophone::Protophone;

  const Tensor<float>& get_embedding() const { return embedding_; }
  const Weights<float>& get_conv3() const { return conv3_; }
  const Weights<float>& get_conv5() const { return conv5_; }
  const Weights<float>& get_conv7() const { return conv7_; }
  const Weights<float>& get_mixer() const { return mixer_; }
  const std::vector<Weights<float>>& get_shift_mlp() const { return shift_mlp_; }
  const Weights<float>& get_ctc_proj() const { return ctc_proj_; }
};

REGISTER_TEST(protophone_loading_test) {
  const std::string model_path = std::string(PHONEMIS_PROJECT_ROOT) + "/data/en-us/phonemizer_en_us.bin";
  
  // Test parameters based on architecture
  const size_t EMBEDDING_DIM = 16;
  const size_t K3 = 28;
  const size_t K5 = 20;
  const size_t K7 = 10;
  const size_t F = 64;
  const size_t L = 256;
  const size_t TOTAL_K = K3 + K5 + K7;

  try {
    TestableProtophone model(model_path);

    // Verify Embedding (Shape: [vocab_size + 1, EMBEDDING_DIM])
    // vocab_size is loaded from file, EMBEDDING_DIM is 16.
    const auto& embed = model.get_embedding();
    ASSERT_EQUALS(size_t(2), embed.shape().size());
    ASSERT_EQUALS(EMBEDDING_DIM, embed.shape()[1]);

    // Verify Conv3
    // Weights are repacked at load from PyTorch's [OutChannels, InChannels,
    // KernelSize] to [OutChannels, KernelSize, InChannels] so a convolution
    // output is a single contiguous dot product.
    // OutChannels = 2 * F * K3 = 2 * 64 * 28 = 3584
    // KernelSize = 3
    // InChannels = EMBEDDING_DIM = 16
    const auto& conv3 = model.get_conv3();
    ASSERT_EQUALS(size_t(3), conv3.weight.shape().size());
    ASSERT_EQUALS(2 * F * K3, conv3.weight.shape()[0]);
    ASSERT_EQUALS(size_t(3), conv3.weight.shape()[1]);
    ASSERT_EQUALS(EMBEDDING_DIM, conv3.weight.shape()[2]);
    ASSERT_EQUALS(size_t(1), conv3.bias.shape().size());
    ASSERT_EQUALS(2 * F * K3, conv3.bias.shape()[0]);

    // Verify Conv5 (repacked layout: [OutChannels, KernelSize, InChannels])
    // OutChannels = 2 * F * K5 = 2 * 64 * 20 = 2560
    // KernelSize = 5
    // InChannels = EMBEDDING_DIM = 16
    const auto& conv5 = model.get_conv5();
    ASSERT_EQUALS(size_t(3), conv5.weight.shape().size());
    ASSERT_EQUALS(2 * F * K5, conv5.weight.shape()[0]);
    ASSERT_EQUALS(size_t(5), conv5.weight.shape()[1]);
    ASSERT_EQUALS(EMBEDDING_DIM, conv5.weight.shape()[2]);
    ASSERT_EQUALS(size_t(1), conv5.bias.shape().size());
    ASSERT_EQUALS(2 * F * K5, conv5.bias.shape()[0]);

    // Verify Conv7 (repacked layout: [OutChannels, KernelSize, InChannels])
    // OutChannels = 2 * F * K7 = 2 * 64 * 10 = 1280
    // KernelSize = 7
    // InChannels = EMBEDDING_DIM = 16
    const auto& conv7 = model.get_conv7();
    ASSERT_EQUALS(size_t(3), conv7.weight.shape().size());
    ASSERT_EQUALS(2 * F * K7, conv7.weight.shape()[0]);
    ASSERT_EQUALS(size_t(7), conv7.weight.shape()[1]);
    ASSERT_EQUALS(EMBEDDING_DIM, conv7.weight.shape()[2]);
    ASSERT_EQUALS(size_t(1), conv7.bias.shape().size());
    ASSERT_EQUALS(2 * F * K7, conv7.bias.shape()[0]);

    // Verify Kernel Mixer
    // Expected weight shape: [OutFeatures, InFeatures]
    // OutFeatures = L = 256
    // InFeatures = TOTAL_K * F = (28+20+10) * 64 = 58 * 64 = 3712
    const auto& mixer = model.get_mixer();
    ASSERT_EQUALS(size_t(2), mixer.weight.shape().size());
    ASSERT_EQUALS(L, mixer.weight.shape()[0]);
    ASSERT_EQUALS(TOTAL_K * F, mixer.weight.shape()[1]);
    ASSERT_EQUALS(size_t(1), mixer.bias.shape().size());
    ASSERT_EQUALS(L, mixer.bias.shape()[0]);

    // Verify ShiftMLP
    // Expected weight shape for each layer: [OutFeatures, InFeatures]
    // OutFeatures = L = 256
    // InFeatures = L = 256
    const auto& shift_mlp = model.get_shift_mlp();
    ASSERT_EQUALS(size_t(4), shift_mlp.size());
    for (const auto& layer : shift_mlp) {
      ASSERT_EQUALS(size_t(2), layer.weight.shape().size());
      ASSERT_EQUALS(L, layer.weight.shape()[0]);
      ASSERT_EQUALS(L, layer.weight.shape()[1]);
      ASSERT_EQUALS(size_t(1), layer.bias.shape().size());
      ASSERT_EQUALS(L, layer.bias.shape()[0]);
    }

    // Verify CTC Projection
    // Expected weight shape: [Phonemes, L]
    // Phonemes = 116
    // L = 256
    const auto& ctc = model.get_ctc_proj();
    ASSERT_EQUALS(size_t(2), ctc.weight.shape().size());
    ASSERT_EQUALS(L, ctc.weight.shape()[1]);
    ASSERT_EQUALS(size_t(1), ctc.bias.shape().size());
    ASSERT_EQUALS(ctc.weight.shape()[0], ctc.bias.shape()[0]);

  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return false;
  }

  return true;
}

/**
 * End-to-end inference regression test.
 *
 * The golden outputs below were produced by running the reference PyTorch model
 *  with the exact same weights as
 * data/en-us/phonemizer_en_us.bin, on the five fixed token sequences. For each
 * input we record, per output frame:
 *   - the argmax phoneme id (the CTC decode target), checked for an exact match;
 *   - the corresponding max logit value, checked within a tolerance that
 *     comfortably absorbs float32 rounding (observed drift was < 5e-5).
 *
 * Reproducing the goldens (from the repo root, in the PyTorch environment):
 *   1. Load the flat dump tensors in file order into a state_dict with keys
 *      embedding.weight, multi_kernel_conv.conv{3,5,7}.conv.{weight,bias},
 *      kernel_mixer.{weight,bias}, shift_mlp.layers.{0..3}.linear_mixer.{weight,bias},
 *      ctc_proj.{weight,bias}.
 *   2. model = ProtophoneModel(num_tokens=144); model.load_state_dict(...); model.eval()
 *   3. logits = model(tokens.unsqueeze(0)).squeeze(0); take argmax(-1) and max(-1).
 */
REGISTER_TEST(protophone_inference_test) {
  const std::string model_path =
      std::string(PHONEMIS_PROJECT_ROOT) + "/data/en-us/phonemizer_en_us.bin";

  struct GoldenCase {
    std::vector<int64_t> tokens;    // input token ids
    std::vector<int> argmax;        // expected argmax phoneme id per frame
    std::vector<float> max_logit;   // expected max logit value per frame
  };

  // Golden outputs from the reference PyTorch ProtophoneModel.
  const std::vector<GoldenCase> cases = {
    {
      {5},
      {34, 105},
      {17.293667f, 11.399468f},
    },
    {
      {5, 12, 7},
      {34, 106, 38, 40, 106, 22},
      {30.182238f, 21.936455f, 20.173151f, 11.886073f, 16.990683f, 11.358883f},
    },
    {
      {20, 4, 88, 33, 1},
      {48, 48, 40, 40, 41, 41, 105, 69, 0, 0},
      {11.601090f, 8.272846f, 9.738823f, 15.582560f, 26.574055f, 28.665665f, 14.206701f, 12.473943f, 12.517691f, 14.909207f},
    },
    {
      {10, 20, 30, 40, 50, 60, 70, 80, 90, 100},
      {105, 75, 48, 0, 67, 67, 0, 0, 106, 106, 24, 0, 0, 0, 0, 75, 43, 43, 0, 55},
      {28.983246f, 13.490829f, 22.462673f, 10.353145f, 13.927274f, 17.840904f, 7.521559f, 5.368897f, 14.411487f, 9.266109f, 7.295331f, 4.518578f, 6.780549f, 8.464772f, 9.246365f, 7.864829f, 21.272200f, 26.481148f, 13.068054f, 20.386507f},
    },
    {
      {7, 14, 21, 28, 35, 42, 49, 56, 63, 70, 77, 84, 91, 98, 105, 112, 119, 126, 133, 140},
      {106, 69, 36, 106, 69, 42, 48, 48, 38, 106, 22, 22, 22, 106, 106, 0, 48, 48, 0, 0, 41, 41, 41, 0, 43, 43, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 67, 24},
      {30.170258f, 43.761005f, 26.708910f, 15.302250f, 30.567976f, 7.343648f, 14.141235f, 8.548877f, 12.679736f, 5.520651f, 4.477153f, 7.766502f, 1.748577f, 3.272353f, 5.310675f, 5.910730f, 23.569044f, 15.950327f, 16.462831f, 14.186303f, 27.789640f, 32.652851f, 14.667854f, 16.963350f, 49.475151f, 42.640015f, 13.447174f, 23.183472f, 40.127178f, 30.590143f, 32.656738f, 28.424124f, 30.832556f, 37.373898f, 23.266411f, 42.647198f, 62.006470f, 59.603817f, 23.496277f, 58.076485f},
    },
  };

  // Tolerance for the max logit value (float32 inference vs. float64 reference).
  const float kLogitTol = 1e-2f;

  // Timing results accumulator.
  struct TimedCase {
    size_t tokens;
    size_t frames;
    double elapsed_ms;
  };
  std::vector<TimedCase> timings;

  try {
    Protophone model(model_path);

    for (size_t ci = 0; ci < cases.size(); ++ci) {
      const GoldenCase& gc = cases[ci];

      const auto t0 = std::chrono::steady_clock::now();
      Tensor<float> logits = model.forward(gc.tokens);
      const auto t1 = std::chrono::steady_clock::now();

      const double elapsed_ms =
          std::chrono::duration<double, std::milli>(t1 - t0).count();

      // Expected shape: (2 * seq_len, NUM_PHONEMES).
      ASSERT_EQUALS(size_t(2), logits.shape().size());
      const size_t frames = logits.shape()[0];
      const size_t phonemes = logits.shape()[1];
      ASSERT_EQUALS(2 * gc.tokens.size(), frames);
      ASSERT_EQUALS(gc.argmax.size(), frames);
      ASSERT_EQUALS(gc.max_logit.size(), frames);

      const float* data = logits.data().data();
      for (size_t t = 0; t < frames; ++t) {
        const float* row = data + t * phonemes;

        // Argmax over the phoneme dimension.
        size_t best = 0;
        for (size_t p = 1; p < phonemes; ++p) {
          if (row[p] > row[best]) best = p;
        }

        ASSERT_EQUALS(gc.argmax[t], static_cast<int>(best));

        const float diff = std::fabs(row[best] - gc.max_logit[t]);
        if (diff > kLogitTol) {
          std::cerr << COLOR_RED "Test failed: " COLOR_RESET
                    << "case " << ci << " frame " << t << " max logit "
                    << row[best] << " differs from expected " << gc.max_logit[t]
                    << " by " << diff << " (tol " << kLogitTol << ")\n";
          return false;
        }
      }

      timings.push_back({gc.tokens.size(), frames, elapsed_ms});
    }
  } catch (const std::exception& e) {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return false;
  }

  // ----- Timing report -----
  std::cout << COLOR_BLUE "\nInference performance:\n" COLOR_RESET;
  std::cout << "  " << std::setw(4) << "Case"
            << "  " << std::setw(6) << "Tokens"
            << "  " << std::setw(6) << "Frames"
            << "  " << std::setw(10) << "Time"
            << "  " << std::setw(12) << "Throughput"
            << "\n";
  std::cout << "  " << std::string(4, '-') << "  "
            << std::string(6, '-') << "  "
            << std::string(6, '-') << "  "
            << std::string(10, '-') << "  "
            << std::string(12, '-') << "\n";

  for (size_t ci = 0; ci < timings.size(); ++ci) {
    const auto& tc = timings[ci];
    const double kfr_sec = tc.frames / (tc.elapsed_ms / 1000.0) / 1000.0;
    std::cout << "  " << std::setw(4) << ci
              << "  " << std::setw(6) << tc.tokens
              << "  " << std::setw(6) << tc.frames
              << "  " << std::setw(8) << std::fixed << std::setprecision(3)
              << tc.elapsed_ms << " ms"
              << "  " << std::setw(8) << std::fixed << std::setprecision(1)
              << kfr_sec << " Kfr/s"
              << "\n";
  }
  std::cout << std::string(48, '-') << "\n\n";

  return true;
}

} // namespace phonemis::test
