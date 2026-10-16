"""
Naming check

Runs clang-tidy's identifier naming check (configured by .clang-tidy) and keeps
the findings which land on the changed lines.
"""

import os
import re
import subprocess

from changes import Changes, overlaps
from issue import Issue

# Mirrors the include paths and standard from CMakeLists.txt, so no build directory is needed
COMPILER_ARGS = ["-xc++", "-std=c++20", "-Isrc", "-Isrc/third-party"]

# 'src/phonemis/x.h:12:8: warning: invalid case style for variable 'isSpace' [readability-identifier-naming]'
WARNING = re.compile(r"^(.+?):(\d+):\d+: warning: (.+) \[readability-identifier-naming\]$")


def check(changes: Changes, clang_tidy: str) -> list[Issue]:
    if not changes:
        return []

    # Compilation errors (e.g. in the middle of an edit) are ignored - we only look for naming warnings
    output = subprocess.run([clang_tidy, "--quiet", *changes, "--", *COMPILER_ARGS],
                            capture_output=True, text=True).stdout

    issues = set()  # A header included by several files is reported once
    for line in output.splitlines():
        warning = WARNING.match(line)
        if not warning:
            continue

        path, line_no = os.path.relpath(warning[1]), int(warning[2])
        if path in changes and overlaps(changes[path], line_no, line_no):
            issues.add(Issue(path, line_no, "naming", warning[3]))

    return list(issues)
