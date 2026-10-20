# Project Instructions

## Project Overview

Phonemis is a C++20 G2P (grapheme-to-phoneme) library for high-performance CPU phonemization on
mobile and edge devices. It targets many languages, and adding a new language must take minimal
effort. It has zero runtime dependencies: `json.hpp` and `xsimd` are bundled header-only.

```bash
./scripts/run_tests.sh       # Builds the entire library (including tests) and runs all tests
./scripts/lint.sh            # Lints changed lines: formatting, naming, comment length (see below)
./scripts/run_phonemizer.sh <LANG> --lexicon <PATH> --tagger <PATH> --model <PATH> <TEXT>
./scripts/build_android.sh   # Static lib for Android ARM
./scripts/build_ios.sh       # Static lib for iOS
```

```
data/<lang>/              # Lexicon JSON, Protophone model (.bin), optional tagger JSON
src/phonemis/base/        # Language-agnostic modules: pipeline, tokenizer, tagger, phonemizer, processor
src/phonemis/lang/<xx>/   # Language-specific modules: constants, num2word, pipeline (+ overrides)
src/phonemis/protophone/  # In-house neural inference (tensors, CTC model)
src/phonemis/utils/       # strings, unicode, conversions, io
src/third-party/          # Bundled header-only libraries
test/                     # In-house harness: REGISTER_TEST + ASSERT_EQUALS
```

For more detail, see `knowledge/code_style.md`. `lang/*/num2word.cpp` and
`protophone/protophone.cpp` are mostly AI-generated; do not use them as style references.

## Linting (mandatory)

Every code change under `src/phonemis` must pass the linter before the work counts as done:

```bash
./scripts/lint.sh               # Checks lines changed since HEAD (staged, unstaged, untracked)
./scripts/lint.sh --fix         # Applies clang-format to those lines first, then checks
./scripts/lint.sh --base main   # Checks everything changed on the current branch
```

- **Run it after every code change**, fix every reported issue, and re-run until it prints
  `lint: OK`. Never finish a task while it fails.
- It checks formatting (`.clang-format`, section 4), naming (`.clang-tidy`, section 3) and
  comment length (section 5). `--fix` handles formatting; fix naming and comments by hand.
- A PostToolUse hook (`.claude/settings.json`) also lints each C++ file right after you edit it.
  Its output is must-fix, just like the script's.
- Only changed lines are checked, so legacy code is flagged only where you touch it. Do not
  reformat or rename untouched code.
- Never weaken `.clang-format`, `.clang-tidy`, `.clang-format-ignore` or the comment limit to make
  the linter pass. If a rule seems wrong, ask.
- It needs python3 and LLVM >= 18 (`clang-format`, `clang-tidy`). If a tool is missing, the script
  says so, and that check has not run.

---

## 1. Modelling the code

**The simpler a piece of code, the better.** Write high-level, minimal, readable code. A reader
should understand a class or function at a glance.

- **One class, one responsibility**, describable in one sentence without "and".
- **Classes are intuitive objects for a human.** A `Tokenizer` tokenizes, a `Tagger` tags, a
  `Processor` applies `Layer`s, a `Pipeline` runs stages. Name the class after its role and its
  main method after the verb.
- **Minimal public interface.** Expose only what a caller needs: `Processor` has `add_layer()` and
  `process()`, `Layer` has `apply()`, `Tokenizer` has `tokenize()`. Helpers are private or
  file-local, and private helpers are still documented in the header.
- **Model with types:**
  - Give domain concepts names even when the alias is trivial: `using Lang = std::string;`,
    `using Tag = std::string;`, `split::Rules`, `split::Exceptions`.
  - Pass data between stages in plain structs with default member initializers (`Token`,
    `StringifiedNumber`, `PhonemizedWord`).
  - Use `enum class` for modes and rules, with values whose order means something when that is
    useful (`stress::Level`: `UNSTRESSED = -3 … FORCED = 3`).
