#pragma once

#include "tokenizer.h"
#include "../phonemizer.h"

#include <vector>
#include <cstdint>

#ifdef ET_ON
#include <executorch/extension/module/module.h>
#include <memory>
#endif

namespace phonemis::phonemizer::nn {

class NeuralPhonemizer : public Phonemizer {
public:
  explicit NeuralPhonemizer(const std::string& model_path);

  NeuralPhonemizer(const std::string& model_path,
                   const std::unordered_map<char32_t, int64_t>& grapheme_mapping,
                   const std::unordered_map<char32_t, int64_t>& phone_mapping);

  NeuralPhonemizer(const std::string& model_path,
                   const std::string& grapheme_mapping_path,
                   const std::string& phone_mapping_path);

  std::optional<std::u32string> phonemize(const tokenizer::Token& token) const override;
  void update_context(std::span<const tokenizer::Token> tokens, size_t next_token_id) override {}

private:
  // Removes BLANK_TOKEN (id=0) from the token vector.
  std::vector<int64_t> remove_blanks(const std::vector<int64_t>& tokens) const;

  // Removes consecutive duplicate tokens from the token vector.
  std::vector<int64_t> remove_duplicates(const std::vector<int64_t>& tokens) const;

  // Tokenizers
  Tokenizer grapheme_tokenizer_;
  Tokenizer phone_tokenizer_;

#ifdef ET_ON
  std::unique_ptr<executorch::extension::module::Module> module_;
#endif
};

} // namespace phonemis::phonemizer::nn