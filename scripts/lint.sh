#!/bin/bash

# Lints the C++ sources under src/phonemis: formatting (clang-format), identifier naming (clang-tidy)
# and comment length. By default only lines changed since HEAD (staged, unstaged, untracked) are checked.
#
# Usage: ./scripts/lint.sh [--fix] [--all] [--base <rev>] [<file>...]
#   --fix         applies clang-format to the checked lines first
#   --all         checks every line, not only the changed ones
#   --base <rev>  checks changes since the merge-base with <rev> (e.g. main)
#
# Requires python3 and LLVM >= 18 (clang-format, clang-tidy).

exec python3 "$(dirname "$0")/tools/lint/lint.py" "$@"