- **The base class owns the algorithm; a subclass fills in one step** (Template Method):
  - `IPipeline::operator()` → `preprocess` / `process` / `postprocess`
  - `Phonemizer::phonemize(std::span<const Token>)` → `phonemize(const Token&) const = 0`
    + `update_context()`
  - `Tagger::tag()` → `tag_sentence()`
  - `Num2WordLayer<Reading>::apply()` → `resolve()`, `read_number()`, `read_date()`, …
- **Compose submodules.** A language pipeline is a preprocessor, a tokenizer and a phonemizer,
  plus an optional tagger:
  - Required parts are value members.
  - Optional parts are `std::unique_ptr<T> x_ = nullptr`, created only when configured and checked
    with `if (x_)`.
- **Specialise with data first, subclasses second:**
  - Language behaviour lives in `lang/xx/constants.h` tables, passed to generic components as
    `const T*`.
  - Generic layers take callables or tables: `SanitizerLayer(Filter, Mapper)`,
    `Sym2WordLayer(Symbols)`, `Term2WordLayer(Terms)`, `DatePattern(DateFormat)`.
  - Context-dependent data is still data: a reading with a condition (`{U"+", {{U"plus", after_name}, U"más"}}`).
  - Subclass only when data is not enough. The subclass reuses the base name inside the language
    namespace (`en::LexiconPhonemizer : phonemizer::LexiconPhonemizer`, `en::Num2Word :
    Num2WordLayer`), passes its tables to the base constructor and aliases the base
    (`using Base = …; using Base::phonemize;`).
- **Constrain templates with concepts:**
  `template <std::derived_from<LexiconPhonemizer> LXP, std::derived_from<nn::NeuralPhonemizer> NNP>`.
- **One `struct Config` per module**, and the top-level `Config` aggregates them. Optional
  resources are `std::optional<std::string> xxx_filepath = std::nullopt`; leaving one out disables
  the feature.
- **No state means free functions** in a namespace (`en::stress::apply()`), not a class.
- **Queries are `const`; state changes are explicit.** Lookups are `const`, and all mutation goes
  through one non-const hook (`update_context()`) that fills a context struct.
- **Fallback chains go from most specific to most general**, with early returns. "Not found" is
  `std::nullopt` at an interface and `U""` inside chains of helpers.
- **The phonemization path never throws.** Throw only while constructing or loading resources.
- **Avoid:**
  - speculative generality;
  - deep hierarchies;
  - "manager" or god classes;
  - one-line helpers extracted for a single condition;
  - exposing internals for convenience.

## 2. Code organization

- **A file holds one semantic piece**, which is one of:
  - one class;
  - a set of free functions that are very close in meaning (`utils/strings.h` holds only string
    operations, `utils/unicode.h` character classification, `utils/conversions.h` UTF-8 ⇄ UTF-32,
    `utils/io.h` file loading);
  - constants;
  - types;
  - a config.
- **Standard module files:**
  - `types.h`: enums, type aliases, small data structs (`tokenizer/types.h`: `split::Rule`,
    `Rules`, `Exceptions`).
  - `constants.h`: `inline const` tables in `<ns>::constants`, grouped into nested namespaces by
    the code that uses them (`constants::num2word`, `constants::tokenizer`, `constants::ipa`).
  - `config.h`: the module's `struct Config`.
  - `<interface>.h/.cpp`: an abstract base and its driver (`ipipeline.h/.cpp`,
    `phonemizer.h/.cpp`, `tagger.h/.cpp`, `layer.h`).
  - `<impl>_<module>.h/.cpp`: a concrete implementation (`lexicon_phonemizer`, `hmm_tagger`).
  - Each processor layer has its own subdirectory with a `layer.h/.cpp` and its helpers
    (`processor/layers/trim/`, `processor/layers/num2word/`), in namespace `processor::<layer>`.
    Their shared building blocks (Document, Context, conditions, text scanning) live in
    `processor/common/`, in namespace `processor`.
  - A central data struct can have its own file (`tokenizer/token.h`).
- Templates and small declarative classes are header-only (`HybridPhonemizer`,
  `lang/*/pipeline.h`, `utils/strings.h`). Everything else is split into `.h` and `.cpp`.
