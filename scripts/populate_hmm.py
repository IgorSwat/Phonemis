import argparse
import json
import os
from collections import defaultdict
from tqdm import tqdm

# Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("--input", required=True, help="Input dataset file path")
parser.add_argument("--output", required=True, help="Output JSON filepath for HMM data")
args = parser.parse_args()

input_file = args.input
output_file = args.output

# Sentence end tokens
END_TOKENS = {".", "?", "!", ";"}

# Data containers
sentences = []
current_sentence = []

# Read dataset and build sentences
with open(input_file, "r", encoding="utf-8") as f:
    lines = f.readlines()
    after_end_token = False
    for line in tqdm(lines, desc="Reading dataset"):
        parts = line.strip().split()
        if len(parts) < 2:
            continue
        word, tag = parts[0], parts[1]
        current_sentence.append((word, tag))
        if word in END_TOKENS and len(word) == 1:
            if not after_end_token:
                sentences.append(current_sentence)
            else:
                sentences[-1].append((word, tag))
            current_sentence = []
            after_end_token = True
        else:
            after_end_token = False
    if current_sentence:
        sentences.append(current_sentence)

# HMM probability containers
transition = defaultdict(lambda: defaultdict(int))
emission = defaultdict(lambda: defaultdict(int))
start_prob = defaultdict(int)
tag_counts = defaultdict(int)

# Calculate HMM probabilities
for sentence in sentences:
    prev_tag = None
    for i, (word, tag) in enumerate(sentence):
        tag_counts[tag] += 1
        emission[tag][word] += 1
        if i == 0:
            start_prob[tag] += 1
        else:
            transition[prev_tag][tag] += 1
        prev_tag = tag

# Helper - normalize dictionary
def normalize(d):
    total = sum(d.values())
    return {k: v / total for k, v in d.items()} if total > 0 else {}

# Normalize probabilities
start_prob = normalize(start_prob)
for tag in emission:
    emission[tag] = normalize(emission[tag])
for prev in transition:
    transition[prev] = normalize(transition[prev])

# Ensure directory for output file exists
os.makedirs(os.path.dirname(os.path.abspath(output_file)) or ".", exist_ok=True)

# Save single JSON containing all HMM dicts
hmm_data = {
    "start_prob": start_prob,
    "emission": emission,
    "transition": transition
}

with open(output_file, "w", encoding="utf-8") as f:
    json.dump(hmm_data, f, ensure_ascii=False, indent=2)
