#pragma once

#include "../../common/conditions.h"

#include <string>

namespace phonemis::processor::term2word {

// Abbreviations with their readings: nr -> numer before a number; St. -> Saint before a name, Street otherwise.
using Abbreviations = Readings;

// The words between the parts of a web address.
struct WebWords {
  std::u32string dot;   // example dot com
  std::u32string at;    // jan at example dot com
  std::u32string slash; // example dot com slash news
};

// The terms of a language: written forms which are not said as written.
struct Terms {
  Abbreviations abbreviations;
  WebWords web;
};

} // namespace phonemis::processor::term2word
