#include "constants.h"
#include "num2word.h"
#include <phonemis/base/processor/common/constants.h>
#include <phonemis/base/processor/layers/num2word/numerals.h>
#include <phonemis/base/processor/layers/num2word/patterns.h>
#include <phonemis/base/processor/layers/num2word/primitives.h>
#include <phonemis/utils/strings.h>
#include <phonemis/utils/unicode.h>

#include <algorithm>
#include <optional>
#include <unordered_set>

namespace phonemis::pl {

using namespace constants::num2word;
using namespace processor;
using namespace processor::num2word;
using namespace utils;

namespace {
size_t index(Case c) {
  return static_cast<size_t>(c);
}

uint64_t value(const Number& number) {
  return value_of(number.integer).value_or(0);
}

bool starts_with_any(std::u32string_view word, const std::vector<std::u32string>& stems) {
  return std::ranges::any_of(stems, [word](const auto& stem) { return word.starts_with(stem); });
}

// ------------------------
// ----- Number words -----
// ------------------------

// The column of kCardinals: nominative and accusative, genitive and locative, dative, instrumental
size_t column(Case c) {
  switch (c) {
    case Case::NOMINATIVE:
    case Case::ACCUSATIVE:
      return 0;
    case Case::GENITIVE:
    case Case::LOCATIVE:
      return 1;
    case Case::DATIVE:
      return 2;
    default:
      return 3;
  }
}

// The form of a noun after a number: 1 / 2-4 / 5+ in the nominative, the plural of the case otherwise
const std::u32string& form(const Noun& noun, uint64_t n, Case c, bool fraction = false) {
  if (fraction) {
    return noun.singular[index(Case::GENITIVE)];
  }
  if (n == 1) {
    return noun.singular[index(c)];
  }
  if (c == Case::NOMINATIVE || c == Case::ACCUSATIVE) {
    bool few = n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 12 || n % 100 > 14);
    return noun.plural[index(few ? Case::NOMINATIVE : Case::GENITIVE)];
  }
  return noun.plural[index(c)];
}

std::u32string below_thousand(uint64_t n, Case c, Gender g) {
  if (n == 0) {
    return std::u32string(kCardinals.at(0)[column(c)]);
  }

  uint64_t r = n % 100;
  std::vector<std::u32string> words;
  for (uint64_t part : {n - r, r < 20 ? r : r - r % 10, r < 20 ? 0 : r % 10}) {
    if (part == 1) {
      words.push_back(U"jeden"); // Inside a compound number "jeden" does not inflect
    } else if (part == 2 && g == Gender::FEMININE &&
               (c == Case::NOMINATIVE || c == Case::ACCUSATIVE || c == Case::INSTRUMENTAL)) {
      words.push_back(c == Case::INSTRUMENTAL ? U"dwiema" : U"dwie");
    } else if (part) {
      words.emplace_back(kCardinals.at(part)[column(c)]);
    }
  }
  return join(words);
}

std::u32string cardinal(uint64_t n, Case c = Case::NOMINATIVE, Gender g = Gender::MASCULINE) {
  if (n == 1) {
    return std::u32string(kOne[static_cast<size_t>(g)][index(c)]);
  }

  std::vector<std::u32string> parts;
  for (auto [scale, noun] : {std::pair{1000000000ULL, &kBillion}, {1000000ULL, &kMillion}, {1000ULL, &kThousand}}) {
    uint64_t q = n / scale;
    n %= scale;
    if (q) {
      parts.push_back(q == 1 ? form(*noun, q, c) : cardinal(q, c) + U" " + form(*noun, q, c));
    }
  }
  if (n || parts.empty()) {
    parts.push_back(below_thousand(n, c, g));
  }
  return join(parts);
}

// An ordinal stem with an adjective ending: drug + y -> drugi, trzec + a -> trzecia
std::u32string adjective(std::u32string_view stem, std::u32string_view ending) {
  std::u32string e(ending);
  if (stem.ends_with(U'g') || stem.ends_with(U'k')) {
    e = e[0] == U'y' ? U"i" + e.substr(1) : e[0] == U'e' ? U"i" + e : e;
  } else if (stem.ends_with(U'c')) {
    e = e[0] == U'y' ? U"i" + e.substr(1) : U"i" + e;
  }
  return std::u32string(stem) + e;
}

// The form of a number inside a compound: czterdziestopięcio(metrowe), stu(letni)
std::u32string combining(uint64_t n) {
  uint64_t h = n - n % 100, r = n % 100;
  if (h >= 1000) {
    return cardinal(n);
  }
  std::u32string words(h ? kCombining.at(h) : U"");
  if (r) {
    words += r < 20 || r % 10 == 0 ? std::u32string(kCombining.at(r)) :
                                     std::u32string(kCombining.at(r - r % 10)) + std::u32string(kCombining.at(r % 10));
  }
  return words;
}

// Only the last two parts inflect: dwa tysiące dwudziestym drugim, tysiąc osiemsetnym
std::u32string ordinal(uint64_t n, Case c = Case::NOMINATIVE, Gender g = Gender::MASCULINE, bool plural = false) {
  std::u32string_view e = kAdjectiveEndings[plural ? 3 : static_cast<size_t>(g)][index(c)];

  uint64_t r = n % 100;
  if (r) {
    std::vector<std::u32string> words = {n - r ? cardinal(n - r) : U""};
    if (r < 20 || r % 10 == 0) {
      words.push_back(adjective(kOrdinalStems.at(r), e));
    } else {
      words.push_back(adjective(kOrdinalStems.at(r - r % 10), e));
      words.push_back(adjective(kOrdinalStems.at(r % 10), e));
    }
    return join(words);
  }

  uint64_t h = n % 1000;
  if (h) {
    std::vector<std::u32string> words = {n - h ? cardinal(n - h) : U"", adjective(kOrdinalStems.at(h), e)};
    return join(words);
  }

  uint64_t t = n / 1000;
  return adjective((t == 1 ? U"" : combining(t)) + U"tysięczn", e);
}

std::u32string digits(std::u32string_view text) {
  std::vector<std::u32string> words;
  for (char32_t c : text) {
    words.emplace_back(c == U'1' ? U"jeden" : kCardinals.at(c - U'0')[0]);
  }
  return join(words);
}

// -------------------
// ----- Context -----
// -------------------

// Inflection of a number: its form, case and gender
struct Features {
  Form form = Form::CARDINAL;
  Case grammatical_case = Case::NOMINATIVE;
  Gender gender = Gender::MASCULINE;
};

// A noun's case, gender and number, guessed from its ending (as seen after an ordinal): sesji, sesję, roku
struct NounForm {
  Case grammatical_case = Case::NOMINATIVE;
  Gender gender = Gender::MASCULINE;
  bool plural = false;
};

// Whether a word is made only of given characters (and is not empty).
bool made_of(std::u32string_view word, std::u32string_view characters) {
  return !word.empty() && std::ranges::all_of(word, [&](char32_t c) { return strings::contains(characters, c); });
}

constexpr std::u32string_view kDigits = U"0123456789";

const std::unordered_set<std::u32string>& genitive_plurals() {
  static const std::unordered_set<std::u32string> forms = [] {
    std::unordered_set<std::u32string> result;
    for (const auto* table : {&kUnits, &kCurrencies, &kScales}) {
      for (const auto& [key, noun] : *table) {
        result.insert(noun.plural[index(Case::GENITIVE)]);
      }
    }
    return result;
  }();
  return forms;
}

// A noun that a number counts: osób, metrów, złotych (as opposed to a verb, or nothing after a year)
bool counted(std::u32string_view word) {
  std::u32string w(word);
  return kCountNouns.contains(w) || genitive_plurals().contains(w) || word.ends_with(U"ów") || word.ends_with(U"ych") ||
         word.ends_with(U"ich");
}

/**
 * The words before a number within its clause (lowercased), and the first word after it, skipping a range
 * or coordination it belongs to (20-30, 3 a 368) and the złote of an amount of grosze (39 532 złotych 20 groszy).
 */
std::pair<std::vector<std::u32string>, std::u32string> surroundings(const Context& context) {
  std::u32string_view head = context.head();
  size_t clause = 0;
  for (size_t i = 0; i + 1 < head.size(); ++i) {
    if (strings::contains(std::u32string_view(U",;:.!?"), head[i]) && unicode::isspace(head[i + 1])) {
      clause = i + 1;
    }
  }

  std::vector<std::u32string> before;
  for (auto word : words_of(head.substr(clause))) {
    before.push_back(strings::to_lower(stripped(word)));
  }
  auto range_start = [](std::u32string_view w) {
    return w.size() > 1 && unicode::isdigit(w[0]) && w.back() == U'-' &&
           made_of(w.substr(0, w.size() - 1), U"0123456789,. ");
  };
  while (!before.empty() && range_start(before.back())) {
    before.pop_back();
  }
  while (before.size() >= 2 && kConjunctions.contains(before.back()) &&
         made_of(before[before.size() - 2], U"0123456789,.")) {
    before.resize(before.size() - 2);
  }
  if (!before.empty() && kCurrencyWords.contains(before.back())) {
    before.pop_back();
    while (!before.empty() && made_of(before.back(), kDigits)) {
      before.pop_back();
    }
  }

  // The first word after, beyond the end of a range (-30)
  std::u32string next;
  std::u32string_view tail = context.tail();
  if (tail.empty() || !strings::contains(processor::constants::kPunctuation, tail[0])) {
    for (auto word : words_of(tail)) {
      std::u32string_view w = stripped(word);
      if (!(w.size() > 1 && w[0] == U'-' && made_of(w.substr(1), U"0123456789,."))) {
        next = strings::to_lower(w);
        break;
      }
    }
  }
  return {before, next};
}

// The context of the number this one continues after a comma (140 g mąki, 130 g mąki): both share a case.
std::optional<Context> enumerated(const Context& context) {
  std::u32string_view head = context.head();
  size_t end = head.find_last_not_of(U" \t\n\r");
  if (end == std::u32string_view::npos || head[end] != U',') {
    return std::nullopt;
  }

  // The last number before the comma, at most four words before it
  std::u32string_view text = head.substr(0, end);
  std::optional<Span> number;
  for (size_t i = 0; i < text.size();) {
    if (unicode::isdigit(text[i]) && is_clear_before(text, i, U".,")) {
      number = Span{i, digits_end(text, i)};
      i = number->end;
    } else {
      i++;
    }
  }
  if (!number || words_of(text.substr(number->end)).size() > 4) {
    return std::nullopt;
  }
  return Context(text, *number);
}

// The case of a number governed by an amount noun, a genitive verb or a negation right before it.
std::optional<Case> governed(const std::vector<std::u32string>& before) {
  std::u32string w1 = before.empty() ? U"" : before.back();
  for (const auto& stem : kAmountNouns) {
    if (w1.starts_with(stem) && kNounEndings.contains(std::u32string(w1.substr(stem.size())))) {
      return Case::GENITIVE; // od kwoty trzystu tysięcy
    }
  }

  std::vector<std::u32string> window(before.end() - std::min<size_t>(before.size(), 3), before.end());
  std::vector<std::u32string> verbs;
  std::ranges::copy_if(window, std::back_inserter(verbs), [](const auto& w) { return !kClitics.contains(w); });
  if (!verbs.empty() && starts_with_any(verbs.back(), kGenitiveVerbs)) {
    return Case::GENITIVE; // potrzebowali stu czterdziestu gramów, brakuje nam trzech osób
  }

  // nie powinno przekraczać pięciu gramów, but not: nie jest to pięć, nie więcej niż pięć
  auto no = std::ranges::find(window, U"nie");
  if (no != window.end() && std::none_of(no + 1, window.end(), [](const auto& w) {
        return kCopulas.contains(w) || w == U"niż" || w == U"tylko";
      })) {
    return Case::GENITIVE;
  }
  return std::nullopt;
}

NounForm noun_form(std::u32string_view word, std::u32string_view preposition) {
  bool locative = kLocativePrepositions.contains(std::u32string(preposition));
  bool instrumental = preposition == U"z" || preposition == U"ze" || preposition == U"przed" || preposition == U"nad" ||
                      preposition == U"pod";

  const std::vector<std::pair<std::u32string_view, NounForm>> endings = {
      {U"ami", {Case::INSTRUMENTAL, Gender::MASCULINE, true}},
      {U"ach", {Case::LOCATIVE, Gender::MASCULINE, true}},
      {U"ych", {Case::GENITIVE, Gender::MASCULINE, true}},
      {U"ów", {Case::GENITIVE, Gender::MASCULINE, true}},
      {U"iem", {Case::INSTRUMENTAL, Gender::MASCULINE}},
      {U"em", {Case::INSTRUMENTAL, Gender::MASCULINE}},
      {U"ą", {instrumental ? Case::INSTRUMENTAL : Case::ACCUSATIVE, Gender::FEMININE}},
      {U"ę", {Case::ACCUSATIVE, Gender::FEMININE}},
      {U"a", {Case::NOMINATIVE, Gender::FEMININE}},
      {U"i", {locative ? Case::LOCATIVE : Case::GENITIVE, Gender::FEMININE}},
      {U"y", {locative ? Case::LOCATIVE : Case::GENITIVE, Gender::FEMININE}},
      {U"u", {locative ? Case::LOCATIVE : Case::GENITIVE, Gender::MASCULINE}},
      {U"ie", {Case::LOCATIVE, Gender::MASCULINE}},
      {U"e", {Case::NOMINATIVE, Gender::NEUTER}},
      {U"o", {Case::NOMINATIVE, Gender::NEUTER}}};
  for (const auto& [ending, form] : endings) {
    if (word.ends_with(ending)) {
      return form;
    }
  }
  return {};
}

// The form, case and gender of a number from its context: an ordinal for years, days and headings.
Features features(const Context& context, uint64_t n, bool decimal, bool year_abbreviation = false) {
  auto [before, next] = surroundings(context);
  std::u32string w1 = before.empty() ? U"" : before.back();
  std::u32string w2 = before.size() < 2 ? U"" : before[before.size() - 2];
  std::u32string noun = year_abbreviation ? U"roku" : next; // 2020 r.

  // 1. Years, days and headings are ordinals
  bool locative = kLocativePrepositions.contains(w1);
  if (noun == U"roku" || noun == U"rok" || noun == U"rokiem") {
    Case c = noun == U"rok"    ? Case::NOMINATIVE :
             noun == U"rokiem" ? Case::INSTRUMENTAL :
             locative          ? Case::LOCATIVE :
                                 Case::GENITIVE;
    return {Form::ORDINAL, c};
  }
  if (w1 == U"dnia") {
    return {Form::ORDINAL, Case::GENITIVE}; // z dnia 1.
  }
  if (w1 == U"roku" || w1 == U"rok") {
    bool w2_locative = kLocativePrepositions.contains(w2);
    return {Form::ORDINAL, w1 == U"rok" ? Case::NOMINATIVE : w2_locative ? Case::LOCATIVE : Case::GENITIVE};
  }
  if (std::ranges::find(kMonthsGenitive, noun) != kMonthsGenitive.end() && n >= 1 && n <= 31) {
    return {Form::ORDINAL, kCopulas.contains(w1) ? Case::NOMINATIVE : Case::GENITIVE}; // od pierwszego kwietnia
  }
  if (kMonths.contains(w1) && n >= 1000 && n <= 2100) {
    return {Form::ORDINAL, Case::GENITIVE};
  }
  if ((w1 == U"w" || w1 == U"we") && n >= 1000 && n <= 2100 && !genitive_plurals().contains(noun)) {
    return {Form::ORDINAL, Case::LOCATIVE};
  }
  if (auto it = kYearPrepositions.find(w1); it != kYearPrepositions.end() && n >= 1900 && n <= 2100 && !counted(noun)) {
    return {Form::ORDINAL, it->second}; // od dwa tysiące dwudziestego do dwa tysiące dwudziestego drugiego
  }
  if (starts_with_any(w1, kOrdinalHeadings) && n < 100) {
    return {Form::ORDINAL, noun_form(w1, w2).grammatical_case}; // w punkcie drugim
  }

  // 2. The case of a cardinal: from a preposition, a governing word or a number it continues
  Features result;
  if (auto it = kPrepositionCases.find(w1); it != kPrepositionCases.end()) {
    result.grammatical_case = it->second;
  } else if (auto it = kComplexPrepositions.find(w1); it != kComplexPrepositions.end() && it->second == w2) {
    result.grammatical_case = Case::GENITIVE;
  } else if (auto governing = decimal ? std::nullopt : governed(before)) {
    result.grammatical_case = *governing;
  } else if (auto earlier = enumerated(context)) {
    result.grammatical_case = features(*earlier, 0, false).grammatical_case;
  }

  // 3. The noun after it, as it agrees with the number
  bool few = n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 12 || n % 100 > 14);
  bool nominative = result.grammatical_case == Case::NOMINATIVE || result.grammatical_case == Case::ACCUSATIVE;
  if ((noun.ends_with(U"ami") || noun.ends_with(U"mi")) && noun.size() > 4) {
    result.grammatical_case = Case::INSTRUMENTAL;
  } else if (noun.ends_with(U"ach") && noun.size() > 4) {
    result.grammatical_case = Case::LOCATIVE;
  } else if (nominative && few && counted(noun)) {
    result.grammatical_case = Case::GENITIVE; // trzech osób: the noun is genitive, so the number is too
  }