- **Namespaces mirror directories, without `base`:**
  - `base/tokenizer` → `phonemis::tokenizer`
  - `base/phonemizer/nn` → `phonemis::phonemizer::nn`
  - `lang/en` → `phonemis::en`
  - `utils/strings.h` → `phonemis::utils::strings`
- Use nested syntax, `namespace phonemis::phonemizer::nn {`, and close with the full correct name:
  `} // namespace phonemis::phonemizer::nn`.
  - Leave a blank line after the opening line and before the closing one.
  - Do not indent top-level content.
  - In `constants.h`, sub-namespaces are indented by 2, introduced by `// --- Section Name ---`
    and closed with `} // namespace num2word`.
- **File-local helpers** go in an anonymous namespace at the top of the `.cpp`, each with a
  one-line comment. Data used by only one function is a `static const` local in that function.
- Every header uses `#pragma once`.
- **Include order** (groups in this order, alphabetical inside each group):
  1. Local quoted headers (`"constants.h"`, `"../tokenizer/token.h"`).
  2. Project headers as `<phonemis/...>` (path from `src/`).
  3. A blank line, then standard library headers.
  4. Conditional `#ifdef ET_ON` includes last.

  Use relative quoted paths only between modules inside `base/`. Code in `utils/`, `lang/` or
  `protophone/` is included as `<phonemis/...>`, and third-party code as
  `<third-party/json.hpp>`.

## 3. Naming

The table below is enforced by `.clang-tidy` (through `./scripts/lint.sh`).

| Entity | Convention | Examples |
|---|---|---|
| Classes, structs | PascalCase; acronyms stay upper case | `LexiconPhonemizer`, `TensorView`, `HMMTagger` |
| Dynamic top-level interface | `I` prefix (other abstract bases have none) | `IPipeline`; `Phonemizer`, `Tagger`, `Layer` |
| Functions, variables, params, files | snake_case | `update_context`, `curr_token_id`, `hmm_tagger.cpp` |
| Private / protected members | snake_case + trailing `_` | `dict_`, `rules_`, `impl_`, `global_context_` |
| Public struct fields | snake_case, no suffix | `text`, `whitespace`, `lexicon_filepath` |
| Constants | `k` + PascalCase | `kPrimary`, `kEosCharacters`, `kCardinals` |
| Enumerators, macros | SCREAMING_SNAKE | `KEEP_WITH_RIGHT`, `EMPHASIZED`, `MAKE_NON_INPLACE` |
| Type aliases | PascalCase domain word | `Lang`, `Tag`, `Rules`, `Filter`, `Mapper` |
| Template params | short | `T`, `StringT`, `CharT`, `Pred`, `LXP`, `NNP` |

- In-place (mutating) variants end with `__` (`to_lower__`, `remove__`). The plain name returns a
  copy, generated with `MAKE_NON_INPLACE`.
- Predicates start with `is_`, `has_` or `should_` (`is_known`, `has_primary`, `should_lowercase`).
  Boolean flags are adjectives (`british_`, `first`, `whitespace`).
- Variants share a stem and add a suffix: `lookup`, `lookup_and_restress`, `lookup_stem_ed`.
- Prefixes and suffixes carry meaning: `*_filepath`, `nn_*` for neural config, and converted copies
  are named for what they are (`utf8_text`, `lower_word`).
- Short names (`it`, `idx`, `c`, `p1`) are fine in tight scopes.

## 4. Formatting

`.clang-format` is the source of truth for layout, and `./scripts/lint.sh --fix` applies it. Its
main choices:
- 2-space indentation, no tabs, and a 120-column limit. This also applies to the lines you change
  in legacy tab or 4-space files.
- Braces are attached (K&R): `} else if (...) {`, `} else {`. `public:` / `protected:` /
  `private:` line up with `class`.
- `&` and `*` attach to the type: `const Config& config`.
- Lines are filled up to the column limit. Wrapped arguments align with the opening parenthesis,
  and constructor initializer lists start with `: ` on the next line.
