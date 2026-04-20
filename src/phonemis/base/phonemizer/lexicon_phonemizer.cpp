
#include "lexicon_phonemizer.h"
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/io.h>
#include <phonemis/utils/strings.h>

#include <stdexcept>

namespace phonemis::phonemizer {

using namespace utils;

LexiconPhonemizer::LexiconPhonemizer(const Config& config) {
	if (!config.lexicon_filepath.has_value()) {
		throw std::runtime_error("LexiconPhonemizer: lexicon_filepath must be provided in the configuration.");
	}

	auto json_obj = utils::io::load_json(config.lexicon_filepath.value());

	// We assume the lexicon has a strict string -> string structure.
	for (auto& item : json_obj.items()) {
		std::string key = item.key(); // `word` or `word|context`
		auto value = item.value();

		if (!value.is_string()) {
			throw std::runtime_error("Lexicon phonemizer expects a string-to-string JSON structure.");
		}

		dict_[key] = value.get<std::string>();
	}
}

std::optional<std::u32string> LexiconPhonemizer::phonemize(const Token& token) const {
  if (token.text.empty()) {
    return U"";
  }

  std::string utf8_text = conversions::u32_to_utf8(token.text);

  // Standard pipeline - if exact case match fails to find a phonemization,
  // always try to match the lowercase to make it more robust.
  auto result = lookup(utf8_text);
  if (result.empty()) {
    std::u32string lower_text = strings::to_lower(token.text);
    std::string lower_utf8 = conversions::u32_to_utf8(lower_text);
    result = lookup(lower_utf8);
  }

  if (result.empty()) {
    return std::nullopt;
  }
  
  return result;
}

void LexiconPhonemizer::update_context(size_t /*curr_token_id*/, std::span<const Token> /*tokens*/) {
}

std::u32string LexiconPhonemizer::lookup(const std::string& word, std::string_view context) const {
	std::string key;
  key.reserve(word.size() + 1 + context.size());

  key.append(word);
  if (!context.empty()) {
      key.push_back('|');
      key.append(context);
  }

  auto it = dict_.find(key);
  if (it == dict_.end())
    return U"";

  return conversions::utf8_to_u32(it->second);
}

std::u32string LexiconPhonemizer::lookup(const std::u32string& word, std::string_view context) const {
  return lookup(conversions::u32_to_utf8(word), context);
}

bool LexiconPhonemizer::is_known(const std::string& word) const {
  return dict_.contains(word);
}

bool LexiconPhonemizer::is_known(const std::u32string& word, bool try_lowercase) const {
  return is_known(conversions::u32_to_utf8(word)) ||
         (try_lowercase && is_known(conversions::u32_to_utf8(strings::to_lower(word))));
}

} // namespace phonemis::phonemizer
