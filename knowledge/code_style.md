# Phonemis: Code Style and Logic Modelling Notes

These notes describe how the author writes and structures code in `src/phonemis`. Use them to write
new code (a language, a component, a fix) that reads like the author wrote it.

## 0. Sources and weighting

- **Primary evidence** (hand-written by the author, confirmed with `git blame`):
  - `base/`: pipeline, phonemizer core, lexicon and hybrid phonemizers, tokenizer, tagger
    interface, processor and layers.
  - `lang/en/*`, except `num2word.cpp`.
  - Every `lang/*/pipeline.h`.
  - `utils/*`.
  - `protophone/{tensor,tensor_view,tensor_utils,types,meta,numeric}.h`.
- **Excluded or weighted lower:**
  - `lang/*/num2word.cpp` and `protophone/protophone.cpp`, which are mostly AI-generated.
  - The long linguistic commentary in the non-English `constants.h` files. It was written
    together with the num2word code.
  - Commit `c8d8744`, an external AI-assisted refactor. It rewrote `tagger/hmm_tagger.*` and
    `nn/tokenizer.*` and split `num2word/layer.cpp` into helpers.
  - Commit `36cef09`, which added `phonemize_words()` to `ipipeline.cpp`.
  - The comment style of these commits is **not** the author's voice (see §9).

---

## 1. Modelling the logic (architecture)

### 1.1 Three-stage pipeline behind a facade
- Every language is an `IPipeline` with three overridable stages. The base `operator()` runs
  them in order:
  ```cpp
  class IPipeline {
  public:
    virtual ~IPipeline() = default;
    virtual std::u32string operator()(std::u32string_view text, bool preprocess = true, bool postprocess = true);
    // A processing parts to be implemented by derived classes.
    virtual std::u32string preprocess(const std::u32string& input) = 0;
    virtual std::u32string process(const std::u32string& input) = 0;
    virtual std::u32string postprocess(const std::u32string& input) = 0;
  };
  ```
- The public `Pipeline` class is a facade over `std::unique_ptr<IPipeline> impl_`:
  - A private static factory, `create_pipeline(const Config&)`, chooses the implementation.
  - The factory is a flat `if` chain on language codes. Each line ends with a comment naming the
    language (`// German`).
  - It throws `std::invalid_argument` for an empty or unsupported language.
  - Every public method is a one-line delegation to `impl_`.
- Each stage is public on its own (`preprocess`/`process`/`postprocess`), as are the `preprocess`
  and `postprocess` flags. This keeps stages testable and lets callers bypass them.
- There are UTF-8 (`std::string_view`) and UTF-32 (`std::u32string_view`) overloads of the entry
  points. The UTF-8 overload converts the input and forwards to the UTF-32 one.

### 1.2 Template Method everywhere: the base owns the loop, the subclass handles one unit
| Base driver (implemented once) | Hooks for subclasses |
|---|---|
| `IPipeline::operator()` | `preprocess` / `process` / `postprocess` (`= 0`) |
| `Phonemizer::phonemize(std::span<const Token>)` | `phonemize(const Token&) const = 0`, `update_context(idx, tokens)` (no-op by default) |
| `Tagger::tag(std::span<Token>)` (splits the stream on EOS characters) | `tag_sentence(std::span<Token>) const = 0` |
| `Num2WordLayer<Reading>::apply()` (each pattern rewrites a draft: scan → `accept` → `resolve` → verbalize) | `resolve`, `read_number`, `read_code_digits` (`= 0`), `accept`, `read_date`, `read_money`, … (defaults) |
| `HMMTagger::tag_sentence()` | `should_lowercase(word)`: protected virtual, defaults to `false`, English overrides it |

- Per-unit hooks are `const` (pure lookups). All state changes go through one hook that is
  explicitly non-const (`update_context`), and its `@details` says why:
  > `@details Non-const as it manages stateful context updates across the sequence.`
  > `@details Const as it is a pure lookup based on current internal state.`

### 1.3 A pipeline is a composition of submodules
```cpp
private:
  // Required submodules
  processor::Preprocessor preprocessor_;
  tokenizer::Tokenizer tokenizer_;
  phonemizer::HybridPhonemizer<en::LexiconPhonemizer, phonemizer::nn::NeuralPhonemizer> phonemizer_;

  // Optional submodules
  std::unique_ptr<tagger::HMMTagger> tagger_ = nullptr;
```
- Required parts are value members. Optional parts are a `unique_ptr`, created only when their
  config is present and checked with `if (tagger_)`.
