# Field Ranger – UI Specification

**App type:** Standalone desktop (JUCE)
**Platforms:** macOS (Intel/Apple Silicon), Windows 10+, Linux (optional)
**Purpose:** Visual front-end for the existing console tools (linear→min-phase conversion, batch header bank generation, diffs & plots)
**Branding:** Field Ranger - Patrol quality, phase, and oversampling

---

## 1) User Goals

* Drag/drop one or more **linear-phase CSV** files
* Instantly **see** impulse, step, and magnitude plots
* Convert to **minimum-phase** (cepstral) with selectable **normalization**
* **Compare** generated taps vs a **baseline** (CSV folder)
* **Export**: per-design `_min.csv` and combined `MinPhaseBank.h`
* Keep a simple **history** (recent files), support **save sessions**

---

## 2) High-Level UX

### Layout (3-pane)

* **Left: Files & Sets**

  * **Loaded Files** (list): shows each CSV; parses **Order** (e.g., 63)
  * **Baseline Folder** selector + status (optional)
  * **Batch Queue** (selected files)
* **Center: Plots**

  * Tabs: **Impulse**, **Step**, **Magnitude**
  * Overlay: Baseline vs Generated (toggle)
  * Toolbar: Zoom fit, Zoom in/out, Copy to clipboard, Save PNG
* **Right: Settings & Actions**

  * **Normalization**: None / Unity / DC (radio)
  * **FFT Pad**: Auto / 4K / 8K / 16K / 32K (dropdown)
  * **Output Prefix**: text ("HB")
  * **Emit CSV**: checkbox
  * **Generate** (selected file) / **Generate All**
  * **Export `MinPhaseBank.h`**
  * **Diff Thresholds**: |Δsample| (abs), |Δmag| dB (for on-screen badges)

### Theme & Feel

* **Field Integration**: Use Field's `FieldLNF` LookAndFeel palette (dark, accent color).
* **Consistent Branding**: Match Field's visual design language and component styling.
* **Theme Propagation**: Automatic theme switching with Field's theme system.
* Clear status line at bottom: "3 files loaded • baseline: /path • ready".

### Minimal flow

1. Drag in `HB63_linear.csv` → parsed → plots show **linear**.
2. Click **Generate** → **min-phase** computed → overlay added.
3. Toggle **Baseline** overlay → see differences (badges turn green if under thresholds).
4. Click **Export** → saves `_min.csv` + `MinPhaseBank.h`.

---

## 3) Component Tree (JUCE)

```
FieldRangerMainWindow (DocumentWindow, FieldLNF)
└── AppRoot (Component, Focus Traversal, FieldLNF)
    ├── LeftPanel: FilePane (FieldLNF styling)
    │   ├── FileDropTarget (FieldRendering::drawButtonBackground)
    │   ├── LoadedFilesList (TableListBox, FieldLNF colors)
    │   ├── BaselineFolderRow (FieldRendering::drawButton, FieldRendering::drawLabel)
    │   └── BatchQueueList (optional, FieldLNF styling)
    ├── CenterPanel: PlotPane (FieldLNF colors, FieldRendering plots)
    │   ├── PlotToolbar (FieldRendering::drawButton, FieldRendering::drawComboBox)
    │   ├── PlotTabs (TabbedComponent, FieldLNF tab styling)
    │   │   ├── ImpulsePlot  (Component, FieldRendering::drawPath)
    │   │   ├── StepPlot     (Component, FieldRendering::drawPath)
    │   │   └── MagnitudePlot(Component, FieldRendering::drawPath)
    │   └── Legend/Overlays (FieldRendering::drawLabel, FieldLNF colors)
    └── RightPanel: SettingsPane (FieldLNF styling)
        ├── Normalization (FieldRendering::drawButton, FieldRendering::drawLabel)
        ├── FFT Pad (FieldRendering::drawComboBox)
        ├── Output Prefix (FieldRendering::drawTextEditor)
        ├── Emit CSV (FieldRendering::drawToggleButton)
        ├── Diff thresholds (FieldRendering::drawSlider, FieldRendering::drawLabel)
        ├── Buttons:
        │   ├── Generate (FieldRendering::drawButton)
        │   ├── Generate All (FieldRendering::drawButton)
        │   └── Export MinPhaseBank.h (FieldRendering::drawButton)
        └── Status Hints (FieldRendering::drawLabel, FieldLNF colors)
```

