#include "layer.h"
#include "../constants.h"
#include <phonemis/utils/unicode.h>
#include <phonemis/utils/conversions.h>

#include <algorithm>
#include <optional>
#include <stdexcept>

namespace phonemis::processor::num2word {

namespace {
// Checks if a character serves as a word boundary.
// In other words, it's either white space or special character.
bool is_boundary(char32_t c) {
  return !utils::unicode::isalnum(c) && c != U'_';
}

// Counts and returns the number of consecutive digits from the start index
size_t count_digits(std::u32string_view input, size_t start) {
  auto it = std::find_if(
      input.begin() + start, input.end(),
      [](char32_t c) { return !utils::unicode::isdigit(c); });
  return static_cast<size_t>(it - (input.begin() + start));
}

// Checks if a character is a known currency symbol.
bool is_currency(char32_t c) {
  return std::ranges::any_of(constants::kCurrencies, [c](char32_t sym) { return c == sym; });
}

// Rewrites a language-specific decimal separator to the canonical U'.' whenever
// it sits between two digits, copying everything else verbatim. The first and
// last characters can never be between two digits, so they bypass the scan.
std::u32string normalize_decimal_separator(std::u32string_view input, char32_t dec_sep) {
  const size_t n = input.size();
  std::u32string out;
  out.reserve(n);
  if (n > 0) {
    out.push_back(input[0]);
  }
  for (size_t i = 1; i + 1 < n; ++i) {
    if (input[i] == dec_sep &&
        utils::unicode::isdigit(input[i - 1]) && utils::unicode::isdigit(input[i + 1])) {
      out.push_back(U'.');
    } else {
      out.push_back(input[i]);
    }
  }
  if (n > 1) {
    out.push_back(input[n - 1]);
  }
  return out;
}

// True when `pos` marks the end of the current token: either the end of the
// string or a word-boundary character.
bool at_word_end(std::u32string_view s, size_t pos) {
  return pos == s.size() || is_boundary(s[pos]);
}

// True when a single currency symbol sits at `pos` and the token ends right
// after it (e.g. the '$' in "50$" or "3.50$").
bool is_trailing_currency(std::u32string_view s, size_t pos) {
  return pos < s.size() && is_currency(s[pos]) && at_word_end(s, pos + 1);
}
} // namespace

std::u32string Num2WordLayer::transform(std::u32string_view input) const {
  // Normalize this language's decimal separator to U'.' between digits, so the
  // scanning logic below can treat the dot as the canonical float/date separator.
  // For languages whose separator already is U'.' (e.g. English) this is a no-op
  // and we parse the original string in-place.
  char32_t dec_sep = decimal_separator();
  std::u32string owned;
  std::u32string_view work = input;
  if (dec_sep != U'.') {
    owned = normalize_decimal_separator(input, dec_sep);
    work = owned;
  }

  std::u32string result;
  result.reserve(work.size());

  size_t last_pos = 0;
  for (size_t i = 0; i < work.size(); ++i) {
    // Look for the start of a number, ensuring it's at a word boundary
    if (!utils::unicode::isdigit(work[i])) continue;
    if (i > 0 && !is_boundary(work[i - 1])) continue;

    auto [len, mode] = match_number_at(work, i);

    // Apply translation if a valid numeric chunk was found
    if (len > 0) {
      result.append(work.substr(last_pos, i - last_pos)); // Append preceding text
      result.append(verbalize({work.substr(i, len), mode})); // Append converted number
      last_pos = i + len;
      // Fast-forward the loop index to avoid re-processing the characters in the current numeric chunk.
      i += len - 1;
    }
  }

  result.append(work.substr(last_pos));
  return result;
}

Num2WordLayer::NumberMatch Num2WordLayer::match_number_at(std::u32string_view work, size_t start) const {
  size_t len = 0;
  Mode mode = Mode::CARDINAL;

  // Parse the first group of digits
  size_t p1 = count_digits(work, start);
  size_t curr = start + p1;

  // Check for compound numbers like dates, floats, fractions, or dot-ordinals
  if (curr < work.size() && (work[curr] == U'.' || work[curr] == U'-' || work[curr] == U'/')) {
    char32_t sep = work[curr];
    size_t p2 = count_digits(work, curr + 1);

    if (p2 > 0) {
      size_t next_curr = curr + 1 + p2;

      // Try to match a DATE pattern (e.g., DD.MM.YYYY, YYYY-MM-DD)
      if (next_curr < work.size() && work[next_curr] == sep && (sep == U'.' || sep == U'-')) {
        size_t p3 = count_digits(work, next_curr + 1);
        size_t date_end = next_curr + 1 + p3;
        // Ensure there is a third part, it ends at a boundary, and the parts
        // have date-plausible lengths.
        if (p3 > 0 && at_word_end(work, date_end) && p1 <= 4 && p2 <= 2 && p3 <= 4) {
          len = date_end - start;
          mode = Mode::DATE;
        }
      }

      // If it's not a date, evaluate for FLOAT, FRACTION, or trailing CURRENCY
      if (len == 0 && at_word_end(work, next_curr)) {
        len = next_curr - start;
        // start + len == next_curr, so the symbol (if any) sits at next_curr.
        if (sep != U'/' && is_trailing_currency(work, next_curr)) {
          len++;
          mode = Mode::CURRENCY;
        } else {
          mode = (sep == U'/') ? Mode::FRACTION : Mode::CARDINAL;
        }
      }
    } else if (sep == U'.') {
      // No second digit part found, might be an ORDINAL ending with a dot (e.g. '1.')
      len = p1 + 1;
      mode = Mode::ORDINAL;

      // Check the trailing context for ordinal notation validation (typically expects a lowercase word next)
      size_t next_pos = start + len;
      while (next_pos < work.size() && utils::unicode::isspace(work[next_pos])) {
        next_pos++;
      }
      if (!config_.allow_general_ord_notation || next_pos >= work.size() || !utils::unicode::islower(work[next_pos])) {
        // Revert to plain Cardinal if context does not match ordinal expectations
        mode = Mode::CARDINAL;
        len--; // Exclude the trailing dot
      }
    }
  }

  // If no compound pattern was successfully identified
  if (len == 0) {
    // Check for CURRENCY (number followed by single currency symbol)
    if (is_trailing_currency(work, curr)) {
      len = p1 + 1;
      mode = Mode::CURRENCY;
    } else {
      // Unicode-aware scan so language suffixes with non-ASCII letters
      // (e.g. French "ème", Spanish "º") are captured in full.
      size_t alpha = 0;
      while (curr + alpha < work.size() && utils::unicode::isalpha(work[curr + alpha])) {
        alpha++;
      }

      // Check for POTENTIALLY ORDINAL values (numbers directly followed by letters, e.g., '1st', '2nd')
      if (alpha > 0 && at_word_end(work, curr + alpha)) {
        len = p1 + alpha;
        mode = Mode::POTENTIALLY_ORDINAL;
      }
      // Otherwise, assume it's a standard integer CARDINAL
      else if (at_word_end(work, curr)) {
        len = p1;
        mode = Mode::CARDINAL;
      }
    }
  }

  return {len, mode};
}

std::u32string Num2WordLayer::verbalize(const StringifiedNumber& number) const {
  switch (number.conversionMode) {
    case Mode::CARDINAL: {
      if (number.text.find(U'.') != std::u32string_view::npos) {
        auto val = as_float(number.text);
        if (!val) return U"";
        return to_cardinal_float(*val, number.text);
      } else {
        auto val = as_int(number.text);
        if (!val) return U"";
        return to_cardinal_int(*val);
      }
    }
    case Mode::POTENTIALLY_ORDINAL: {
      size_t i = 0;
      while (i < number.text.size() && utils::unicode::isdigit(number.text[i])) i++;
      std::u32string_view num_part = number.text.substr(0, i);
      std::u32string_view suffix = number.text.substr(i);
      auto val = as_int(num_part);
      if (!val) return U"";
      if (is_ordinal_suffix(suffix)) {
        return to_ordinal_int(*val, suffix);
      } else {
        return to_ordinal_int(*val) + std::u32string(suffix);
      }
    }
    case Mode::ORDINAL: {
      std::u32string_view num_part = number.text.substr(0, number.text.size() - 1);
      auto val = as_int(num_part);
      if (!val) return U"";
      return to_ordinal_int(*val);
    }
    case Mode::FRACTION: {
      size_t pos = number.text.find(U'/');
      auto num = as_int(number.text.substr(0, pos));
      auto den = as_int(number.text.substr(pos + 1));
      if (!num || !den) return U"";
      return to_fraction(*num, *den);
    }
    case Mode::CURRENCY: {
      char32_t currency = number.text.back();
      std::u32string_view num_part = number.text.substr(0, number.text.size() - 1);
      if (num_part.find(U'.') != std::u32string_view::npos) {
        auto val = as_float(num_part);
        if (!val) return U"";
        return to_cardinal_float(*val, num_part) + U" " + to_currency(currency, *val);
      } else {
        auto val = as_int(num_part);
        if (!val) return U"";
        return to_cardinal_int(*val) + U" " + to_currency(currency, *val);
      }
    }
    case Mode::DATE: {
      char32_t sep = (number.text.find(U'.') != std::u32string_view::npos) ? U'.' : U'-';
      size_t first_sep = number.text.find(sep);
      size_t second_sep = number.text.find(sep, first_sep + 1);

      std::u32string_view p1 = number.text.substr(0, first_sep);
      std::u32string_view p2 = number.text.substr(first_sep + 1, second_sep - first_sep - 1);
      std::u32string_view p3 = number.text.substr(second_sep + 1);

      auto val1 = as_int(p1);
      auto val2 = as_int(p2);
      auto val3 = as_int(p3);

      if (!val1 || !val2 || !val3) return U"";

      // ISO format: YYYY-MM-DD
      if (p1.size() == 4) {
        return to_day(static_cast<uint32_t>(*val3)) + U" " + to_month(static_cast<uint32_t>(*val2)) + U" " + to_year(static_cast<uint32_t>(*val1));
      }
      // Common format: DD.MM.YYYY (or similar)
      return to_day(static_cast<uint32_t>(*val1)) + U" " + to_month(static_cast<uint32_t>(*val2)) + U" " + to_year(static_cast<uint32_t>(*val3));
    }
    case Mode::MONTH: {
      auto m = as_int(number.text);
      if (!m) return U"";
      return (*m >= 1 && *m <= 12) ? to_month(*m) : to_cardinal_int(*m);
    }
    case Mode::YEAR: {
      auto y = as_int(number.text);
      if (!y) return U"";
      return to_year(*y);
    }
    default:
      return std::u32string(number.text);
  }
}

std::optional<int32_t> Num2WordLayer::as_int(std::u32string_view s) const {
  // Returns nullopt (rather than throwing) on malformed or out-of-range input,
  // so a single unconvertible number degrades to "" instead of aborting the
  // whole phonemization. Callers rely on this via `if (!val) return U"";`.
  try {
    return std::stoi(utils::conversions::u32_to_utf8(s));
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

std::optional<float> Num2WordLayer::as_float(std::u32string_view s) const {
  try {
    return std::stof(utils::conversions::u32_to_utf8(s));
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

} // namespace phonemis::processor::num2word
