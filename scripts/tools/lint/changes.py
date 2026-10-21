"""
Changed lines selection

Collects the source files and line ranges which should be linted,
either the ones changed since a git revision or all of them.
"""

import re
import subprocess
from pathlib import Path

# A file path (relative to the repository root) -> line ranges (1-based, inclusive)
Changes = dict[str, list[tuple[int, int]]]

SOURCE_ROOT = "src/phonemis"
SOURCE_SUFFIXES = (".h", ".cpp")

# Generated files are never linted
GENERATED_FILES = {
    "src/phonemis/lang/hi/characters.h",
    "src/phonemis/utils/unicode_table.h",
}

# '@@ -12,3 +14,5 @@' -> new file lines 14..18
HUNK_HEADER = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@")


def git(*args: str) -> str:
    return subprocess.run(["git", *args], check=True, capture_output=True, text=True).stdout


def is_lintable(path: str) -> bool:
    return path.startswith(SOURCE_ROOT + "/") and path.endswith(SOURCE_SUFFIXES) and path not in GENERATED_FILES


def overlaps(ranges: list[tuple[int, int]], first: int, last: int) -> bool:
    return any(start <= last and first <= end for start, end in ranges)


def whole_file(path: str) -> list[tuple[int, int]]:
    line_count = len(Path(path).read_text(encoding="utf-8").splitlines())
    return [(1, max(line_count, 1))]


def changed_lines(base: str) -> Changes:
    """Lines added or modified since `base` (staged or not), plus untracked files as a whole."""
    changes: Changes = {}

    # 1. Tracked files - a zero-context diff gives exactly the new/modified lines
    path = None
    for line in git("diff", "-U0", "--no-color", "--no-ext-diff", base, "--", SOURCE_ROOT).splitlines():
        if line.startswith("+++ "):
            path = line.removeprefix("+++ b/") if line.startswith("+++ b/") else None  # None for deleted files
        elif path and (hunk := HUNK_HEADER.match(line)):
            start, count = int(hunk[1]), int(hunk[2] or 1)
            if count > 0:  # Pure deletions leave nothing to check
                changes.setdefault(path, []).append((start, start + count - 1))

    # 2. Untracked files
    for path in git("ls-files", "--others", "--exclude-standard", "--", SOURCE_ROOT).splitlines():
        changes[path] = whole_file(path)

    return {path: ranges for path, ranges in changes.items() if is_lintable(path)}


def all_lines() -> Changes:
    """Every line of every source file, tracked or not."""
    paths = git("ls-files", "--cached", "--others", "--exclude-standard", "--", SOURCE_ROOT).splitlines()
    return {path: whole_file(path) for path in paths if is_lintable(path) and Path(path).exists()}
