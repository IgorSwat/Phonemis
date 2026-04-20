"""
This script transforms a nested lexicon JSON file into a flattened JSON format.
Input: A JSON file where keys are words and values are either phoneme strings or maps of tags to phoneme strings.
Output: A JSON file where all keys are strings (word or word|tag) and values are phoneme strings.

Usage:
    python create_lexicon.py --input lexicon.json --output flattened.json [--normalize-uppercase]
"""

import json
import argparse
from typing import Dict, Union

def flatten_lexicon(input_path: str, output_path: str, normalize_uppercase: bool):
    with open(input_path, 'r', encoding='utf-8') as f:
        data: Dict[str, Union[str, Dict[str, str]]] = json.load(f)

    flattened = {}

    for word, value in data.items():
        # Handle tags or direct phoneme strings
        entries = {}
        if isinstance(value, dict):
            for tag, phonemes in value.items():
                if not phonemes:
                    continue
                if tag == "DEFAULT":
                    entries[word] = phonemes
                else:
                    entries[f"{word}|{tag}"] = phonemes
        elif value:
            entries[word] = value

        # Update main lexicon
        flattened.update(entries)

        # Handle normalization if flag is set
        if normalize_uppercase and any(c.isupper() for c in word):
            lower_word = word.lower()
            if isinstance(value, dict):
                for tag, phonemes in value.items():
                    if not phonemes:
                        continue
                    if tag == "DEFAULT":
                        flattened[lower_word] = phonemes
                    else:
                        flattened[f"{lower_word}|{tag}"] = phonemes
            elif value:
                flattened[lower_word] = value

    with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(flattened, f, indent=2, ensure_ascii=False)

def main():
    parser = argparse.ArgumentParser(description="Flatten a lexicon JSON file.")
    parser.add_argument("--input", required=True, help="Path to the input JSON file.")
    parser.add_argument("--output", required=True, help="Path to the output JSON file.")
    parser.add_argument("--normalize-uppercase", action="store_true", default=False, help="Duplicate uppercase entries as lowercase.")
    
    args = parser.parse_args()
    
    flatten_lexicon(args.input, args.output, args.normalize_uppercase)

if __name__ == "__main__":
    main()
