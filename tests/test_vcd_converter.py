#!/usr/bin/env python3
"""Tests for tools/vcd_to_sniffer_log.py and tools/parse_sniffer_log.py.

Run: python3 -I tests/test_vcd_converter.py
Uses tests/vcd/synthetic_2ch_1MHz.vcd (synthetic, produced with sigrok-cli, see make_synthetic_vcd.py).
"""
import pathlib
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
VCD = ROOT / "tests" / "vcd" / "synthetic_2ch_1MHz.vcd"
CONVERT = ROOT / "tools" / "vcd_to_sniffer_log.py"
PARSE = ROOT / "tools" / "parse_sniffer_log.py"

failures = 0


def check(cond, msg):
    global failures
    if not cond:
        failures += 1
        print("FAIL:", msg)


def run(script, *args):
    return subprocess.run([sys.executable, "-I", str(script), *map(str, args)], capture_output=True, text=True)


def fixture_frame(file, index):
    lines = [l for l in (ROOT / "tests" / "fixtures" / file).read_text().splitlines() if l.strip() and not l.startswith("#")]
    return lines[index].split("|")[0].strip()


with tempfile.TemporaryDirectory() as tmp:
    log = pathlib.Path(tmp) / "out.log"

    # normal case
    res = run(CONVERT, VCD, "-o", log)
    check(res.returncode == 0, f"converter failed: {res.stderr}")
    text = log.read_text()
    lines = text.splitlines()
    check(len(lines) == 8, f"expected 8 blocks, got {len(lines)}")
    check(" B 100 R40000023DB" in text, "poll on channel B at 100 ms")
    check(f" M 140 {fixture_frame('r_frames.txt', 0)}" in text, "first R reply on M at 140 ms")
    check(" M 1060 W00E10001OK88" in text, "write ACK on M")
    check(f" M 1300 {fixture_frame('z_frames.txt', 0)}" in text, "Z frame on M")
    check("0 framing errors" in res.stderr, "no framing errors reported")
    check("9615 baud" in res.stderr or "9600 baud" in res.stderr, "baud estimate near 9600")

    # the parser accepts the converted log without checksum problems
    res = run(PARSE, log)
    check(res.returncode == 0 and "WARNING" not in res.stdout, "parser finds no channel swap")
    check("<-- BAD" not in res.stdout, "no bad checksums in the parsed log")

    # swapped channels are detected by the parser
    swapped = pathlib.Path(tmp) / "swapped.log"
    res = run(CONVERT, VCD, "--machine", "1", "--module", "0", "-o", swapped)
    check(res.returncode == 0, "converter accepts --machine/--module")
    res = run(PARSE, swapped)
    check("swapped" in res.stdout, "parser warns when the poll shows up on channel M")

    # wrong polarity: many framing errors must be reported
    res = run(CONVERT, VCD, "--no-invert", "-o", pathlib.Path(tmp) / "x.log")
    check("check --baud and --no-invert" in res.stderr, "wrong polarity triggers the plausibility warning")
    ok = run(CONVERT, VCD, "-o", pathlib.Path(tmp) / "ok.log")
    check("check --baud" not in ok.stderr, "correct settings do not warn")

    # unknown signal name
    res = run(CONVERT, VCD, "--machine", "nope", "-o", pathlib.Path(tmp) / "y.log")
    check(res.returncode != 0, "unknown signal name is rejected")

print("OK" if failures == 0 else f"{failures} failure(s)")
sys.exit(1 if failures else 0)
