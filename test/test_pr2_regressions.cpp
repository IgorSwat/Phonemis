#include <phonemis/pipeline.h>
#include <phonemis/phonemizer/lexicon.h>
#include <phonemis/utilities/string_utils.h>
#include <iostream>
#include <string>

using namespace phonemis;
using namespace phonemis::utilities;

static int pass_count = 0;
static int fail_count = 0;

static std::string to_utf8(const std::u32string& s) {
  return string_utils::u32string_to_utf8(s);
}

// Check that string contains expected substring.
static void assert_contains(const std::string& actual,
                            const std::string& expected,
                            const std::string& description) {
  if (actual.find(expected) != std::string::npos) {
    std::cout << "  PASS: " << description << "\n";
    pass_count++;
  } else {
    std::cout << "  FAIL: " << description << "\n";
    std::cout << "        expected to contain: " << expected << "\n";
    std::cout << "        got: " << actual << "\n";
    fail_count++;
  }
}

// Check that string does NOT contain unwanted substring.
static void assert_not_contains(const std::string& actual,
                                const std::string& unwanted,
                                const std::string& description) {
  if (actual.find(unwanted) == std::string::npos) {
    std::cout << "  PASS: " << description << "\n";
    pass_count++;
  } else {
    std::cout << "  FAIL: " << description << "\n";
    std::cout << "        expected NOT to contain: " << unwanted << "\n";
    std::cout << "        got: " << actual << "\n";
    fail_count++;
  }
}

// Check exact equality.
static void assert_equals(const std::string& actual,
                           const std::string& expected,
                           const std::string& description) {
  if (actual == expected) {
    std::cout << "  PASS: " << description << "\n";
    pass_count++;
  } else {
    std::cout << "  FAIL: " << description << "\n";
    std::cout << "        expected: " << expected << "\n";
    std::cout << "        got: " << actual << "\n";
    fail_count++;
  }
}

// Helper: phonemize a single word via Lexicon with an explicit POS tag.
static std::string lex_get(phonemizer::Lexicon& lex, const std::string& word,
                           const std::string& tag) {
  return to_utf8(lex.get(word, tagger::Tag{tag}));
}

// Helper: phonemize via full pipeline.
static std::string pipe(Pipeline& p, const std::string& input) {
  return to_utf8(p.process(input));
}

