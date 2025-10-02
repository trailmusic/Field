#!/usr/bin/env python3
import argparse, os, glob
import numpy as np
import matplotlib.pyplot as plt

def load_csv(p):
    return np.loadtxt(p, delimiter=',', dtype=float)

def nextpow2(n):
    return 1 << (n - 1).bit_length()

def mag_db(x, nfft=None):
    if nfft is None:
        nfft = nextpow2(2*len(x))
    H = np.fft.rfft(x, n=nfft)
    mag = 20*np.log10(np.maximum(1e-12, np.abs(H)))
    f = np.linspace(0.0, 0.5, len(H))
    return f, mag

def plot_pair(base, gen, title, out_png):
    hb = load_csv(base)
    hg = load_csv(gen)

    # IR
    plt.figure()
    plt.title(f"IR – {title}")
    plt.plot(hb, label="baseline")
    plt.plot(hg, label="generated", alpha=0.8)
    plt.xlabel("samples"); plt.ylabel("amp")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_png.replace(".png", "_ir.png"))
    plt.close()

    # Step
    sb = np.cumsum(hb); sg = np.cumsum(hg)
    plt.figure()
    plt.title(f"Step – {title}")
    plt.plot(sb, label="baseline")
    plt.plot(sg, label="generated", alpha=0.8)
    plt.xlabel("samples"); plt.ylabel("amp")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_png.replace(".png", "_step.png"))
    plt.close()

    # Magnitude
    fb, Mb = mag_db(hb); fg, Mg = mag_db(hg)
    plt.figure()
    plt.title(f"Magnitude – {title}")
    plt.plot(fb, Mb, label="baseline")
    plt.plot(fg, Mg, label="generated", alpha=0.8)
    plt.xlabel("normalized freq (×Fs)"); plt.ylabel("dB")
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_png.replace(".png", "_mag.png"))
    plt.close()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--baseline", required=True)
    ap.add_argument("--generated", required=True)
    ap.add_argument("--out-dir", required=True)
    args = ap.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)

    base_files = sorted(glob.glob(os.path.join(args.baseline, "HB*_min.csv")))
    gen_files  = sorted(glob.glob(os.path.join(args.generated, "HB*_min.csv")))

    def order(p):
        import re, os
        s = os.path.basename(p)
        d = "".join(ch for ch in s if ch.isdigit())
        return int(d) if d else -1

    bmap = {order(p): p for p in base_files}
    gmap = {order(p): p for p in gen_files}

    for o in sorted(set(bmap.keys()) & set(gmap.keys())):
        base = bmap[o]; gen = gmap[o]
        out_png = os.path.join(args.out_dir, f"HB{o}_cmp.png")
        plot_pair(base, gen, f"HB{o}", out_png)

if __name__ == "__main__":
    main()
