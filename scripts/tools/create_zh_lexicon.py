"""
Builds the Mandarin lexicon: misaki's Kokoro v1.0 phonemes for every Han character
pypinyin can read and for every pypinyin phrase.

zh::LexiconPhonemizer splits words into these phrases the way pypinyin does, so it
reproduces misaki's readings without running pypinyin.

Usage: python create_zh_lexicon.py <output.json>
Requires: pip install "misaki[zh]"
"""

import json
import sys

from misaki.zh import ZHG2P
from pypinyin import phrases_dict
from pypinyin.constants import PINYIN_DICT


def is_han(text: str) -> bool:
    return all("\u4e00" <= c <= "\u9fff" for c in text)


def phonemize(text: str):
    try:
        return ZHG2P.word2ipa(text).replace(chr(815), "")
    except ValueError:
        # misaki has no phonemes for a few readings, such as the interjections "hm" and "ng".
        return None


def main(output_path: str) -> None:
    chars = [chr(c) for c in range(0x4E00, 0xA000) if c in PINYIN_DICT]
    phrases = [w for w in phrases_dict.phrases_dict if len(w) > 1 and is_han(w)]

    lexicon, skipped = {}, []
    for entry in chars + phrases:
        phonemes = phonemize(entry)
        if phonemes:
            lexicon[entry] = phonemes
        else:
            skipped.append(entry)

    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(lexicon, f, ensure_ascii=False, separators=(",", ":"), sort_keys=True)

    print(f"{len(lexicon)} entries -> {output_path}; skipped {len(skipped)}: {''.join(skipped[:20])}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    main(sys.argv[1])
