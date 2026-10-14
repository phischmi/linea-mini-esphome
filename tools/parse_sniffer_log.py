#!/usr/bin/env python3
"""Evaluate a log written by examples/sniffer.yaml.

Reads lines like "... snf: M 123456 R4000..." (channel M = machine->module,
B = module->machine), splits them into GICAR frames, verifies checksums and
prints a summary plus every non-poll command with its timestamp.

Usage: python3 -I tools/parse_sniffer_log.py sniffer.log [--all]
Own code, no third-party dependencies.
"""
import collections
import re
import sys

LINE = re.compile(r"\b([MB]) (\d+) (\S*)\s*$")
ANSI = re.compile(r"\x1b\[[0-9;]*m")


def cs_ok(frame: str) -> bool:
    return len(frame) > 2 and f"{sum(frame[:-2].encode()) & 0xFF:02X}" == frame[-2:]


def hexlen(s: str):
    try:
        return int(s, 16)
    except ValueError:
        return None


def frame_length(channel: str, text: str):
    """Expected total length of the frame starting at text[0], or None if unknown/too short."""
    c = text[0]
    if channel == "B":
        if c == "R" or c == "X":
            return 11
        if c == "W" and len(text) >= 9:
            n = hexlen(text[5:9])
            return None if n is None else 9 + 2 * n + 2
        return None
    if c == "Z":
        return 55
    if c == "X":
        return 11
    if c == "R" and len(text) >= 9:
        n = hexlen(text[5:9])
        return None if n is None else 9 + 2 * n + 2
    if c == "W":
        return 13
    return None


def split_frames(channel: str, text: str):
    """Yield (frame, ok) pairs. Resyncs on the next start character after garbage."""
    i = 0
    while i < len(text):
        if text[i] not in "RWXZ":
            i += 1
            continue
        n = frame_length(channel, text[i:])
        if n is None or i + n > len(text):
            yield text[i:], False
            return
        frame = text[i:i + n]
        yield frame, cs_ok(frame)
        i += n


def main(argv) -> int:
    if len(argv) < 2:
        print(__doc__)
        return 2
    show_all = "--all" in argv
    counts = collections.Counter()
    bad = collections.Counter()
    commands = []
    first_ts = last_ts = None
    poll_on = collections.Counter()
    for raw in open(argv[1], errors="replace"):
        m = LINE.search(ANSI.sub("", raw))
        if not m:
            continue
        ch, ts, text = m.group(1), int(m.group(2)), m.group(3)
        first_ts = ts if first_ts is None else first_ts
        last_ts = ts
        for frame, ok in split_frames(ch, text):
            kind = frame[:9] if frame.startswith("R") else frame[:1]
            key = (ch, kind)
            counts[key] += 1
            if not ok:
                bad[key] += 1
            if frame == "R40000023DB":
                poll_on[ch] += 1
            if ch == "B" and frame != "R40000023DB":
                commands.append((ts, frame, ok))
            elif show_all:
                print(f"{ts:>9} M {frame}{'' if ok else '  <-- BAD'}")
    if first_ts is None:
        print("no sniffer lines found")
        return 1
    print(f"span: {(last_ts - first_ts) / 1000:.1f} s\n")
    print("channel frame-type          count  bad-checksum/incomplete")
    for (ch, kind), n in sorted(counts.items()):
        print(f"{ch:>7}  {kind:<18} {n:>6}  {bad[(ch, kind)]}")
    if poll_on["M"] and not poll_on["B"]:
        print("\nWARNING: the poll R40000023DB appears on channel M. The two wires/pins are swapped; swap them.")
    elif not poll_on:
        print("\nNote: no poll R40000023DB seen (machine off, wrong wiring or original module not polling).")
    print("\nnon-poll commands (module -> machine):")
    for ts, frame, ok in commands:
        print(f"{ts:>9}  {frame}{'' if ok else '  <-- BAD'}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
