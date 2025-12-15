import argparse
import json
import sys
from typing import Any, Dict

# Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("--primary", required=True, help="Primary dictionary .json file (preferred on conflicts)")
parser.add_argument("--secondary", required=True, help="Secondary dictionary .json file")
parser.add_argument("--output", required=True, help="Output merged .json file")
args = parser.parse_args()

primary_path = args.primary
secondary_path = args.secondary
output_path = args.output

# Load JSON files
def load_json(path: str) -> Dict[str, Any]:
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        print(f"Failed to load JSON from {path}: {e}", file=sys.stderr)
        sys.exit(1)

primary_dict = load_json(primary_path)
secondary_dict = load_json(secondary_path)

# Merge dictionaries: prefer primary values on conflicts
merged_dict: Dict[str, Any] = {}

# Add secondary entries first
for key, value in secondary_dict.items():
    merged_dict[key] = value

# Override/add with primary entries
conflict_count = 0
for key, value in primary_dict.items():
    if key in merged_dict:
        conflict_count += 1
    merged_dict[key] = value

# Save merged JSON
try:
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(merged_dict, f, ensure_ascii=False, indent=2)
except Exception as e:
    print(f"Failed to write JSON to {output_path}: {e}", file=sys.stderr)
    sys.exit(1)

# Summary
print(f"Primary keys: {len(primary_dict)}")
print(f"Secondary keys: {len(secondary_dict)}")
print(f"Merged keys: {len(merged_dict)}")
print(f"Conflicts (resolved in favor of primary): {conflict_count}")
print(f"Saved merged dictionary to: {output_path}")