- `HybridPhonemizer` is a template constrained with C++20 concepts:
  `template <std::derived_from<LexiconPhonemizer> LXP, std::derived_from<nn::NeuralPhonemizer> NNP>`.
  - It owns both sub-phonemizers through `unique_ptr`.
  - A sub-phonemizer exists only if its file path was given: "Not passing filepath … disables it".
  - Lookup order is lexicon first, then the neural model as a fallback, written as ternaries on
    the pointers.

### 1.4 Layered text processing (chain of transformations)
- The interface is `processor::Layer`, with
  `virtual std::u32string transform(std::u32string_view input) const = 0;`.
- `Processor` holds a `std::vector<std::unique_ptr<Layer>>`, exposes `add_layer(...)`, and
  `process()` applies the layers in order.
- Role names are aliases, not new types:
  `using Preprocessor = Processor; using Postprocessor = Processor; // Naming abstraction`.
- Generic layers are configured with callables rather than subclasses. `SanitizerLayer(Filter, Mapper)`
  takes `std::function` aliases whose defaults are identity lambdas. Language pipelines pass small
  lambdas that look up a table from `constants.h`.

### 1.5 Language specialisation is data first, subclasses second
- Generic components receive language data as non-owning pointers to static tables:
  `tokenizer_(&constants::tokenizer::kSpecialCharacters, &constants::tokenizer::kExceptions)`.
- A language is mostly tables in `lang/xx/constants.h`: split rules, exception words, number
  words, currencies, months, and character replacements.
- Subclass only where tables are not enough (`en::LexiconPhonemizer`, `en::HMMTagger`,
  `xx::Num2Word`).
- A subclass **reuses the base class name inside the language namespace**: `phonemis::en::Pipeline`,
  `phonemis::en::LexiconPhonemizer`, `phonemis::en::HMMTagger`.
- Constructors are inherited: `using tagger::HMMTagger::HMMTagger;`, `using Num2WordLayer::Num2WordLayer;`.
- The base class alias pattern:
  ```cpp
  // A shortcut for base class calls
  using Base = phonemizer::LexiconPhonemizer;
  using Base::phonemize;          // un-hide the overloaded base method
  ...
  auto result = Base::lookup(utf8_word, tag);
  ```
- When a language needs a modified config, a small private static helper copies it and patches
  only the missing fields (`hi::Pipeline::make_phonemizer_config`).

### 1.6 Configuration modelling
- Each module has one `struct Config` in its own `config.h`, inside the module's namespace
  (`phonemis::Config`, `phonemizer::Config`, `tagger::Config`, `num2word::Config`).
- The top-level `Config` aggregates sub-configs. A required sub-config is a plain member; an
  optional one is wrapped: `std::optional<tagger::Config> tagger;`.
- Optional resources are `std::optional<std::string> xxx_filepath = std::nullopt;`. A missing
  resource **disables the feature**, documented as `@details If not provided, … is disabled.`
- Overrides of default tables are raw pointers to externally owned data, defaulting to null:
  `const std::unordered_map<char32_t, int64_t>* nn_grapheme_mapping = nullptr;`.
- Every field has its own `/** … */` block, with a blank line between fields.

### 1.7 Plain data structs carry state between stages
- `Token` holds the text, context flags (`first`, `whitespace`) and optional decorations
  (`std::optional<tagger::Tag> tag`).
  - Decorator modules such as the tagger annotate tokens **in place**, rather than producing
    parallel arrays.
  - Position is stored as flags, not offsets.
- Small named structs are used instead of tuples or out-parameters: `NumberMatch {len, mode}`,
  `GlobalContext`, `Weights<T> {weight, bias}`, `Range<T> {begin, end}`. Every field gets a default
  member initializer.

### 1.8 Enums encode rules, and their order means something
- `enum class` is used for rule sets and modes: `split::Rule`, `num2word::Mode`,
  `stress::Level : int32_t`.
- Each enumerator gets a short doc comment, often with an example:
  `// Separator attached to the start of the following word (e.g., "well", "-being")`.
- Values are chosen so that comparisons make sense: `UNSTRESSED = -3 … NORMAL = 0 … FORCED = 3`
  allows `*stress >= stress::Level::NORMAL`. The first enumerator is explicitly `= 0`.
- Blank lines group related enumerators (`POTENTIALLY_ORDINAL, ORDINAL` / `FRACTION, CURRENCY`).

### 1.9 Stateless logic goes in free functions in a namespace
- Example: `en::stress::apply(std::u32string& phonemes, Level)` and `en::stress::restress(...)`.
  They are in-place and have no class around them.
