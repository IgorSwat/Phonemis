#pragma once

#include "tokenizer.h"
#include "../phonemizer.h"
#include "../config.h"

#include <cstdint>
#include <memory>
#include <vector>

#ifdef ET_ON
#include <executorch/extension/module/module.h>
#else
#include <phonemis/protophone/protophone.h>
#endif

namespace phonemis::phonemizer::nn {

class NeuralPhonemizer : public Phonemizer {
public:
  explicit NeuralPhonemizer(const Config& config);

  std::optional<std::u32string> phonemize(const tokenizer::Token& token) const override;

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
#else
  std::unique_ptr<protophone::Protophone> module_;
#endif
};

} // namespace phonemis::phonemizer::nn