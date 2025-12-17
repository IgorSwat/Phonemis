# Phonemis

## From Text to Sound
Phonemis is a high-performance C++ library designed for **Grapheme-to-Phoneme (G2P)** conversion. It provides a robust pipeline for transforming raw text into *phonetic transcriptions* using the **International Phonetic Alphabet (IPA)**. The library is optimized for efficiency and portability, being pure C++ with no external dependencies, which makes it easy to implement on a wide range of systems including standard and mobile applications requiring text-to-speech frontend processing.

Currently supported languages:
*   🇺🇸 English (US)
*   🇬🇧 English (British)

## The Mechanics of Pronunciation
The phonemization pipeline consists of several distinct stages designed to maximize accuracy and context awareness:

1.  **Preprocessing**: Raw input text is normalized to handle encoding issues and standard formatting.
2.  **Rule-based Tokenizer**: The text is segmented into tokens based on linguistic rules, separating words from punctuation and handling special cases.
3.  **Part-of-Speech Tagging**: A Hidden Markov Model (HMM) bigram tagger is employed to assign grammatical categories to words. This model is trained on the Brown Corpus to resolve homograph ambiguities based on context.
4.  **Viterbi Decoding**: The optimal sequence of tags is determined using the [Viterbi algorithm](https://en.wikipedia.org/wiki/Viterbi_algorithm), ensuring the most probable grammatical structure is selected.
5.  **Lexicon-based Phonemization**: Words are converted to phonemes using extensive dictionaries, with fallback mechanisms for unknown tokens.

This library is inspired by the Python package [misaki](https://github.com/hexgrad/misaki).

## Installation

### Building with CMake
Phonemis uses CMake as its build system. To build the static library:

```bash
mkdir build
cd build
cmake ..
make
```

### Mobile Builds
The repository includes dedicated scripts for cross-compiling the library for mobile platforms:
*   **Android**: Use the provided Android build script to generate `.a` libraries for various ABIs (armeabi-v7a, arm64-v8a, x86, x86_64).
*   **iOS**: Use the iOS build script to generate a universal static library or framework.

## Sample Usage

Below is a minimalistic example demonstrating how to instantiate the pipeline and process text.

```cpp
#include <phonemis/pipeline.h>
#include <phonemis/utilities/string_utils.h>
#include <iostream>

using namespace phonemis;
using namespace phonemis::utilities;

int main() {
    // Paths to required data files
    std::string tagger_path = "../data/hmm.json";
    std::string lexicon_path = "../data/dictionaries/us_merged.json";

    // Initialize pipeline for US English
    Pipeline pipeline(Lang::EN_US, tagger_path, lexicon_path);

    // Process text
    std::string text = "I love it! This is the best day of my entire life.";
    auto phonemes = pipeline.process(text);

    // Output result
    std::cout << "Text: " << text << "\n";
    std::cout << "Phonemes: " << string_utils::u32string_to_utf8(phonemes) << "\n";

    return 0;
}
```