  if ((noun.ends_with(U'a') && n == 1) || starts_with_any(noun, kFemininePlurals)) {
    result.gender = Gender::FEMININE;
  } else if (noun == U"euro") {
    result.gender = Gender::NEUTER;
  }
  return result;
}

// The word governing a dot ordinal: the one before it, or before the first ordinal of "lat 50. i 60."
std::u32string ordinal_cue(const Context& context) {
  std::u32string w1 = context.before();
  auto head = words_of(context.head());
  if (w1 != U"czy" && kConjunctions.contains(w1) && head.size() > 1) {
    std::u32string_view previous = head[head.size() - 2];
    if (previous.ends_with(U'.') && made_of(previous.substr(0, previous.size() - 1), kDigits)) {
      return context.before(3);
    }
  }
  return w1;
}

// The gender of a counted noun, for the number before it: dwie tony, dwa metry
Gender gender_of(const Noun& noun) {
  const auto& nominative = noun.singular[index(Case::NOMINATIVE)];
  return nominative.ends_with(U'a') ? Gender::FEMININE : nominative == U"euro" ? Gender::NEUTER : Gender::MASCULINE;
}

// A decimal number in a case: dwa przecinek pięć, półtora, dwa i pół
std::u32string decimal(const Number& number, Case c) {
  // 801.04, 4.0: a dotted code rather than a decimal
  if (number.decimal_mark == U'.' && (number.fraction.size() == 2 || number.fraction == U"0")) {
    uint64_t f = value_of(number.fraction).value_or(0);
    return cardinal(value(number)) + U" " + (number.fraction[0] == U'0' ? digits(number.fraction) : cardinal(f));
  }

  bool zero = !number.integer.empty(); // .5 has no zero before its point
  if (number.integer == U"1" && number.fraction == U"5") {
    return U"półtora";
  }
  std::u32string head = zero ? cardinal(value(number), c) + U" " : U"";
  if (number.fraction == U"5") {
    return head + U"i pół";
  }

  std::vector<std::u32string> words = {U"przecinek"};
  size_t zeros = number.fraction.find_first_not_of(U'0');
  words.insert(words.end(), std::min(zeros, number.fraction.size()), U"zero");
  if (zeros != std::u32string::npos) {
    words.push_back(cardinal(value_of(number.fraction).value_or(0), c));
  }
  return head + join(words);
}