- All utilities are namespaces of free functions, often templates: `utils::strings`,
  `utils::unicode`, `utils::conversions`, `utils::io`, `protophone::numeric`, `protophone::meta`,
  `protophone::utils`.

### 1.10 Choosing the backend at build time
- `#ifdef ET_ON` switches the includes, the member type (`std::unique_ptr<executorch::…::Module>`
  vs `std::unique_ptr<protophone::Protophone>`), and the inference code. Both branches sit side by
  side inside the same function.

### 1.11 Protophone (the in-house tensor library)
- The vocabulary mirrors PyTorch:
  - `Tensor<T>` owns its data (`std::vector<T, xsimd::default_allocator<T>>`).
  - `TensorView<T>` is non-owning: pointer, shape, strides and offset.
  - `reshape`, `slice(args...)` and `transpose(dim0 = 0, dim1 = 1)` all return new views by value.
- Variadic slicing arguments are normalised to `std::variant<size_t, Range<size_t>, All>` in
  `meta::to_slice_variant` (with `if constexpr`).
- In-place operations are free functions declared as friends (`utils::repack`) rather than member
  methods.
- Comment headings group members: `// Friends declarations`, `// Tensor data`,
  `// Tensor metadata`, `// Simple accessors` (one-line inline accessors).
- Forward declarations are collected in a commented block to break header cycles.

---

## 2. Domain logic patterns (G2P)

### 2.1 Cascade from most specific to most generic, with early returns
The order in `en::LexiconPhonemizer::phonemize`:
1. `lookup_special`: hand-coded special words (a/an/the/to/in/am/used/supposed/…), with context.
2. Text in all capitals → `lookup_nnp` (acronyms, spelled out letter by letter).
3. Exact lexicon entry (`is_known`), then apostrophe variants (`s'` → `'s`, trailing `'`).
4. Morphological stems: `lookup_stem_s`, then `lookup_stem_ed`, then `lookup_stem_ing`.
5. `std::nullopt`. The caller (`HybridPhonemizer`) falls back to the neural model, and the base
   `Phonemizer` keeps punctuation.

Each step follows the same shape:
```cpp
phonemes = lookup_stem_s(token.text, tag, stress);
if (!phonemes.empty()) {
  return std::make_optional(phonemes);
}
```

### 2.2 Two ways to say "not found", by layer
- At the interface boundary, `Phonemizer::phonemize(const Token&)` returns
  `std::optional<std::u32string>`, and `std::nullopt` means "cannot phonemize".
- Internal helpers (`lookup*`) return `std::u32string`, where an empty `U""` means "not found".
  This keeps chains cheap. Convert to an optional only at the boundary.
- `std::optional<stress::Level>` means "leave stress unchanged" when empty, and
  `std::optional<bool>` is used deliberately as a three-way value (`vowel_next`: vowel,
  consonant, or unknown).

### 2.3 Lookups relax the context step by step
- Lexicon keys are `"word"` or `"word|context"`, where the context is the part-of-speech tag.
- `lookup(word, tag)` tries the exact tag, then the parent tag (`get_parent_tag`: `VB*` → `VERB`,
  `NN*` → `NOUN`, …), then no context.
- Case also relaxes: exact case first, then lowercase (`is_known(word, try_lowercase = true)`).

### 2.4 Morphology is a stem lookup plus a phonological suffix rule
- First guard on length and suffix. Then an `if / else if` ladder finds a known stem. Then the stem
  is phonemized, and a suffix allophone is chosen from its **last phoneme**.
- The small phoneme sets used for that choice are `static const std::u32string` locals tested with
  `.find(c) != npos`:
  ```cpp
  // https://en.wiktionary.org/wiki/-s
  static const std::u32string hard_s_suffixes = U"ptkfθ";
  static const std::u32string soft_s_suffixes = U"szʃʒʧʤ";
  if (hard_s_suffixes.find(phonemes.back()) != std::u32string::npos) {
    return phonemes + U"s";
  }
  ```
- Dialect differences are a member flag set in the constructor (`british_(config.lang == "en-gb")`)
  and resolved inline: `british_ ? U"ɪ" : U"ᵻ"`.
- IPA literals used only once stay inline (`U"ɾᵻd"`, `U"ɐm"`). They move to `constants.h` only
  when shared or language-configurable.

### 2.5 Lookahead context
- `update_context(curr_token_id, tokens)` fills a member `GlobalContext` before each token:
  - `vowel_next`: whether the next word starts with a vowel.
  - `future_to`: scans the next 2 tokens that are not punctuation for "to".
