#pragma once

#include "../../layer.h"
#include "types.h"

#include <optional>
#include <string>
#include <string_view>

namespace phonemis::processor::term2word {

/**
 * Handles non-numeric and non-symbolic terms.
 * 
 * Example: web addresses (www.example.com/news, jan@example.com), abbreviations (nr 5, e.g.).
 */
class Term2WordLayer : public Layer {
public:
  explicit Term2WordLayer(Terms terms);

  void apply(Document& document) const override;

  // A term found in a text, with its words
  struct Term {
    Span span;
    std::u32string words;
  };

private:
  // https://www.example.com/news, jan.kowalski@example.pl: its parts joined with the words of the separators
  std::optional<Term> read_address(std::u32string_view text, size_t position) const;

  // The longest abbreviation at a position with a reading in its context
  std::optional<Term> read_abbreviation(std::u32string_view text, size_t position) const;

  Terms terms_;
  Keys keys_; // Of the abbreviations
};

} // namespace phonemis::processor::term2word
