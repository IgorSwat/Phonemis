#pragma once

#include <cstdint>
#include <unordered_map>

namespace phonemis::hi::constants {

// Character-to-token mapping for the Hindi neural phonemizer.
// Generated from characters.json.
// Note: multi-codepoint entries (nukta combinations 177-184) are omitted
// because the tokenizer only maps single Unicode codepoints.
inline const std::unordered_map<char32_t, int64_t> kCharToToken = {
    {U'\'',      1},
    {U'a',       2},
    {U'b',       3},
    {U'c',       4},
    {U'd',       5},
    {U'e',       6},
    {U'f',       7},
    {U'g',       8},
    {U'h',       9},
    {U'i',      10},
    {U'j',      11},
    {U'k',      12},
    {U'l',      13},
    {U'm',      14},
    {U'n',      15},
    {U'o',      16},
    {U'p',      17},
    {U'q',      18},
    {U'r',      19},
    {U's',      20},
    {U't',      21},
    {U'u',      22},
    {U'v',      23},
    {U'w',      24},
    {U'x',      25},
    {U'y',      26},
    {U'z',      27},
    {U'ª', 28},  // ª  FEMININE ORDINAL INDICATOR
    {U'µ', 29},  // µ  MICRO SIGN
    {U'º', 30},  // º  MASCULINE ORDINAL INDICATOR
    {U'½', 31},  // ½  VULGAR FRACTION ONE HALF
    {U'ß', 32},  // ß  LATIN SMALL LETTER SHARP S
    {U'à', 33},  // à
    {U'á', 34},  // á
    {U'â', 35},  // â
    {U'ã', 36},  // ã
    {U'ä', 37},  // ä
    {U'æ', 38},  // æ
    {U'ç', 39},  // ç
    {U'è', 40},  // è
    {U'é', 41},  // é
    {U'ê', 42},  // ê
    {U'ë', 43},  // ë
    {U'ì', 44},  // ì
    {U'í', 45},  // í
    {U'î', 46},  // î
    {U'ï', 47},  // ï
    {U'ñ', 48},  // ñ
    {U'ò', 49},  // ò
    {U'ó', 50},  // ó
    {U'ô', 51},  // ô
    {U'ö', 52},  // ö
    {U'ø', 53},  // ø
    {U'ú', 54},  // ú
    {U'ü', 55},  // ü
    {U'þ', 56},  // þ
    {U'ÿ', 57},  // ÿ
    {U'ā', 58},  // ā
    {U'ć', 59},  // ć
    {U'č', 60},  // č
    {U'ē', 61},  // ē
    {U'ğ', 62},  // ğ
    {U'ī', 63},  // ī
    {U'ı', 64},  // ı  LATIN SMALL LETTER DOTLESS I
    {U'ł', 65},  // ł
    {U'ń', 66},  // ń
    {U'ō', 67},  // ō
    {U'œ', 68},  // œ
    {U'š', 69},  // š
    {U'ū', 70},  // ū
    {U'ž', 71},  // ž
    {U'ƒ', 72},  // ƒ
    {U'ơ', 73},  // ơ
    {U'ư', 74},  // ư
    {U'ǐ', 75},  // ǐ
    {U'ँ', 99},  // ँ  DEVANAGARI SIGN CHANDRABINDU
    {U'ं', 100}, // ं  DEVANAGARI SIGN ANUSVARA
    {U'ः', 101}, // ः  DEVANAGARI SIGN VISARGA
    {U'अ', 102}, // अ
    {U'आ', 103}, // आ
    {U'इ', 104}, // इ
    {U'ई', 105}, // ई
    {U'उ', 106}, // उ
    {U'ऊ', 107}, // ऊ
    {U'ऋ', 108}, // ऋ
    {U'ऍ', 109}, // ऍ
    {U'ऎ', 110}, // ऎ
    {U'ए', 111}, // ए
    {U'ऐ', 112}, // ऐ
    {U'ऑ', 113}, // ऑ
    {U'ऒ', 114}, // ऒ
    {U'ओ', 115}, // ओ
    {U'औ', 116}, // औ
    {U'क', 117}, // क
    {U'ख', 118}, // ख
    {U'ग', 119}, // ग
    {U'घ', 120}, // घ
    {U'ङ', 121}, // ङ
    {U'च', 122}, // च
    {U'छ', 123}, // छ
    {U'ज', 124}, // ज
    {U'झ', 125}, // झ
    {U'ञ', 126}, // ञ
    {U'ट', 127}, // ट
    {U'ठ', 128}, // ठ
    {U'ड', 129}, // ड
    {U'ढ', 130}, // ढ
    {U'ण', 131}, // ण
    {U'त', 132}, // त
    {U'थ', 133}, // थ
    {U'द', 134}, // द
    {U'ध', 135}, // ध
    {U'न', 136}, // न
    {U'ऩ', 137}, // ऩ
    {U'प', 138}, // प
    {U'फ', 139}, // फ
    {U'ब', 140}, // ब
    {U'भ', 141}, // भ
    {U'म', 142}, // म
    {U'य', 143}, // य
    {U'र', 144}, // र
    {U'ऱ', 145}, // ऱ
    {U'ल', 146}, // ल
    {U'ळ', 147}, // ळ
    {U'ऴ', 148}, // ऴ
    {U'व', 149}, // व
    {U'श', 150}, // श
    {U'ष', 151}, // ष
    {U'स', 152}, // स
    {U'ह', 153}, // ह
    {U'ऺ', 154}, // ऺ  DEVANAGARI VOWEL SIGN OE
    {U'़', 155}, // ़  DEVANAGARI SIGN NUKTA
    {U'ऽ', 156}, // ऽ  DEVANAGARI SIGN AVAGRAHA
    {U'ा', 157}, // ा
    {U'ि', 158}, // ि
    {U'ी', 159}, // ी
    {U'ु', 160}, // ु
    {U'ू', 161}, // ू
    {U'ृ', 162}, // ृ
    {U'ॄ', 163}, // ॄ
    {U'ॅ', 164}, // ॅ
    {U'ॆ', 165}, // ॆ
    {U'े', 166}, // े
    {U'ै', 167}, // ै
    {U'ॉ', 168}, // ॉ
    {U'ॊ', 169}, // ॊ
    {U'ो', 170}, // ो
    {U'ौ', 171}, // ौ
    {U'्', 172}, // ्  DEVANAGARI SIGN VIRAMA
    {U'ॐ', 173}, // ॐ  DEVANAGARI OM
    {U'॑', 174}, // ॑  DEVANAGARI STRESS SIGN UDATTA
    {U'॒', 175}, // ॒  DEVANAGARI STRESS SIGN ANUDATTA
    {U'॓', 176}, // ॓  DEVANAGARI GRAVE ACCENT
    // Entries 177-184 (nukta combinations) are multi-codepoint and skipped.
    {U'ॠ', 185}, // ॠ
    {U'ॢ', 186}, // ॢ
    {U'॰', 189}, // ॰  DEVANAGARI ABBREVIATION SIGN
    {U'ॱ', 190}, // ॱ
    {U'ॲ', 191}, // ॲ
    {U'ॿ', 193}, // ॿ
};

} // namespace phonemis::hi::constants
