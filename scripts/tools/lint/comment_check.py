"""
Comment length check

Flags comments longer than MAX_LINES lines (see .claude/CLAUDE.md, section 5).
Doc comments (/** */) may be longer only because of their @tags - their description is limited too.
"""

import re

from changes import Changes, overlaps
from issue import Issue

MAX_LINES = 4

DOC_TAG = re.compile(r"^@\w+")


def find_long_comments(lines: list[str]) -> list[tuple[int, int, str]]:
    """Returns (first line, last line, message) of every overly long comment, 1-based."""
    found = []

    i = 0
    while i < len(lines):
        stripped = lines[i].strip()

        # A run of full-line // comments
        if stripped.startswith("//"):
            start = i
            while i < len(lines) and lines[i].strip().startswith("//"):
                i += 1
            if i - start > MAX_LINES:
                found.append((start + 1, i, f"{i - start}-line comment"))
            continue

        # A block comment
        if stripped.startswith("/*"):
            start = i
            while i < len(lines) and "*/" not in lines[i]:
                i += 1
            block = lines[start:i + 1]
            i += 1

            if stripped.startswith("/**"):
                description = doc_description(block)
                if len(description) > MAX_LINES:
                    found.append((start + 1, start + len(block), f"{len(description)}-line doc comment description"))
            elif len(block) > MAX_LINES:
                found.append((start + 1, start + len(block), f"{len(block)}-line block comment"))
            continue

        i += 1

    return found


def doc_description(block: list[str]) -> list[str]:
    """The non-empty lines of a doc comment before its first @tag."""
    description = []
    for line in block:
        text = line.strip().removeprefix("/**").removesuffix("*/").strip().removeprefix("*").strip()
        if DOC_TAG.match(text):
            break
        if text:
            description.append(text)
    return description


def check(changes: Changes) -> list[Issue]:
    issues = []

    for path, ranges in changes.items():
        with open(path, encoding="utf-8") as f:
            lines = f.read().splitlines()

        for first, last, what in find_long_comments(lines):
            if overlaps(ranges, first, last):
                message = f"{what}, at most {MAX_LINES} lines allowed - shorten it or simplify the code"
                issues.append(Issue(path, first, "comment", message))

    return issues