- `phonemize` stays `const` and only reads the context.

### 2.6 Tokenization driven by rule tables
- The input is split into phrases on whitespace.
- **Hard** separators (non-alphanumeric characters with no rule) always split and become their own
  tokens.
- **Soft** separators (keys in `split::Rules`) are resolved by their `Rule` in the recursive
  `process_chunk`.
- `split::Exceptions` (matched case-insensitively) stop any further splitting.
- Scanning avoids copies. The buffer is an `(offset, len)` pair into the input, handed on as
  `std::u32string_view` sub-views. Only the final tokens are copied into `std::u32string`.
- Assumptions about earlier stages are stated rather than re-checked:
  `// We assume the input got preprocessed in a correct way (i.e. with TrimLayer)`.

### 2.7 num2word: scan → classify → verbalize
- The scanner finds a number at a word boundary and classifies it as a `Mode`. It then builds
  `StringifiedNumber{text, mode}` and calls `verbalize`, which parses the text and dispatches to
  the virtual primitives.
- Each language's decimal separator is first normalised to `.`, so a single scanner works for every
  language.
- Languages override the primitives. They override `verbalize` itself only for grammar quirks
  (for example, Spanish "de" in dates).

### 2.8 Punctuation and whitespace are kept by structure
- The base `Phonemizer::phonemize` appends:
  - the phonemes;
  - the raw punctuation character when the token was not phonemized (except a soft `.` or `-` in
    the middle of a word);
  - `' '` when `token.whitespace` is set.
- Output spacing therefore matches token boundaries.

---

## 3. Code organisation

```
base/<module>/
  config.h                 // struct Config
  types.h                  // enums, aliases, small structs
  constants.h              // inline tables in <module>::constants
  <module>.h/.cpp          // interface / abstract base with the driver
  <impl>_<module>.h/.cpp   // concrete: lexicon_phonemizer, hmm_tagger
base/processor/<layer>/
  layer.h/.cpp             // one directory per layer: trim, sanitizer, term2word, num2word, sym2word
lang/<code>/
  constants.h  pipeline.h (header-only)  num2word.h/.cpp  [lexicon_phonemizer|hmm_tagger|stress].h/.cpp
```
- One responsibility per file; a file is named after its main class or concept (snake_case).
- These are header-only:
  - Language pipelines (an explicit decision: "Make lang-specific pipelines header only").
  - Templates (`HybridPhonemizer`, `strings.h`, tensors).
  - `constexpr` Unicode helpers.

  Non-template logic of real size goes in a `.cpp`.
- Namespaces mirror directories, **but `base/` is not a namespace level**:
  - `base/tokenizer` → `phonemis::tokenizer`
  - `base/phonemizer/nn` → `phonemis::phonemizer::nn`
  - `lang/en` → `phonemis::en`
  - `utils/strings.h` → `phonemis::utils::strings`
- Use nested namespace syntax: `namespace phonemis::phonemizer::nn {`. Always close with the full
  name: `} // namespace phonemis::phonemizer::nn`.
- Constants live in `<ns>::constants`, split by which code consumes them: `constants::num2word`,
  `constants::tokenizer`, `constants::alphabet`, `constants::ipa`, `constants::puncts`,
  `constants::sanitizer`, `constants::ipa::stress`.
  - In language `constants.h` files, each block opens with a banner such as
    `// --- Tokenization Rules ---`.
  - The **contents of the nested namespace are indented by 2**, and the block closes with
    `} // namespace tokenizer`.
  - Base types are imported with a fully qualified, leading-`::` using-declaration:
    `using ::phonemis::tokenizer::split::Rule;`.
- File-local helpers go in an anonymous namespace at the top of the `.cpp`
  (`// Helper functions` / `} // namespace`).
- Use `#pragma once` (not include guards).

**Include order**
```cpp
#include "constants.h"                       // 1. local, quoted, alphabetical (own header not forced first)
#include "lexicon_phonemizer.h"
#include "stress.h"
#include <phonemis/base/phonemizer/constants.h>   // 2. project headers, <phonemis/...> path from src/
#include <phonemis/utils/strings.h>

#include <algorithm>                         // 3. standard library, alphabetical
#include <string>

#ifdef ET_ON                                 // 4. backend / conditional includes last
```
- Inside `base/`, sibling modules are included by relative path (`"../tokenizer/token.h"`,
  `"../types.h"`).
- Crossing into `utils/`, `lang/` or `protophone/` uses `<phonemis/...>`.
- Third-party headers use `<third-party/json.hpp>`.

---

## 4. Naming

