"""
This script concatenates multiple flattened lexicon JSON files into a single JSON file.
It ensures that only unique keys are preserved and that the final output is sorted by key.

Usage:
    python concatenate_lexicons.py --inputs lexion1.json lexicon2.json --output concatenated.json
"""

import json
import argparse
from typing import List, Dict

def concatenate_lexicons(input_paths: List[str], output_path: str):
    combined_lexicon: Dict[str, str] = {}

    for path in input_paths:
        with open(path, 'r', encoding='utf-8') as f:
            data = json.load(f)
            # This naturally handles uniqueness by overwriting previous entries for the same key.
            # If multiple files have the same word, the one from the later file in --inputs wins.
            combined_lexicon.update(data)

    # Sort the dictionary by keys
    sorted_lexicon = {k: combined_lexicon[k] for k in sorted(combined_lexicon.keys())}

    with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(sorted_lexicon, f, indent=2, ensure_ascii=False)

def main():
    parser = argparse.ArgumentParser(description="Concatenate multiple lexicon JSON files.")
    parser.add_argument("--inputs", nargs='+', required=True, help="List of input JSON files to concatenate.")
    parser.add_argument("--output", required=True, help="Path to the output concatenated JSON file.")
    
    args = parser.parse_args()
    
    concatenate_lexicons(args.inputs, args.output)

if __name__ == "__main__":
    main()