**Plot components** reuse a shared renderer with:

* polyline drawing (juce::Path)
* axes with ticks/labels
* zoom & pan (mousewheel + drag)
* overlay series (linear, generated, baseline) w/ legend

---

## 4) App State Model

```text
AppState
- files: [TapFile]              // loaded CSVs
- selection: index
- baselineDir: optional path
- settings:
    normalization: enum { None, Unity, DC }
    fftPad: enum { Auto, N4096, N8192, N16384, N32768 }
    prefix: string ("HB")
    emitCsv: bool
    threshAbs: double (1e-6)
    threshDb: double (0.10)
- results: map<FileID, GeneratedResult>
- diffs: map<FileID, DiffSummary>   // computed against baseline if present
- history: MRU list of paths
```

**TapFile**

* id, path, name, **order** (int parsed from filename digits)
* rawLinear: vector<double> (loaded on demand)
* meta: length, md5

**GeneratedResult**

* minPhase: vector<double>
* headerSymbol: string (e.g., HB63_min)
* notes: normalization, fftN used, elapsed ms

**DiffSummary**

* maxAbsSample: double
* maxMagDb: double
* pass: bool (under thresholds)

**Persistence**

* Save `Session.json` (relative paths if inside repo)
* Restore last state on launch (optional)

---

## 5) Actions & Threading

* **File load / baseline set:** UI thread → offload file IO + parse to a **BackgroundThread** → post back model update.
* **Generate / Generate All:** UI thread schedules **Worker Thread** job(s).

  * Jobs are small (FFT + logs + IFFT); batch safely queues per file.
  * UI **never blocks**; show small spinner on the button and a progress bar in status line.
* **Plot render:** UI thread only; no allocations in paint; precompute decimated paths for large N.
* **Export:** UI thread kickoff; file IO on background; notify complete (toast/status).

---

## 6) Data Flow

1. **Load CSV** → parse to vector<double> (linear).
2. **Generate** → call the same **cepstral converter** you use in CLI (shared static lib is ideal).
3. **Normalization** → apply per current setting (None/Unity/DC).
4. **Store result** → update plots.
5. **Diff** (if baseline present):

   * load matching baseline `_min.csv` (by order)
   * compute **max |Δsample|** and **max |Δmag| dB** (rfft)
   * show PASS/FAIL with thresholds.
6. **Export**

   * If multiple results exist: emit **`MinPhaseBank.h` + registry**;
   * Always allow per-design `_min.csv` if "Emit CSV" checked.

---

## 7) File & Format Conventions

* **Input**: one tap per line; uses **first column** only.
* **Order**: digits from filename (`HB63_linear.csv` → `63`).
* **Output (batch)**: `MinPhaseBank.h` with arrays and registry:

  ```cpp
  namespace MinPhaseBank {
    struct TapSet { const double* data; int length; int order; };
    extern const TapSet registry[];
  }
  ```
* **Output (single)**: `<Name>_min.csv` and `<Name>_min.h` with a `constexpr std::array<double, N>`.

---

## 8) Plot Details

* **Impulse**: y in linear amplitude; x in samples
* **Step**: cumulative sum; verify no pre-echo for min-phase
* **Magnitude**: rFFT (nextpow2(2N)), x = normalized 0..0.5 (Fs/2); y = dB
* **Overlays**:

  * Linear (source)
  * Generated min-phase (current settings)
  * Baseline min-phase (if folder set)
* **Legend** with colored tags; **pass/fail badge** if diffs under thresholds.

---

## 9) Keyboard & Shortcuts

* `⌘/Ctrl + O`: Open CSV(s)
* `⌘/Ctrl + B`: Choose Baseline Folder
* `Space`: Generate (selected)
* `Shift + Space`: Generate All
* `⌘/Ctrl + E`: Export MinPhaseBank.h
* `⌘/Ctrl + S`: Save Session
* `⌘/Ctrl + ,`: Preferences (theme, precision)

---

## 10) Preferences

* Theme: Dark (default) / Light (optional, if you want)
* Precision: Float (default) / Double (for FFT math)
* Default normalization: DC
* Default thresholds: abs = 1e-6, dB = 0.10
* Default FFT pad: Auto

