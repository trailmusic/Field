Absolutely. Here’s a single, consolidated “master doc” that merges the two audits, keeps your technical depth, adds clear dividers, a compact index, and a dev note up top. I avoided aggressive shortening and aligned paths to your **current tree**.

---

# FIELD — Master Architecture & Triage (Phase 1→2)

> **Dev Note (read first):**
> This doc is the **authoritative** reference for Field’s code structure, DSP routing, dev toggles, and the Live glitch triage. Legacy include paths still on disk are **poisoned** and must not be used. Audio thread reads from APVTS are **forbidden**; use `HostParams` snapshots only. The clean signal path is **locked in** (non-aliased DRY/WET + ordered blend). All dev-only scaffolding compiles out in Release.

**Last updated:** 2025-10-06 • **Branch:** `feature` • **Maintainers:** @trail @grant

---

## Index

1. [Executive Summary](#executive-summary)
2. [Target Topology & Source Layout](#target-topology--source-layout)
3. [Guardrails & Principles](#guardrails--principles)
4. [Routing Lock-In (Glitch Fix)](#routing-lockin-glitch-fix)
5. [Phase 1 Results](#phase-1-results)
6. [Phase 2 Scope & Results](#phase-2-scope--results)
7. [Work Orders (WO) Ledger](#work-orders-wo-ledger)
8. [Parameter Integration Plan — Full Implementation (Hardened)](#parameter-integration-plan--full-implementation-hardened)
9. [CI Tripwires & Build Fences](#ci-tripwires--build-fences)
10. [Verification Matrix](#verification-matrix)
11. [Risk & Mitigation](#risk--mitigation)
12. [Rollback Plan](#rollback-plan)
13. [Cleanup Map (dev artifacts)](#cleanup-map-dev-artifacts)
14. [Live Glitch Triage Log](#live-glitch-triage-log)
15. [Ownership & Cadence](#ownership--cadence)
16. [Appendix A — Code Sketches](#appendix-a--code-sketches)
17. [Appendix B — Tripwire Patterns](#appendix-b--tripwire-patterns)

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

**End of Master Doc**
(Keep this file in-repo, update alongside code. All paths match your current tree.)
