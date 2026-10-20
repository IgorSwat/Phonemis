#include "../../common/constants.h"
#include "../../common/text.h"
#include "layer.h"
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

#include <vector>

namespace phonemis::processor::term2word {

using namespace utils;

namespace {
// Whether a host label is a top-level domain, which makes a bare host an address: example.com
bool is_domain(std::u32string_view label) {
  static const std::unordered_set<std::u32string> domains = {U"com", U"org", U"net",  U"edu", U"gov", U"io", U"pl",
                                                             U"de",  U"fr",  U"es",   U"it",  U"pt",  U"uk", U"eu",
                                                             U"at",  U"ch",  U"info", U"tv",  U"co",  U"br"};
  return domains.contains(strings::to_lower(label));
}

// A character of the user name of an email: jan.kowalski+news
bool is_user(char32_t c) {
  return unicode::isword(c) || strings::contains(std::u32string_view(U".-+%"), c);
}

// A character of a host label: my-site
bool is_label(char32_t c) {
  return unicode::isword(c) || c == U'-';
}

// A part of an address with its dots read: jan.kowalski -> jan dot kowalski
std::u32string dotted(std::u32string_view part, const std::u32string& dot) {
  std::u32string words;
  for (char32_t c : part) {
    words += c == U'.' ? U" " + dot + U" " : std::u32string(1, c);
  }
  return words;
}
} // namespace

Term2WordLayer::Term2WordLayer(Terms terms) : terms_(std::move(terms)) {
  for (const auto& [abbreviation, readings] : terms_.abbreviations) {
    keys_.push_back(abbreviation);
  }
}

void Term2WordLayer::apply(Document& document) const {
  std::u32string_view text = document.text();

  for (size_t position = 0; position < text.size();) {
    // Terms start at word boundaries, but not after a dot: e.g. is not g.
    std::optional<Term> term;
    if (is_clear_before(text, position, U".")) {
      term = read_address(text, position);
      if (!term) {
        term = read_abbreviation(text, position);
      }
    }

    if (!term) {
      position++;
      continue;
    }
    document.replace(term->span, std::move(term->words));
    position = term->span.end;
  }
}

std::optional<Term2WordLayer::Term> Term2WordLayer::read_address(std::u32string_view text, size_t position) const {
  const WebWords& web = terms_.web;

  // 1. An optional scheme, which is not read: https://
  size_t host = position;
  while (host < text.size() && unicode::is_latin(text[host])) {
    host++;
  }
  bool scheme = host > position && text.substr(host).starts_with(U"://");
  host = scheme ? host + 3 : position;

  // 2. The user of an email: jan.kowalski@
  std::u32string_view user;
  if (size_t at = host; !scheme) {
    while (at < text.size() && is_user(text[at])) {
      at++;
    }
    if (at > host && at < text.size() && text[at] == U'@') {
      user = text.substr(host, at - host);
      host = at + 1;
    }
  }

  // 3. The host: labels joined by dots
  std::vector<std::u32string_view> labels;
  size_t end = host;
  while (true) {
    size_t label_end = end;
    while (label_end < text.size() && is_label(text[label_end])) {
      label_end++;
    }
    if (label_end == end) {
      break;
    }
    labels.push_back(text.substr(end, label_end - end));
    end = label_end;
    if (end + 1 >= text.size() || text[end] != U'.' || !is_label(text[end + 1])) {
      break;
    }
    end++;
  }

  // A bare host needs a known top-level domain: example.com, but not file.txt or e.g
  bool www = !labels.empty() && strings::to_lower(labels[0]) == U"www";
  bool domain = labels.size() >= 2 && is_domain(labels.back());
  if (labels.size() < 2 || !(scheme || www || domain || !user.empty())) {
    return std::nullopt;
  }

  // 4. An optional path, without the punctuation closing it: (see example.com/news).
  size_t path_end = end;
  if (end < text.size() && text[end] == U'/') {
    while (path_end < text.size() && !unicode::isspace(text[path_end])) {
      path_end++;
    }
    while (path_end > end + 1 && strings::contains(constants::kPunctuation, text[path_end - 1])) {
      path_end--;
    }
  }

  // 5. The words: jan at example dot com, w w w dot example dot com slash news
  std::u32string words = user.empty() ? U"" : dotted(user, web.dot) + U" " + web.at + U" ";
  for (size_t i = 0; i < labels.size(); ++i) {
    bool spelled = i == 0 && www;
    words += (i > 0 ? U" " + web.dot + U" " : U"") + (spelled ? U"w w w" : std::u32string(labels[i]));
  }
  for (auto path = text.substr(end, path_end - end); !path.empty();) {
    size_t next = path.find(U'/', 1);
    std::u32string_view segment = path.substr(1, next == std::u32string_view::npos ? next : next - 1);
    words += U" " + web.slash + (segment.empty() ? U"" : U" " + dotted(segment, web.dot));
    path = next == std::u32string_view::npos ? U"" : path.substr(next);
  }
  return Term{.span = {position, path_end}, .words = words};
}

std::optional<Term2WordLayer::Term> Term2WordLayer::read_abbreviation(std::u32string_view text, size_t position) const {
  for (auto key : keys_at(text, position, keys_)) {
    // An abbreviation ending with a letter ends a word, or touches a number: nr5
    size_t end = position + key.size();
    if (unicode::isword(key.back()) && !is_clear_after(text, end) && !unicode::isdigit(text[end])) {
      continue;
    }

    auto words = read(terms_.abbreviations.at(std::u32string(key)), Context(text, {position, end}));
    if (!words) {
      continue;
    }

    // A dot which also ends the text stays with it; a word right after gets a space: nr5 -> numer 5
    if (key.ends_with(U'.') && is_sentence_end(text, end)) {
      *words += U".";
    }
    if (end < text.size() && unicode::isword(text[end])) {
      *words += U" ";
    }
    return Term{.span = {position, end}, .words = *words};
  }
  return std::nullopt;
}

} // namespace phonemis::processor::term2word