int main() {
  std::string TAGGER_DATA = "../data/hmm.json";
  std::string US_DICT = "../data/dictionaries/us_merged.json";
  std::string GB_DICT = "../data/dictionaries/gb_merged.json";

  Pipeline us_pipe(Lang::EN_US, TAGGER_DATA, US_DICT);
  Pipeline gb_pipe(Lang::EN_GB, TAGGER_DATA, GB_DICT);
  phonemizer::Lexicon us_lex(phonemizer::Lang::EN_US, US_DICT);
  phonemizer::Lexicon gb_lex(phonemizer::Lang::EN_GB, GB_DICT);

  // =========================================================================
  // 1. read VBP fix — present tense should be /riːd/, not /rɛd/
  // =========================================================================
  std::cout << "=== 1. read VBP fix ===\n";

  // Direct lexicon lookups with explicit POS tags
  assert_equals(lex_get(us_lex, "read", "VBP"), "ɹˈid",
                "US: read VBP = /ɹˈid/ (present tense)");
  assert_equals(lex_get(us_lex, "read", "VBD"), "ɹˈɛd",
                "US: read VBD = /ɹˈɛd/ (past tense)");
  assert_equals(lex_get(us_lex, "read", "VBN"), "ɹˈɛd",
                "US: read VBN = /ɹˈɛd/ (past participle)");
  assert_equals(lex_get(us_lex, "read", "ADJ"), "ɹˈɛd",
                "US: read ADJ = /ɹˈɛd/ (well-read)");

  // DEFAULT should be present tense
  assert_contains(lex_get(us_lex, "read", ""), "ɹˈid",
                  "US: read DEFAULT = /ɹˈid/");

  // GB was already correct — verify it still is
  assert_equals(lex_get(gb_lex, "read", "VBP"), "ɹˈiːd",
                "GB: read VBP = /ɹˈiːd/ (present tense)");
  assert_equals(lex_get(gb_lex, "read", "VBD"), "ɹˈɛd",
                "GB: read VBD = /ɹˈɛd/ (past tense)");

  // Pipeline test — tagger gives VB, parent VERB, falls to DEFAULT /ɹˈid/
  assert_contains(pipe(us_pipe, "I read books every day"), "ɹˈid",
                  "US pipeline: 'I read books every day' uses present /ɹˈid/");

  // =========================================================================
  // 2. GOAT reduction removed from fallback_dp()
  // =========================================================================
  std::cout << "\n=== 2. GOAT reduction removed from DP fallback ===\n";

  // These words go through fallback_dp — GOAT vowel must NOT be reduced
  assert_contains(pipe(us_pipe, "Rosemont"), "O",
                  "US: 'Rosemont' retains GOAT vowel in DP");
  assert_not_contains(pipe(us_pipe, "Rosemont"), "ˈə",
                      "US: 'Rosemont' GOAT not reduced to schwa");

  assert_contains(pipe(us_pipe, "Oakville"), "O",
                  "US: 'Oakville' retains GOAT vowel in DP");

  assert_contains(pipe(us_pipe, "Coltrane"), "O",
                  "US: 'Coltrane' retains GOAT vowel in DP");

  assert_not_contains(pipe(gb_pipe, "Rosemont"), "ˈə",
                      "GB: 'Rosemont' GOAT not reduced to schwa");

  // =========================================================================
  // 3. GOAT reduction still works at compound boundaries in fallback()
  // =========================================================================
  std::cout << "\n=== 3. GOAT reduction at compound boundaries (preserved) ===\n";

  // These go through fallback() compound splitting — GOAT SHOULD reduce
  assert_contains(pipe(us_pipe, "Holloway"), "ə",
                  "US: 'Holloway' reduces GOAT at compound boundary");
  assert_contains(pipe(us_pipe, "Galloway"), "ə",
                  "US: 'Galloway' reduces GOAT at compound boundary");
  assert_contains(pipe(us_pipe, "Calloway"), "ə",
                  "US: 'Calloway' reduces GOAT at compound boundary");
  assert_contains(pipe(gb_pipe, "Holloway"), "ə",
                  "GB: 'Holloway' reduces GOAT at compound boundary");

  // =========================================================================
  // 4. US homograph entries: lead, protest, export
  // =========================================================================
  std::cout << "\n=== 4. US homograph entries ===\n";

  // lead — direct lexicon lookups
  assert_equals(lex_get(us_lex, "lead", "NOUN"), "lˈɛd",
                "US: lead NOUN = /lˈɛd/ (metal)");
  assert_contains(lex_get(us_lex, "lead", ""), "lˈid",
                  "US: lead DEFAULT = /lˈid/ (verb form)");
  assert_contains(lex_get(us_lex, "lead", "VB"), "lˈid",
                  "US: lead VB falls to DEFAULT /lˈid/");

  // GB already had these — verify
  assert_equals(lex_get(gb_lex, "lead", "NOUN"), "lˈɛd",
                "GB: lead NOUN = /lˈɛd/ (metal)");
  assert_contains(lex_get(gb_lex, "lead", ""), "lˈiːd",
                  "GB: lead DEFAULT = /lˈiːd/");

  // protest — noun vs verb stress
  assert_contains(lex_get(us_lex, "protest", "VERB"), "pɹə",
                  "US: protest VERB has reduced first syllable /pɹə/");
  assert_contains(lex_get(us_lex, "protest", ""), "pɹˈO",
                  "US: protest DEFAULT has initial stress");

  // export — noun vs verb stress
  assert_contains(lex_get(us_lex, "export", "VERB"), "ɪks",
                  "US: export VERB has reduced first syllable /ɪks/");
  assert_contains(lex_get(us_lex, "export", ""), "ˈɛks",
                  "US: export DEFAULT has initial stress");

  // =========================================================================
  // 5. blessed/aged adjective forms (both dicts)
  // =========================================================================
  std::cout << "\n=== 5. -ed adjective homographs ===\n";

  // blessed — ADJ is 2-syllable /blˈɛsɪd/, DEFAULT is 1-syllable /blˈɛst/
  assert_contains(lex_get(us_lex, "blessed", "ADJ"), "ɪd",
                  "US: blessed ADJ is 2-syllable (contains /ɪd/)");
  assert_not_contains(lex_get(us_lex, "blessed", ""), "ɪd",
                      "US: blessed DEFAULT is 1-syllable (no /ɪd/)");
  assert_contains(lex_get(gb_lex, "blessed", "ADJ"), "ɪd",
                  "GB: blessed ADJ is 2-syllable (contains /ɪd/)");

  // aged — ADJ is 2-syllable /ˈAʤɪd/, DEFAULT is 1-syllable /ˈAʤd/
  assert_contains(lex_get(us_lex, "aged", "ADJ"), "ɪd",
                  "US: aged ADJ is 2-syllable (contains /ɪd/)");
  assert_not_contains(lex_get(us_lex, "aged", ""), "ɪd",
                      "US: aged DEFAULT is 1-syllable (no /ɪd/)");
  assert_contains(lex_get(gb_lex, "aged", "ADJ"), "ɪd",
                  "GB: aged ADJ is 2-syllable (contains /ɪd/)");

  // =========================================================================
  // 6. supposed-to handling
  // =========================================================================
  std::cout << "\n=== 6. supposed-to handling ===\n";

  // DEFAULT has voiced /z/; VBD without future_to also gets DEFAULT
  // because lookup_special intercepts all "supposed" lookups
  assert_contains(lex_get(us_lex, "supposed", ""), "Ozd",
                  "US: supposed DEFAULT ends in /zd/ (voiced)");
  assert_contains(lex_get(us_lex, "supposed", "VBD"), "Ozd",
                  "US: supposed VBD (no future_to) = DEFAULT /zd/");
  assert_contains(lex_get(gb_lex, "supposed", ""), "Qzd",
                  "GB: supposed DEFAULT ends in /zd/ (voiced)");
  assert_contains(lex_get(gb_lex, "supposed", "VBD"), "Qzd",
                  "GB: supposed VBD (no future_to) = DEFAULT /zd/");

  // lookup_special: supposed + VBD + future_to → VBD pronunciation
  auto supposed_special = to_utf8(
    us_lex.get("supposed", tagger::Tag{"VBD"}, std::nullopt, std::nullopt, true));
  assert_contains(supposed_special, "Ost",
                  "US: supposed VBD+future_to triggers special /st/ form");

  // supposed + JJ + future_to should also trigger (like "used to" pattern)
  auto supposed_jj = to_utf8(
    us_lex.get("supposed", tagger::Tag{"JJ"}, std::nullopt, std::nullopt, true));
  assert_contains(supposed_jj, "Ost",
                  "US: supposed JJ+future_to triggers special /st/ form");

  // supposed without future_to → DEFAULT
  auto supposed_no_to = to_utf8(
    us_lex.get("supposed", tagger::Tag{"VBD"}, std::nullopt, std::nullopt, false));
  assert_contains(supposed_no_to, "Ozd",
                  "US: supposed VBD without future_to uses DEFAULT /zd/");

  // =========================================================================
  // 7. aspirates typo fix
  // =========================================================================
  std::cout << "\n=== 7. aspirates typo fix ===\n";

  assert_contains(lex_get(us_lex, "aspirates", ""), "ts",
                  "US: aspirates DEFAULT contains /ts/ (not missing /t/)");
  assert_not_contains(lex_get(us_lex, "aspirates", ""), "ˌAs\n",
                      "US: aspirates DEFAULT does not end in just /s/");

  // =========================================================================
  // 8. foretoken label fix
  // =========================================================================
  std::cout << "\n=== 8. foretoken label fix ===\n";

  // DEFAULT should have initial stress, VERB should have final stress
  assert_contains(lex_get(us_lex, "foretoken", ""), "fˈɔɹ",
                  "US: foretoken DEFAULT has initial stress /fˈɔɹ/");
  assert_contains(lex_get(us_lex, "foretoken", "VERB"), "tˈO",
                  "US: foretoken VERB has stress on second syllable");

  // =========================================================================
  // 9. Existing behavior preserved — used-to still works
  // =========================================================================
  std::cout << "\n=== 9. used-to handling still works ===\n";

  auto used_special = to_utf8(
    us_lex.get("used", tagger::Tag{"VBD"}, std::nullopt, std::nullopt, true));
  assert_contains(used_special, "ust",
                  "US: used VBD+future_to still gives /st/ form");

  auto used_default = to_utf8(
    us_lex.get("used", tagger::Tag{"VBD"}, std::nullopt, std::nullopt, false));
  assert_contains(used_default, "uzd",
                  "US: used VBD without future_to gives DEFAULT /zd/");

  // =========================================================================
  // Summary
  // =========================================================================
  std::cout << "\n========================================\n";
  std::cout << pass_count << " passed, " << fail_count << " failed, "
            << (pass_count + fail_count) << " total\n";
  if (fail_count == 0)
    std::cout << "All PR #2 regression tests passed!\n";
  std::cout << "========================================\n";

  return fail_count > 0 ? 1 : 0;
}
