import argparse
import json
import sys
from typing import Any, Dict

# Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("--input", required=True, help="Input .json file path")
parser.add_argument("--output", required=True, help="Output .json file path")
args = parser.parse_args()

input_file = args.input
output_file = args.output

# Load JSON file
try:
    with open(input_file, "r", encoding="utf-8") as f:
        data: Dict[str, Any] = json.load(f)
except Exception as e:
    print(f"Failed to load JSON from {input_file}: {e}", file=sys.stderr)
    sys.exit(1)

# Process entries: replace dict values that contain a "DEFAULT" string with that string
replaced_count = 0
total_count = 0

for key, value in list(data.items()):
    total_count += 1
    if isinstance(value, dict):
        default_value = value.get("DEFAULT")
        if isinstance(default_value, str):
            data[key] = default_value
            replaced_count += 1

# Save processed JSON
try:
    with open(output_file, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
except Exception as e:
    print(f"Failed to write JSON to {output_file}: {e}", file=sys.stderr)
    sys.exit(1)

# Print brief summary
print(f"Total keys processed: {total_count}")
print(f"Replaced entries with DEFAULT: {replaced_count}")
print(f"Saved cleaned dictionary to: {output_file}")
