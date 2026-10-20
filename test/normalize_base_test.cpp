#include "processing.h"
#include "test.h"
#include <phonemis/base/processor/layers/normalize/layer.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using processor::normalize::NormalizeLayer;

REGISTER_TEST(normalize_characters_test) {
  NormalizeLayer layer;

  // One form of each class
  ASSERT_EQUALS(U"\"Hello\", 'quoted' \"text\" and \"more\"", apply(layer, U"„Hello“, ‘quoted’ «text» and “more”"));
  ASSERT_EQUALS(U"It's the '90s...", apply(layer, U"It’s the ’90s…"));

  // Invisible characters are removed
  ASSERT_EQUALS(U"wordword", apply(layer, U"word​word﻿"));

  return true;
}

REGISTER_TEST(normalize_dashes_test) {
  NormalizeLayer layer;

  // Inside a word or a number
  ASSERT_EQUALS(U"Szczurek-Żelazko, 1990-2000, a-b", apply(layer, U"Szczurek–Żelazko, 1990—2000, a--b"));

  // Between words, a run of dashes with its spaces
  ASSERT_EQUALS(U"Warszawa - Kraków - Gdańsk", apply(layer, U"Warszawa – Kraków  –  — Gdańsk"));
  ASSERT_EQUALS(U"a - b", apply(layer, U"a ---- b"));

  // Between numbers, a range: 2020 – 2022
  ASSERT_EQUALS(U"2020-2022, but 5 - 3", apply(layer, U"2020 – 2022, but 5 - 3"));

  return true;
}

} // namespace phonemis::test
