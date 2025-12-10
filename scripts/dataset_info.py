import argparse
from tqdm import tqdm
from collections import Counter

# Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("--input", required=True, help="Input dataset file path")
args = parser.parse_args()

input_file = args.input

# Sets for unique words and tags
unique_words = set()
unique_tags = set()

# Word frequency counter
word_counter = Counter()

# Count total lines
with open(input_file, "r", encoding="utf-8") as f:
    lines = f.readlines()

# Process lines and collect stats
for line in tqdm(lines, desc="Processing lines"):
    parts = line.strip().split()
    if len(parts) < 2:
        continue
    word, tag = parts[0], parts[1]
    unique_words.add(word)
    unique_tags.add(tag)
    word_counter[word] += 1

# Words with at least 2 and 5 occurrences
words_at_least_2 = sum(1 for count in word_counter.values() if count >= 2)
words_at_least_5 = sum(1 for count in word_counter.values() if count >= 5)

# Print stats
print(f"Total lines: {len(lines)}")
print(f"Unique words: {len(unique_words)}")
print(f"Words occurring at least 2 times: {words_at_least_2}")
print(f"Words occurring at least 5 times: {words_at_least_5}")
print(f"Unique tags: {len(unique_tags)}")
print(f"Tags: {', '.join(sorted(unique_tags))}")