- Trivial one-statement member functions defined inside the class become one-liners.
- `} // namespace phonemis::xx` comments are added and corrected automatically.
- Includes are sorted inside each block (quoted, `<phonemis/...>`, `<third-party/...>`, standard
  library), as described in section 2.

The formatter doesn't handle these, so do them by hand:
- Leave out braces only for trivial guards, which stay on one line (`if (chunk.empty()) return;`).
  Every other block gets braces.
- `constants.h` files and generated tables are excluded (`.clang-format-ignore`). Keep their tables
  aligned in columns with several entries per line, and indent their sub-namespaces by 2.
- Separate logical blocks inside a function with a blank line plus a short comment.
- Leave a blank line after `namespace … {` and before its closing brace.

## 5. Comments and docs

### Length: always short (strict)
- A `//` comment is **1–2 lines**. **3–4 lines is the hard maximum**; anything longer is not
  acceptable.
- The only exception is a `/** … */` doc comment on a function, class or `Config` field. It may be
  longer only because it lists `@param`, `@returns`, `@throws` and so on; its description itself
  stays 1–2 sentences.
- If an explanation does not fit, simplify or split the code instead of writing a longer comment.
- `./scripts/lint.sh` enforces these limits. It flags more than 4 consecutive `//` lines, a
  `/* */` block over 4 lines, and a doc comment whose description (the text before its first `@`
  tag) is over 4 lines.

### Placement: follow the code flow
- Spread small comments through the function or algorithm, each directly above the block it
  describes. Number the steps of an algorithm.
- **Do not** gather the explanation of a whole function or algorithm in one block at the top.

```cpp
std::u32string Phonemizer::phonemize(std::span<const Token> tokens) {
  std::u32string result;
  result.reserve(tokens.size() * 5); // Heuristic allocation

  for (size_t i = 0; i < tokens.size(); ++i) {
    const auto& token = tokens[i];

    // 1. Context Update
    update_context(i, tokens);

    // 2. Core Phonemization
    auto phonemes = phonemize(token);
    if (phonemes) {
      result.append(*phonemes);
    }

    // 3. Fallback/Punctuation Logic
    // Handles punctuation that doesn't have explicit phonemes but must be preserved.
    if (!phonemes.has_value() && !token.text.empty()) {
      …
    }

    // 4. Whitespace handling
    if (token.whitespace) {
      result.push_back(U' ');
    }
  }

  return result;
}
```

```cpp
// Bad: an essay on a member
// Reverse mapping: ID -> Character (used for decoding).
// Stored as a flat vector indexed directly by token ID instead of a hash
// map: token IDs are assigned as a dense, contiguous range [0, N), so the
// ID *is* a valid array index. This gives a single contiguous allocation
// (vs. one heap node per entry), no hashing on the hot decode() path, and
// better cache locality. An unset slot holds U'\0' to mean "no character".
std::vector<char32_t> token_to_char_;

// Good: what it is, plus the one fact a reader needs
// Reverse mapping: ID -> Character (used for decoding).
// Indexed directly by token ID; U'\0' marks an unused slot.
std::vector<char32_t> token_to_char_;
```

### Doc comments
```cpp
/**
 * Allows to customize Processor's behavior by expanding it with custom layers.
 * @param layer a new transformation layer to be added.
 */
void add_layer(std::unique_ptr<Layer> layer);

// Performs a preprocessing stage of the pipeline.
std::u32string preprocess(const std::u32string& input) override;
```
- Document the public API in headers with `/** … */`, using `@param`, `@returns`, `@details` and
  `@throws`. The first sentence is short and parameter descriptions are brief.
- Every `Config` field gets its own doc block.
- Obvious members get a single `//` line instead of a doc block.

### Content and voice
- Say what the code does and, briefly, why.
- Descriptions often start with "A …" ("A shortcut for base class calls").
- Use "we" for assumptions ("We assume the input got preprocessed …").
- Be candid about hacks and heuristics (`// A hack to omit the leading whitespaces`).
- Show a short example instead of describing one in prose:
  ```cpp
  case split::Rule::KEEP_WITH_LEFT:
    // xyz-abc -> xyz-, abc (unless abc empty)
  ```
