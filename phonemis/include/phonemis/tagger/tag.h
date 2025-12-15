#pragma once

#include <string>

namespace phonemis::tagger {

// Tag class definition
// An abstraction layer which wrapps a simple string-based tag definition
// with some additional logic.
class Tag : public std::string {
public:
  // Inherit constructors and assignment from std::string
  using std::string::string;
  using std::string::operator=;
  Tag(std::string const& s) : std::string(s) {}
  Tag(std::string&& s) : std::string(std::move(s)) {}

  // Extra logic
  Tag parent_tag() const {
    auto this_tag = static_cast<const std::string&>(*this);
    if (this_tag == "VERB" || substr(0, 2) == "VB")
      return {"VERB"};
    if (this_tag == "NOUN" || substr(0, 2) == "NN")
      return {"NOUN"};
    if (substr(0, 3) == "ADV" || substr(0, 2) == "RB")
      return {"ADV"};
    if (substr(0, 3) == "ADJ" || substr(0, 2) == "JJ")
      return {"ADJ"};
    return (*this);
  }
};

} // namespace phonemis::tagger

// Hash definition
// Required to use Tag objects as map keys.
namespace std {
template<>
struct hash<phonemis::tagger::Tag> {
  size_t operator()(phonemis::tagger::Tag const& t) const noexcept {
    // Use std::string's hash implementation
    return std::hash<std::string>()(static_cast<std::string const&>(t));
  }
};
} // namespace std