import argparse
import json
import nltk
import nltk.corpus
import os
import spacy
from typing import List, Tuple
from tqdm import tqdm

# Constants - paths
DATA_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "data")
TAGS_FILE = os.path.join(DATA_DIR, "tags.json")


if __name__ == "__main__":

    # Argument parsing
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, help="Output file path")
    args = parser.parse_args()

    output_file = args.output

    # --------------------------------
    # Step 1 - download the raw corpus
    # --------------------------------

    nltk.download("brown")

    corpus = nltk.corpus.brown

    # --------------------------------------
    # Step 2 - load tag set from config file
    # --------------------------------------

    with open(TAGS_FILE, "r", encoding="utf-8") as f:
        data = json.load(f)

    # Build mapping from tag id to tag description
    tags = {}
    for tag in data.get("tags", []):
        tag_id = tag.get("id")
        tag_desc = tag.get("desc")
        if tag_id is not None:
            tags[tag_id] = tag_desc
    
    # ------------------------------------
    # Step 3 - tokenization & tags mapping
    # ------------------------------------

    model_name = "en_core_web_sm"
    if not spacy.util.is_package(model_name):
        spacy.cli.download(model_name)
        
    nlp = spacy.load("en_core_web_sm")

    # Helper function - transform sentence with Spacy model
    def tokenize_and_tag(sentence: str) -> List[Tuple[str, str]]:
        doc = nlp(sentence)
        result = [(token.text, token.tag_) for token in doc]

        return result
    
    # ------------------------------
    # Step 4 - save corpus to a file
    # ------------------------------

    # We save corpus data line by line, in form of < <word> <tag> > records
    words = list(corpus.words())
    with open(output_file, "w", encoding="utf-8") as f:
        for word in tqdm(words, desc="Processing words"):
            for word, tag in tokenize_and_tag(word):
                f.write(f"{word} {tag}\n")