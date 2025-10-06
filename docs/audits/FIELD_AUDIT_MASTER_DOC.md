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
8. [Param Integration Plan (Proper tie-in)](#param-integration-plan-proper-tiein)
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

## Param Integration Plan (Proper tie-in)

**Goals:** Single source of parameter truth, no audio-thread APVTS reads, consistent mapping from UI → `HostParams` → nodes/engines.

1. **Snapshot completeness**

   * Ensure `core/params/Snapshot.h` captures **all** right-side sliders (e.g., Tilt/Bass/etc.) and mix gains, plus dev flags behind `#if JUCE_DEBUG`.

2. **Processor wiring**

   * Rebuild snapshot in `parameterChanged` and targeted listeners (mix/input/output & any critical dev flags).
   * **Never** touch APVTS in `processBlock`.

3. **FieldChain fan-out**

   * Add `setParameters(const HostParams&)` on `FieldChain` that forwards **only POD** structs to nodes.
   * Each node owns a tiny runtime struct; engines receive plain scalars/vectors (no locks/trees).

4. **Sanity tests**

   * **Nulls:** mix=0 → DRY; mix=1 → WET; both null at < −140 dBFS.
   * **Param smoke:** toggling each mapped ID changes output hash in an offline render.
   * **Parity:** float vs double null (< −140 dBFS) at OS=1x.

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
