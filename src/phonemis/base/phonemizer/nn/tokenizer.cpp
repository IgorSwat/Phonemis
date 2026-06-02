#include "constants.h"
#include "tokenizer.h"
#include <phonemis/utils/io.h>
#include <phonemis/utils/unicode.h>

#include <algorithm>
#include <stdexcept>

namespace phonemis::phonemizer::nn {

Tokenizer::Tokenizer() : char_to_token_(constants::DEFAULT_CHAR_TO_TOKEN) {
  build_reverse_mapping();
}

Tokenizer::Tokenizer(const std::unordered_map<char32_t, int64_t>& external_mapping)
  : char_to_token_(external_mapping) {
  build_reverse_mapping();
}

Tokenizer::Tokenizer(const std::string& json_path) {
  nlohmann::json json_obj = utils::io::load_json(json_path);
  
  for (const auto& [key, value] : json_obj.items()) {
    if (!value.is_number_integer()) {
      throw std::invalid_argument("Tokenizer JSON error: value for '" + key + "' must be an integer ID.");
    }
    
    // Convert UTF-8 JSON key to UTF-32 to find the Unicode codepoint
    std::u32string u32_key = utils::conversions::utf8_to_u32(key);

    // We only support mapping single characters to single IDs.
    // Multi-character strings in the JSON (like escape sequences) are rejected.
    if (u32_key.length() == 1) {
      char_to_token_[u32_key[0]] = value.get<int64_t>();
    }
  }
  build_reverse_mapping();
}

std::vector<int64_t> Tokenizer::tokenize(std::u32string_view text) const {
  std::vector<int64_t> tokens;
  tokens.reserve(text.size());

  for (char32_t c : text) {
    if (auto it = char_to_token_.find(c); it != char_to_token_.end()) {
      tokens.push_back(it->second);
    }
    // NOTE: Characters not found in the mapping are skipped.
    // This makes the tokenizer robust but potentially silent about filtered symbols.
  }

  return tokens;
}

std::u32string Tokenizer::decode(const std::vector<int64_t>& tokens) const {
  std::u32string text;
  text.reserve(tokens.size());

  for (int64_t t : tokens) {
    // token_to_char_ is indexed directly by ID (dense [0, N) range), so this is
    // a bounds-checked array access rather than a hash lookup. U'\0' marks a
    // slot with no mapped character and is skipped, matching the previous
    // "missing key" behaviour.
    if (t >= 0 && t < static_cast<int64_t>(token_to_char_.size())) {
      char32_t c = token_to_char_[t];
      if (c != U'\0') {
        text.push_back(c);
      }
    }
  }

  return text;
}

void Tokenizer::build_reverse_mapping() {
  // Token IDs form a dense, contiguous range, so we store the reverse map as a
  // vector indexed by ID. Size it to hold the largest ID and fill unused slots
  // with U'\0' ("no character"). assign() reuses the existing buffer on repeat
  // calls instead of reallocating from scratch.
  int64_t max_id = -1;
  for (const auto& [character, id] : char_to_token_) {
    max_id = std::max(max_id, id);
  }

  token_to_char_.assign(static_cast<size_t>(max_id + 1), U'\0');
  for (const auto& [character, id] : char_to_token_) {
    token_to_char_[static_cast<size_t>(id)] = character;
  }
}

} // namespace phonemis::phonemizer::nn
