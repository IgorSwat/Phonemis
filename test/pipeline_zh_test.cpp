#include "test.h"
#include <phonemis/lang/zh/pipeline.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

// Expected values are misaki.zh.ZHG2P() outputs, the G2P of Kokoro v1.0.
REGISTER_TEST(pipeline_zh_test)
{
    zh::Pipeline pipeline(Config{
        .lang = "zh",
        .tagger = tagger::Config{
            .data_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/zh/segmenter.json"
        },
        .phonemizer = phonemizer::Config{
            .lang = "zh",
            .lexicon_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/zh/lexicon.json"
        }
    });

    ASSERT_EQUALS(U"ni↓xau↓, ʨi→ntʰjɛ→ntʰjɛ→nʨʰi↘ xə↓n xau↓.",
                  pipeline("你好，今天天气很好。"));
    ASSERT_EQUALS(U"wo↓mən ʨʰy↘ i↗nxa↗ŋ ʨʰy↓ ʨʰjɛ↗n, ɻa↗nxou↘ ʨʰy↘ kʊ→ŋɥɛ↗n sa↘npu↘.",
                  pipeline("我们去银行取钱，然后去公园散步。"));
    ASSERT_EQUALS(U"tʰa→ jou↓ sa→nkɤ↘ pʰi↗ŋkwo↓.", pipeline("他有3个苹果。"));
    ASSERT_EQUALS(U"", pipeline("  "));

    return true;
}

REGISTER_TEST(pipeline_zh_without_data_test)
{
    // Like the other languages, the pipeline builds without data files.
    zh::Pipeline pipeline(Config{.lang = "zh", .phonemizer = phonemizer::Config{.lang = "zh"}});

    // Han characters, including spelled-out numbers, are dropped; the rest stays.
    ASSERT_EQUALS(U", ok .", pipeline("你好，ok 1。"));

    return true;
}

} // namespace phonemis::test
