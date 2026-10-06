#include "test.h"
#include <phonemis/lang/zh/lexicon_phonemizer.h>
#include <phonemis/utils/io.h>

#include <filesystem>
#include <fstream>

namespace phonemis::test {

namespace {

// A lexicon with pinyin in place of phonemes, written to a temporary file.
std::string write_lexicon() {
  auto path = std::filesystem::temp_directory_path() / "phonemis_zh_lexicon_test.json";
  std::ofstream(path) << R"({
    "人": "ren2 ", "行": "xing2 ", "道": "dao4 ", "眼": "yan3 ", "不": "bu4 ", "见": "jian4 ",
    "心": "xin1 ", "烦": "fan2 ", "银": "yin2 ",
    "人行": "ren2 xing2 ", "行道": "hang2 dao4 ", "不见": "bu2 jian4 ",
    "眼不见心不烦": "yan3 bu2 jian4 xin1 bu2 fan2 ", "银行": "yin2 hang2 "
  })";
  return path.string();
}

} // namespace

// Expected values follow pypinyin's phrase segmentation.
REGISTER_TEST(lexicon_phonemizer_zh_test)
{
    zh::LexiconPhonemizer phonemizer(phonemizer::Config{.lang = "zh", .lexicon_filepath = write_lexicon()});

    ASSERT_EQUALS(U"yin2 hang2 ", phonemizer.phonemize_word(U"银行"));

    // The longest phrase from the left wins, even when a later one would overlap it.
    ASSERT_EQUALS(U"ren2 xing2 dao4 ", phonemizer.phonemize_word(U"人行道"));
    ASSERT_EQUALS(U"xing2 hang2 dao4 ", phonemizer.phonemize_word(U"行行道"));

    // What remains only starts a longer phrase, so each character is read on its own.
    ASSERT_EQUALS(U"yan3 bu4 jian4 ", phonemizer.phonemize_word(U"眼不见"));
    ASSERT_EQUALS(U"yan3 bu2 jian4 xin1 bu2 fan2 ", phonemizer.phonemize_word(U"眼不见心不烦"));

    // Unknown characters are skipped.
    ASSERT_EQUALS(U"ren2 dao4 ", phonemizer.phonemize_word(U"人丁道"));

    return true;
}

} // namespace phonemis::test
