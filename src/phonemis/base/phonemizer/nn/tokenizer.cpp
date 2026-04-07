#include "constants.h"
#include "tokenizer.h"
#include <phonemis/utils/io.h>
#include <phonemis/utils/unicode.h>

#include <stdexcept>

namespace phonemis::phonemizer::nn {

Tokenizer::Tokenizer() : char_to_token_(constants::DEFAULT_CHAR_TO_TOKEN) {
  initialize_reverse_map();
}

Tokenizer::Tokenizer(const std::unordered_map<char32_t, int64_t>& external_mapping)
  : char_to_token_(external_mapping) {
  initialize_reverse_map();
}

Tokenizer::Tokenizer(const std::string& json_path) {
  nlohmann::json json_obj = utils::io::load_json(json_path);
  
  for (auto& item : json_obj.items()) {
    if (!item.value().is_number_integer()) {
      throw std::invalid_argument("JSON mapping must be string: int");
    }
    
    std::u32string u32_key = utils::conversions::utf8_to_u32(item.key());

    // Skip multi-character keys.
    if (u32_key.length() != 1) {
      continue;
    }
    char_to_token_[u32_key[0]] = item.value().get<int64_t>();
  }
  initialize_reverse_map();
}

std::vector<int64_t> Tokenizer::tokenize(std::u32string_view text) const {
  std::vector<int64_t> tokens;
  tokens.reserve(text.size());

  for (char32_t c : text) {
    auto it = char_to_token_.find(c);
    if (it != char_to_token_.end()) {
      tokens.push_back(it->second);
    }
  }

  return tokens;
}

std::u32string Tokenizer::decode(const std::vector<int64_t>& tokens) const {
  std::u32string text;
  text.reserve(tokens.size());

  for (int64_t t : tokens) {
    auto it = token_to_char_.find(t);
    if (it != token_to_char_.end()) {
      text.push_back(it->second);
    }
  }

  return text;
}

void Tokenizer::initialize_reverse_map() {
  token_to_char_.clear();
  
  for (const auto& [c, t] : char_to_token_) {
    token_to_char_[t] = c;
  }
}

} // namespace phonemis::phonemizer::nn