- Mark caveats with `// NOTE:` and cite sources as bare URLs
  (`// https://en.wiktionary.org/wiki/-ed`).
- Annotate non-ASCII literals with their name and code point:
  `U'।', // Hindi Purna Viram (U+0964)`.
- Use section banners: `// --- Tokenization Rules ---` in constants and
  `// ----- Predicates -----` in utilities.

### Never write
- comments longer than 4 lines, except doc comments;
- one block at the top that explains the whole function or algorithm;
- em dashes;
- comments that restate the code;
- history notes such as "matching the previous behaviour".

## 6. C++ idioms

- Use C++20: concepts, `std::span`, `contains()`, designated initializers, structured bindings,
  `if (auto it = m.find(c); it != m.end())`, `if constexpr`. Classic iterator algorithms and
  `std::ranges` are both fine.
- **Text is UTF-32 inside** (`std::u32string`, `std::u32string_view`, `char32_t`, `U'x'`). UTF-8
  appears only at the boundaries (API overloads, JSON, lexicon storage), converted with
  `utils::conversions`.
- Unicode support is in-house (`utils::unicode`, a lookup table plus `constexpr` predicates); do not
  use `<locale>`. Do not add dependencies.
- **Parameters:**
  - `std::u32string_view` for read-only text.
  - `const std::u32string&` when a real string is needed.
  - `std::span<const T>` for read-only sequences and `std::span<T>` for in-place edits.
  - Return by value.
- **Ownership:**
  - Value members for parts that are always present.
  - `std::unique_ptr` for optional or polymorphic parts.
  - A nullable `const T*` for static tables owned elsewhere, always null-checked.
- **Classes:** `explicit` on one-argument constructors, `virtual ~Base() = default;`, `override`
  on every override, and `const` on every query.
- **Constants:**
  - Tables are `inline const` containers in headers.
  - Scalars are `inline constexpr`.
  - A constant derived from another is an immediately invoked lambda.
  - A single-use set is a `static const` local in its function.
- Use `std::optional` for absence (`std::optional<bool>` is acceptable as a three-way value), and
  `value_or()` for defaults.
- **Performance** (mobile and edge targets):
  - `reserve()` before loops;
  - views instead of substring copies;
  - flat vectors on hot paths.
- **Errors:**
  - A missing required config throws `std::runtime_error("<Class>: <field> must be provided in the configuration.")`.
  - Bad data or arguments throw `std::invalid_argument`.
  - Validate once at load time.
- `using namespace utils;` is allowed only in a `.cpp`. Headers use targeted `using` declarations
  (`using tokenizer::Token;`).
- Use macros only to generate code. Keep parameter names of unused parameters in comments:
  `size_t /*idx*/`.

## 7. Adding a language `xx`

1. `lang/xx/constants.h`: all the language's data (`num2word`, `sym2word`, `term2word`, `tokenizer`
   rules and exceptions). In `num2word`, besides the words, how numbers are written: `kNotation`,
   `kJoiners` and the formats of configurable patterns (`kDateFormat`, `kPhoneFormat`, suffixes).
2. `lang/xx/num2word.h/.cpp`: a `NumberReading` struct (what context decides) and
   `class Num2Word : public processor::num2word::Num2WordLayer<NumberReading>`: the list of its
   patterns, `resolve()` and readings. Patterns beyond the configurable ones go to `lang/xx/patterns.h`.
3. `lang/xx/pipeline.h`: header-only. Copy the shape of `lang/de/pipeline.h`: its preprocessing is
   `processor::make_preprocessor<Num2Word>(config.preprocessor, {...its tables...})`, which adds the
   layers `processor::Config` enables in their fixed order (Normalize, Trim, Term2Word, Num2Word,
   Sym2Word, Sanitizer).
4. Register it in `Pipeline::create_pipeline` (`base/pipeline.cpp`).
5. Put data in `data/xx/` and add tests in `test/`.
