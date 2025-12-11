import argparse
import json
import os

# Arguments
parser = argparse.ArgumentParser()
parser.add_argument("--input", required=True, help="Input JSON filepath with HMM data")
args = parser.parse_args()

input_file = args.input

# Load single HMM JSON
with open(os.path.join(input_file), "r", encoding="utf-8") as f:
    hmm_data = json.load(f)

start_prob = hmm_data.get("start_prob", {})
emission = hmm_data.get("emission", {})
transition = hmm_data.get("transition", {})

# States
states = list(emission.keys())

def lowerize(s):
    return s[:1].lower() + s[1:]

def capitalize(s):
    return s[:1].upper() + s[1:]

# Viterbi algorithm
def viterbi(sentence, states, start_p, trans_p, emit_p):
    V = [{}]
    path = {}
    for state in states:
        V[0][state] = start_p.get(state, 0) * emit_p.get(state, {}).get(sentence[0], 1e-6)
        path[state] = [state]
    for t in range(1, len(sentence)):
        V.append({})
        new_path = {}
        for curr_state in states:
            max_prob, prev_state = max(
                (V[t - 1][y0] * trans_p.get(y0, {}).get(curr_state, 1e-6) * emit_p.get(curr_state, {}).get(sentence[t], 1e-6), y0)
                for y0 in states
            )
            V[t][curr_state] = max_prob
            new_path[curr_state] = path[prev_state] + [curr_state]
        path = new_path
    n = len(sentence) - 1
    prob, final_state = max((V[n][y], y) for y in states)
    return path[final_state]

# Example words (existing test set)
# example_words = [
#     "dog", "cat", "run", "walk", "quickly", "slowly", "beautiful", "ugly", "happiness", "sadness",
#     "car", "drive", "road", "city", "country", "eat", "drink", "water", "food", "computer",
#     "keyboard", "mouse", "screen", "phone", "call", "message", "read", "write", "book", "letter",
#     "music", "song", "dance", "jump", "play", "game", "work", "job", "school", "student",
#     ",", ".", "!", "?", ";", "n't", "'s", "yolo", "(", ")", "[", "Emma", "Michael"
# ]
example_words = [
    "My", "Halloween", "was", "a", "very", "nice", "thing", "!"
]

# Print results for each word
for word in example_words:
    sentence = [word]
    tags = viterbi(sentence, states, start_prob, transition, emission)
    result_tag = tags[0]
    print(f"{word}: \033[92m{result_tag}\033[0m")
