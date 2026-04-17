#!/bin/bash

# Configuration
BINARY="./build/phonemis"

if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found at $BINARY. Please build the project first."
    exit 1
fi

if [ $# -lt 2 ]; then
    echo "Usage: $0 <lang_code> [options] <text>"
    echo "Options:"
    echo "  --lexicon <path>     Path to lexicon file"
    echo "  --model <path>       Path to neural model file"
    echo "  --tagger <path>      Path to tagger data file"
    exit 1
fi

LANG=$1
shift

# Run the program
"$BINARY" --lang "$LANG" "$@"
