# Min-Phase FIR Tools – Complete User Manual

This document provides comprehensive guidance for the min-phase FIR toolchain, including console tools, desktop applications, CI/CD automation, and Field plugin integration.

* `minphase` – converts **one** linear CSV → minimum-phase CSV + header
* `batch_minphase` – converts **many** linear CSVs → per-design CSVs + a combined `MinPhaseBank.h` (with a tiny registry: order → (pointer, length))

Both tools are self-contained. Default FFT backend is **KissFFT** (BSD-style, portable). A JUCE-FFT variant is also described.

---

## Contents

* [1. Prerequisites](#1-prerequisites)
* [2. Directory layout](#2-directory-layout)
* [3. Build (KissFFT backend)](#3-build-kissfft-backend)

  * [3.1 macOS](#31-macos)
  * [3.2 Windows (MSVC)](#32-windows-msvc)
  * [3.3 Linux](#33-linux)
* [4. Usage](#4-usage)

  * [4.1 Single converter: `minphase`](#41-single-converter-minphase)
  * [4.2 Batch generator: `batch_minphase`](#42-batch-generator-batch_minphase)
* [5. Output formats](#5-output-formats)
* [6. Plotting (Python)](#6-plotting-python)
* [7. Integration in plugin](#7-integration-in-plugin)
* [8. Troubleshooting](#8-troubleshooting)
* [9. JUCE-FFT variant (optional)](#9-jucefft-variant-optional)
* [10. FAQ](#10-faq)
* [11. Desktop UI (Optional)](#11-desktop-ui-optional)
* [12. Field Plugin UI Integration](#12-field-plugin-ui-integration)

---

## 1. Prerequisites

* **CMake 3.18+**
* **C++17** compiler

  * macOS: Xcode/Clang 13+
  * Windows: Visual Studio 2019/2022 (MSVC)
  * Linux: GCC 10+ / Clang 12+
* **Python 3.9+** (optional – for plotting only)

> Precision: float is fine for design. If you prefer double precision FFT math, define `KISS_FFT_DOUBLE` at compile time (see build commands).

---

## 2. Directory layout

```
tools/
├── docs/                           # Documentation
│   ├── README.md                   # This file
│   ├── QUICKSTART.md               # Quick start guide
│   ├── CI_SETUP.md                 # CI/CD setup
│   ├── UI_SPECIFICATION.md         # Desktop UI spec
│   └── UI_INTEGRATION_NOTES.md     # Field plugin integration
├── minphase/                       # single-file converter
│   ├── CMakeLists.txt
│   ├── main.cpp
│   └── fft_real.h                  # KissFFT wrapper (rfft/irfft)
├── batch_minphase/                 # multi-file generator
│   ├── CMakeLists.txt
│   ├── batch_minphase.cpp
│   └── fft_real.h
├── examples/
│   ├── HB63_linear.csv
│   ├── HB95_linear.csv
│   └── HB127_linear.csv
├── scripts/                        # CI/CD automation
│   ├── build_batch.sh
│   ├── compare_csv.py
│   └── plot_bank.py
└── .github/workflows/              # GitHub Actions
    └── minphase.yml
```

> Place your **linear-phase** CSV taps in `examples/` or anywhere; commands accept absolute or relative paths.

---

## 3. Build (KissFFT backend)

Each `CMakeLists.txt` uses **FetchContent** to pull KissFFT automatically.

### 3.1 macOS

```bash
cd tools/batch_minphase
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# Optional: double precision
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DKISS_FFT_DOUBLE=ON
```

### 3.2 Windows (MSVC)

```bat
cd tools\batch_minphase
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
REM Optional: /DKISS_FFT_DOUBLE
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DKISS_FFT_DOUBLE=ON
```

### 3.3 Linux

```bash
cd tools/batch_minphase
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
# Optional: double precision
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DKISS_FFT_DOUBLE=ON
```

> Compiler optimizations are enabled by default; you can add `-O3 -ffast-math -fno-math-errno -funroll-loops` on GCC/Clang or `/O2 /fp:fast` on MSVC in the CMake files if desired.

---

## 4. Usage

### 4.1 Single converter: `minphase`

**Purpose:** Convert one **linear-phase FIR** CSV into a **minimum-phase** version with the same magnitude (via cepstral spectral factorization).

```
minphase --in <linear.csv> --out-base <Name> --normalize (none|unity|dc) [--fft-pad <N>]
```

**Arguments**

* `--in` : path to linear CSV (one tap per line; first column used)
* `--out-base` : base name for outputs → `<Name>_min.csv` + `<Name>_min.h`
* `--normalize` :

  * `none` — leave amplitude as is
  * `unity` — normalize L2 energy to 1
  * `dc` — normalize DC gain to 1 (**recommended** for halfband/LP)
* `--fft-pad` : optional FFT size (power-of-two ≥ 2×taps). Defaults to `nextpow2(2*N)`.

**Example**

```bash
cd tools/minphase/build
./minphase --in ../../examples/HB63_linear.csv --out-base HB63 --normalize dc
# Outputs: HB63_min.csv, HB63_min.h
```

---

### 4.2 Batch generator: `batch_minphase`

**Purpose:** Convert many CSVs and emit a single **`MinPhaseBank.h`** with all arrays + a registry.

```
batch_minphase --out-header MinPhaseBank.h --prefix HB --normalize (none|unity|dc) [--emit-csv] --in <file1.csv> <file2.csv> ...
```

**Arguments**

* `--out-header` : output header name (default `MinPhaseBank.h`)
* `--prefix` : symbol prefix for arrays (`HB` → `HB63_min`, `HB95_min`, …)
* `--normalize` : `none|unity|dc` (use `dc` for halfband)
* `--emit-csv` : also write per-design `<Prefix><Order>_min.csv`
* `--in` : list of input CSVs. **Order** is parsed from filename digits (e.g., `HB63_linear.csv` → `63`).

**Example**

```bash
cd tools/batch_minphase/build
./batch_minphase \
  --out-header MinPhaseBank.h \
  --prefix HB \
  --normalize dc \
  --emit-csv \
  --in ../../examples/HB63_linear.csv ../../examples/HB95_linear.csv ../../examples/HB127_linear.csv
```

**Outputs**

* `MinPhaseBank.h` (constexpr arrays + registry)
* `HB63_min.csv`, `HB95_min.csv`, `HB127_min.csv` (if `--emit-csv`)

---

## 5. Output formats

### CSV

* One tap per line (double precision text).
* Use these to plot IR/step or design magnitude.

### C++ header (single converter)

```cpp
#pragma once
#include <array>
namespace MinPhaseTaps {
  constexpr std::array<double, N> HB63_min = { /* taps… */ };
}
```

### Combined C++ header (batch)

```cpp
#pragma once
#include <array>
#include <cstddef>
namespace MinPhaseBank {
  struct TapSet { const double* data; int length; int order; };

  constexpr std::array<double, 63> HB63_min = { /* taps… */ };
  constexpr std::array<double, 95> HB95_min = { /* taps… */ };

  constexpr TapSet registry[] = {
    { HB63_min.data(), (int)HB63_min.size(), 63 },
    { HB95_min.data(), (int)HB95_min.size(), 95 }
  };
  constexpr int registryCount = (int)(sizeof(registry)/sizeof(registry[0]));
}
```

---

## 6. Plotting (Python)

Save as `tools/plotting/plot_ir_step_mag.py` and run with `python plot_ir_step_mag.py path/to/HB63_min.csv`.

```python
import sys, numpy as np
import matplotlib.pyplot as plt

def load_csv(path):
    return np.loadtxt(path, dtype=float, delimiter=',')

def plot_ir_step(h, title):
    step = np.cumsum(h)
    fig1 = plt.figure()
    plt.title(f"Impulse Response – {title}")
    plt.plot(h)
    plt.xlabel("samples"); plt.ylabel("amplitude")
    fig2 = plt.figure()
    plt.title(f"Step Response – {title}")
    plt.plot(step)
    plt.xlabel("samples"); plt.ylabel("amplitude")
    return fig1, fig2

def plot_mag(h, title, fftN=None):
    N = len(h)
    if fftN is None:
        fftN = 1 << (2*N - 1).bit_length()  # nextpow2(2N)
    H = np.fft.rfft(h, n=fftN)
    f = np.linspace(0, 0.5, len(H))  # normalized 0..Fs/2
    mag = 20*np.log10(np.maximum(1e-12, np.abs(H)))
    plt.figure()
    plt.title(f"Magnitude – {title}")
    plt.plot(f, mag)
    plt.xlabel("normalized freq (×Fs)"); plt.ylabel("dB")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot_ir_step_mag.py <taps.csv> [<taps2.csv> ...]")
        sys.exit(1)
    for p in sys.argv[1:]:
        h = load_csv(p)
        base = p.split("/")[-1]
        plot_ir_step(h, base)
        plot_mag(h, base)
    plt.show()
```

**Tip:** Plot **linear vs min-phase** together to confirm same magnitude: load both CSVs and overlay.

---

## 7. Integration in plugin

* Include the header you produced:

  * Single: `#include "HB63_min.h"` → `MinPhaseTaps::HB63_min`
  * Batch: `#include "MinPhaseBank.h"` → `MinPhaseBank::registry`
* In your oversampler **min-phase path**, copy the taps into your FIR stage(s).
* Choose the **order** per stage (e.g., 63/95/127) based on latency/stopband needs.

**Lookup helper (batch header):**

```cpp
#include "MinPhaseBank.h"
using namespace MinPhaseBank;

static const TapSet* findByOrder (int order) {
  for (int i=0; i<registryCount; ++i)
    if (registry[i].order == order) return &registry[i];
  return nullptr;
}
```

---

## 8. Troubleshooting

* **CSV parse errors**: ensure first column is numeric, one tap per line.
* **Weird ringing**: confirm input is **linear-phase** (symmetric). Prefer **odd length**.
* **Magnitude mismatch**: don't double-normalize; for halfband, use `--normalize dc`.
* **Different results across runs**: check FFT size; keep default `nextpow2(2N)`.
* **Performance concerns**: these tools are offline; runtime perf depends on your FIR engine, not these generators.

---

## 9. JUCE-FFT variant (optional)

If you already ship JUCE in your toolchain, you can swap KissFFT for JUCE-FFT:

* Replace `fft_real.h` calls with helpers that use `juce::dsp::FFT`
* Link `juce::juce_core` + `juce::juce_dsp` in CMake
* No other changes needed to the algorithms

This doesn't affect the plugin; it's just for the **tool**.

---

## 10. FAQ

**Q: Do I have to use halfband filters?**
A: No—any linear-phase FIR works. Halfband is common for oversampling ladders.

**Q: Which normalization should I choose?**
A: For oversampling LP/halfband, `--normalize dc` keeps unity at DC. For comparisons, `unity` is fine.

**Q: Can I regenerate taps later without changing plugin code?**
A: Yes—these headers are pure data. Regenerate and rebuild.

**Q: Are min-phase taps sparse like halfband?**
A: No. The min-phase conversion removes the every-other-tap zeros. That's expected.

---

## 11. Desktop UI (Optional)

For a visual, drag-and-drop interface to the min-phase tools, see **[UI_SPECIFICATION.md](UI_SPECIFICATION.md)**. This provides a complete JUCE-based desktop application specification with:

* **3-pane layout**: Files & Sets, Plots, Settings & Actions
* **Visual feedback**: Impulse, Step, and Magnitude plots with overlays
* **Baseline comparison**: Visual diff against reference taps
* **Export integration**: Generate `MinPhaseBank.h` and individual CSVs
* **Threading**: Non-blocking generation with progress feedback
* **Session management**: Save/restore workspace state

The UI specification includes component trees, state models, keyboard shortcuts, accessibility features, and acceptance criteria for building a professional desktop application around the existing console tools.

---

## 12. Field Plugin UI Integration

For integrating the min-phase FIR toolchain with the Field plugin's existing UI system, see **[UI_INTEGRATION_NOTES.md](UI_INTEGRATION_NOTES.md)**. This provides comprehensive technical guidance for:

* **Theme Integration**: Proper `FieldLNF` theme propagation and color management
* **EQ Visualization**: Consistent color usage across EQ components and visualizations
* **ComboBox Behavior**: Abbreviation mode, chevron positioning, and text bounds
* **Performance Optimization**: Shadow caching and rendering efficiency
* **Accessibility**: VoiceOver/Narrator support for custom controls
* **Drop-in Patches**: Ready-to-implement code snippets for common UI issues

The integration notes include detailed code examples, test matrices, and implementation checklists to ensure seamless integration with Field's existing Look & Feel system.

---

Happy generating! If you want a CI job to rebuild the bank on push (with CSV diffs and plots), we can add a GitHub Actions workflow next.
