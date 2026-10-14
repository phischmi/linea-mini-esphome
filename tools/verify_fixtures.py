#!/usr/bin/env python3
"""Check the checksum and expected length of every frame in tests/fixtures/*.txt.

Own helper, no third-party code. Usage: python3 tools/verify_fixtures.py
"""
import pathlib
import sys

FIXTURES = pathlib.Path(__file__).resolve().parent.parent / "tests" / "fixtures"
# first char / prefix -> expected length (None = any)
EXPECTED = {"R40000023": 81, "R00000020": 75, "Z": 55, "W": None, "X": None, "R": None}


def checksum(body: str) -> str:
    return f"{sum(body.encode('ascii')) & 0xFF:02X}"


def main() -> int:
    bad = 0
    total = 0
    for path in sorted(FIXTURES.glob("*.txt")):
        for n, line in enumerate(path.read_text().splitlines(), 1):
            if not line.strip() or line.startswith("#"):
                continue
            frame = line.split("|")[0].strip()
            total += 1
            want = checksum(frame[:-2])
            if want != frame[-2:]:
                print(f"{path.name}:{n}: checksum {frame[-2:]} != {want}")
                bad += 1
            for prefix, length in EXPECTED.items():
                if frame.startswith(prefix):
                    if length and path.name != "commands.txt" and len(frame) != length:
                        print(f"{path.name}:{n}: length {len(frame)} != {length}")
                        bad += 1
                    break
    print(f"{total} frames checked, {bad} problem(s)")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
