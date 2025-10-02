#!/usr/bin/env python3
import argparse, os, glob, math
import numpy as np

def load_csv(path):
    return np.loadtxt(path, delimiter=',', dtype=float)

def max_abs_diff(a, b):
    n = min(len(a), len(b))
    if len(a) != len(b):
        return float('inf'), n
    return float(np.max(np.abs(a[:n] - b[:n]))), n

def mag_db(x, nfft=None):
    if nfft is None:
        nfft = 1 << (2*len(x) - 1).bit_length()
    H = np.fft.rfft(x, n=nfft)
    mag = 20*np.log10(np.maximum(1e-12, np.abs(H)))
    return mag

def mag_diff_db(a, b):
    nfft = 1 << (2*max(len(a), len(b)) - 1).bit_length()
    A = mag_db(a, nfft)
    B = mag_db(b, nfft)
    n = min(len(A), len(B))
    return np.max(np.abs(A[:n] - B[:n]))

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--baseline", required=True)
    ap.add_argument("--generated", required=True)
    ap.add_argument("--report", required=True)
    ap.add_argument("--abs", type=float, default=1e-6)
    ap.add_argument("--db", type=float, default=0.1)
    args = ap.parse_args()

    base_files = sorted(glob.glob(os.path.join(args.baseline, "HB*_min.csv")))
    gen_files  = sorted(glob.glob(os.path.join(args.generated, "HB*_min.csv")))

    os.makedirs(os.path.dirname(args.report), exist_ok=True)
    fail_flag = os.path.join(os.path.dirname(args.report), "FAIL")
    if os.path.exists(fail_flag):
        os.remove(fail_flag)

    m = []
    m.append(f"| Order | Max |Δsample| | Max |Δmag| dB | Status |")
    m.append(f"|:-----:|:------------:|:-------------:|:------:|")

    status_fail = False
    # Map by order (digits in filename)
    def order_from_name(p):
        import re
        s = os.path.basename(p)
        digits = "".join(ch for ch in s if ch.isdigit())
        return int(digits) if digits else -1

    base_map = {order_from_name(p): p for p in base_files}
    gen_map  = {order_from_name(p): p for p in gen_files}

    orders = sorted(set(base_map.keys()) | set(gen_map.keys()))
    for o in orders:
        b = base_map.get(o)
        g = gen_map.get(o)
        if not b or not g:
            m.append(f"| {o} | – | – | MISSING |")
            status_fail = True
            continue

        a = load_csv(b)
        c = load_csv(g)
        mdiff, n = max_abs_diff(a, c)
        mdb = mag_diff_db(a, c)

        ok = (mdiff <= args.abs) and (mdb <= args.db)
        status = "OK" if ok else "FAIL"
        if not ok:
            status_fail = True
        m.append(f"| {o} | {mdiff:.3e} | {mdb:.3f} | {status} |")

    with open(args.report, "w") as f:
        f.write("**Min-Phase Bank – Numeric Comparison**\n\n")
        f.write("\n".join(m))
        f.write("\n\nThresholds: ")
        f.write(f"`abs ≤ {args.abs}` samples, `Δmag ≤ {args.db} dB`.\n")

    if status_fail:
        open(fail_flag, "w").close()

if __name__ == "__main__":
    main()
