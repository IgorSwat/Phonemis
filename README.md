# Phonemis

## From Text to Sound

Phonemis is a high-performance C++ library for **Grapheme-to-Phoneme (G2P)** conversion. It
delivers universal **IPA phonemization** across many platforms with **zero external
dependencies**, making it an ideal frontend for text-to-speech systems on desktop, mobile, and
embedded devices.

Currently supported languages:

*  🇺🇸 English (US) — `en-us`
*  🇬🇧 English (British) — `en-gb`
*  🇩🇪 German — `de`
*  🇫🇷 French — `fr`
*  🇪🇸 Spanish — `es`
*  🇮🇹 Italian — `it`
*  🇵🇱 Polish — `pl`
*  🇵🇹 Portuguese — `pt`
*  🇮🇳 Hindi — `hi`

## Repository Structure

```
phonemis/
├── data/                          # Bundled resources (one subdirectory per language)
├── src/
│   ├── phonemis/
│   │   ├── base/                  # Core API: Pipeline, IPipeline, Config, phonemizer, tokenizer
│   │   ├── lang/                  # Language-specific modules (en/, de/, fr/, …)
│   │   ├── protophone/            # Pure C++ neural model inference engine
│   │   └── utils/                 # Conversions, string utilities, Unicode support
│   └── third-party/               # Bundled header-only dependencies (xsimd)
└── CMakeLists.txt
```

Every language folder under `data/` contains the resources needed for phonemization — a lexicon
JSON file (word-to-phoneme dictionary), a Protophone model weights file (`.bin`) for neural
inference, and optionally a tagger JSON file for part-of-speech disambiguation (currently
available for English). These are the files you point to in `phonemizer::Config`.

## Installation

### Requirements
- **C++20** compiler
- **CMake** ≥ 3.10
- **xsimd** (bundled as a header-only library under `src/third-party/`)

### Building the Library

The below script builds the package as a static library:

```bash
mkdir build
cd build
cmake ..
make
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `-DBUILD_RUNNER=ON` | OFF | Builds the `phonemis_runner` CLI tool |
| `-DBUILD_TESTS=ON` | OFF | Builds the test suite |
| `-DET_ON=ON` | OFF | Enables ExecuTorch inference (see below) |

### ExecuTorch Integration

With `ET_ON=ON`, the built-in Protophone sources are excluded and neural phonemization is
delegated to ExecuTorch. You must link against the ExecuTorch runtime:

```bash
cmake .. -DET_ON=ON -DCMAKE_PREFIX_PATH=/path/to/executorch
make
```

When `ET_ON` is left OFF (the default), all neural inference runs through the built-in
Protophone model — no external runtime dependencies required.

### SIMD Acceleration

SIMD is configured automatically at build time:
- **x86_64 desktop** (non-Emscripten, non-Android, non-iOS): AVX2 + FMA are enabled if the
  compiler supports them (`-mavx2 -mfma`), accelerating Protophone's convolution and mixing
  stages.
- **ARM** (Android arm64-v8a, iOS arm64): NEON is implicitly available and used by xsimd with
  no extra flags.

### Mobile Builds

Dedicated scripts are provided for cross-compiling:
- **Android**: Produces `.a` libraries for armeabi-v7a, arm64-v8a, x86, and x86_64 ABIs.
- **iOS**: Produces a universal static library or framework.

## Sample Usage

All lexicons and trained neural model weights are bundled in the `./data/` subdirectory, organized
by language code. The example below uses the English (US) resources:

```cpp
#include <phonemis/base/pipeline.h>
#include <phonemis/utils/conversions.h>
#include <iostream>

int main() {
    using namespace phonemis;

    phonemizer::Config phon_config;
    phon_config.lang = "en-us";
    phon_config.lexicon_filepath = "data/en-us/lexicon_full.json";
    phon_config.nn_model_filepath = "data/en-us/phonemizer_en_us.bin";

    Config config;
    config.lang = "en-us";
    config.phonemizer = phon_config;

    Pipeline pipeline(config);

    std::string text = "I love it! This is the best day of my entire life.";
    auto phonemes = pipeline(text);

    std::cout << "Text: " << text << "\n";
    std::cout << "Phonemes: " << utils::conversions::u32_to_utf8(phonemes) << "\n";

    return 0;
}
```

The `Pipeline::operator()` accepts both `std::string_view` (UTF-8) and `std::u32string_view`
(UTF-32), returning phonemes as `std::u32string`. Individual pipeline stages —
`preprocess()`, `process()`, `postprocess()` — are also exposed.

### CLI Runner

```bash
./build/phonemis_runner --lang en-us \
    --lexicon data/en-us/lexicon_full.json \
    --model data/en-us/phonemizer_en_us.bin \
    "Hello world"
```

## The Mechanics of Pronunciation

Phonemis combines three complementary phonemization strategies, selected per word to maximize
accuracy:

### Rule-based

A language-aware tokenizer segments text using configurable rules for words, punctuation, and
special characters. A number-to-word layer verbalizes digits, dates, currencies, fractions, and
ordinals into their textual forms according to language-specific conventions (e.g., German
"einundzwanzig" unit-before-tens order, comma decimal separator).

### Lexicon (lookup-based)

An O(1) dictionary lookup provides the fastest and most reliable phonemization. It is cheap, can
be prepared offline, and handles exception words like "read" (present vs. past tense) correctly.
Other languages can be extended simply by providing a lexicon JSON file.

### Neural (Protophone)

For words not found in the lexicon, a neural phonemizer takes over. **Protophone** is a pure
C++ CTC-based model with SIMD-accelerated inference via xsimd (AVX2+FMA on x86_64, NEON on ARM).
When built with `ET_ON=ON`, inference is delegated to **ExecuTorch** instead, enabling optimized
on-device acceleration.

These strategies are combined through a **hybrid phonemizer**: the lexicon is tried first, and
the neural model serves as a fallback. Each level can be individually enabled or disabled via
configuration.

### Extending to a New Language

Phonemis is designed to be contribution-friendly. Adding support for a new language requires
three files under `src/phonemis/lang/XX/`:

| File | Purpose | Requirement |
|------|---------|-------------|
| `constants.h` | Language-specific data: number words (cardinals, ordinals), currency names, months, tokenizer special-character rules. | Mandatory |
| `num2word.h` / `num2word.cpp` | Number-to-word converter extending `processor::num2word::Num2WordLayer`. Implements verbalization rules (decimal separator, ordinal suffixes, currency handling, year pronunciation). | Mandatory |
| `pipeline.h` | Pipeline class extending `IPipeline`. Orchestrates preprocessing layers (trimming, number verbalization), a tokenizer, and a `HybridPhonemizer<LexiconPhonemizer, NeuralPhonemizer>`. | Mandatory |
| Lexicon JSON / Protophone model | Word-to-phoneme mappings **or** trained neural model (`.bin`). | At least one required |

Then register the language in the factory method at `src/phonemis/base/pipeline.cpp`.
