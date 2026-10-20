#!/usr/bin/env python3
"""
Claude Code PostToolUse hook: lints the file an agent has just edited or written,
and reports the issues back to the agent (exit code 2) so it can fix them right away.
"""

import json
import os
import subprocess
import sys

LINT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "lint.py")


def main() -> int:
    event = json.load(sys.stdin)
    path = event.get("tool_input", {}).get("file_path", "")
    if not path.endswith((".h", ".cpp")):
        return 0

    result = subprocess.run([sys.executable, LINT, path], capture_output=True, text=True)
    if result.returncode == 0:
        return 0

    # Exit code 2 hands stderr over to the agent
    print(result.stdout + result.stderr, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())