// A number counting a noun, both in a case: pięćset dwadzieścia cztery kilometry
std::u32string amount(const Number& number, const Noun& noun, Case c) {
  std::u32string words;
  if (!number.fraction.empty()) {
    words = decimal(number, c);
  } else {
    auto n = value_of(number.integer);
    words = !n || spoken_digits(number, 7) ? digits(number.integer) : cardinal(*n, c, gender_of(noun));
  }
  return words + U" " + form(noun, value(number), c, !number.fraction.empty());
}

bool is_zloty(std::u32string_view currency) {
  return currency == U"zł" || currency == U"PLN";
}
} // namespace

Num2Word::Num2Word() : Num2WordLayer(kNotation, kJoiners) {
  add_pattern(std::make_unique<DatePattern>(kDateFormat));
  for (const auto& format : kPhoneFormats) {
    add_pattern(std::make_unique<PhonePattern>(format));
  }
  add_pattern(std::make_unique<DotOrdinalPattern>(true, true));
  add_pattern(std::make_unique<SuffixPattern>(Kind::NUMBER, kYearSuffixes, kYearFormat));
}

// -----------------------
// ----- Resolution -----
// -----------------------

bool Num2Word::accept(const Match& match, const Context& context) const {
  switch (match.kind) {
    case Kind::PHONE:
      // 600 123 456 is a count, unless a word says it is a phone number
      return match.separator != U' ' || kPhoneWords.contains(context.before());
    case Kind::TIME: {
      const auto& minutes = match.numbers[1].integer;
      bool round = minutes == U"00" || minutes == U"15" || minutes == U"30" || minutes == U"45";
      bool decimal = match.separator == U'.' && !round && !kTimeWords.contains(context.before());
      return value(match.numbers[0]) <= 24 && value(match.numbers[1]) <= 59 && !decimal;
    }
    case Kind::ORDINAL: {
      // An ordinal agrees with the noun after it: "29. sesji", but "Punkt 3. Dalej" ends a sentence
      std::u32string w1 = ordinal_cue(context);
      auto words = words_of(context.tail());
      std::u32string_view next = words.empty() ? U"" : stripped(words[0]);
      bool heading = starts_with_any(w1, kHeadings) && value(match.numbers[0]) < 100;
      return kDecadeWords.contains(w1) || heading || (!next.empty() && unicode::islower(next[0]));
    }
    default:
      return true;
  }
}