Persist to a small JSON in user settings.

---

## 11) Error Handling

* Bad CSV: Toast "Can't parse line X (not a number)"; skip line, retain others.
* Empty/broken file: Red tag in list + disabled Generate button.
* Baseline order mismatch: Grey "no match" chip in legend.
* Export path unwritable: Modal alert + choose new folder.
* Background exceptions: Captured → UI toast → open log panel (optional).

---

## 12) Dev Notes

* **Reuse your existing converter** as static library: `libminphase_core`

  * API: `linearToMinPhase(linear, fftN, normMode) -> vector<double>`
  * Keep identical math to CLI so results match CI.
* **FFT backend**: KissFFT (same as CLI) or JUCE-FFT (if you already link JUCE).
* **No blocking** on message thread; use `ThreadPoolJob` or `juce::Thread` (single worker is enough).
* **Determinism**: Ensure identical results across UI/CLI (same FFT length, same eps).
* **Unit tests** (JUCE UnitTestRunner) for:

  * Order parsing
  * Normalizations
  * Magnitude parity check (linear vs min-phase) within tolerance
  * Diff math vs CLI script

---

## 13) Accessibility & UX Polish

* All controls reachable by tab; labels with accessible names.
* Right panel shows **live derived info** (tap count, FFT N, latency estimate ~N/2 at OS stage).
* "Copy PNG" copies current plot to clipboard.
* **Drag-in folder** with multiple CSVs: auto-filter `*_linear.csv`.
* MRU list in File menu.

---

## 14) Acceptance Criteria

* Load 3 CSVs (HB63/HB95/HB127), **Generate All**, **Export** → a valid `MinPhaseBank.h` compiles into plugin and matches CLI.
* With baseline folder set, each generated result shows **PASS** under default thresholds.
* Magnitude overlays: **linear** vs **min-phase** are visually identical (within ~0.1 dB).
* **No UI hitches** during generation; spinner/progress visible; cancellation supported.
* Session save/load restores file list, baseline path, and settings.

---

## 15) Nice-to-haves (phase 2)

* **Audio audition**: convolve a test click/loop and A/B linear vs min-phase.
* **Tap editor**: normalize, trim, smooth tiny ends, re-center.
* **Design helper**: rudimentary Parks-McClellan or windowed-sinc designer for halfband taps (linear), then convert inside the app.
* **CI hook**: Button to run the CI locally and open the artifact folder.

---

## 16) Build Targets

* `FieldRanger` (app)

  * deps: `libminphase_core` (shared with CLI), JUCE modules (`juce_gui_basics`, `juce_graphics`, `juce_dsp` if JUCE-FFT)
  * **Field Integration**: Links against Field's `FieldLNF` and `FieldRendering` for consistent theming
* `libminphase_core` (static)

  * exports: converter + normalization + FFT adapter
* Reuse `fft_real.h` if keeping KissFFT; otherwise compile JUCE-FFT adapter.
* **Field Look & Feel**: Integrate with Field's existing theming system for seamless visual consistency.

---

## 17) Strings (copy)

* "**Field Ranger** - Patrol quality, phase, and oversampling"
* "Drop linear-phase CSV files here or **Open…**"
* "Baseline: Not set" / "Baseline: …/tools/baseline (3 matches)"
* "Generated (DC, auto FFT)"
* "Exported **MinPhaseBank.h** to /path/out/"
* "PASS: |Δsample| ≤ 1e-6 and |Δmag| ≤ 0.10 dB"
* "FAIL: exceeds thresholds (see Plots)"
* "**Field Integration**: Using Field's Look & Feel system"

---

## 18) Laymen explanation (for the help/about panel)

**Field Ranger** converts **linear-phase** filter taps (perfectly even, great for parallel) into **minimum-phase** taps with the same frequency shape but **no pre-echo**, which often feels more **punchy**. You can **see** the impulse and step responses, and **compare** with a baseline. Then export a single header to use in your **Field plugin's** oversampling.

**Field Integration**: Uses the same visual design and theming as your Field plugin for a seamless workflow.

---

If you want, I can also draft **UI wireframes** (as PNGs) or a **component skeleton** you can paste into a JUCE project next.
