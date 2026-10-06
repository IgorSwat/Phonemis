#pragma once

#include <array>
#include <string>
#include <unordered_map>

// The Mandarin pipeline reproduces misaki's Kokoro v1.0 G2P (misaki.zh.ZHG2P without
// a frontend): cn2an number verbalization, punctuation mapping, jieba word segmentation
// and pypinyin readings, precomputed into the lexicon.
namespace phonemis::zh::constants {

// --- Character classes ---
namespace han {
  // Characters phonemized as Mandarin (CJK Unified Ideographs). Anything else passes
  // through unchanged.
  inline constexpr char32_t kFirst = U'一';
  inline constexpr char32_t kLast = U'鿿';

  // jieba only segments up to this character; later ones stand as words of their own.
  inline constexpr char32_t kLastSegmentable = U'鿕';

  constexpr bool is_han(char32_t c) { return c >= kFirst && c <= kLast; }
  constexpr bool is_segmentable(char32_t c) { return c >= kFirst && c <= kLastSegmentable; }
} // namespace han

// --- Number to Word Normalization (cn2an's an2cn transform) ---
namespace num2word {
  inline constexpr std::array<char32_t, 10> kDigits = {
    U'零', U'一', U'二', U'三', U'四', U'五', U'六', U'七', U'八', U'九'
  };

  // Unit of the digit at each power of ten, up to 10^15.
  inline const std::array<std::u32string, 16> kUnits = {
    U"", U"十", U"百", U"千", U"万", U"十", U"百", U"千",
    U"亿", U"十", U"百", U"千", U"万", U"十", U"百", U"千"
  };

  // Decimal digits read after the point; longer fractions are truncated.
  inline constexpr size_t kMaxDecimals = 16;

  inline constexpr char32_t kNegative = U'负';
  inline constexpr char32_t kPoint = U'点';
  inline const std::u32string kRangeTo = U"到";
  inline const std::u32string kFraction = U"分之";
  inline const std::u32string kPercent = U"百分之";
  inline const std::u32string kCelsius = U"摄氏度";

  // A range like "3-5" is only read as one when a measure word follows it.
  inline const std::array<std::u32string, 34> kMeasureWords = {
    U"斤", U"克", U"千克", U"公斤", U"吨", U"米", U"厘米", U"毫米", U"公里", U"升", U"毫升",
    U"元", U"角", U"分", U"个", U"只", U"条", U"张", U"块", U"瓶", U"杯", U"份", U"本",
    U"辆", U"台", U"匹", U"头", U"位", U"亩", U"小时", U"分钟", U"秒", U"天", U"半"
  };
} // namespace num2word

// --- Punctuation ---
namespace punctuation {
  // Full-width punctuation mapped onto the forms Kokoro reads.
  inline const std::unordered_map<char32_t, std::u32string> kReplacements = {
    {U'、', U", "}, {U'，', U", "}, {U'。', U". "}, {U'．', U". "},
    {U'！', U"! "}, {U'：', U": "}, {U'；', U"; "}, {U'？', U"? "},
    {U'«', U" “"},  {U'»', U"” "},  {U'《', U" “"}, {U'》', U"” "},
    {U'「', U" “"}, {U'」', U"” "}, {U'【', U" “"}, {U'】', U"” "},
    {U'（', U" ("}, {U'）', U") "}
  };
} // namespace punctuation

} // namespace phonemis::zh::constants
