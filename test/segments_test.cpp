#include "test.h"
#include <phonemis/base/pipeline.h>
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

static std::string data_path(const std::string& file) {
  return std::string(PHONEMIS_PROJECT_ROOT) + "/data/" + file;
}

static Pipeline g_en_pipeline(Config{
    .lang = "en-us",
    .tagger = tagger::Config{.data_filepath = data_path("en-us/tagger.json")},
    .phonemizer = phonemizer::Config{.lexicon_filepath = data_path("en-us/lexicon_small.json")}
});

// Renders segments as "[source|phonemes]", with "_" marking a trailing space.
static std::u32string show(std::u32string_view text, const std::vector<Segment>& segments) {
  std::u32string result;
  for (const auto& segment : segments) {
    result += U"[" + std::u32string(text.substr(segment.begin, segment.end - segment.begin)) +
              U"|" + segment.phonemes + U"]";
    if (segment.whitespace) result += U"_";
  }
  return result;
}

// Checks that segments are ordered, disjoint, within the text, and join into the pipeline's result.
static bool is_consistent(Pipeline& pipeline, std::u32string_view text) {
  auto segments = pipeline.phonemize_segments(text);

  size_t last_end = 0;
  for (const auto& segment : segments) {
    if (segment.begin < last_end || segment.end < segment.begin || segment.end > text.size()) {
      std::cerr << "Misplaced segment [" << segment.begin << ", " << segment.end << ") in: "
                << std::u32string(text) << "\n";
      return false;
    }
    last_end = segment.end;
  }

  ASSERT_EQUALS(pipeline(text), join(segments));
  return true;
}

REGISTER_TEST(segments_en_us_test)
{
  std::u32string text = U"  He's 25 years   old, on 12.05.2024. Hello world!";

  ASSERT_EQUALS(U"[He's|hˌiz]_[25|twˈɛnti fˈIv]_[years|jˈɪɹz]_[old|ˈOld][,|,]_[on|ˌɔn]_"
                U"[12.05.2024|twˈɛlfθ mˈA twˈɛnti twˈɛnti fˈɔɹ][.|.]_[Hello|həlˈO]_[world|wˈɜɹld][!|!]",
                show(text, g_en_pipeline.phonemize_segments(text)));

  return true;
}

REGISTER_TEST(segments_en_us_utf8_test)
{
  // Positions count characters, not UTF-8 bytes
  auto segments = g_en_pipeline.phonemize_segments(std::string_view("naïve world"));

  ASSERT_EQUALS(2, segments.size());
  ASSERT_EQUALS(6, segments[1].begin);
  ASSERT_EQUALS(11, segments[1].end);

  return true;
}

REGISTER_TEST(segments_en_us_no_preprocessing_test)
{
  // Without number verbalization "2" has no pronunciation, but still gets its segment
  std::u32string text = U"I have 2 cats";

  ASSERT_EQUALS(U"[I|ˌI]_[have|hæv]_[2|]_[cats|kˈæts]",
                show(text, g_en_pipeline.phonemize_segments(text, false)));

  return true;
}

REGISTER_TEST(segments_join_matches_pipeline_test)
{
  if (!is_consistent(g_en_pipeline, U"The FBI paid $5 on 2024-03-27, didn't it? Yes: 1/2 of 3.5!")) {
    return false;
  }

  // Neural phonemization of the other languages, including Polish postprocessing
  const std::vector<std::pair<std::string, std::u32string>> cases = {
    {"de", U"Am 12.05.2024 kostete es 3,5 Euro, nicht wahr?"},
    {"fr", U"J'ai 25 chats et 3,5 kilos de pommes."},
    {"es", U"Hoy es el 27-03-2026 y tengo 99€."},
    {"it", U"Ho 21 gatti, e l'altro ne ha 3,5!"},
    {"pl", U"Zima: mam 25 kotów i 3,5 źrebaka."},
    {"pt", U"Eu tenho 25 gatos, não é?"},
    {"hi", U"मेरे पास 25 बिल्लियाँ हैं।"},
  };

  for (const auto& [lang, text] : cases) {
    Pipeline pipeline(Config{
        .lang = lang,
        .phonemizer = phonemizer::Config{
            .lang = lang,
            .nn_model_filepath = data_path(lang + "/phonemizer_" + lang + ".bin")}});

    if (!is_consistent(pipeline, text)) {
      return false;
    }
  }

  return true;
}

} // namespace phonemis::test
