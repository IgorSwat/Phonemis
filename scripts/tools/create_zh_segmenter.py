"""
Exports jieba's default word segmentation data for the Mandarin pipeline.

The output holds everything zh::Segmenter needs to reproduce jieba.lcut():
- total: the sum of all dictionary frequencies (route probabilities use it)
- words: word -> frequency, for words made only of the characters jieba segments
- hmm: the BMES model jieba uses to join runs of single characters into words

Usage: python create_zh_segmenter.py <output.json>
Requires: pip install jieba
"""

import json
import sys

import jieba
from jieba.finalseg import prob_emit, prob_start, prob_trans


def is_segmentable(word: str) -> bool:
    # jieba's HMM and the pipeline's character runs only cover U+4E00..U+9FD5.
    return all("一" <= c <= "鿕" for c in word)


def main(output_path: str) -> None:
    jieba.initialize()
    words = {w: f for w, f in jieba.dt.FREQ.items() if f > 0 and is_segmentable(w)}

    data = {
        "total": jieba.dt.total,
        "words": words,
        "hmm": {
            "start": prob_start.P,
            "trans": prob_trans.P,
            "emit": prob_emit.P,
        },
    }

    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, separators=(",", ":"), sort_keys=True)

    print(f"{len(words)} words, total frequency {jieba.dt.total} -> {output_path}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    main(sys.argv[1])
