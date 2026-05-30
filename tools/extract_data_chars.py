#!/usr/bin/env python3
"""Extract unique CJK characters used in data .h files.

Walks a list of files, pulls everything between double quotes (string
literals), and emits the set of CJK chars. Used by gen_cjk_font.py to
auto-include translation pool chars without hand-maintaining a giant
SYMBOL_BUCKETS entry.

Range covered: CJK Unified Ideographs (U+4E00..U+9FFF) + Extension A
(U+3400..U+4DBF) + common CJK punctuation (U+3000..U+303F + U+FF00..
U+FFEF). ASCII is skipped (covered separately by --range in the
font conv command).
"""

import re
import sys
from pathlib import Path

_STRING_RE = re.compile(r'"([^"\\]*(?:\\.[^"\\]*)*)"')


def is_cjk(ch: str) -> bool:
    cp = ord(ch)
    return (
        0x4E00 <= cp <= 0x9FFF
        or 0x3400 <= cp <= 0x4DBF
        or 0x3000 <= cp <= 0x303F
        or 0xFF00 <= cp <= 0xFFEF
    )


def extract_from_file(path: Path) -> set[str]:
    chars: set[str] = set()
    text = path.read_text(encoding="utf-8")
    for match in _STRING_RE.finditer(text):
        for ch in match.group(1):
            if is_cjk(ch):
                chars.add(ch)
    return chars


def extract_from_files(paths: list[Path]) -> set[str]:
    seen: set[str] = set()
    for p in paths:
        seen |= extract_from_file(p)
    return seen


def main() -> None:
    if len(sys.argv) < 2:
        print("usage: extract_data_chars.py <file.h> [file2.h ...]",
              file=sys.stderr)
        sys.exit(2)
    paths = [Path(p) for p in sys.argv[1:]]
    chars = extract_from_files(paths)
    print("".join(sorted(chars)))


if __name__ == "__main__":
    main()