NumberReading Num2Word::resolve(const Match& match, const Context& context) const {
  NumberReading reading;

  switch (match.kind) {
    case Kind::DATE:
      reading.grammatical_case = kCopulas.contains(context.before()) ? Case::NOMINATIVE : Case::GENITIVE;
      return reading;

    case Kind::TIME: {
      // o osiemnastej trzydzieści: the hour is a feminine ordinal (godzina) in the case of the word before
      std::u32string w1 = context.before();
      reading.grammatical_case = w1 == U"o" || w1 == U"godzinie" || w1 == U"po"                ? Case::LOCATIVE :
                                 w1 == U"do" || w1 == U"od" || w1 == U"około" || w1 == U"koło" ? Case::GENITIVE :
                                 w1 == U"przed" || w1 == U"między"                             ? Case::INSTRUMENTAL :
                                 w1 == U"na" || w1 == U"za"                                    ? Case::ACCUSATIVE :
                                                                                                 Case::NOMINATIVE;
      return reading;
    }

    case Kind::ORDINAL: {
      // "29. sesji": an ordinal agreeing with the noun after it, the decade of "lat 70.", or a heading
      reading.form = Form::ORDINAL;
      std::u32string w1 = ordinal_cue(context);
      if (auto it = kDecadeWords.find(w1); it != kDecadeWords.end()) {
        reading.grammatical_case = it->second;
        reading.plural = true;
      } else if (starts_with_any(w1, kHeadings) && value(match.numbers[0]) < 100) {
        reading.grammatical_case = noun_form(w1, context.before(2)).grammatical_case; // Punkt 3., w punkcie 5.
      } else {
        auto words = words_of(context.tail());
        NounForm next = noun_form(words.empty() ? U"" : strings::to_lower(stripped(words[0])), w1);
        reading.grammatical_case = next.grammatical_case;
        reading.gender = next.gender;
        reading.plural = next.plural;
      }
      return reading;
    }

    case Kind::DIMENSION: {
      // numerem 3x38: a document number, where "x" stands for a slash
      std::u32string_view head = context.head();
      size_t digits = head.size();
      while (digits > 0 && unicode::isdigit(head[digits - 1])) {
        digits--;
      }
      size_t word_end = digits;
      while (word_end > 0 && unicode::isspace(head[word_end - 1])) {
        word_end--;
      }
      size_t word_begin = word_end;
      while (word_begin > 0 && unicode::isword(head[word_begin - 1])) {
        word_begin--;
      }
      std::u32string_view word = head.substr(word_begin, word_end - word_begin);
      reading.numbered = digits < head.size() && word_end < digits && (word.starts_with(U"numer") || word == U"nr");
      return reading;
    }

    default:
      break;
  }

  // A number: its own inflection (the start of a range for amounts), and the case of what it counts
  if (!match.numbers.empty()) {
    const Number& first = match.numbers.front();
    bool year_abbreviation = match.suffix == U"r.";
    Features own = first.fraction.empty() ? features(context, value(first), false, year_abbreviation) :
                                            features(context, 0, true, year_abbreviation);
    reading.form = own.form;
    reading.grammatical_case = own.grammatical_case;
    reading.gender = own.gender;

    const Number& last = match.numbers.back();
    bool grosze = match.kind == Kind::MONEY && match.scale.empty() && last.fraction.size() == 2 && is_zloty(match.unit);
    reading.amount_case = features(context, 0, !last.fraction.empty() && !grosze).grammatical_case;
  }
  return reading;
}

