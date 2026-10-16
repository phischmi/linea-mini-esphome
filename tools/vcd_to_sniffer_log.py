#!/usr/bin/env python3
"""Convert a logic analyzer capture (VCD) of the CN11 lines into a sniffer log.

The output has the same line format as examples/sniffer.yaml ("M <ms> <text>" / "B <ms> <text>")
and can be read by tools/parse_sniffer_log.py.

Usage:
  python3 -I tools/vcd_to_sniffer_log.py capture.vcd [--machine D0] [--module D1] [-o sniffer.log]

Get a VCD from PulseView (File -> Export) or from sigrok-cli:
  sigrok-cli -i capture.sr -O vcd -o capture.vcd

Channel M = machine -> module (CN11 TXD line), channel B = module -> machine (CN11 RXD line).
Without --machine/--module the first declared signal is M and the second is B; the parser
warns if the poll R40000023DB shows up on M (then swap the two names).

Own code, no third-party dependencies.
"""
import argparse
import bisect
import re
import sys

UNITS = {"s": 1.0, "ms": 1e-3, "us": 1e-6, "ns": 1e-9, "ps": 1e-12, "fs": 1e-15}


def parse_vcd(path):
    """Return ({id: name}, {id: [(time_s, level)]}) from a VCD file."""
    timescale = 1.0
    names = {}
    changes = {}
    in_defs = True
    now = 0.0
    with open(path, errors="replace") as fh:
        text = fh.read()
    tokens = text.split()
    i = 0
    while i < len(tokens):
        tok = tokens[i]
        if in_defs:
            if tok == "$timescale":
                j = i + 1
                parts = []
                while tokens[j] != "$end":
                    parts.append(tokens[j])
                    j += 1
                joined = "".join(parts)
                m = re.fullmatch(r"(\d+)([a-z]+)", joined)
                if not m or m.group(2) not in UNITS:
                    raise SystemExit(f"unsupported $timescale: {' '.join(parts)}")
                timescale = int(m.group(1)) * UNITS[m.group(2)]
                i = j
            elif tok == "$var":
                # $var wire 1 <id> <name> $end
                names[tokens[i + 3]] = tokens[i + 4]
                changes[tokens[i + 3]] = []
                i += 5
            elif tok == "$enddefinitions":
                in_defs = False
                while tokens[i] != "$end":
                    i += 1
            i += 1
            continue
        if tok.startswith("#"):
            now = int(tok[1:]) * timescale
        elif tok.startswith("$"):
            pass  # $dumpvars, $end, ...
        elif tok[0] in "01xXzZ" and len(tok) > 1 and tok[1:] in changes:
            if tok[0] in "01":
                changes[tok[1:]].append((now, int(tok[0])))
        i += 1
    return names, changes


class Decoded:
    def __init__(self):
        self.bytes = []  # (start_s, value, framing_ok)
        self.min_pulse = None


def decode_uart(changes, baud, invert):
    """Decode 8N1 from a list of (time_s, level) transitions."""
    out = Decoded()
    if not changes:
        return out
    times = [t for t, _ in changes]
    levels = [lv for _, lv in changes]
    bit = 1.0 / baud
    start_level = 1 if invert else 0
    idle_level = 1 - start_level

    for a, b in zip(times, times[1:]):
        w = b - a
        if w > 0 and (out.min_pulse is None or w < out.min_pulse):
            out.min_pulse = w

    def level_at(t):
        k = bisect.bisect_right(times, t) - 1
        return levels[k] if k >= 0 else idle_level

    k = 0
    n = len(times)
    search_from = -1.0
    while k < n:
        if levels[k] != start_level or times[k] <= search_from:
            k += 1
            continue
        t0 = times[k]
        value = 0
        for i in range(8):
            raw = level_at(t0 + (1.5 + i) * bit)
            logical = 1 - raw if invert else raw
            value |= logical << i
        stop_raw = level_at(t0 + 9.5 * bit)
        out.bytes.append((t0, value, stop_raw == idle_level))
        search_from = t0 + 9.5 * bit
        k = bisect.bisect_right(times, search_from)
    return out


def to_blocks(decoded, baud, gap_s):
    """Group bytes into blocks separated by a pause of at least gap_s."""
    bit = 1.0 / baud
    blocks = []
    cur = None
    prev_end = None
    for t0, value, ok in decoded.bytes:
        if cur is None or t0 - prev_end >= gap_s:
            cur = [t0, []]
            blocks.append(cur)
        cur[1].append((value, ok))
        prev_end = t0 + 10 * bit
    return blocks


def printable(value, ok):
    return chr(value) if ok and 32 <= value < 127 else "."


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("vcd")
    ap.add_argument("--machine", help="signal name or id for channel M (machine -> module)")
    ap.add_argument("--module", help="signal name or id for channel B (module -> machine)")
    ap.add_argument("--baud", type=int, default=9600)
    ap.add_argument("--no-invert", action="store_true", help="line is NOT inverted (CN11 is idle-low, so default is inverted)")
    ap.add_argument("--gap-ms", type=float, default=15.0, help="pause that ends a block (default 15 ms, like the ESP32 sniffer)")
    ap.add_argument("-o", "--output", help="write the log here (default: stdout)")
    args = ap.parse_args(argv)

    names, changes = parse_vcd(args.vcd)
    if len(names) < 2:
        raise SystemExit("the VCD needs at least two signals")

    def resolve(want, default_index):
        if want is None:
            return list(names)[default_index]
        for sid, name in names.items():
            if want in (name, sid):
                return sid
        raise SystemExit(f"signal {want!r} not found; available: {', '.join(names.values())}")

    sid_m = resolve(args.machine, 0)
    sid_b = resolve(args.module, 1)
    print(f"channel M = {names[sid_m]}, channel B = {names[sid_b]}", file=sys.stderr)

    lines = []
    for label, sid in (("M", sid_m), ("B", sid_b)):
        dec = decode_uart(changes[sid], args.baud, not args.no_invert)
        bad = sum(1 for _, _, ok in dec.bytes if not ok)
        est = f"{1.0 / dec.min_pulse:.0f} baud" if dec.min_pulse else "n/a"
        print(f"{label}: {len(dec.bytes)} bytes, {bad} framing errors, shortest pulse implies ~{est}", file=sys.stderr)
        # All GICAR frames are printable ASCII. Garbage means wrong polarity or baud rate, even
        # when few framing errors are counted.
        ascii_ok = sum(1 for _, v, ok in dec.bytes if ok and 32 <= v < 127)
        if dec.bytes and (bad > len(dec.bytes) // 10 or ascii_ok < 0.9 * len(dec.bytes)):
            print(f"{label}: {ascii_ok}/{len(dec.bytes)} bytes are printable ASCII - "
                  "check --baud and --no-invert", file=sys.stderr)
        for t0, chars in to_blocks(dec, args.baud, args.gap_ms / 1000.0):
            text = "".join(printable(v, ok) for v, ok in chars)
            lines.append((t0, label, f"[vcd][I][snf:0]: {label} {int(round(t0 * 1000))} {text}"))
    lines.sort(key=lambda x: (x[0], x[1]))

    out = open(args.output, "w") if args.output else sys.stdout
    for _, _, line in lines:
        print(line, file=out)
    if args.output:
        out.close()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
