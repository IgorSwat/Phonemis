#include "test.h"
#include <phonemis/lang/zh/pipeline.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

namespace {

zh::Pipeline make_pipeline() {
  return zh::Pipeline(Config{
      .lang = "zh",
      .tagger = tagger::Config{
        .data_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/zh/segmenter.json"
      },
      .phonemizer = phonemizer::Config{
        .lang = "zh",
        .lexicon_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/zh/lexicon.json"
      }
  });
}

// Lists each located word with its phonemes, as "word:phonemes|word:phonemes".
std::u32string describe(std::u32string_view text, const PhonemizedText& result) {
  std::u32string out;
  for (const auto& word : result.words) {
    if (!out.empty()) {
      out += U'|';
    }
    out += text.substr(word.offset, word.length);
    out += U':';
    out += std::u32string_view{result.phonemes}.substr(word.phoneme_offset, word.phoneme_length);
  }
  return out;
}

} // namespace

// Expected values are misaki.zh.ZHG2P() outputs, the G2P of Kokoro v1.0.
REGISTER_TEST(pipeline_zh_test)
{
    auto pipeline = make_pipeline();

    ASSERT_EQUALS(U"ni↓xau↓, ʨi→ntʰjɛ→ntʰjɛ→nʨʰi↘ xə↓n xau↓.",
                  pipeline("你好，今天天气很好。"));
    ASSERT_EQUALS(U"wo↓mən ʨʰy↘ i↗nxa↗ŋ ʨʰy↓ ʨʰjɛ↗n, ɻa↗nxou↘ ʨʰy↘ kʊ→ŋɥɛ↗n sa↘npu↘.",
                  pipeline("我们去银行取钱，然后去公园散步。"));
    ASSERT_EQUALS(U"tʰa→ jou↓ sa→nkɤ↘ pʰi↗ŋkwo↓.", pipeline("他有3个苹果。"));
    ASSERT_EQUALS(U"", pipeline("  "));

    return true;
}

REGISTER_TEST(pipeline_zh_words_test)
{
    auto pipeline = make_pipeline();
    auto words = [&](std::u32string_view text) {
        auto result = pipeline.phonemize_words(text);
        return result.phonemes == pipeline(text) ? describe(text, result) : U"phonemes differ";
    };

    // A word is one phoneme group, with its punctuation.
    ASSERT_EQUALS(U"你好，:ni↓xau↓,|今天天气:ʨi→ntʰjɛ→ntʰjɛ→nʨʰi↘|很:xə↓n|好。:xau↓.",
                  words(U"你好，今天天气很好。"));

    // A spelled-out number, percentage or date is one word, even across phoneme groups.
    ASSERT_EQUALS(U"他:tʰa→|有:jou↓|3个:sa→nkɤ↘|苹果。:pʰi↗ŋkwo↓.", words(U"他有3个苹果。"));
    ASSERT_EQUALS(U"会议:xwei↘i↘|在:ʦai↘|2024年5月1日，:ɚ↘li↗ŋɚ↘ sɨ↘njɛ↗n u↓ɥe↘ i→ɻɨ↘,|约:ɥe→|"
                  U"有:jou↓|30%:pai↓fə→nꭧɨ→sa→nʂɨ↗|的:tɤ|人来。:ɻə↗nlai↗.",
                  words(U"会议在2024年5月1日，约有30%的人来。"));

    // Offsets count code points of the original text, spaces included.
    ASSERT_EQUALS(U"我:wo↓|爱:ai↘|ok:ok|你:ni↓", words(U"  我爱 ok 你  "));
    ASSERT_EQUALS(2, pipeline.phonemize_words(std::u32string_view{U"  我爱 ok 你  "}).words[0].offset);

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