| Entity | Convention | Examples |
|---|---|---|
| Classes / structs | PascalCase; acronyms stay upper case | `LexiconPhonemizer`, `TensorView`, `HMMTagger`, `GlobalContext` |
| Interfaces | `I` prefix only on the top-level pipeline interface | `IPipeline`; other abstract bases have no prefix (`Phonemizer`, `Tagger`, `Layer`) |
| Functions, methods, locals, params | snake_case | `update_context`, `lookup_stem_ing`, `curr_token_id`, `token_vec` |
| Non-public data members | snake_case + trailing `_` | `dict_`, `rules_`, `impl_`, `british_`, `global_context_` |
| Public struct fields | snake_case, no suffix | `text`, `whitespace`, `lexicon_filepath`, `phoneme_offset` |
| Constants | `k` + PascalCase, `inline const` / `inline constexpr` | `kPrimary`, `kEosCharacters`, `kSpecialCharacters`, `kCardinals` |
| Enumerators | SCREAMING_SNAKE | `KEEP_WITH_RIGHT`, `POTENTIALLY_ORDINAL`, `EMPHASIZED` |
| Type aliases | PascalCase domain words, even for trivial aliases | `using Lang = std::string;`, `using Tag = std::string;`, `Filter`, `Mapper`, `Rules`, `Exceptions`, `Buffer` |
| Template params | short | `T`, `U`, `StringT`, `CharT`, `Pred`, `LXP`, `NNP` |
| Macros | SCREAMING_SNAKE | `MAKE_NON_INPLACE`, `REGISTER_TEST`, `ET_ON` |

- **In-place (mutating) string operations end in `__`**: `to_lower__`, `remove__`, `replace__`,
  `filter__`. The non-mutating version has the plain name and is generated by
  `MAKE_NON_INPLACE(name)`.
- The string utilities borrow Python's vocabulary: `strip`, `split`, `capitalize`, `is_alpha`,
  `starts_with`, `ends_with`.
- Predicates start with `is_`, `has_` or `should_` (`is_known`, `is_soft_separator`,
  `has_primary`, `should_lowercase`). Boolean flags are adjectives or nouns: `british_`, `first`,
  `whitespace`, `vowel_next`, `future_to`, `last_was_space`.
- Variants of one operation share a stem and add a suffix: `lookup`, `lookup_and_restress`,
  `lookup_stem_s|_ed|_ing`, `lookup_nnp`, `lookup_special`.
- Short or abbreviated local names are normal:
  - `currw_offset` / `currw_len` (current word)
  - `fp` (file path)
  - `bpoint`, `lbound` / `rbound`
  - `no_steps` / `no_classes` ("number of")
  - `p1` / `p2` / `p3`, `dec_sep`, `idx`, `it`
- Prefixes and suffixes carry meaning:
  - `nn_` marks neural-network config fields (`nn_model_filepath`).
  - `*_filepath` marks paths.
  - Converted copies are named after what they are: `utf8_text`, `lower_word`, `word_stripped`,
    `word_without_dots`.
- Legacy: `nn/constants.h` uses SCREAMING constants (`MAX_SEQ_LEN`, `PAD_TOKEN`,
  `DEFAULT_CHAR_TO_TOKEN`). New constants should use `kPascalCase`.

---

## 5. Formatting

- **Indentation: 2 spaces.** This is the dominant style; use it for new code. Some legacy files use
  tabs (`tokenizer.cpp/.h`, base `lexicon_phonemizer.cpp`, `io.cpp`, `strings.h`) or 4 spaces
  (`nn/tokenizer.h`, `num2word/layer.h`, `lang/*/num2word.h`, `main.cpp`, tests). When editing
  those, match the surrounding lines.
- Braces are attached (K&R) for functions, classes and control flow. `public:` / `protected:` /
  `private:` line up with `class`.
- Short guard clauses can be one line without braces: `if (chunk.empty()) return;`,
  `if (t == 0) break;`, `if (!val) return U"";`. Everything else gets braces.
- When every branch of an `else if` ladder has its own comment, `}` goes on its own line and
  `else if (...)` starts the next one (see `stress::apply`, `lookup_stem_ed`). Otherwise use
  `} else if (...) {`.
- Long conditions break after `&&` or `||` and align with the opening parenthesis. Long parameter
  lists are wrapped and aligned to the first parameter.
- `&` and `*` attach to the type: `const Config& config`, `T* data_`.
- Constructor initializer lists start on the next line with an indented `: `, one member per line
  when they are long.
