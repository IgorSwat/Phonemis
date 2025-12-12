#include <phonemis/preprocessor/num2word.h>
#include <phonemis/preprocessor/constants.h>
#include <algorithm>
#include <cmath>
#include <exception>
#include <iterator>
#include <sstream>
#include <vector>

namespace phonemis::preprocessor::num2words {

namespace {
// Helper function - split string by delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;

  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter)) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }

  return tokens;
}

// Helper function - join string vector with delimiter
std::string join(const std::vector<std::string>& vec, const std::string& delimiter) {
  if (vec.empty()) return "";

  std::ostringstream oss;
  std::copy(vec.begin(), vec.end() - 1, 
            std::ostream_iterator<std::string>(oss, delimiter.c_str()));
  oss << vec.back();
  
  return oss.str();
}

// Helper function - get sorted keys for large cardinals
const std::vector<int64_t>& get_sorted_large_keys() {
  // Optimize by creating and sorting array only once
  static std::vector<int64_t> keys;
  if (keys.empty()) {
    for (const auto& pair : constants::kLargeCardinals) {
      keys.push_back(pair.first);
    }
    std::sort(keys.rbegin(), keys.rend());
  }

  return keys;
}

// Helper function - get ordinal suffix word
std::string get_ordinal_suffix_word(const std::string& word) {
  if (constants::kOrdinals.count(word)) {
    return constants::kOrdinals.at(word);
  }
  if (!word.empty() && word.back() == 'y') {
    return word.substr(0, word.length() - 1) + "ieth";
  }
  if (!word.empty() && word.back() == 'e') {
    return word + "th";
  }
  return word + "th";
}
} // namespace

// Integer to cardinal string conversion
std::string to_cardinal_int(long long value) {
  if (value < 0) {
    return "minus " + to_cardinal_int(std::abs(value));
  }

  // Direct lookup
  if (constants::kCardinals.count(static_cast<int>(value))) {
    return constants::kCardinals.at(static_cast<int>(value));
  }

  // < 100
  if (value < 100) {
    long long tens = value / 10;
    long long units = value % 10;
    return constants::kCardinals.at(static_cast<int>(tens * 10)) + "-" + 
            constants::kCardinals.at(static_cast<int>(units));
  }

  // < 1000
  if (value < 1000) {
    long long hundreds = value / 100;
    long long rest = value % 100;
    std::string res = constants::kCardinals.at(static_cast<int>(hundreds)) + " hundred";
    if (rest > 0) {
      res += " and " + to_cardinal_int(rest);
    }
    return res;
  }

  // Large numbers
  const auto& large_keys = get_sorted_large_keys();
  for (long long base : large_keys) {
    if (value >= base) {
      long long high = value / base;
      long long low = value % base;
      std::string res = to_cardinal_int(high) + " " + constants::kLargeCardinals.at(base);
      if (low > 0) {
        std::string sep = (low < 100) ? " and " : ", ";
        res += sep + to_cardinal_int(low);
      }
      return res;
    }
  }

  return std::to_string(value);
}

// Float to cardinal string conversion
std::string to_cardinal_float(double value) {
  if (value < 0.0) {
    return "minus " + to_cardinal_float(std::abs(value));
  }

  // Convert to a string representation and eliminate the trailing zeros
  std::string tmp_str = std::to_string(value);
  auto rit = std::find_if(tmp_str.rbegin(), tmp_str.rend(), 
                          [](char c) -> bool { return c != '0'; });
  std::string value_str = std::string(tmp_str.rend().base(), rit.base());

  // Integer fallback - in case user passed an integer
  size_t dot_pos = value_str.find('.');
  if (dot_pos == std::string::npos) {
    return to_cardinal_int(std::stoll(value_str));
  }

  std::string integer_part_str = value_str.substr(0, dot_pos);
  std::string fractional_part_str = value_str.substr(dot_pos + 1);

  long long integer_part = 0;
  if (!integer_part_str.empty()) {
    integer_part = std::stoll(integer_part_str);
  }

  std::string res = to_cardinal_int(integer_part) + " point";
  for (char c : fractional_part_str) {
    if (isdigit(c)) {
      res += " " + to_cardinal_int(c - '0');
    }
  }
  return res;
}

// Integer to ordinal string conversion
std::string to_ordinal(long long value) {
  std::string card = to_cardinal_int(value);
  std::vector<std::string> words = split(card, ' ');
  
  if (words.empty()) return card;

  std::string& last = words.back();
  if (last.find('-') != std::string::npos) {
    std::vector<std::string> parts = split(last, '-');
    if (!parts.empty()) {
      parts.back() = get_ordinal_suffix_word(parts.back());
      last = join(parts, "-");
    }
  } else {
    last = get_ordinal_suffix_word(last);
  }

  return join(words, " ");
}

// Integer to year string conversion
std::string to_year(long long value) {
  if (value < 0) 
    return to_year(std::abs(value)) + " BC";

  long long high = value / 100;
  long long low = value % 100;

  if (high == 0 || (high % 10 == 0 && low < 10) || high >= 100) {
      return to_cardinal_int(value);
  }

  std::string high_text = to_cardinal_int(high);
  std::string low_text;
  if (low == 0) {
    low_text = "hundred";
  } else if (low < 10) {
    low_text = "oh-" + to_cardinal_int(low);
  } else {
    low_text = to_cardinal_int(low);
  }

  return high_text + " " + low_text;
}

} // namespace phonemis::preprocessing::num2words
