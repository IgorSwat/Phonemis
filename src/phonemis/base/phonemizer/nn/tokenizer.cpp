#include "constants.h"
#include "tokenizer.h"
#include <phonemis/utils/io.h>
#include <phonemis/utils/unicode.h>

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
    if (auto it = token_to_char_.find(t); it != token_to_char_.end()) {
      text.push_back(it->second);
    }
  }

  return text;
}

void Tokenizer::build_reverse_mapping() {
  token_to_char_.clear();
  token_to_char_.reserve(char_to_token_.size());
  
  for (const auto& [character, id] : char_to_token_) {
    token_to_char_[id] = character;
  }
}

} // namespace phonemis::phonemizer::nn
