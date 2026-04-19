# Project Instructions

## Project Overview

This is a G2P (grapheme to phoneme) C++ library focusing on high-performance CPU phonemization for mobile & edge devices.
The library targets many languages, and one of it's main goals is to provide a high flexibility in adding support for a new, not yet implemented langauge phonemization, with minimal development effort.

---

## Quick Reference

### Running Commands
```bash
# Utility scripts
./scripts/run_tests.sh # Builds the entire library (including tests) and run all tests
./scripts/run_phonemizer.sh <LANG> --lexicon <PATH> --tagger <PATH> --model <PATH> <TEXT> # Runs the phonemization for given input and language.
./scripts/build_android.sh # Builds the static lib for Android ARM systems
./scripts/build_ios.sh # Builds the static lib for iOS mobile systems
```

---

### Project Structure

```
data/ # Data files (lexicons, precomputed tables, model weights, etc.)
├── english/  # English pipeline data
├── ...

scripts/   # Utility scripts for building & running the library

src/
├── phonemis/
    ├── base/ # General implementations of phonemization submodules (language-agnostic) - with interfaces & abstract base classes
    ├── lang/ # Language-specific implementations of concrete submodules
    ├── utils/ # Shared utilities, including strings & IO utilities
├── third-party/ # Third-party libraries
```

---

## Code Quality Standards

### File Organization & Size
- Each file should resolve around one specific functionality, like one class implementation or semantically connected set of functions
- Prioritize logical cohesion and single responsibility over strict line counts

#### When to split files:
- Multiple classes/functions with different responsibilities
- Mixed concerns (business logic + configuration + utilities)
- Difficulty understanding the file's purpose at a glance

#### When larger files are acceptable:
- Configuration files with extensive settings
- Complex workflow definitions that need to stay together
- Single-purpose modules with high internal cohesion
