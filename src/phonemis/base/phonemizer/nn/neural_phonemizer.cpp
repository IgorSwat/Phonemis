#include "constants.h"
#include "neural_phonemizer.h"
#include <phonemis/base/tokenizer/token.h>
#include <phonemis/utils/strings.h>

#include <algorithm>
#include <iterator>

#ifdef ET_ON
#include <executorch/extension/module/module.h>
#include <executorch/extension/tensor/tensor.h>
#include <executorch/runtime/core/evalue.h>

using executorch::extension::module::Module;
using executorch::extension::make_tensor_ptr;
#else
#include <phonemis/protophone/protophone.h>

using phonemis::protophone::Protophone;
#endif

namespace phonemis::phonemizer::nn {

NeuralPhonemizer::NeuralPhonemizer(const Config& config)
  : grapheme_tokenizer_(config.nn_grapheme_mapping ? Tokenizer(*config.nn_grapheme_mapping) : Tokenizer(constants::DEFAULT_CHAR_TO_TOKEN)),
    phone_tokenizer_(config.nn_phone_mapping ? Tokenizer(*config.nn_phone_mapping) : Tokenizer(constants::DEFAULT_PHONEME_TO_TOKEN)) {
  if (!config.nn_model_filepath.has_value()) {
    throw std::runtime_error("NeuralPhonemizer: nn_model_filepath must be provided in the configuration.");
  }

#ifdef ET_ON
  module_ = std::make_unique<Module>(config.nn_model_filepath.value(), Module::LoadMode::MmapUseMlockIgnoreErrors);
#else
  module_ = std::make_unique<Protophone>(config.nn_model_filepath.value());
#endif
}

std::optional<std::u32string> NeuralPhonemizer::phonemize(const tokenizer::Token& token) const {
  std::u32string_view text = token.text;

  if (text.empty()) {
    return std::make_optional(U"");
  }

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
  std::vector<int64_t> input_tokens = grapheme_tokenizer_.tokenize(utils::strings::to_lower(text));

  // If there are no valid input tokens, do not even proceed with the phonemization.
  if (input_tokens.empty() || std::ranges::all_of(input_tokens, [](int64_t t) { return t == constants::PAD_TOKEN; })) {
    return std::nullopt;
  }

  // Step 3: Infer the model
  #ifdef ET_ON
  const std::vector<int32_t> input_shape = {1, static_cast<int32_t>(input_tokens.size())};
  auto input_tensor = make_tensor_ptr(
      input_shape,
      input_tokens.data(),
      executorch::aten::ScalarType::Long
  );

  auto result = module_->forward(input_tensor);
  if (!result.ok()) {
    return std::nullopt;
  }

  // The expected shape of the output tensor is [1, 2 * seq_len, no_classes].
  const auto& logits = result->at(0).toTensor();
  const float* logits_data = logits.const_data_ptr<float>();
  size_t no_steps = logits.sizes()[1], no_classes = logits.sizes()[2];
  #else
  auto result = module_->forward(input_tokens);

  const auto& logits = result;
  const float* logits_data = logits.data().data();
  size_t no_steps = logits.shape()[0], no_classes = logits.shape()[1];
  #endif

  // Perform an argmax over the class dimension to obtain exact phonemes.
  std::vector<int64_t> output_tokens(no_steps);
  for (size_t i = 0; i < no_steps; ++i) {
    const float* row = logits_data + i * no_classes;
    output_tokens[i] = static_cast<int64_t>(std::max_element(row, row + no_classes) - row);
  }

  // Step 4: CTC-specific postprocessing.
  output_tokens = remove_duplicates(output_tokens);
  output_tokens = remove_blanks(output_tokens);

  return std::make_optional(phone_tokenizer_.decode(output_tokens));
}

std::vector<int64_t> NeuralPhonemizer::remove_blanks(const std::vector<int64_t>& tokens) const {
  std::vector<int64_t> result;
  result.reserve(tokens.size());

  std::ranges::copy_if(tokens, std::back_inserter(result),
               [](int64_t token) { return token != constants::BLANK_TOKEN; });

  return result;
}

std::vector<int64_t> NeuralPhonemizer::remove_duplicates(const std::vector<int64_t>& tokens) const {
  std::vector<int64_t> result;
  result.reserve(tokens.size());

  std::ranges::unique_copy(tokens, std::back_inserter(result));

  return result;
}

} // namespace phonemis::phonemizer::nn