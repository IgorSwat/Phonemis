import json
import argparse
import ast
import spacy
import sys
from collections import Counter, defaultdict
from tqdm import tqdm
from nltk.corpus import brown

def preprocess_word(word, tag):
    """
    Preprocesses a word and its tag based on the provided rules.
    """
    # Lowercase word unless it's a proper noun or "I" (PRP)
    if tag not in ["NNP", "NNPS"] and not (tag == "PRP" and word == "I"):
        word = word.lower()
    return word

def get_tagged_sentences():
    """
    Downloads spaCy and NLTK Brown corpus, then tags the sentences using spaCy.
    Returns a list of lists of (word, tag) tuples.
    """
    # Load spaCy model
    try:
        nlp = spacy.load("en_core_web_sm")
    except OSError:
        print("Downloading 'en_core_web_sm' model...")
        spacy.cli.download("en_core_web_sm")
        nlp = spacy.load("en_core_web_sm")

    # Ensure brown corpus is downloaded
    import nltk
    try:
        nltk.data.find('corpora/brown')
    except LookupError:
        nltk.download('brown')

    # Get all sentences from Brown corpus
    raw_sentences = []
    for sent in brown.sents():
        # Find the first index that contains an alphanumeric character
        start_idx = 0
        while start_idx < len(sent) and not any(c.isalnum() for c in sent[start_idx]):
            start_idx += 1
        
        # Only add sentence if there's at least one alphanumeric-starting word
        if start_idx < len(sent):
            raw_sentences.append(" ".join(sent[start_idx:]))

    print(f"Running spaCy PoS tagging on {len(raw_sentences)} sentences from Brown corpus...")
    
    tagged_sentences = []
    for doc in tqdm(nlp.pipe(raw_sentences, batch_size=50), total=len(raw_sentences), desc="Tagging"):
        tagged_sentences.append([(token.text, token.tag_) for token in doc])
    
    return tagged_sentences

def main():
    parser = argparse.ArgumentParser(description="Create HMM tagger data from Brown corpus or tagged input file.")
    parser.add_argument("--input", type=str, help="Path to the input file with tagged sentences (optional)")
    parser.add_argument("--output", type=str, required=True, help="Path to the output .json file")
    parser.add_argument("--top-words", type=int, default=10000, help="Number of most frequent words to keep for emission")
    args = parser.parse_args()

    # Phase 1: Preprocessing & Frequency counts
    print("Phase 1: Gathering sentences & counting word frequencies...")
    
    if args.input:
        print(f"Reading tagged sentences from {args.input}...")
        tagged_sentences = []
        with open(args.input, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                try:
                    tagged_sentences.append(ast.literal_eval(line))
                except Exception as e:
                    print(f"Skipping malformed line: {e}")
    else:
        tagged_sentences = get_tagged_sentences()

    word_counts = Counter()
    for sentence in tqdm(tagged_sentences, desc="Counting"):
        for word, tag in sentence:
            processed_word = preprocess_word(word, tag)
            word_counts[processed_word] += 1

    # Create set of top words
    print(f"Selecting top {args.top_words} words...")
    most_common = word_counts.most_common(args.top_words)
    allowed_words = {word for word, count in most_common}

    # Phase 2: Calculate HMM statistics
    print("Phase 2: Calculating HMM tables...")
    start_counts = Counter()
    tag_counts = Counter()
    emission_counts = defaultdict(Counter)
    transition_counts = defaultdict(Counter)

    for sentence in tqdm(tagged_sentences, desc="Calculating"):
        if not sentence:
            continue

        prev_tag = None
        for i, (word, tag) in enumerate(sentence):
            processed_word = preprocess_word(word, tag)
            
            # Tag counts for normalization
            tag_counts[tag] += 1

            # Start probabilities
            if i == 0:
                start_counts[tag] += 1
            
            # Emission counts (only for allowed words)
            if processed_word in allowed_words:
                emission_counts[tag][processed_word] += 1
            
            # Transition counts
            if prev_tag is not None:
                transition_counts[prev_tag][tag] += 1
            
            prev_tag = tag

    # Convert counts to probabilities
    print("Phase 3: Converting counts to probabilities...")
    num_sentences = sum(start_counts.values()) or 1
    
    # Use a very small probability for tags that never start a sentence
    # but still need to be in the start_prob map for tag set completeness
    EPSILON_PROB = 1e-7
    all_tags = set(tag_counts.keys())
    
    start_prob = {}
    for tag in all_tags:
        if tag in start_counts:
            start_prob[tag] = start_counts[tag] / num_sentences
        else:
            start_prob[tag] = EPSILON_PROB
    
    emission = {}
    for tag, words in emission_counts.items():
        total_tag_count = tag_counts[tag]
        # Sort words by probability descending
        sorted_words = sorted(words.items(), key=lambda x: x[1], reverse=True)
        emission[tag] = {word: count / total_tag_count for word, count in sorted_words}
        
    transition = {}
    for from_tag, to_tags in transition_counts.items():
        total_transitions = sum(to_tags.values())
        # Sort transitions by probability descending
        sorted_transitions = sorted(to_tags.items(), key=lambda x: x[1], reverse=True)
        transition[from_tag] = {to_tag: count / total_transitions for to_tag, count in sorted_transitions}

    # Save to JSON
    # Phase 4: Prepare final JSON structure
    output_data = {
        "start_prob": dict(sorted(start_prob.items())),
        "emission": emission,
        "transition": transition
    }

    print(f"Saving results to {args.output}...")
    with open(args.output, "w", encoding="utf-8") as f:
        json.dump(output_data, f, indent=2)

    print("Done.")

if __name__ == "__main__":
    main()
