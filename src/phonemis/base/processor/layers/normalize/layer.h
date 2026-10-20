#pragma once

#include "../../layer.h"
#include "constants.h"
#include "types.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace phonemis::processor::normalize {

/**
 * Writes equivalent characters in one canonical form, before anything reads the text:
 * quotes as ", apostrophes as ', an ellipsis as ..., dashes as - or " - ", and no invisible characters.
 */
class NormalizeLayer : public Layer {
public:
  explicit NormalizeLayer(const Characters& characters = constants::kCharacters);

  void apply(Document& document) const override;

private:
  /**
   * A dash at a position: "-" inside a word or a number (1990–2000), and " - " for a run of dashes with
   * the spaces around it elsewhere (a – b), except between numbers, which stay a range (2020 – 2022 -> 2020-2022).
   * @returns its edit, or std::nullopt if no dash starts there ("-" alone is a hyphen).
   */
  std::optional<Document::Edit> read_dash(std::u32string_view text, size_t position) const;

  std::unordered_map<char32_t, std::u32string> canonical_; // Of each character of the classes: ’ -> '
};

} // namespace phonemis::processor::normalize
