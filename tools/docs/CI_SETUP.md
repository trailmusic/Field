# CI: Min-Phase Bank Auto-Build, Diff, and Plots (GitHub Actions)

This adds a CI job that, on every push/PR:

1. **Builds** the `batch_minphase` tool
2. **Regenerates** min-phase taps + `MinPhaseBank.h` into `out/`
3. **Diffs** against a committed **baseline** (`tools/baseline/`)
4. **Plots** IR/Step/Magnitude comparisons as PNGs
5. **Publishes** artifacts and a **PR comment summary**
6. **Optionally fails** if numeric deviation exceeds thresholds

Copy/paste the files below into your repo in the indicated paths.

---

## 1) Repo Layout (additions)

```
tools/
  batch_minphase/
    CMakeLists.txt            # already present
    batch_minphase.cpp        # already present
    fft_real.h                # already present
  baseline/                   # <-- commit your current goldens here
    HB63_min.csv
    HB95_min.csv
    HB127_min.csv
.github/
  workflows/
    minphase.yml              # <-- NEW: CI workflow
scripts/
  build_batch.sh              # <-- NEW
  compare_csv.py              # <-- NEW (numeric + dB diffs)
  plot_bank.py                # <-- NEW (PNG plots)
```

> Keep your **linear** designs wherever you prefer; the CI reads whatever your `build_batch.sh` uses.

---

## 2) GitHub Actions Workflow

**File:** `.github/workflows/minphase.yml`

```yaml
name: MinPhase Bank – Build, Diff, Plots

on:
  push:
    branches: [ main, develop, feature/** ]
  pull_request:
    branches: [ main, develop ]

jobs:
  minphase:
    runs-on: ubuntu-latest

    permissions:
      contents: write        # for uploading artifacts
      pull-requests: write   # for PR comments

    env:
      BUILD_TYPE: Release
      PYTHON_VERSION: "3.11"
      THRESH_ABS: "1e-6"     # absolute sample diff threshold
      THRESH_DB: "0.10"      # dB magnitude tolerance (pass/stop bands)

    steps:
      - name: Checkout
        uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: Set up Python
        uses: actions/setup-python@v5
        with:
          python-version: ${{ env.PYTHON_VERSION }}

      - name: Install Python deps
        run: |
          python -m pip install --upgrade pip
          pip install numpy matplotlib scipy

      - name: Configure & Build batch_minphase (CMake + KissFFT)
        working-directory: tools/batch_minphase
        run: |
          cmake -S . -B build -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
          cmake --build build --config ${BUILD_TYPE}

      - name: Regenerate bank (CSV + header)
        run: bash scripts/build_batch.sh

      - name: Numeric diff vs baseline
        run: |
          python scripts/compare_csv.py \
            --baseline tools/baseline \
            --generated out/csv \
            --report out/report/minphase_report.md \
            --abs ${THRESH_ABS} --db ${THRESH_DB}

      - name: Plot IR/Step/Mag comparisons
        run: |
          python scripts/plot_bank.py \
            --baseline tools/baseline \
            --generated out/csv \
            --out-dir out/plots

      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: minphase_artifacts
          path: |
            out/MinPhaseBank.h
            out/csv/*.csv
            out/plots/*.png
            out/report/minphase_report.md

      - name: Post PR comment (summary)
        if: ${{ github.event_name == 'pull_request' }}
        uses: actions/github-script@v7
        with:
          script: |
            const fs = require('fs');
            const path = 'out/report/minphase_report.md';
            let body = fs.existsSync(path) ? fs.readFileSync(path, 'utf8') : 'No report generated.';
            body = `### Min-Phase Bank CI Report\n\n${body}\n\n_Artifacts:_ **minphase_artifacts**`;
            await github.rest.issues.createComment({
              owner: context.repo.owner,
              repo: context.repo.repo,
              issue_number: context.issue.number,
              body
            });

      - name: Fail build on threshold breach
        run: |
          if [ -f out/report/FAIL ]; then
            echo "Deviation thresholds exceeded."
            cat out/report/minphase_report.md || true
            exit 1
          fi
