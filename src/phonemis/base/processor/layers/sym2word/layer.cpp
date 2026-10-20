#include "../../common/constants.h"
#include "../../common/text.h"
#include "layer.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

namespace phonemis::processor::sym2word {

using namespace utils;

Sym2WordLayer::Sym2WordLayer(Symbols symbols) : symbols_(std::move(symbols)) {
  for (const auto& [symbol, readings] : symbols_) {
    keys_.push_back(symbol);
  }
}

void Sym2WordLayer::apply(Document& document) const {
  std::u32string_view text = document.text();

  for (size_t start = 0; start < text.size();) {
    // 1. A run of symbols, with the whitespaces around it: " & ", "++"
    size_t symbol = start;
    while (symbol < text.size() && unicode::isspace(text[symbol])) {
      symbol++;
    }
    std::vector<std::u32string_view> symbols;
    size_t end = symbol;
    for (auto keys = keys_at(text, end, keys_); !keys.empty(); keys = keys_at(text, end, keys_)) {
      symbols.push_back(keys.front());
      end += keys.front().size();
    }
    if (symbols.empty()) {
      start = symbol > start ? symbol : start + 1;
      continue;
    }
    while (end < text.size() && unicode::isspace(text[end])) {
      end++;
    }

    // 2. The words of each symbol in the context of the run
    Context context(text, {start, end});
    std::vector<std::u32string> words;
    for (auto s : symbols) {
      auto reading = read(symbols_.at(std::u32string(s)), context);
      if (std::u32string word = reading.value_or(std::u32string(s)); !word.empty()) {
        words.push_back(word);
      }
    }

    // 3. The words, separated from the text around by single spaces (none before punctuation).
    // A run right after another one already has its space: "™ &" -> "trademark and"
    bool left = start > 0 && !unicode::isspace(text[start - 1]);
    std::u32string replacement;
    if (words.empty()) {
      replacement = left && end < text.size() ? U" " : U"";
    } else {
      bool right = end < text.size() && !strings::contains(constants::kPunctuation, text[end]);
      replacement = (left ? U" " : U"") + join(words) + (right ? U" " : U"");
    }
    document.replace({start, end}, replacement);
    start = end;
  }
}

} // namespace phonemis::processor::sym2word
