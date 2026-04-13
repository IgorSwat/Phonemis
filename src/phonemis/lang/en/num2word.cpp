#include "num2word.h"
#include "constants.h"
#include <phonemis/base/preprocessor/constants.h>
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/strings.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>
#include <iterator>

namespace phonemis::en {

using namespace constants;
using namespace phonemis::preprocessor;
using namespace utils;

std::u32string Num2Word::to_cardinal_int(int32_t value) const {
  if (value < 0) {
    return U"minus " + to_cardinal_int(std::abs(value));
  }

  if (num2word::kCardinals.contains(static_cast<int>(value))) {
    return num2word::kCardinals.at(static_cast<int>(value));
  }

  if (value < 100) {
    int32_t tens = value / 10;
    int32_t units = value % 10;
    return num2word::kCardinals.at(tens * 10) + U" " + num2word::kCardinals.at(units);
  }

  // Handle large numbers by iterating through the large scales map (hundred, thousand, etc.)
  // in descending order to find the largest applicable base.
  for (auto it = num2word::kLargeCardinals.rbegin(); it != num2word::kLargeCardinals.rend(); ++it) {
    int64_t base = it->first;
    if (static_cast<int64_t>(value) >= base) {
      int32_t high = value / static_cast<int32_t>(base);
      int32_t low = value % static_cast<int32_t>(base);

      std::u32string res = to_cardinal_int(high) + U" " + it->second;
      if (low > 0) {
        // Use "and" for values below 100 as per English counting convention (e.g., "one hundred and five")
        std::u32string sep = (low < 100) ? U" and " : U", ";
        res += sep + to_cardinal_int(low);
      }

      return res;
    }
  }

  // Fallback - return the number without verbalization.
  return conversions::utf8_to_u32(std::to_string(value));
}

std::u32string Num2Word::to_cardinal_float(float value, std::u32string_view repr) const {
    if (value < 0.0f) {
        return U"minus " + to_cardinal_float(std::abs(value), repr);
    }

    // Find dot (brak point) and split the float into 2 parts: integer part and fractional part.
    // We will handle both parts separately and concatenate the results.
    std::string tmp_str = conversions::u32_to_utf8(repr);
    auto rit = std::find_if(tmp_str.rbegin(), tmp_str.rend(),
                            [](char c) -> bool { return c != '0'; });
    std::string value_str = std::string(tmp_str.begin(), rit.base());

    // Handle cases such as 5. - where float can be represented directly as an integer
    if (!value_str.empty() && value_str.back() == '.') {
      value_str.pop_back();
    }

    size_t dot_pos = value_str.find('.');
    if (dot_pos == std::string::npos) {
      return to_cardinal_int(std::stoi(value_str));
    }

    std::string integer_part_str = value_str.substr(0, dot_pos);
    std::string fractional_part_str = value_str.substr(dot_pos + 1);

    // Standard, cardinal verbalization of the integer part
    int32_t integer_part = integer_part_str.empty() ? 0 : std::stoi(integer_part_str);
    std::u32string res = to_cardinal_int(integer_part) + U" point";

    // Digit-by-digit verbalization of the fractional part.
    for (char c : fractional_part_str) {
      if (isdigit(c)) {
        res += U" " + to_cardinal_int(c - '0');
      }
    }

    return res;
}

std::u32string Num2Word::to_ordinal_int(int32_t value, std::u32string_view suffix) const {
  // A helper function to create a proper suffix based on verbalized representation.
  auto get_ordinal_suffix_word = [this](const std::u32string& word) -> std::u32string {
    return num2word::kOrdinals.contains(word) ? num2word::kOrdinals.at(word) :
           word.empty()                       ? U"th" :
           word.back() == U'y'                ? word.substr(0, word.length() - 1) + U"ieth" :
                                                word + U"th";
  };

  std::u32string card = to_cardinal_int(value);
  std::vector<std::u32string> words = strings::split(card, U' ');
  
  if (words.empty()) {
    return card;
  }

  std::u32string& last = words.back();
  size_t dash_pos = last.find(U'-');
  if (dash_pos != std::u32string::npos) {
    std::vector<std::u32string> parts = strings::split(last, U'-');
    if (!parts.empty()) {
      parts.back() = get_ordinal_suffix_word(parts.back());
      last = parts.size() > 1 ? parts[0] + U"-" + parts[1] : parts[0];
    }
  } else {
    last = get_ordinal_suffix_word(last);
  }

  std::u32string res = words[0];
  for (size_t i = 1; i < words.size(); ++i) {
    res += U" " + words[i];
  }
  return res;
}

std::u32string Num2Word::to_currency(char32_t currency, std::variant<int32_t, float> number) const {
  return num2word::kCurrencies.contains(currency) ?
         num2word::kCurrencies.at(currency) : std::u32string(1, currency);
}

std::u32string Num2Word::to_month(uint32_t month) const {
  if (month >= 1 && month <= 12) {
    return num2word::kMonths[month];
  }

  return U"";
}

std::u32string Num2Word::to_year(uint32_t year) const {
  int32_t value = static_cast<int32_t>(year);
  if (value == 0) return to_cardinal_int(0);

  int32_t high = value / 100;
  int32_t low = value % 100;

  if (high == 0 || (high % 10 == 0 && low < 10) || high >= 100) {
    return to_cardinal_int(value);
  }

  std::u32string high_text = to_cardinal_int(high);
  std::u32string low_text = low == 0 ? U"hundred" :
                            low < 10 ? U"oh-" + to_cardinal_int(low) :
                                       to_cardinal_int(low);

  return high_text + U" " + low_text;
}

bool Num2Word::is_ordinal_suffix(std::u32string_view suffix) const {
    return suffix == U"st" || suffix == U"nd" || suffix == U"rd" || suffix == U"th";
}

} // namespace phonemis::en
