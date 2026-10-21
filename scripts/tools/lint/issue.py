from dataclasses import dataclass


# A single lint finding, reported as `path:line: [check] message`.
@dataclass(frozen=True)
class Issue:
    path: str
    line: int
    check: str  # "format", "naming" or "comment"
    message: str