- Lookup tables are aligned in columns and pack several entries per line. Trailing comments in
  structs are aligned too:
  ```cpp
  bool first = false;       // True if it's the first token in a sentence.
  bool whitespace = false;  // True if there is trailing whitespace implicitly included.
  ```
- Leave a blank line after the opening `namespace …{` line and before the closing one. Inside a
  function, logical blocks are separated by a blank line plus a comment.
- There is no hard column limit; lines of about 120–130 characters are common.

---

## 6. Comments and documentation

- **Public API in headers** uses Javadoc-style `/** … */` with `@param`, `@returns` / `@return`,
  `@details`, `@throws`, `@tparam` and `@ref`.
  - The first sentence is brief and the details follow.
  - Parameter descriptions are short, often lowercase, often with an article:
    `@param text an input text (utf-8) to be processed.`
- Simple members get a single `//` line, or nothing:
  `// Performs a preprocessing stage of the pipeline.`. Groups of overrides sit under
  `// Base class overrides`.
- **Voice:**
  - Descriptions often start with an article: "A lexicon based phonemizer, which utilizes …",
    "A shortcut for base class calls", "A (word | context) -> phonemes mapping.", "An interface
    which allows to dynamically resolve pipelines …".
  - Rationale and assumptions use "we": "We store both keys and values as utf-8 strings to save
    memory comparing to u32string keys".
  - Comments are short (1–3 lines), practical and candid: `// A hack to omit the leading
    whitespaces`, `// Heuristic allocation`, `// WARNING: This file is called 'big szpont'`.
  - `// NOTE:` marks behavioural caveats.
- **Numbered steps** in algorithms and setup code:
  - `// 1. Context Update`, `// 2. Core Phonemization`, `// 3. Fallback/Punctuation Logic`,
    `// 4. Whitespace handling`
  - `// Step 0: Initialization`
  - Pipelines: `// 1. Setup Preprocessing layers`, `// 2. Setup PoS tagger if specified`;
    `// 1. Tokenize`, `// 2. Apply Part-of-Speech tags`,
    `// 3. Generate phonemes (Lexicon with Neural fallback)`.
- **Examples inside comments:** `// xyz'abc -> xyz, 'abc (if xyz not empty)`,
  `// '110xxxxx' pattern`, `(e.g., "it'", "s")`.
- External references are bare URLs: `// https://en.wiktionary.org/wiki/-ed`.
- Non-ASCII literals are annotated with a name and code point:
  `U'।', // Hindi Purna Viram (U+0964)`.
- **Section banners:**
  - utilities: `// ----------------------` / `// ----- Predicates -----` / `// ----------------------`
  - language constants: `// --- Number to Word Normalization ---`
  - long class bodies: `// ---- Weight loading ----…`
- Utility headers have a doc block above the namespace describing the whole module
  (`/** Type conversions … */`,
  `/** String utilities. … NOTE: Inline (mutable) versions … trailing __. */`).
- In dense declaration lists, end-of-line comments describe each item: the `lookup_*`
  declarations in `en/lexicon_phonemizer.h`, and buffer shapes in protophone (`// S x L`).

---

## 7. C++ idioms and technical choices

- **C++20, used where it helps:**
  - concepts (`std::derived_from`), `std::span`, `contains()`
  - designated initializers (`{.data_filepath = argv[++i]}`)
  - structured bindings (`for (const auto& [key, value] : json_obj.items())`)
  - `if` with an initializer (`if (auto it = m.find(c); it != m.end())`)
  - `if constexpr`, and CTAD (`std::basic_string_view suffix_view{suffix}`)
  - Algorithms: the author's own code mostly uses classic iterator pairs (`std::find_if`,
    `std::any_of(begin, end, …)`) plus some `std::ranges::*`. Both are fine.
- **Text model: UTF-32 inside, UTF-8 at the edges.**
  - Internal text is `std::u32string`, `std::u32string_view`, `char32_t`, `U'x'` and `U"…"`.
  - UTF-8 appears only in the public API overloads, JSON loading and lexicon storage.
    Dictionaries store UTF-8 keys and values to save memory and convert on lookup.
- **No dependencies:**
  - Unicode handling is in-house: a lookup table (`unicode_table.h`) plus `constexpr inline`
    predicates (`unicode::isalpha`, `tolower`, …), not `<locale>`.
  - Generic helpers use a paired `using` so the same code works for `char` and `char32_t`:
    `using std::isalpha; using unicode::isalpha;`.
- **Parameter passing:**
  - `std::u32string_view` for read-only text.
  - `const std::u32string&` when the callee needs a real string (map keys, `substr` plus
    concatenation).
  - `std::span<const Token>` vs `std::span<Token>` to signal read-only vs in-place.
  - Small types by value. Return values by value.
