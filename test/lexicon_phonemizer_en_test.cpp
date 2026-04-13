#include "test.h"
#include <phonemis/base/types.h>
#include <phonemis/lang/en/lexicon_phonemizer.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace en;

static LexiconPhonemizer g_phonemizer(phonemizer::Config{
  .lang = Lang::EN_US,
  .lexicon_filepath = std::string(PHONEMIS_PROJECT_ROOT) + "/data/english/us_small.json"
});

REGISTER_TEST(lexicon_phonemizer_en_standard_words_test)
{
  // Standard popular english words
  ASSERT_EQUALS(U"həlˈO", g_phonemizer.lookup(U"hello", Tag("")));
  ASSERT_EQUALS(U"wˈɜɹld", g_phonemizer.lookup(U"world", Tag("")));
  ASSERT_EQUALS(U"kəmpjˈuɾəɹ", g_phonemizer.lookup(U"computer", Tag("")));
  ASSERT_EQUALS(U"sˈIəns", g_phonemizer.lookup(U"science", Tag("")));
  ASSERT_EQUALS(U"lˈæŋɡwɪʤ", g_phonemizer.lookup(U"language", Tag("")));
  ASSERT_EQUALS(U"tˈɛst", g_phonemizer.lookup(U"test", Tag("")));

  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_stem_s_test)
{
  // Standard words with various -s/ -es/ -'s style suffixes
  ASSERT_EQUALS(U"dˈɔɡz", g_phonemizer.lookup_stem_s(U"dogs", Tag(""), std::nullopt));
  ASSERT_EQUALS(U"kˈæts", g_phonemizer.lookup_stem_s(U"cats", Tag(""), std::nullopt));
  ASSERT_EQUALS(U"wˈɪʃᵻz", g_phonemizer.lookup_stem_s(U"wishes", Tag(""), std::nullopt)); // -es
  ASSERT_EQUALS(U"ʤˈɑnz", g_phonemizer.lookup_stem_s(U"John's", Tag(""), std::nullopt)); // -'s
  ASSERT_EQUALS(U"pˈɑɹɾiz", g_phonemizer.lookup_stem_s(U"parties", Tag(""), std::nullopt)); // -ies tricky case
  ASSERT_EQUALS(U"fɹˈIz", g_phonemizer.lookup_stem_s(U"fries", Tag(""), std::nullopt));

  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_stem_ed_test)
{
  // Standard words with -ed style suffixes
  ASSERT_EQUALS(U"plˈAd", g_phonemizer.lookup_stem_ed(U"played", Tag(""), std::nullopt));
  ASSERT_EQUALS(U"wˈɑntᵻd", g_phonemizer.lookup_stem_ed(U"wanted", Tag(""), std::nullopt));
  ASSERT_EQUALS(U"lˈʊkt", g_phonemizer.lookup_stem_ed(U"looked", Tag(""), std::nullopt));
  ASSERT_EQUALS(U"fɹˈid", g_phonemizer.lookup_stem_ed(U"freed", Tag(""), std::nullopt)); // tricky -eed ending
  ASSERT_EQUALS(U"dˌɪsəɡɹˈid", g_phonemizer.lookup_stem_ed(U"disagreed", Tag(""), std::nullopt));

  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_stem_ing_test)
{
  // Standard words with -ing style suffixes
  ASSERT_EQUALS(U"plˈAɪŋ", g_phonemizer.lookup_stem_ing(U"playing", Tag(""), std::nullopt));
  ASSERT_EQUALS(U"dˈuɪŋ", g_phonemizer.lookup_stem_ing(U"doing", Tag(""), std::nullopt));
  ASSERT_EQUALS(U"mˌAkɪŋ", g_phonemizer.lookup_stem_ing(U"making", Tag(""), std::nullopt));
  ASSERT_EQUALS(U"ɹˈʌnɪŋ", g_phonemizer.lookup_stem_ing(U"running", Tag(""), std::nullopt)); // doubled consonant tricky case
  ASSERT_EQUALS(U"hˈɑpɪŋ", g_phonemizer.lookup_stem_ing(U"hoping", Tag(""), std::nullopt)); // stem with trailing 'e' removed
  ASSERT_EQUALS(U"tɹˈækɪŋ", g_phonemizer.lookup_stem_ing(U"tracking", Tag(""), std::nullopt)); // cking

  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_nnp_test)
{
  // Initialisms and acronyms
  ASSERT_EQUALS(U"dˌivˌidˈi", g_phonemizer.lookup_nnp(U"DVD"));
  ASSERT_EQUALS(U"jˌuˌɛsˈA", g_phonemizer.lookup_nnp(U"USA"));
  ASSERT_EQUALS(U"ˌɛfbˌiˈI", g_phonemizer.lookup_nnp(U"FBI"));
  ASSERT_EQUALS(U"jˌukˈA", g_phonemizer.lookup_nnp(U"U.K.")); // with dots

  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_special_words_test)
{
  // Special words based on lookup_special behavior
  ASSERT_EQUALS(U"vˈɜɹsəs", g_phonemizer.lookup_special(U"vs.", Tag(""), std::nullopt, std::nullopt));
  ASSERT_EQUALS(U"vˈɜɹsəs", g_phonemizer.lookup_special(U"vs", Tag(""), std::nullopt, std::nullopt));
  ASSERT_EQUALS(U"æm", g_phonemizer.lookup_special(U"am", Tag(""), std::nullopt, std::nullopt));
  ASSERT_EQUALS(U"ɐn", g_phonemizer.lookup_special(U"an", Tag(""), std::nullopt, std::nullopt));
  ASSERT_EQUALS(U"ˌI", g_phonemizer.lookup_special(U"I", Tag("PRP"), std::nullopt, std::nullopt));
  ASSERT_EQUALS(U"tu", g_phonemizer.lookup_special(U"to", Tag("TO"), std::nullopt, {}));
  ASSERT_EQUALS(U"ði", g_phonemizer.lookup_special(U"the", Tag("DT"), std::nullopt, {true}));
  ASSERT_EQUALS(U"jˈuzd", g_phonemizer.lookup_special(U"used", Tag("VBD"), std::nullopt, true)); // future_to true
  ASSERT_EQUALS(U"sˈɔɹs", g_phonemizer.lookup_special(U"src", Tag(""), std::nullopt, std::nullopt));

  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_phonemize_sentence_1_test)
{
  std::vector<Token> tokens = {
    {U"The", true, true, std::make_optional("DT")},
    {U"world", false, true, std::make_optional("NN")},
    {U"is", false, true, std::make_optional("VBZ")},
    {U"used", false, true, std::make_optional("VBN")},
    {U"to", false, true, std::make_optional("TO")},
    {U"be", false, true, std::make_optional("VB")},
    {U"better", false, false, std::make_optional("JJR")},
    {U".", false, false, std::make_optional(".")}
  };

  std::u32string result = g_phonemizer.phonemize(std::span<const Token>(tokens));
  ASSERT_EQUALS(U"ðə wˈɜɹld ɪz jˈuzd tə bi bˈɛɾəɹ.", result);
  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_phonemize_sentence_2_test)
{
  std::vector<Token> tokens = {
    {U"The", true, true, std::make_optional("DT")},
    {U"apple", false, true, std::make_optional("NN")},
    {U"is", false, true, std::make_optional("VBZ")},
    {U"supposed", false, true, std::make_optional("VBN")},
    {U"and", false, true, std::make_optional("CC")},
    {U"used", false, true, std::make_optional("VBN")},
    {U"to", false, true, std::make_optional("TO")},
    {U"be", false, true, std::make_optional("VB")},
    {U"eaten", false, false, std::make_optional("VBN")},
    {U"!", false, false, std::make_optional(".")}
  };

  std::u32string result = g_phonemizer.phonemize(std::span<const Token>(tokens));
  ASSERT_EQUALS(U"ði ˈæpᵊl ɪz səpˈOzd ænd jˈuzd tə bi ˈitn!", result);
  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_phonemize_sentence_3_test)
{
  std::vector<Token> tokens = {
    {U"I", true, true, std::make_optional("PRP")},
    {U"am", false, true, std::make_optional("VBP")},
    {U"an", false, true, std::make_optional("DT")},
    {U"artist", false, true, std::make_optional("NN")},
    {U"vs.", false, true, std::make_optional("IN")},
    {U"the", false, true, std::make_optional("DT")},
    {U"FBI", false, false, std::make_optional("NNP")},
    {U".", false, false, std::make_optional(".")}
  };
  std::u32string result = g_phonemizer.phonemize(std::span<const Token>(tokens));
  ASSERT_EQUALS(U"ˌI ɐm ɐn ˈɑɹɾɪst vˈɜɹsəs ðə ɡˈʌvəɹnmənt.", result);
  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_phonemize_sentence_4_test)
{
  std::vector<Token> tokens = {
    {U"The", true, true, std::make_optional("DT")},
    {U"dogs", false, true, std::make_optional("NNS")},
    {U"looked", false, true, std::make_optional("VBD")},
    {U"at", false, true, std::make_optional("IN")},
    {U"USA", false, true, std::make_optional("NNP")},
    {U"src", false, false, std::make_optional("NN")},
    {U"?", false, false, std::make_optional(".")}
  };
  std::u32string result = g_phonemizer.phonemize(std::span<const Token>(tokens));
  ASSERT_EQUALS(U"ðə dˈɔɡz lˈʊkt æt jˌuˌɛsˈA sˈɔɹs?", result);
  return true;
}

REGISTER_TEST(lexicon_phonemizer_en_phonemize_sentence_5_test)
{
  std::vector<Token> tokens = {
    {U"He", true, true, std::make_optional("PRP")},
    {U"is", false, true, std::make_optional("VBZ")},
    {U"playing", false, true, std::make_optional("VBG")},
    {U"with", false, true, std::make_optional("IN")},
    {U"the", false, true, std::make_optional("DT")},
    {U"DVD", false, true, std::make_optional("NNP")},
    {U"to", false, true, std::make_optional("TO")},
    {U"be", false, true, std::make_optional("VB")},
    {U"freed", false, false, std::make_optional("VBN")},
    {U"!", false, false, std::make_optional(".")}
  };
  std::u32string result = g_phonemizer.phonemize(std::span<const Token>(tokens));
  ASSERT_EQUALS(U"hˌi ɪz plˈAɪŋ wɪð ðə dˌivˌidˈi tə bi fɹˈid!", result);
  return true;
}

} // namespace phonemis::test