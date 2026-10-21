"""
Formatting check

Runs clang-format (configured by .clang-format) on the changed lines only.
"""

import difflib
import subprocess

from changes import Changes
from issue import Issue


def line_args(ranges: list[tuple[int, int]]) -> list[str]:
    return [f"--lines={first}:{last}" for first, last in ranges]


def fix(changes: Changes, clang_format: str) -> None:
    """Formats the changed lines in-place."""
    for path, ranges in changes.items():
        subprocess.run([clang_format, "-i", *line_args(ranges), path], check=True)


def check(changes: Changes, clang_format: str) -> list[Issue]:
    issues = []

    for path, ranges in changes.items():
        with open(path, encoding="utf-8", newline="") as f:
            original = f.read()
        formatted = subprocess.run([clang_format, *line_args(ranges), path],
                                   check=True, capture_output=True, encoding="utf-8").stdout

        # clang-format prints nothing for files listed in .clang-format-ignore
        if formatted == original or (not formatted and original):
            continue

        # Report the expected result as a diff, anchored at its first changed line
        original_lines, formatted_lines = original.splitlines(), formatted.splitlines()
        first_change = next(i1 for tag, i1, *_ in difflib.SequenceMatcher(None, original_lines, formatted_lines).get_opcodes()
                            if tag != "equal")
        diff = difflib.unified_diff(original_lines, formatted_lines, "current", "expected", n=1, lineterm="")
        message = "not clang-formatted (fix with --fix), expected:\n" + "\n".join(f"    {line}" for line in diff)
        issues.append(Issue(path, first_change + 1, "format", message))

    return issues
