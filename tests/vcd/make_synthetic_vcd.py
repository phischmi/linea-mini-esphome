#!/usr/bin/env python3
"""Generate the synthetic logic analyzer capture used by tests/test_vcd_converter.py.

Not a real capture: frames from tests/fixtures are rendered as an inverted 9600 8N1 UART
signal at 1 MS/s (bit0 = channel M, bit1 = channel B). Convert the result with sigrok:

  python3 -I tests/vcd/make_synthetic_vcd.py /some/dir/synthetic.bin
  sigrok-cli -I binary:numchannels=2:samplerate=1000000 -i /some/dir/synthetic.bin \
      -O vcd -o tests/vcd/synthetic_2ch_1MHz.vcd

Own code, no third-party dependencies.
"""
import pathlib
import sys

RATE = 1_000_000
BIT = RATE / 9600.0
FIXTURES = pathlib.Path(__file__).resolve().parent.parent / "fixtures"


def frames(name):
    out = []
    for line in (FIXTURES / name).read_text().splitlines():
        if line.strip() and not line.startswith("#"):
            out.append(line.split("|")[0].strip())
    return out


def main(path):
    r, z = frames("r_frames.txt"), frames("z_frames.txt")
    n = int(RATE * 2.0)
    chan = [[0] * n, [0] * n]

    def put(c, t_ms, text):
        pos = t_ms * RATE / 1000.0
        for ch in text.encode():
            # inverted line: idle low, start bit high, data bits inverted, stop bit low
            bits = [1] + [0 if (ch >> i) & 1 else 1 for i in range(8)] + [0]
            for b in bits:
                for k in range(int(round(pos)), int(round(pos + BIT))):
                    chan[c][k] = b
                pos += BIT

    put(1, 100, "R40000023DB")
    put(0, 140, r[0])
    put(1, 860, "R40000023DB")
    put(0, 900, r[0])
    put(1, 1000, "W00E100018157")
    put(0, 1060, "W00E10001OK88")
    put(0, 1300, z[0])
    put(0, 1400, z[1])
    pathlib.Path(path).write_bytes(bytes(chan[0][i] | (chan[1][i] << 1) for i in range(n)))


if __name__ == "__main__":
    main(sys.argv[1])
