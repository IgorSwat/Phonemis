#!/usr/bin/env python3
"""
Lints the C++ sources under src/phonemis: formatting (clang-format), identifier naming (clang-tidy)
and comment length. By default only the lines changed since HEAD are checked.
"""

import argparse
import os
import sys

import comment_check
import format_check
import naming_check
from binaries import find_tool
from changes import all_lines, changed_lines, git


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="*", help="limit the check to these files")
    parser.add_argument("--base", default="HEAD",
                        help="check changes since the merge-base with this revision (default: uncommitted changes)")
    parser.add_argument("--all", action="store_true", help="check every line, not only the changed ones")
    parser.add_argument("--fix", action="store_true", help="apply clang-format to the checked lines first")
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    # Work from the repository root, with requested paths made relative to it
    root = git("rev-parse", "--show-toplevel").strip()
    requested = {os.path.relpath(os.path.abspath(path), root) for path in args.paths}
    os.chdir(root)

    # 1. Select the lines to check
    if args.all:
        changes = all_lines()
    else:
        base = "HEAD" if args.base == "HEAD" else git("merge-base", args.base, "HEAD").strip()
        changes = changed_lines(base)
    if requested:
        changes = {path: ranges for path, ranges in changes.items() if path in requested}
    if not changes:
        print("lint: nothing to check")
        return 0

    # 2. Run the checks (a missing tool skips its check, loudly)
    issues = []

    clang_format = find_tool("clang-format")
    if clang_format:
        if args.fix:
            format_check.fix(changes, clang_format)
        issues += format_check.check(changes, clang_format)
    else:
        print("lint: clang-format not found - formatting NOT checked", file=sys.stderr)

    clang_tidy = find_tool("clang-tidy")
    if clang_tidy:
        issues += naming_check.check(changes, clang_tidy)
    else:
        print("lint: clang-tidy not found - naming NOT checked", file=sys.stderr)

    issues += comment_check.check(changes)

    # 3. Report
    for issue in sorted(issues, key=lambda issue: (issue.path, issue.line, issue.check)):
        print(f"{issue.path}:{issue.line}: [{issue.check}] {issue.message}")

    if issues:
        print(f"lint: {len(issues)} issue(s) in {len({issue.path for issue in issues})} file(s)")
        return 1

    print(f"lint: OK ({len(changes)} file(s) checked)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