// --------------------
// ----- Readings -----
// --------------------

std::u32string Num2Word::read_number(const Number& number, const NumberReading& reading) const {
  if (!number.fraction.empty()) {
    return decimal(number, reading.grammatical_case);
  }

  auto n = value_of(number.integer);
  if (!n || spoken_digits(number, 7)) {
    return digits(number.integer);
  }
  if (reading.form == Form::ORDINAL) {
    return ordinal(*n, reading.grammatical_case, reading.gender);
  }
  return cardinal(*n, reading.grammatical_case, reading.gender);
}

std::u32string Num2Word::read_code_digits(std::u32string_view text) const {
  auto n = value_of(text);
  if ((text.size() > 1 && text[0] == U'0') || text.size() >= 7 || !n) {
    return digits(text);
  }
  return cardinal(*n);
}

std::u32string Num2Word::read_date(const Match& match, const NumberReading& reading) const {
  std::vector<std::u32string> words = {ordinal(value(match.numbers[0]), reading.grammatical_case),
                                       std::u32string(kMonthsGenitive[value(match.numbers[1]) - 1]),
                                       ordinal(value(match.numbers[2]), Case::GENITIVE)};
  if (!match.suffix.empty()) {
    words.push_back(U"roku");
  }
  return join(words);
}

std::u32string Num2Word::read_phone(const Match& match, const NumberReading& reading) const {
  std::vector<std::u32string> words;
  // In groups: sześćset sto dwadzieścia trzy; a longer one digit by digit
  for (const auto& group : match.numbers) {
    words.push_back(group.integer.size() > 3 ? digits(group.integer) : cardinal(value(group)));
  }
  return join(words);
}

