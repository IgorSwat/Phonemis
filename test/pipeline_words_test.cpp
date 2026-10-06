#include "test.h"
#include <phonemis/base/ipipeline.h>
#include <phonemis/lang/en/pipeline.h>
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/io.h>

#include <map>

namespace phonemis::test {

namespace {

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

// A pipeline whose preprocessing is scripted per input, and whose phonemes are
// the preprocessed text itself, so that only the word matching is under test.
class StubPipeline : public IPipeline {
public:
  explicit StubPipeline(std::map<std::u32string, std::u32string> normalizations)
      : normalizations_(std::move(normalizations)) {}

  std::u32string preprocess(const std::u32string& input) override {
    if (auto it = normalizations_.find(input); it != normalizations_.end()) {
      return it->second;
    }
    return input;
  }

  std::u32string process(const std::u32string& input) override { return input; }

  std::u32string postprocess(const std::u32string& input) override { return input; }

private:
  std::map<std::u32string, std::u32string> normalizations_;
};

en::Pipeline g_en_pipeline(Config{
    .lang = "en-us",
    .tagger = tagger::Config{
      .data_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/en-us/tagger.json"
    },
    .phonemizer = phonemizer::Config{
      .lexicon_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/en-us/lexicon_small.json"
    }
});

} // namespace

REGISTER_TEST(pipeline_words_offsets_test) {
  StubPipeline pipeline({{U"  Hello \t man world!", U"Hello man world!"}});
  std::u32string text = U"  Hello \t man world!";
  auto result = pipeline.phonemize_words(text);

  ASSERT_EQUALS(U"Hello man world!", result.phonemes);
  ASSERT_EQUALS(3, result.words.size());
  ASSERT_EQUALS(2, result.words[0].offset);
  ASSERT_EQUALS(10, result.words[1].offset);
  ASSERT_EQUALS(14, result.words[2].offset);
  ASSERT_EQUALS(U"Hello:Hello|man:man|world!:world!", describe(text, result));
  return true;
}

REGISTER_TEST(pipeline_words_spelled_out_number_test) {
  StubPipeline pipeline({{U"I have 25 cats", U"I have twenty five cats"},
                         {U"25", U"twenty five"}});
  std::u32string text = U"I have 25 cats";

  ASSERT_EQUALS(U"I:I|have:have|25:twenty five|cats:cats",
                describe(text, pipeline.phonemize_words(text)));
  return true;
}

REGISTER_TEST(pipeline_words_dropped_number_test) {
  // A number that preprocessing removes keeps the words after it in place.
  StubPipeline pipeline({{U"a 99 b", U"a b"}, {U"99", U""}});
  std::u32string text = U"a 99 b";

  ASSERT_EQUALS(U"a:a|99:|b:b", describe(text, pipeline.phonemize_words(text)));
  return true;
}

REGISTER_TEST(pipeline_words_mismatch_test) {
  // Every word after the number would be off by one, so none is returned.
  StubPipeline pipeline({{U"x 7 y", U"x seven y"}, {U"7", U"seven more"}});
  auto result = pipeline.phonemize_words(std::u32string_view{U"x 7 y"});

  ASSERT_EQUALS(U"x seven y", result.phonemes);
  ASSERT_EQUALS(0, result.words.size());
  return true;
}

REGISTER_TEST(pipeline_words_non_ascii_offsets_test) {
  // Offsets count code points, so a word after a multi-byte one is not shifted.
  StubPipeline pipeline({});
  std::u32string text = U"żółw 🐢 kot";

  auto result = pipeline.phonemize_words(utils::conversions::u32_to_utf8(text));
  ASSERT_EQUALS(7, result.words[2].offset);
  ASSERT_EQUALS(U"żółw:żółw|🐢:🐢|kot:kot", describe(text, result));
  return true;
}

REGISTER_TEST(pipeline_words_empty_test) {
  StubPipeline pipeline({});
  auto result = pipeline.phonemize_words(std::u32string_view{U""});

  ASSERT_EQUALS(0, result.words.size());
  return true;
}

REGISTER_TEST(pipeline_words_en_us_test) {
  std::u32string text = U"He's 25 years old. It's 10:30 PM.";
  auto result = g_en_pipeline.phonemize_words(text);

  ASSERT_EQUALS(g_en_pipeline(text), result.phonemes);
  ASSERT_EQUALS(U"He's:hˌiz|25:twˈɛnti fˈIv|years:jˈɪɹz|old.:ˈOld.|It's:ˌɪts|10:30:tˈɛn:θˈɜɹɾi|PM.:pˌiˈɛm.",
                describe(text, result));
  return true;
}

} // namespace phonemis::test
