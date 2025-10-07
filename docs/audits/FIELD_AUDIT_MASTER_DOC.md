Absolutely. Here’s a single, consolidated “master doc” that merges the two audits, keeps your technical depth, adds clear dividers, a compact index, and a dev note up top. I avoided aggressive shortening and aligned paths to your **current tree**.

---

# FIELD — Master Architecture & Triage (Phase 1→2)

> **Dev Note (read first):**
> This doc is the **authoritative** reference for Field’s code structure, DSP routing, dev toggles, and the Live glitch triage. Legacy include paths still on disk are **poisoned** and must not be used. Audio thread reads from APVTS are **forbidden**; use `HostParams` snapshots only. The clean signal path is **locked in** (non-aliased DRY/WET + ordered blend). All dev-only scaffolding compiles out in Release.

**Last updated:** 2025-10-07 • **Branch:** `feature` • **Maintainers:** @trail @grant

---

## Index

1. [Executive Summary](#executive-summary)
2. [Target Topology & Source Layout](#target-topology--source-layout)
3. [Guardrails & Principles](#guardrails--principles)
4. [Routing Lock-In (Glitch Fix)](#routing-lockin-glitch-fix)
   - [2025-10-07 — Routing Update (smoothing, voicing, latency)](#2025-10-07-%E2%80%94-routing-update)
5. [Phase 1 Results](#phase-1-results)
6. [Phase 2 Scope & Results](#phase-2-scope--results)
7. [Work Orders (WO) Ledger](#work-orders-wo-ledger)
8. [Parameter Integration Plan — Full Implementation (Hardened)](#parameter-integration-plan--full-implementation-hardened)
9. [CI Tripwires & Build Fences](#ci-tripwires--build-fences)
10. [Verification Matrix](#verification-matrix)
11. [Risk & Mitigation](#risk--mitigation)
12. [Rollback Plan](#rollback-plan)
13. [Cleanup Map (dev artifacts)](#cleanup-map-dev-artifacts)
14. [Editor Lifecycle — UI Close Suspension Fix](#editor-lifecycle--ui-close-suspension-fix)
15. [UI Stability — BandTab Width Crash Fix](#ui-stability--bandtab-width-crash-fix)
16. [Master Param Registry (Source of Truth)](#master-param-registry-source-of-truth)
17. [Live Glitch Triage Log](#live-glitch-triage-log)
18. [Ownership & Cadence](#ownership--cadence)
19. [Appendix A — Code Sketches](#appendix-a--code-sketches)
20. [Appendix B — Tripwire Patterns](#appendix-b--tripwire-patterns)
21. [CI — Param ID Drift Check](#ci--param-id-drift-check)
22. [DynEQ — 24-band Spec & Wiring Plan](#dyneq--24-band-spec--wiring-plan)
23. [DynEQ — Hold/Program-Dependent Release + UI bindings](#dyneq--holdprogram-dependent-release--ui-bindings)

---

## Executive Summary

* **Clean architecture is in place**: engines in `Source/engines/**`, orchestration in `Source/modules/**`, processor glue in `Source/processor/**`, and shared signal/runtime/params in `Source/core/**`.
* **Glitch fixed** by **processor routing**: non-aliased DRY capture, WET built in a separate buffer, ordered blend on host output, single per-block snapshot.
* **Reverb/FDN exonerated**: wrap mirroring, DC guard, 5 ms ramp are kept; SpikeSilencer remains debug-only.
* **APVTS on audio thread eliminated**: double-buffered `HostParams` snapshot, listeners refresh on message thread.
* **Legacy include paths poisoned**: headers under `shared/Core/**` and `features/reverb/DSP/**` error out if included; CI tripwires guard regressions.

---

## Target Topology & Source Layout

```
Source/
├─ app/                         # host notes only
├─ core/                        # params, runtime, signal utils, telemetry
│  ├─ params/ (ParamIDs.h, ParamLayout.cpp, Snapshot.h)
│  ├─ runtime/ (LatencyManager.h, RebuildGate.h, LiveSwapPlanner.h, ...)
│  ├─ signal/  (SignalGraph.h, OversamplingStage.h, FrameAccumulator.h, Sanitize.h, ...)
│  └─ telemetry/ (DebugTelemetry.h, GlitchHunt.h, StateSanity.h, LiveSwapHUD.h)
├─ engines/                     # pure DSP (no UI)
│  └─ reverb/DSP (ReverbFDN.h, ReverbEQ.*, DecayRateEQ.*, SimdBiquad.h), etc.
├─ modules/                     # nodes + FieldChain (zero-alloc stages)
├─ processor/                   # JUCE AudioProcessor glue
├─ ui/ + features/              # visual only; no audio buffer access
├─ presets/
└─ tests/ (offline & perf)
```

**On disk but fenced:** `shared/Core/**` and `features/reverb/DSP/**` (poisoned headers) and `features/reverb/Core/**` (UI wrapper only).

---

## Guardrails & Principles

* **One DSP home per engine.** Reverb DSP lives under `engines/reverb/**` only.
* **No APVTS reads** on audio thread. Use `HostParams` snapshot (`snapshotPtr_`).
* **Prepare parity**: float/double prepared identically; debug cookies assert parity.
* **Tripwires**: CI fails on legacy includes, APVTS touches in callbacks, and dry/wet alias patterns.
* **Latency single source**: `core/runtime/LatencyManager` applied message-thread-side.

---

## Routing Lock-In (Glitch Fix)

**Required per-block sequence (float & double paths):**

1. **Snapshot once:** `const auto hp = *snapshotPtr_;`
2. **Deep-copy DRY** from host buffer into `dryCopy`.
3. **Build WET** in a separate buffer: `fieldChain.setParameters(hp); fieldChain.process(wetBlock);`
4. **Bypass**: copy `dryCopy` → `buffer`; keep meters ticking; return.
5. **Ordered blend**: `out = (1−mix)*DRY + mix*WET`.
6. **Meters/Viz** updated on the **post** buffer.

**Don’ts:** no in-place “processed − dry”, no reuse of `buffer` as both DRY source and WET destination.

### 2025-10-07 — Routing Update

- Input gain now applied pre-chain to both DRY and WET paths (float and double).
- Equal-power blend retained; `gain.output.db` applied post-blend.
- Constant-power balance applied post-blend on stereo output.
- `global.bypass` returns DRY-only while meters/viz still tap the returned buffer.
- Float/double parity maintained; build verified across AU/VST3/Standalone.

### 2025-10-07 — Routing Update (smoothing, voicing, latency)

- Added light automation smoothing:
  - Mix equal-power gains smoothed (~2 ms) to avoid zipper during fast automation.
  - Post output gain smoothed (~7 ms) for click-safe gain rides.
- Reverb voicing now fanned out via `FieldChain` → `Node_Reverb::Params` (`preDelaySec`, `sizeNorm`, `dampingHz`).
- Removed internal chain reverb wet to avoid accidental double-mix; global `mix.wet01` remains single source of DRY/WET.
- Latency reporting hooked on the message thread using lookahead-derived samples (host-rate). FIR/engine latency remains reported by engines when applicable.
- Verified builds; float/double paths mirror smoothing and routing.

---

## Phase 1 Results

* Engines rehomed; processor lifecycle hardened; FieldChain stage list; tripwires active; legacy reverb headers poisoned; tests compile.

---

## Phase 2 Scope & Results

* **Stability & parity:** fade-in, wrap mirror, DC guard, SpikeSilencer (Debug), float/double prepare parity.
* **Live glitch isolation:** FDN/Phase cleared; processor routing implicated and fixed.
* **SignalGraph/OS seam**: frame guards kept; sanitize once; insert fade optional.

---

## Work Orders (WO) Ledger

**Legend:** ✅ landed • 🧪 tested • ☣️ poisoned • 🔒 CI tripwire • 🔧 planned/landing

* **WO-55** — FDN wet fade-in + wrap mirror: ✅🧪
* **WO-56** — SpikeSilencer (Debug only): ✅
* **WO-57** — Dev cut switches + silence contract: 🔧 (dev-only; keep optional)
* **WO-58** — Double-path parity & prepare cookies: ✅🧪
* **WO-59** — Ableton Insert-Safe hardening: ✅
* **WO-60** — Wet-path DC guard & clamp (Debug clamp): ✅
* **WO-61** — APVTS Freeze (snapshot): ✅🔒
* **WO-62** — Transport/tempo probes: 🔧
* **WO-63** — pluginval + Host Matrix CI: 🔧
* **WO-64** — MasterSafe (float-only minimal chain on Master): 🔧
* **WO-65** — Phase engine hard toggle (dev-only): 🧪 / landing

---

***

## Parameter Integration Plan — Full Implementation (Hardened)

> **Contract:** Zero APVTS reads in `processBlock`. One snapshot read per block. No dry/wet aliasing. Latency always correct after any param flip that affects it.

## 1) Inventory & IDs (authoritative)

**Keep (existing):**

* `kChainDelayEnable` (bool)
* `kChainDynEqEnable` (bool)
* `kChainReverbEnable` (bool)
* `kQualityOSFactor` (int: {1,2,4,8})
* `kReverbLinearPhase` (bool)
* `kReverbFIRHalfLen` (int samples)
* `kDynEqLookAheadMs` (float ms)
* `kDelayLookAheadMs` (float ms)
* `kDevHudEnable` (bool, dev only)

**Add (right-side & master controls):**

* **Mix**

  * `mix.wet01` (0…1) — **equal-power law** in blend (see §4).
* **Output**

  * `gain.output.db` (−24…+24 dB) — log scale; snap to 0 dB detent.
  * `pan.balance` (−1…+1) — optional; constant-power.
* **Tone (broadband, “bold & musical”)**

  * `tone.tilt.dbPerOct` (−6…+6 dB/oct), default **+1.5 dB/oct**.
  * `tone.bass.db` (−12…+12 dB @ 120 Hz shelf), default **+2 dB**.
* **Reverb voicing**

  * `reverb.preDelay.ms` (0…120 ms, **default 20 ms**).
  * `reverb.size.norm` (0…1, perceptual map to late density).
  * `reverb.damping.hz` (1 kHz…16 kHz, log taper, default **6 kHz**).
* **Phase/Imager (if exposed)**

  * `phase.mode` (enum, dev)
  * `imager.width` (0…200 %, default **115 %** for “heard” demos)

> **Naming:** snake groups with dot-paths (e.g., `reverb.size.norm`). **IDs are final** once merged; deprecate via alias redirect only, never rename in place.

---

## 2) Layout & Ranges (ParamLayout.cpp)

**Design rules**

* Use **log tapers** for frequency, time, and gain (dB).
* “Heard immediately” defaults: slightly wet, slight tilt up, width >100 %, mild pre-delay.
* Provide **text converters** with units (ms, Hz, dB) and **step** that feels right for automation.

**Suggested specs**

* `mix.wet01`: range [0, 1], step 0.005, skew 0.5, default **0.33**.
* `gain.output.db`: [−24, +24], step 0.1, default **0**, text `±x.x dB`.
* `tone.tilt.dbPerOct`: [−6, +6], step 0.1, default **+1.5** (audible).
* `tone.bass.db`: [−12, +12], step 0.1, default **+2.0**.
* `reverb.preDelay.ms`: [0, 120], step 0.5, default **20**; **log-ish** display: 0-10 by 1, 10-120 by 5.
* `reverb.size.norm`: [0, 1], step 0.01, default **0.62** (modern).
* `reverb.damping.hz`: [1000, 16000], **log**, default **6000**.
* `imager.width`: [0.0, 2.0], step 0.01, default **1.15**.
* `kQualityOSFactor`: {1,2,4,8}, default **2** for demos, **1** in CI.

**Dev behind flag**

* Any `dev.*` and experimental enums inside `#if FIELD_DEV_HUD_ON`.

---

## 3) Snapshot & Safety (Snapshot.h)

**Snapshot struct** (excerpt):

```cpp
struct ChainParamSnapshot {
  // mix/output
  float wet01{0.33f}, dry01{0.67f};
  float outGainLin{1.0f};          // precomputed from dB

  // tone
  float tilt_dB_per_oct{+1.5f};
  float bassShelf_dB{+2.0f};

  // reverb
  bool  rvEnabled{true};
  float preDelaySec{0.020f};
  float sizeNorm{0.62f};
  float dampingHz{6000.0f};

  // imager
  float width{1.15f};

  // pipeline/quality
  int osFactor{2};
  bool dynEqOn{false};
  bool delayOn{false};
  int dynEqLookaheadSamples{0};
  int delayLookaheadSamples{0};

  // dev gates (debug only)
  #if JUCE_DEBUG
  bool devHud{false};
  #endif
};
```

**buildSnapshot(...)** (message thread only):

* Read via `SafeParamGate`.
* Convert:

  * `outGainLin = dbToLin(gain.output.db)`
  * `dry01 = 1.0f - wet01`
  * `preDelaySec = clamp(ms / 1000.0f)`
  * `dynEqLookaheadSamples = msToSamp(sr, dynEqMs)`
  * `delayLookaheadSamples = msToSamp(sr, delayMs)`
* **Never** touch APVTS on audio thread.

---

## 4) Blend Law & Smoothing (musical & measurable)

**Equal-power mix** (more “musical” than linear):

```cpp
// snapshot precomputes wet01 ∈ [0,1]
const float theta = hp.wet01 * juce::MathConstants<float>::halfPi;
const float a = std::cos(theta);   // dry gain
const float b = std::sin(theta);   // wet gain
// out = a*dry + b*wet
```

* Keeps perceived loudness stable across mix sweep.
* **Automation smoothing:** 1–3 ms one-pole on the **gains** (not on samples) to avoid zipper while staying snappy:

```cpp
// one-pole smoothing per channel (Release only if needed)
gDry.setTimeMs(2.0); gWet.setTimeMs(2.0);
```

**Output gain smoothing:** 5–10 ms (tiny), click-safe for automation:

* Attack 2 ms, release 10 ms if you want slight “feel”.

---

## 5) Processor Glue (double-buffer snapshot)

* Rebuild on parameter change (message thread), swap `snapshotPtr_` atomically.
* In `processBlock` (float & double):

  * Read **once**: `const auto hp = *snapshotPtr_;`
* **DRY deep-copy**, **WET separate buffer**, **equal-power blend** as above.
* Apply `outGainLin` last (with mini smoothing); we currently multiply `out = (a*d + b*w) * outGainLin` in both float/double paths.
  * Meters/correlation read **post-blend**.

---

## 6) FieldChain API & Fan-out

**FieldChain.h**

```cpp
struct GainParams  { float outGainLin{1.f}; };
struct MixParams   { float wet01{0.33f}; float dry01{0.67f}; }; // if a node needs it
struct ToneParams  { float tilt_dB_per_oct{0.f}; float bass_dB{0.f}; };
struct ReverbParams{
  bool  enabled{true};
  float preDelaySec{0.02f};
  float sizeNorm{0.62f};
  float dampingHz{6000.f};
  float wet01{0.33f};               // if engine uses internal wet path
};
struct DelayParams { bool enabled{false}; int lookaheadSamples{0}; };
struct DynEqParams { bool enabled{false}; int lookaheadSamples{0}; };
struct ImagerParams{ float width{1.15f}; };

void setParameters(const field::params::ChainParamSnapshot& s) {
  gainP_.outGainLin      = s.outGainLin;
  toneP_.tilt_dB_per_oct = s.tilt_dB_per_oct;
  toneP_.bass_dB         = s.bassShelf_dB;
  rvP_.enabled           = s.rvEnabled;
  rvP_.preDelaySec       = s.preDelaySec;
  rvP_.sizeNorm          = s.sizeNorm;
  rvP_.dampingHz         = s.dampingHz;
  rvP_.wet01             = s.wet01;
  deP_.enabled           = s.dynEqOn;
  deP_.lookaheadSamples  = s.dynEqLookaheadSamples;
  dlP_.enabled           = s.delayOn;
  dlP_.lookaheadSamples  = s.delayLookaheadSamples;
  imP_.width             = s.width;
}
```

* Each node/engine has a **POD** param struct, no locks, no trees.
* If Reverb mixes internally, pass `wet01` in; otherwise only use at processor.

---

## 7) Latency & Reporting (accurate by construction)

**Conversions**

```cpp
inline int msToSamp(double sr, double ms) {
  return (int) juce::roundToInt(std::max(0.0, ms) * 0.001 * sr);
}
```

* **Lookahead** latency = `max(delay.lookahead, dynEq.lookahead)` (or sum if cascaded pre-ring buffers).
* **Linear-phase FIR**: group delay = `firHalfLen` samples (per engine).
* **Oversampling**: reported latency remains **host-rate samples**; if a stage introduces `L_os` at OS, convert to host domain if needed.

**Plumbing**

* Compute per-stage in `FieldChain::recomputeLatency()` and set `latencySum_`.
* Processor calls `LatencyManager` → `setLatencySamples(latencySum_)` on **message thread** after snapshot change that impacts latency.

---

## 8) Correlation & Meters (correct taps)

* **Tap source:** post-blend output buffer (what the DAW receives).
* **Ballistics:** RMS window 300 ms for peak/RMS, **correlation** window 20–80 ms Hann (fast but stable).
* **Bypass or DRY-only:** we still meter the buffer we return (DRY).

---

## 9) Tests & Verification (must pass)

* **Unity nulls:**

  * `mix=0` → out ≈ DRY (< −140 dBFS).
  * `mix=1` → out ≈ WET (< −140 dBFS).
* **Param smoke:** flip each new ID; hash of a 3-second render changes.
* **Parity:** float vs double at OS=1x null < −140 dBFS.
* **Matrix:** 44.1/48/96 × 64/128/512 renders are glitch-free; reported latency stable & correct.

---

## 10) CI Fences (grep tripwires)

* **No APVTS in callbacks:**
  `AudioProcessorValueTreeState|getRawParameterValue\s*\(.*\).*process(Block|\()`
* **No dry/wet alias use of host buffer:**
  Heuristic regex (keep tight):

  ```
  (AudioBlock<[^>]+>\s*\(\s*buffer\s*\).*(\+=|-=)|=\s*.+\bbuffer\b.*\bbuffer\b)
  ```
* **Legacy includes:**
  `\bDspRuntimeConfig\b|#include\s*"shared/Core/|#include\s*"features/.*/DSP/`

---

## 11) Rollout Order (small PRs)

1. **IDs** in `ParamIDs.h` + **layout** in `ParamLayout.cpp`.
2. Extend **snapshot** + converters.
3. **FieldChain::setParameters** + node param structs.
4. **Latency mapping** + reporting.
5. Tests + CI tripwires.
6. Bind UI.
7. Host matrix smoke.

---

## 12) “Bold & Musical” Defaults (ship demo-friendly)

* `mix.wet01` = **0.33** (audible reverb)
* `reverb.preDelay.ms` = **20 ms** (clarity/punch)
* `reverb.size.norm` = **0.62** (modern plate-ish density)
* `reverb.damping.hz` = **6 kHz** (smooth top)
* `tone.tilt.dbPerOct` = **+1.5 dB/oct** (subtle forward)
* `tone.bass.db` = **+2.0 dB** (weight)
* `imager.width` = **1.15** (wider but safe)
* `gain.output.db` = **0 dB** (detent)

These translate to **immediately heard** changes while staying safe for level/mono.

---

## 13) Implementation Notes (sharp edges handled)

* **Automation bursts:** if needed, smooth only **gains** (`a`,`b`, output lin), not the underlying samples; keep smoothing ≤ 3 ms for feel.
* **PreDelay**: if implemented with a circular buffer, **report latency 0** (it’s musical, not lookahead).
* **OS factor** changes: trigger **re-prepare**/crossfade if latency-stable; otherwise defer until stop or do live-swap if same latency.
* **DSP internal wet**: if an engine maintains its own wet, **do not** double-mix; either supply `wet01` to engine **or** do global blend, never both.

---

### Tiny drop-in helpers

```cpp
inline float dbToLin(float dB) {
  return dB <= -80.f ? 0.f : std::pow(10.f, dB * 0.05f);
}

inline std::pair<float,float> equalPowerGains(float wet01) {
  const float t = juce::jlimit(0.f, 1.f, wet01) * juce::MathConstants<float>::halfPi;
  return { std::cos(t), std::sin(t) }; // (dry, wet)
}
```

**Blend (inside processBlock):**

```cpp
auto [gDry, gWet] = equalPowerGains(hp.wet01);
for (int c=0; c<C; ++c) {
  auto* out = buffer.getWritePointer(c);
  const float* dry = dryCopy.getReadPointer(c);
  const float* wet = wetBuf.getReadPointer(c);
  for (int n=0; n<N; ++n) {
    float y = gDry * dry[n] + gWet * wet[n];
    out[n] = y * hp.outGainLin;
  }
}
```

---

## CI Tripwires & Build Fences

**CMake targets (examples):**

* `ci_tripwire_legacy_includes`
* `ci_tripwire_apvts_audio_thread`
* `ci_tripwire_drywet_alias`

**Include fences:** Targets **do not** add `Source/shared/Core` or `features/reverb/DSP` to include dirs. Poisoned headers `#error` out if included.

---

## Verification Matrix

| Area          | Check                            | Tooling                 | DoD               |
| ------------- | -------------------------------- | ----------------------- | ----------------- |
| Insert safety | Live Master insert while playing | manual + logs           | no pop/crash      |
| Wet silence   | Silent in → silent out           | unit (silence contract) | < −140 dBFS       |
| Parity        | float vs double @ OS=1x          | AB null / hash          | < −140 dBFS       |
| Glitch free   | 64/128/512 buffers, Master/Track | renders + spectrogram   | no periodic spurs |
| APVTS hygiene | No APVTS reads in audio thread   | CI grep                 | clean             |
| pluginval     | AU+VST3                          | pluginval               | pass              |
| Host matrix   | 44.1/48/96 × 64/128/512          | script                  | pass              |

---

## Risk & Mitigation

* **Heisenbugs masked by dev fades** → SpikeSilencer is one-shot, Debug-only, logged; Release unaffected.
* **Param drift/parity issues** → Prepare cookies assert parity + single `prepareCommon`.
* **Tripwire fatigue** → Narrow patterns; whitelist tests; fail with actionable messages.
* **Transport coupling** → Independent toggles: ignore playhead vs disable tempo sync.

---

## Rollback Plan

* Dev scaffolding is additive & guarded—disable `FIELD_DEV_HUD_ON` and dev params to restore Release behavior.
* Remove SpikeSilencer include/hook with one diff.
* Revert debug parity asserts (cookies) if needed; architecture stays Phase-1-clean.

---

## Cleanup Map (dev artifacts)

**Keep (recommended):**

* FDN wrap mirror, 5 ms wet fade-in, DC blocker; APVTS snapshot.

**Optional remove (dev-only):**

* SpikeSilencer, parity cookies, `dev.*` transport/cut/phase/master flags, DBG logs.

**Revertable behavior:**

* Non-latching bypass → original early-return passthrough.

**Quick searches:**

* Dev params/toggles: `dev.dsp.cut|dev.transport.ignore|dev.tempoSync.off|dev.master.safe|dev.phase.off`
* Snapshot bits: `snapshotA_|snapshotB_|snapshotPtr_|makeHostParams|HostParams`
* Parity cookies: `cookieF_|cookieD_|PrepCookie`
* Debug logs: `DBG\(\"\[Field\]`
* FDN debug: `SpikeSilencer|DcBlock|postWrapPad\(|fadeSamplesLeft`

---

## Editor Lifecycle — UI Close Suspension Fix

**Symptom:** Closing the plugin UI via the host’s close control muted audio while the DAW playhead continued to advance.

**Root cause:** During editor teardown we suspended audio processing (`suspendProcessing(true)`) to avoid UAF during UI destruction, but didn't resume it afterward when only the editor was closing (processor still active).

**Fix (landed):** Resume processing after UI teardown completes.

- File: `Source/shared/ui/Managers/CleanupManager.cpp`
- Function: `CleanupManager::performCleanup()`
- Change: call `editor.proc.suspendProcessing(false);` at the end of cleanup.

**Why safe:** Suspension still protects the teardown window; resume occurs only after attachments/listeners/timers/callbacks are cleared, so audio continues without the editor.

**Verification:** Close the editor during playback in Live; audio continues, playhead advances, no mute. Build matrix unchanged.

**Follow-up:** Consider an offline harness test that simulates editor open/close and asserts `isSuspended()==false` post-close when processor remains active.

---

## UI Stability — BandTab Width Crash Fix

**Symptom:** Scrolling the Width knob in BandTab crashed Live with `EXC_BAD_ACCESS` in `juce::CharPointer_ASCII::isEmpty()` from a `String(const char*)` path inside `BandControlsPane::makeCell`.

**Root cause:** The UI code captured a raw `const char*` parameter ID (`pid`) by reference in lambdas and used it to build `juce::String` later, after the original pointer’s lifetime was not guaranteed — causing invalid memory access.

**Fix (landed):** Copy `pid` into a stable `juce::String paramId` immediately and capture only values in callbacks.

- File: `Source/features/band/BandControlsPane.h`
- Function: `makeCell(...)`
- Changes:
  - `const juce::String paramId = (pid != nullptr ? juce::String(pid) : juce::String());`
  - Validate `paramId` exists in APVTS before creating attachments
  - Compute `valueDecimals` once and capture by value in `applyLabel` lambda
  - `s.onValueChange = [applyLabel]() { applyLabel(); };`

**Why safe:** Eliminates dangling pointer risk; all captured data are value-captured and independent of external lifetimes.

**Verification:** Build succeeded; Live test should no longer crash when adjusting Width.

**Follow-up:** Consider centralizing a safe `makeSliderAttachment(apvts, juce::String id, juce::Slider&)` helper to standardize this pattern across panes.

---

## Master Param Registry (Source of Truth)

**Location:** `docs/params/ParamRegistry.yaml`

**Purpose:** Single authoritative registry for parameter IDs, names, ranges/tapers, defaults, owners, and consumers. This file drives:

- Documentation and UI copy
- Sanity checks vs code (CI tripwire planned)
- Uniform wiring from APVTS → Snapshot → FieldChain → Nodes/Engines

**Contents:** Core enable toggles, oversampling, mix/output, tone tilt/bass, reverb voicing, imager width, and guarded dev flags. Defaults align with “bold & musical” settings.

**Integration status:**
- IDs and ranges mirrored in `core/params/ParamIDs.h` and `ParamLayout.cpp`
- Snapshot captures values; `FieldChain::setParameters(...)` fans-out PODs
- `imager.width` consumed by `Node_Imager`
- Equal-power blend + post-blend output gain live in processor
- CI Param ID drift check added (`tools/check_param_ids.py`, CMake target `check_param_ids`)
- DynEQ registry appended (global + templated per-band), snapshot structs and fan-out stubs landed

**Next:**
- CI: grep/diff code IDs against YAML; fail on mismatch
- Wire `tone.tilt.dbPerOct` / `tone.bass.db` into node/engine
- Wire `reverb.preDelay.ms`, `reverb.size.norm`, `reverb.damping.hz` into `Node_Reverb`
- Build DynEQ snapshot builder and full `Node_DynEq` parameter map; add per-band UI bindings

---

## Live Glitch Triage Log

### 2025-10-05 (Day 1)

* A mute → **clean**; B dry-only → **clean**; C wet-only → **glitch**; C.1 FDN pure-thru → **glitch**; D Phase OFF → **glitch**; E postWetBus bypass → **glitch** (earlier than post-sum).
* Recorder showed nothing in Standalone; Live likely cached old build.

### 2025-10-05 (Evening) — Consolidated

* FDN read/write/wrap/Hadamard **not** the source; removing Reverb unchanged; **passthrough clean** → processor path implicated.

### 2025-10-06 — Batch 2 (SignalGraph, Chain, Routing)

* Neutered seam **clean** once; later host-frame variants still **glitch**.
* **SignalGraph-only path clean**, **normal Gain/Meter path glitch** → **processor routing** around chain call implicated.
* **Fix landed**: non-aliased DRY/WET + ordered blend → **glitch removed** at 64/128/512; meters/correlation restored.

---

## Ownership & Cadence

* **DSP/Engines:** @trail
* **Processor/Glue:** @trail, @grant
* **CI/Matrix:** @trail
* **Cadence:** WO clusters: 55→56→57/58→60→61→62→63; run host matrix after each cluster.

---

## Appendix A — Code Sketches

**A1. DC Block (post wet-sum):**

```cpp
struct DcBlock {
  float z{0.f}; float a{0.995f};
  float process(float x) noexcept { float y = x - z; z = x + y * a; return y; }
};
```

**A2. Wrap mirroring (pad after wrap):**

```cpp
inline void postWrapPad(std::vector<float>& v, int pad=3) noexcept {
  const int len = (int)v.size() - pad; if (len <= 0) return;
  for (int i=0; i<pad; ++i) v[(size_t)len + (size_t)i] = v[(size_t)i];
}
```

**A3. Oversampling guard (frame mismatch → bypass):**

```cpp
void processFrame(juce::dsp::AudioBlock<T> frame) {
  if (!enabled || power==0 || !oversampling) return;
  jassert((int)frame.getNumSamples()==frameSize);
  if ((int)frame.getNumSamples()!=frameSize) return;
  auto up = oversampling->processSamplesUp(frame);
  oversampling->processSamplesDown(frame);
}
```

**A4. Prepare cookies (parity asserts, Debug only):**

```cpp
#if JUCE_DEBUG
struct PrepCookie { double sr; int block, chans, bytes; bool set; };
mutable PrepCookie cookieF_{}, cookieD_{};
#endif
```

---

## Appendix B — Tripwire Patterns

* Legacy scope: `\bDspRuntimeConfig\b|#include\s*"shared/Core/|#include\s*"features/.*/DSP/`
* Audio thread param reads: `AudioProcessorValueTreeState|getRawParameterValue\s*\(.*\).*process(Block|\()`
* Dry/Wet alias heuristic: `(AudioBlock<[^>]+>\s*\(\s*buffer\s*\).*(\+=|-=)|=\s*.+\bbuffer\b.*\bbuffer\b)`

---

## CI — Param ID Drift Check

We enforce that `docs/params/ParamRegistry.yaml` stays in lockstep with code IDs.

- Script: `tools/check_param_ids.py` (Python 3.11+)
- CMake target: `check_param_ids` (runs on build if Python found)
- CI: optional GitHub Action job to run the script on push/PR

Behavior:
- Scans YAML `params[].id` and string literals in `Source/**` that look like `group.id` paths
- Reports and fails on drift (IDs in YAML not in code, or vice-versa)
- Ignores templated IDs like `dyneq.b[i].*` (expanded by code)

This prevents “phantom” parameters in docs or stale IDs in code.

---

## DynEQ — 24-band Spec & Wiring Plan

Goal: Best-in-class dynamic EQ with 24 bands, surgical yet musical; fast automation, clean lookahead, and robust null tests.

Spec highlights:
- 24 bands, types: bell / low-shelf / high-shelf / notch / bandpass / tilt
- Per-band: freq (20–20k, log), Q (0.1–24, log), static gain, range dB, ratio,
  threshold (dBFS), soft knee, attack (0.1–200 ms), release (5–2000 ms), hold, makeup,
  sidechain (band/wide/external), per-band wet, direction (down/up/both)
- Global: enable, mode (down/up/split/expand), channel link (stereo/dualMono/MS), lookahead (ms)

Wiring plan:
- Registry: add `dyneq.enabled`, `dyneq.global.*`, and `dyneq.b[i].*` IDs
- Snapshot: `DynEqSnap { enabled, globalMode, link, lookaheadSamples, band[24] }`
- FieldChain: map snapshot → `Node_DynEq::DynEqParams` and call `setParameters`
- Node DSP (follow-ups): RBJ filters; band/wide sidechain; hybrid RMS/peak; soft knee; PDR release;
  single global lookahead FIFO; per-band makeup; equal-power band wet

Done criteria:
- Lookahead adds correct host-rate latency via message thread
- Automation is zipper-free; nulls clean; per-band GR behaves for down/up/split

Status (2025-10-07):
- Registry: global + templated per-band IDs added to `docs/params/ParamRegistry.yaml`.
- Snapshot: `DynEqSnap` + `DynEqBandSnap` implemented; builder populates all 24 bands.
- FieldChain: full fan-out to `Node_DynEq::DynEqParams` (global mode/link, lookahead, per-band fields).
- Node: parameter struct + `setParameters` stub landed (DSP to follow).

Next steps:
- Implement `Node_DynEq` detection and gain law (RBJ filters, hybrid RMS/peak, soft knee, PDR release).
- Add UI bindings for per-band controls and global mode/link/lookahead.
- Add offline tests: band GR correctness across modes; lookahead latency verification; null/parity checks.

---

## DynEQ — Hold/Program-Dependent Release + UI bindings

Date: 2025-10-07

Summary:
- Implemented per-band envelope with Hold and program-dependent Release in `Node_DynEq`.
- Added sidechain bandpass per-band, tracked envelope with attack/release one-pole, and smoothed GR with hold latch.
- Program-dependent release scales release time with current reduction fraction to avoid pumping and improve musical decay.
- UI overlay gained Attack/Release/Hold sliders; wired to per-band IDs (`b_dynAtkMs`, `b_dynRelMs`, `b_dynHoldMs`).

Code:
- `Source/modules/FieldNodes/Node_DynEq.h`:
  - New state: `env_[24]`, `grDbZ_[24]`, `holdCount_[24]`.
  - Hold logic: sample counter from `holdSec * sr`; inhibits release until exhausted.
  - Program-dependent release: `relEff = rel * (0.5 + 1.5 * |GR|/|range|)`.
  - Per-band sidechain BPF follows band center/Q.
- `Source/features/dynEq/DynEqTab.h`:
  - Overlay sliders for Attack/Release/Hold with on-change hooks binding to `dynEq::Band::dynAtkMs`, `dynRelMs`, `dynHoldMs`.

Verification:
- Build OK across Standalone/AU/VST3. No change to reported latency (hold/rel are dynamic only).
- Visual: existing dyn handle shows range; follow-up will add explicit GR meter/overlay.

Follow-ups (tracked in `docs/FIELD_CURRENT_TODO.md`):
- Upgrade DynEQ UI to expose Attack/Release/Hold comprehensively (labels, ranges, presets).
- Audit full DynEQ UI coverage vs backend (bands, modes, sidechain, lookahead).
- Add GR visualization (per-band attenuation path/handle or mini meters).


**End of Master Doc**
(Keep this file in-repo, update alongside code. All paths match your current tree.)