- **Ownership:**
  - Value members for parts that are always present.
  - `std::unique_ptr<T> x_ = nullptr` (created with `std::make_unique`) for optional or
    polymorphic parts.
  - Raw `const T*` for static tables owned elsewhere, always null-checked
    (`rules_ && rules_->contains(c)`).
- **Class hygiene:** `explicit` on one-argument constructors, `virtual ~Base() = default;`,
  `override` on every override, `= default` where possible, and `const` on every query method.
- **Constants:**
  - Tables are `inline const` containers in headers: `std::unordered_map` / `unordered_set`;
    `std::map` where order matters (number scales); `std::vector` for index-addressed lists
    (months, with `U""` at index 0); `std::array` for fixed-size sets.
  - Scalars use `inline constexpr`.
  - A constant derived from another is built with an immediately invoked lambda:
    ```cpp
    inline const std::unordered_set<char32_t> kPunctuations = [] {
      auto s = kNonQuotePunctations;
      s.insert(U'"');
      return s;
    }();
    ```
  - A small lookup string used in one function is a function-local `static const`.
- **Optionals:** `has_value()` / `value()` and `*opt` are both used. `value_or("")` supplies
  defaults, and returns write `std::make_optional(...)` explicitly.
- **Performance defaults for mobile/edge CPUs:**
  - `reserve()` with a rough size before loops (`tokens.size() * 5 // Heuristic allocation`,
    `input.size() / 5`).
  - Offset-and-length views instead of substring copies.
  - Flat vectors and dense ids on hot paths.
  - SIMD-aligned allocators and scratch buffers that only grow, in the neural engine.
- **`using`:**
  - A `.cpp` may start with `using namespace utils;` inside the project namespace, then call
    `strings::…`, `conversions::…`, `unicode::…`.
  - Functions that use many constants add a function-scope `using namespace …::constants;` or a
    targeted `using …::kPrimary;`.
  - Headers only have targeted declarations (`using tokenizer::Token;`, `using tagger::Tag;`).
- Small predicates are inline lambdas, often with an explicit `-> bool`.
- Casts are explicit `static_cast`, and indices are `size_t`.
- Unused parameters keep their names in comments: `size_t /*curr_token_id*/`.
- Macros are used only to generate code (non-inplace wrappers, test registration).

---

## 8. Error handling and robustness

- **Throw only during construction and resource loading:**
  - A missing required config throws `std::runtime_error`, formatted as
    `"<Class>: <field> must be provided in the configuration."`.
  - Bad arguments or malformed data throw `std::invalid_argument` (unsupported language, invalid
    JSON, wrong value type in the lexicon or tokenizer JSON).
  - I/O failures throw `std::runtime_error`.
- Validate once at load time, then trust the data on the hot path. For example, the lexicon is
  checked to be string → string in the constructor.
- **Phonemization never throws:**
  - An unknown word gives `std::nullopt` or `U""`, and the next fallback takes over.
  - Characters not in the tokenizer mapping are skipped silently, with a `// NOTE:` saying so.
  - Words that are too long are split recursively.
  - Invalid UTF-8 bytes become U+FFFD.
- Exceptions are caught only at the outermost layer (`main.cpp`), or around third-party parsing,
  where they are rethrown as `std::invalid_argument` with context added.

---

## 9. Not the author's voice: do not imitate

These patterns are in the tree but come from the external refactor (`c8d8744`, `36cef09`) or from
the AI-generated num2word code:
- Paragraph-long rationale comments on members. Examples: `token_to_char_` in `nn/tokenizer.h`,
  the probability tables in `tagger/hmm_tagger.h`, and `phonemize_words` in `ipipeline.cpp`.
- Phrases such as "matching the previous behaviour", "mirroring the original behavior", "hot path",
  "pure scanning: it does not mutate any state".
- Em dashes (—) and textbook grammar explanations in comments (non-English `constants.h`).
- Helpers split out for a single condition (`at_word_end`, `is_trailing_currency`).

The author's own version of these is a 1–3 line comment saying what the code does (and briefly
why), numbered steps, and an inline example.

---

## 10. Known inconsistencies

Use the dominant form in new code. Do not "fix" these in unrelated changes.
- Indentation: 2 spaces dominant; tabs and 4 spaces in legacy files.
- Constant naming: `kName` dominant; SCREAMING in `nn/constants.h` and `LOG_EPSILON`.
- Some closing namespace comments are stale (`} // namespace phonemis::tagger::constants` in
  `tokenizer/constants.h`, `} // namespace phonemis::utilities::conversions`, `} // phonemis::en`).
  Always write the correct full name.
