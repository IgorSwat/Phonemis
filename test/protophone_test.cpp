#include "test.h"
#include <phonemis/protophone/protophone.h>
#include <phonemis/protophone/tensor.h>
#include <phonemis/protophone/types.h>
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
    // Expected weight shape: [OutChannels, InChannels, KernelSize]
    // OutChannels = 2 * F * K3 = 2 * 64 * 28 = 3584
    // InChannels = EMBEDDING_DIM = 16
    // KernelSize = 3
    const auto& conv3 = model.get_conv3();
    ASSERT_EQUALS(size_t(3), conv3.weight.shape().size());
    ASSERT_EQUALS(2 * F * K3, conv3.weight.shape()[0]);
    ASSERT_EQUALS(EMBEDDING_DIM, conv3.weight.shape()[1]);
    ASSERT_EQUALS(size_t(3), conv3.weight.shape()[2]);
    ASSERT_EQUALS(size_t(1), conv3.bias.shape().size());
    ASSERT_EQUALS(2 * F * K3, conv3.bias.shape()[0]);

    // Verify Conv5
    // Expected weight shape: [OutChannels, InChannels, KernelSize]
    // OutChannels = 2 * F * K5 = 2 * 64 * 20 = 2560
    // InChannels = EMBEDDING_DIM = 16
    // KernelSize = 5
    const auto& conv5 = model.get_conv5();
    ASSERT_EQUALS(size_t(3), conv5.weight.shape().size());
    ASSERT_EQUALS(2 * F * K5, conv5.weight.shape()[0]);
    ASSERT_EQUALS(EMBEDDING_DIM, conv5.weight.shape()[1]);
    ASSERT_EQUALS(size_t(5), conv5.weight.shape()[2]);
    ASSERT_EQUALS(size_t(1), conv5.bias.shape().size());
    ASSERT_EQUALS(2 * F * K5, conv5.bias.shape()[0]);

    // Verify Conv7
    // Expected weight shape: [OutChannels, InChannels, KernelSize]
    // OutChannels = 2 * F * K7 = 2 * 64 * 10 = 1280
    // InChannels = EMBEDDING_DIM = 16
    // KernelSize = 7
    const auto& conv7 = model.get_conv7();
    ASSERT_EQUALS(size_t(3), conv7.weight.shape().size());
    ASSERT_EQUALS(2 * F * K7, conv7.weight.shape()[0]);
    ASSERT_EQUALS(EMBEDDING_DIM, conv7.weight.shape()[1]);
    ASSERT_EQUALS(size_t(7), conv7.weight.shape()[2]);
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

} // namespace phonemis::test
