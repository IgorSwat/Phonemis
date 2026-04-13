#pragma once

#include "lexicon_phonemizer.h"
#include "nn/neural_phonemizer.h"
#include "phonemizer.h"

#include <concepts>
#include <memory>

namespace phonemis::phonemizer {

/**
 * Hybrid phonemizer utilizes both lexicon & neural phonemization to provide results
 * of highest possible quality in shortest possible time.
 * The pipeline is simple: if we can phonemize with lexicon (which is usually more reliable
 * and covers most special cases) - then we use the lexicon. We only use the neural model
 * for custom words, or in case of lack of good phonemization rules and/or lexicon.
 */
template <std::derived_from<LexiconPhonemizer> LXP, std::derived_from<nn::NeuralPhonemizer> NNP>
class HybridPhonemizer : public Phonemizer {
public:
  // Shortcut for base class calls and method visibility
  using Base = phonemizer::Phonemizer;
  using Base::phonemize;

  // Not passing filepath for either lexicon / neural phonemizer resource disables it.
  explicit HybridPhonemizer(const Config& config) {
    if (config.lexicon_filepath.has_value()) {
      lexicon_phonemizer_ = std::make_unique<LXP>(config);
    }
    if (config.nn_model_filepath.has_value()) {
      neural_phonemizer_ = std::make_unique<NNP>(config);
    }
  }
  
  /**
   * Uses lexicon as a primary phonemization method (if not disabled).
   * If lexicon and/or rule-based methods are not enough, uses neural phonemization as a fallback.
   */
  std::optional<std::u32string> phonemize(const Token& token) const override {
    auto lexicon_result = lexicon_phonemizer_ ? lexicon_phonemizer_->phonemize(token) : std::nullopt;

    if (lexicon_result.has_value()) {
      return lexicon_result.value();
    }

    // Neural phonemizer fallback (if available)
    return neural_phonemizer_ ? neural_phonemizer_->phonemize(token) : std::nullopt;
  }

  /**
   * Updates contexts of it's active subphonemizers.
   */
  void update_context(size_t curr_token_id, std::span<const Token> tokens) override {
    if (lexicon_phonemizer_) {
      lexicon_phonemizer_->update_context(curr_token_id, tokens);
    }
    if (neural_phonemizer_) {
      neural_phonemizer_->update_context(curr_token_id, tokens);
    }
  }
  
protected:
  // Phonemizer composition
  std::unique_ptr<LXP> lexicon_phonemizer_ = nullptr;
  std::unique_ptr<NNP> neural_phonemizer_ = nullptr;
};

} // namespace phonemis::phonemizer