```

---

## 3) Build/Generate Script

**File:** `scripts/build_batch.sh`
(Make it executable: `git update-index --chmod=+x scripts/build_batch.sh`)

```bash
#!/usr/bin/env bash
set -euo pipefail

# Where outputs go
OUT_DIR="out"
CSV_DIR="${OUT_DIR}/csv"
REPORT_DIR="${OUT_DIR}/report"
PLOTS_DIR="${OUT_DIR}/plots"
mkdir -p "${CSV_DIR}" "${REPORT_DIR}" "${PLOTS_DIR}"

# Inputs: point at your LINEAR designs
# Example assumes tools/examples contains HB63/HB95/HB127 linear taps
LIN_DIR="tools/examples"
LINEAR_FILES=(
  "${LIN_DIR}/HB63_linear.csv"
  "${LIN_DIR}/HB95_linear.csv"
  "${LIN_DIR}/HB127_linear.csv"
)

# Run batch_minphase
BIN="tools/batch_minphase/build/batch_minphase"
if [ ! -x "${BIN}" ]; then
  echo "ERROR: ${BIN} not found or not executable"
  exit 1
fi

"${BIN}" \
  --out-header "${OUT_DIR}/MinPhaseBank.h" \
  --prefix HB \
  --normalize dc \
  --emit-csv \
  --in "${LINEAR_FILES[@]}"

# Move emitted CSVs next to header into out/csv/
# The batch tool emits HB*_min.csv in cwd; collect them:
for f in HB*_min.csv; do
  if [ -f "$f" ]; then
    mv "$f" "${CSV_DIR}/"
  fi
done

echo "Generated header: ${OUT_DIR}/MinPhaseBank.h"
echo "Generated CSVs to: ${CSV_DIR}"
```

---

## 4) Numeric Diff Script

**File:** `scripts/compare_csv.py`

```python
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
```

---

## 5) Plot Script

**File:** `scripts/plot_bank.py`

```python
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
```

---

## 6) Baseline Files

Commit your **current goldens** to `tools/baseline/`:

```
tools/baseline/
  HB63_min.csv
  HB95_min.csv
  HB127_min.csv
```

> These establish "expected" taps. Any change in generation will be compared against them.

---

## 7) How the CI behaves

* On PRs:

  * Builds the tool, regenerates taps, compares to baseline.
  * Posts a **comment** with a markdown table (max |Δsample| and max |Δmag| dB).
  * Uploads artifacts:

    * `out/MinPhaseBank.h` (new header)
    * `out/csv/*.csv` (generated taps)
    * `out/plots/*_ir.png/_step.png/_mag.png` (visual checks)
    * `out/report/minphase_report.md` (detailed summary)
  * **Fails** if thresholds are exceeded (guards against accidental audible changes).

* On push:

  * Same steps; artifacts are available in the run.

**Adjust thresholds** via env:

```yaml
THRESH_ABS: "1e-6"   # sample-domain tolerance
THRESH_DB:  "0.10"   # magnitude-domain tolerance (dB)
```

---

## 8) Developer Notes

* If you **intend** to update taps (e.g., new design), first review the CI plots, then update `tools/baseline/*.csv` with the new generated CSVs and commit in the same PR.
* You can add a second job for **macOS** if you want to validate build portability of the tools as well.
* If your **linear taps** live elsewhere or have different names, update `scripts/build_batch.sh`'s `LINEAR_FILES`.

---

## 9) Optional: Release bundling

Add a tag job to attach `MinPhaseBank.h` and CSVs to GitHub Releases. (Ask and I'll extend the workflow.)

---

**Done.** This gives you reproducible tap generation, numeric gating, visual sanity checks, and friendly PR feedback—all from one CI job.
