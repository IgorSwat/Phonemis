import os
import shutil

# Common LLVM install locations, for shells whose PATH misses them
FALLBACK_DIRS = ["/opt/homebrew/opt/llvm/bin", "/usr/local/opt/llvm/bin"]


def find_tool(name: str) -> str | None:
    """
    Locates an LLVM tool (e.g. clang-format).
    An environment override (e.g. CLANG_FORMAT=/path/to/clang-format) wins over PATH and the fallbacks.
    """
    override = os.environ.get(name.upper().replace("-", "_"))
    if override:
        return override

    found = shutil.which(name)
    if found:
        return found

    for directory in FALLBACK_DIRS:
        candidate = os.path.join(directory, name)
        if os.access(candidate, os.X_OK):
            return candidate

    return None
