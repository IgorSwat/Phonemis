#include "constants.h"
#include "neural_phonemizer.h"
#include <phonemis/base/tokenizer/token.h>

#include <algorithm>
#include <iterator>

#ifdef ET_ON
#include <executorch/extension/module/module.h>
#include <executorch/extension/tensor/tensor.h>
#include <executorch/runtime/core/evalue.h>

using executorch::extension::module::Module;
using executorch::extension::make_tensor_ptr;
#endif

namespace phonemis::phonemizer::nn {

NeuralPhonemizer::NeuralPhonemizer(const std::string& model_path)
#ifdef ET_ON
    : module_(std::make_unique<Module>(model_path, Module::LoadMode::MmapUseMlockIgnoreErrors))
#endif
{}

NeuralPhonemizer::NeuralPhonemizer(const std::string& model_path,
                                   const std::unordered_map<char32_t, int64_t>& grapheme_mapping,
                                   const std::unordered_map<char32_t, int64_t>& phone_mapping)
    : grapheme_tokenizer_(grapheme_mapping)
    , phone_tokenizer_(phone_mapping)
#ifdef ET_ON
    , module_(std::make_unique<Module>(model_path, Module::LoadMode::MmapUseMlockIgnoreErrors))
#endif
{}

NeuralPhonemizer::NeuralPhonemizer(const std::string& model_path,
                                   const std::string& grapheme_mapping_path,
                                   const std::string& phone_mapping_path)
    : grapheme_tokenizer_(grapheme_mapping_path)
    , phone_tokenizer_(phone_mapping_path)
#ifdef ET_ON
    , module_(std::make_unique<Module>(model_path, Module::LoadMode::MmapUseMlockIgnoreErrors))
#endif
{}

std::optional<std::u32string> NeuralPhonemizer::phonemize(const tokenizer::Token& token) const {
#ifdef ET_ON
  std::u32string_view text = token.text;

  // Step 1: Check if text length exceeds MAX_SEQ_LEN
  // It is possible to apply a little bit more sophisticated split heuristic here,
  // but since words longer than 20 characters are extremaly rare (and mostly just incorrect)
  // it should be enough.
  if (text.length() > constants::MAX_SEQ_LEN) {
    // Process first 20 characters and the rest recursively
    tokenizer::Token first_part_token = token;
    first_part_token.text = std::u32string(text.substr(0, 20));

    tokenizer::Token rest_part_token = token;
    rest_part_token.text = std::u32string(text.substr(20));

    auto first_res = phonemize(first_part_token);
    auto rest_res = phonemize(rest_part_token);

    if (first_res && rest_res) {
      return *first_res + *rest_res;
    }

    return std::nullopt;
  }

  // Step 2: Tokenize the text if length is within limits
  std::vector<int64_t> input_tokens = grapheme_tokenizer_.tokenize(text);

  // Step 3: Infer the model
  const std::vector<int32_t> input_shape = {static_cast<int32_t>(input_tokens.size())};
  auto input_tensor = make_tensor_ptr(
      executorch::aten::ScalarType::Long,
      input_shape,
      input_tokens.data());

  auto result = module_->forward(input_tensor);
  if (!result.ok()) {
    return std::nullopt;
  }

  // The output is a 2D tensor of shape [2*seq_len, num_classes].
  // Apply argmax over the last dimension to get token ids.
  const auto& output_evalue = result->at(0);
  const auto& logits = output_evalue.toTensor();

  const int64_t* logits_data = logits.const_data_ptr<int64_t>();
  const auto sizes = logits.sizes();
  const int64_t num_steps = sizes[0];
  const int64_t num_classes = sizes[1];

  std::vector<int64_t> output_tokens(num_steps);
  for (int64_t i = 0; i < num_steps; ++i) {
    const int64_t* row = logits_data + i * num_classes;
    output_tokens[i] = static_cast<int64_t>(
        std::max_element(row, row + num_classes) - row);
  }

  // Step 4: Postprocessing
  output_tokens = remove_duplicates(output_tokens);
  output_tokens = remove_blanks(output_tokens);

  return std::make_optional(phone_tokenizer_.decode(output_tokens));
#else
  return std::nullopt;
#endif
}

std::vector<int64_t> NeuralPhonemizer::remove_blanks(const std::vector<int64_t>& tokens) const {
  std::vector<int64_t> result;
  result.reserve(tokens.size());

  std::copy_if(tokens.begin(), tokens.end(), std::back_inserter(result),
               [](int64_t token) { return token != constants::BLANK_TOKEN; });

  return result;
}

std::vector<int64_t> NeuralPhonemizer::remove_duplicates(const std::vector<int64_t>& tokens) const {
  std::vector<int64_t> result;
  result.reserve(tokens.size());

  std::unique_copy(tokens.begin(), tokens.end(), std::back_inserter(result));

  return result;
}

} // namespace phonemis::phonemizer::nn