std::u32string Num2Word::read_time(const Match& match, const NumberReading& reading) const {
  std::u32string hours = ordinal(value(match.numbers[0]), reading.grammatical_case, Gender::FEMININE);
  const auto& minutes = match.numbers[1].integer;
  if (value(match.numbers[1]) == 0) {
    return hours;
  }
  return hours + U" " + (minutes[0] == U'0' ? digits(minutes) : cardinal(value(match.numbers[1])));
}

std::u32string Num2Word::read_ordinal(const Match& match, const NumberReading& reading) const {
  return ordinal(value(match.numbers[0]), reading.grammatical_case, reading.gender, reading.plural);
}

std::u32string Num2Word::read_compound(const Match& match, const NumberReading& reading) const {
  const Number& number = match.numbers[0];
  if (!number.fraction.empty()) {
    return read_number(number, reading) + U"-" + std::u32string(match.suffix);
  }

  // lat 70-tych: siedemdziesiątych - the first gender and case taking the ending of the suffix
  if (auto it = kOrdinalSuffixes.find(std::u32string(match.suffix)); it != kOrdinalSuffixes.end()) {
    for (size_t g = 0; g < kAdjectiveEndings.size(); ++g) {
      for (size_t c = 0; c < kAdjectiveEndings[g].size(); ++c) {
        if (kAdjectiveEndings[g][c] == it->second) {
          Gender gender = g == 3 ? Gender::MASCULINE : static_cast<Gender>(g);
          return ordinal(value(number), static_cast<Case>(c), gender, g == 3);
        }
      }
    }
  }

  // 45-metrowe: czterdziestopięciometrowe
  return combining(value(number)) + std::u32string(match.suffix);
}

