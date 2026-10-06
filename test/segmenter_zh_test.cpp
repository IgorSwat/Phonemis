#include "test.h"
#include <phonemis/lang/zh/segmenter.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

namespace {

std::u32string segment(const zh::Segmenter& segmenter, std::u32string_view text) {
  std::u32string out;
  for (auto word : segmenter.segment(text)) {
    if (!out.empty()) {
      out += U'/';
    }
    out += word;
  }
  return out;
}

} // namespace

// Expected values are jieba.lcut() outputs.
REGISTER_TEST(segmenter_zh_test)
{
    zh::Segmenter segmenter(tagger::Config{
        .data_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/zh/segmenter.json"
    });

    ASSERT_EQUALS(U"研究/生命/的/起源", segment(segmenter, U"研究生命的起源"));
    ASSERT_EQUALS(U"孩子/们/在/操场上/快乐/地/玩耍", segment(segmenter, U"孩子们在操场上快乐地玩耍"));
    ASSERT_EQUALS(U"今天天气/很/好", segment(segmenter, U"今天天气很好"));

    // "杭研" is not in the dictionary: the HMM joins it.
    ASSERT_EQUALS(U"他/来到/了/网易/杭研/大厦", segment(segmenter, U"他来到了网易杭研大厦"));

    // Characters past U+9FD5 stand alone.
    ASSERT_EQUALS(U"研究/\u9FEA/生命", segment(segmenter, U"研究\u9FEA生命"));

    ASSERT_EQUALS(U"", segment(segmenter, U""));

    return true;
}

} // namespace phonemis::test
