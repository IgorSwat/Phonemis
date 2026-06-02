#include "test.h"
#include <third-party/xsimd/xsimd.hpp>
#include <vector>
#include <numeric>

namespace phonemis::test {

REGISTER_TEST(xsimd_basic_arithmetic_test)
{
    namespace xs = xsimd;
    using batch_type = xs::batch<float>;

    std::vector<float> a = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> b = {5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> res(4);

    auto b_a = xs::load_unaligned(a.data());
    auto b_b = xs::load_unaligned(b.data());
    auto b_res = b_a + b_b;

    b_res.store_unaligned(res.data());

    ASSERT_EQUALS(6.0f, res[0]);
    ASSERT_EQUALS(8.0f, res[1]);
    ASSERT_EQUALS(10.0f, res[2]);
    ASSERT_EQUALS(12.0f, res[3]);

    return true;
}

REGISTER_TEST(xsimd_load_store_aligned_test)
{
    namespace xs = xsimd;
    using batch_type = xs::batch<float>;
    constexpr size_t alignment = xs::default_arch::alignment();

    float* a = static_cast<float*>(xs::aligned_malloc(4 * sizeof(float), alignment));
    float* b = static_cast<float*>(xs::aligned_malloc(4 * sizeof(float), alignment));
    float* res = static_cast<float*>(xs::aligned_malloc(4 * sizeof(float), alignment));

    for (size_t i = 0; i < 4; ++i) {
        a[i] = static_cast<float>(i + 1);
        b[i] = static_cast<float>((i + 1) * 2);
    }

    auto b_a = xs::load_aligned(a);
    auto b_b = xs::load_aligned(b);
    auto b_res = b_a * b_b;

    b_res.store_aligned(res);

    ASSERT_EQUALS(2.0f, res[0]);
    ASSERT_EQUALS(8.0f, res[1]);
    ASSERT_EQUALS(18.0f, res[2]);
    ASSERT_EQUALS(32.0f, res[3]);

    xs::aligned_free(a);
    xs::aligned_free(b);
    xs::aligned_free(res);

    return true;
}

REGISTER_TEST(xsimd_math_functions_test)
{
    namespace xs = xsimd;
    using batch_type = xs::batch<float>;

    std::vector<float> a = {0.0f, 1.0f, 2.0f, 3.0f};
    std::vector<float> res(4);

    auto b_a = xs::load_unaligned(a.data());
    auto b_res = xs::exp(b_a);

    b_res.store_unaligned(res.data());

    // Approximate comparison for exp
    auto is_around = [](float val, float expected) {
        return std::abs(val - expected) < 0.001f;
    };

    ASSERT_EQUALS(true, is_around(res[0], 1.0f));
    ASSERT_EQUALS(true, is_around(res[1], 2.71828f));
    ASSERT_EQUALS(true, is_around(res[2], 7.38906f));

    return true;
}

} // namespace phonemis::test