std::u32string Num2Word::read_plain(const Match& match, const NumberReading& reading) const {
  return read_number(match.numbers[0], reading) + (match.suffix == U"r." ? U" roku" : U"");
}

std::u32string Num2Word::read_money(const Number& number, const Match& match, const NumberReading& reading) const {
  const Noun& currency = kCurrencies.at(std::u32string(match.unit));
  Case c = reading.amount_case;
  if (!match.scale.empty()) {
    return amount(number, kScales.at(std::u32string(match.scale)), c) + U" " + currency.plural[index(Case::GENITIVE)];
  }

  // dwanaście złotych dziewięćdziesiąt dziewięć groszy
  if (number.fraction.size() == 2 && is_zloty(match.unit)) {
    uint64_t grosze = value_of(number.fraction).value_or(0);
    Number zlote{.integer = number.integer, .grouped = number.grouped};
    return amount(zlote, currency, c) + U" " + cardinal(grosze, c) + U" " + form(kGrosz, grosze, c);
  }
  return amount(number, currency, c);
}

std::u32string Num2Word::read_scale(const Number& number, const Match& match, const NumberReading& reading) const {
  return amount(number, kScales.at(std::u32string(match.scale)), reading.amount_case);
}

std::u32string Num2Word::read_percent(const Number& number, const Match& match, const NumberReading& reading) const {
  std::u32string suffix = match.suffix.empty() ? U"" : U" " + std::u32string(match.suffix);
  return amount(number, kPercent, reading.amount_case) + suffix;
}

std::u32string Num2Word::read_unit(const Number& number, const Match& match, const NumberReading& reading) const {
  return amount(number, kUnits.at(std::u32string(match.unit)), reading.amount_case);
}

std::u32string Num2Word::read_joiner(const Match& match, const NumberReading& reading) const {
  if (match.kind == Kind::DIMENSION) {
    return reading.numbered ? U" przez " : U" na ";
  }
  return Num2WordLayer::read_joiner(match, reading);
}

} // namespace phonemis::pl