- `@return` and `@returns` are both used. Some `@param` names are stale (`tokenVec` vs `token_vec`).
- `i++` and `++i` are both used. Many files have no trailing newline at EOF.

---

## 11. Checklists

### Adding a language `xx`
1. Create `lang/xx/constants.h`, using `namespace phonemis::xx::constants`:
   - `// --- Number to Word Normalization ---` → `namespace num2word { kCardinals, kLargeCardinals (std::map), kCurrencies, kMonths (index 0 = U""), kOrdinalSuffixes }`
   - `// --- Tokenization Rules ---` → `namespace tokenizer { using ::phonemis::tokenizer::split::Rule; using …::Exceptions; kSpecialCharacters; kExceptions }`
2. Create `lang/xx/num2word.h/.cpp`:
   - Declare `class Num2Word : public processor::num2word::Num2WordLayer` with
     `using Num2WordLayer::Num2WordLayer;`.
   - Define `decimal_separator()` inline in the header.
   - Declare the remaining primitives with `override`.
3. Create a header-only `lang/xx/pipeline.h` following this skeleton:
   ```cpp
   namespace phonemis::xx {

   class Pipeline : public IPipeline {
   public:
     explicit Pipeline(const Config& config)
         : tokenizer_(&constants::tokenizer::kSpecialCharacters,
                      &constants::tokenizer::kExceptions),
           phonemizer_(config.phonemizer) {

       // 1. Setup Preprocessing layers
       preprocessor_.add_layer(std::make_unique<processor::trim::TrimLayer>());
       preprocessor_.add_layer(std::make_unique<Num2Word>());
     }

     // Performs a preprocessing stage of the pipeline.
     std::u32string preprocess(const std::u32string& input) override {
       return preprocessor_.process(input);
     }

     // Performs a middle stage of the pipeline - including phonemization.
     std::u32string process(const std::u32string& input) override {
       // 1. Tokenize
       auto tokens = tokenizer_.tokenize(input);

       // 2. Generate phonemes (Lexicon with Neural fallback)
       return phonemizer_.phonemize(tokens);
     }

     // Performs a postprocessing stage of the pipeline.
     std::u32string postprocess(const std::u32string& input) override {
       // No postprocessing
       return input;
     }

   private:
     // Required submodules
     processor::Preprocessor preprocessor_;
     tokenizer::Tokenizer tokenizer_;
     phonemizer::HybridPhonemizer<phonemizer::LexiconPhonemizer, phonemizer::nn::NeuralPhonemizer> phonemizer_;
   };

   } // namespace phonemis::xx
   ```
4. Register the language in `base/pipeline.cpp`: add `#include <phonemis/lang/xx/pipeline.h>` and
   `if (config.lang == "xx") {  // Name` returning `std::make_unique<xx::Pipeline>(config)`.
5. Put data in `data/xx/`, and add tests (`test/num2word_xx_test.cpp`, …).

### Adding a component to `base/`
- Write an abstract base whose non-virtual driver loops over units and calls a pure virtual,
  `const` per-unit hook. Add `virtual ~X() = default;`.
- Put `struct Config` in `config.h`. Document every field; optional resources are
  `std::optional<std::string> …_filepath = std::nullopt`, and a missing one disables the feature.
- Write the concrete implementation as `<impl>_<component>.h/.cpp` with
  `explicit X(const Config& config);`. The constructor throws if a required config value is
  missing and validates loaded data once.
- Put tables in `constants.h` (`kName`, `inline const`), and enums and aliases in `types.h`.
- Plug the component into the language pipelines as a value member if required, or as a
  `unique_ptr` (`= nullptr`) if optional.

---

## Appendix: tests (`test/`)
- The harness is in-house and has no dependencies (`test/test.h`).
  - `REGISTER_TEST(name) { …; return true; }` defines a test.
  - `ASSERT_EQUALS(expected, real)` prints a coloured message with file and line, then returns
    `false`.
  - A single runner lives in `test/test.cpp`.
- There is one file per component and variant, named `<component>_<base|lang>_test.cpp`
  (`tokenizer_base_test.cpp`, `num2word_pl_test.cpp`).
- Test names follow `<component>_<variant>_<scenario>_test`
  (`tokenizer_base_basic_tokenization_test`).
- Test code is older and formatted differently: 4-space indent, function braces on a new line, and
  helper wrapper structs with `operator==` / `operator<<` for readable failures.
