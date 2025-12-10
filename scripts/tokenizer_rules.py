import argparse
import re
from tqdm import tqdm

# Constants - standard token pattern
STANDARD_TOKEN_PATTERN = re.compile(r"^([a-zA-Z0-9]+|[^a-zA-Z0-9\s])$")

if __name__ == "__main__":

    # Argument parsing
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, help="Input file path (word tag format)")
    parser.add_argument("--output", required=True, help="Output file path for non-standard rules")
    args = parser.parse_args()

    input_file = args.input
    output_file = args.output

    # Non-standard tokens set
    non_standard_tokens = set()

    with open(input_file, "r", encoding="utf-8") as f:
        lines = f.readlines()
        for line in tqdm(lines, desc="Processing lines"):
            parts = line.strip().split()
            if not parts:
                continue
            word = parts[0]

            # Add words with apostrophe to non-standard
            if "'" in word:
                non_standard_tokens.add(word)
                continue

            # Add words not matching standard pattern
            if not STANDARD_TOKEN_PATTERN.match(word):
                non_standard_tokens.add(word)

    # Save non-standard tokens
    with open(output_file, "w", encoding="utf-8") as f:
        for token in sorted(list(non_standard_tokens)):
            f.write(f"{token}\n")
