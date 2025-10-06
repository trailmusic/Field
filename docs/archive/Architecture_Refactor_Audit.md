# Field Architecture Refactor — **Phase 1 Audit**

> **Last updated:** 2025-10-04
> **Branch:** `feature`
> **Scope:** Source tree separation (UI ↔ Processor ↔ Engines ↔ Core), DSP single-source-of-truth, host-safe lifecycle, CI tripwires.

---

## Contents

* [Purpose](#purpose)
* [Executive Summary](#executive-summary)
* [Target Topology](#target-topology)
* [Guardrails & Principles](#guardrails--principles)
* [Current Status (Phase 1)](#current-status-phase1)
* [Milestones & DoD](#milestones--definition-of-done)
* [Migration Checklist](#migration-checklist)
* [Legacy → New Mapping](#legacy--new-mapping)
* [Authoritative `Source/` Layout](#authoritative-source-layout)
* [One-Shot Move Script (previewable)](#one-shot-move-script-previewable)
* [Build System Scaffolding](#build-system-scaffolding)
* [Verification Matrix](#verification-matrix)
* [Risk Register](#risk-register)
* [Rollback Plan](#rollback-plan)
* [Ownership](#ownership)
* [Work Orders (WO) Ledger](#work-orders-wo-ledger)

---

## Purpose

Create a **clean separation of concerns** across the codebase:

* UI & feature panels remain **visual only**.
* All DSP lives in **engines/**, wrapped by **modules/** for graph wiring.
* **processor/** contains JUCE `AudioProcessor` glue only.
* **core/** centralizes params, runtime, signal utilities, and telemetry.

This establishes a **stable lane** for latency, warm-swap, and future signal work while preventing ODR drift and header duplication.

---

## Executive Summary

* **Reverb DSP** is now a **single source of truth** under `engines/reverb/**`. Legacy headers in `features/reverb/DSP` are **poisoned** to prevent regressions.
* **`shared/dsp`** has been **retired/moved**; CI **tripwires** block legacy includes.
* **Processor lifecycle** hardened: **zero-buffer/zero-channel** safe, **insert-while-playing** safe, and **no APVTS reads** on the audio thread (policy + tripwires).
* **FDN** wrapped with **debug-only invariants**, canonical wrap, SIMD-safe pads, DC guards, and first-bad-sample scaffolding (all **zero-cost in Release**).
* **Include scope** fenced: engines cannot include `features/` or `shared/`; UI cannot sneak engine scope.
* Full builds (Standalone/AU/VST3) are **green** with the new structure.

---

## Target Topology

**High-level layering**

```
app/              # entry/host notes only
core/             # params, runtime gates, signal utils, telemetry
engines/          # pure DSP packages (no UI)
modules/          # node wrappers, graph wiring, zero-alloc stages
processor/        # JUCE AudioProcessor + editor shell (glue only)
ui/ + features/   # visual-only, no audio buffer access
presets/          # data + loaders
tests/            # offline & perf
```

---

## Guardrails & Principles

* **One DSP home per engine** (e.g., Reverb FDN under `engines/reverb/DSP`).
* **No DSP in `processor/`** beyond orchestration and latency/tail reporting.
* **No APVTS reads** on the audio thread; **prepare/message-thread** only.
* **Latency single source:** `core/runtime/LatencyManager` applied from `processor/` **on the message thread**.
* **Rebuilds are gated**; topology changes occur at `prepareToPlay()` or via crossfades when latency-identical.
* **Tripwires** in CI block: `features/.../DSP` includes, any `shared/dsp`, and `DspRuntimeConfig`.

---

## Current Status (Phase 1)

**Shipped & green**

* Engines rehome for Reverb (FDN/EQ/params) and Phase/Delay artifacts.
* Processor lifecycle guards (prepare flag, zero-buffer safe).
* FieldChain prepared/active stage list; Node_Meter hardened.
* Include-scope hygiene for engines; UI fence for engine scope.
* Legacy reverb headers poisoned; `DspRuntimeConfig` removed + poisoned.
* CI tripwires active for legacy include patterns.

**Open follow-ons (tracked below in WOs)**

* Final sweep to **delete** legacy directories after poison period (retained short-term to unblock teammates).
* CI pattern expansion to **scan content** (ensure poison headers not silently replaced).

---

## Milestones & Definition of Done

* **A. Processor migration complete**
  *DoD:* No `.cpp` bridges from `shared/Core`; all lives in `processor/`.
* **B. Engines/modules split complete**
  *DoD:* No DSP in `features/`; modules wrap engines; UI compiles without engine includes.
* **C. Params/presets relocation**
  *DoD:* Param IDs/layout in `core/params/`; presets in `presets/`.
* **D. Tests online**
  *DoD:* Null-unity + latency smoke + golden swap tests pass; CI green.
* **E. CMake hardened**
  *DoD:* Explicit include scopes; forwarders removed; warnings triaged.

---

## Migration Checklist

* [ ] **Processor**: move remaining bodies to `processor/` (no bridges).
* [ ] **Param IDs/Layout**: ensure all UIs use `core/params/ParamIDs.h`.
* [ ] **Engines only**: verify no `features/**` includes inside `engines/**`.
* [ ] **Tripwires**: keep `ci_tripwire_legacy_includes` in default build.
* [ ] **APVTS discipline**: final sweep & CI checks for audio-thread reads.
* [ ] **Docs**: update dev notes for Ableton insert/PDC behavior.

---

## Legacy → New Mapping

* `shared/Core/SignalGraph.*` → `core/signal/SignalGraph.*` (forwarders removed)
* `shared/Core/{LatencyManager,DebugTelemetry,GlitchHunt,LatencyProbe}.h` → `core/{runtime,telemetry}/*`
* `shared/Core/Plugin{Processor,Editor}.*` → `processor/*`
* `shared/dsp/*` → `engines/{delay,phase,dynamics}/*` (moved; tripwire blocks legacy)
* `features/reverb/{Core,DSP,Presets}` (DSP parts) → `engines/reverb/{Core,DSP,Presets}`
* **All** Reverb Param IDs → `core/params/ParamIDs.h`

---

## Authoritative `Source/` Layout

```text
Source/
├─ app/
├─ core/
│  ├─ params/        (ParamIDs.h, ParamLayout.cpp, Snapshot.h)
│  ├─ runtime/       (LatencyManager.h, RebuildGate.h, LiveSwapPlanner.h, ... )
│  ├─ signal/        (SignalGraph.*, CrossfadeRamp.h, OversamplingStage.h, ...)
│  ├─ telemetry/     (DebugTelemetry.h, GlitchHunt.h, StateSanity.h, ...)
│  └─ util/          (FloatShim.h, DenormGuard.h)
├─ engines/
│  ├─ delay/
│  ├─ dynamics/
│  ├─ phase/
│  └─ reverb/
│     ├─ Core/       (ReverbEngine.* etc.)
│     ├─ DSP/        (ReverbFDN.h, ReverbEQ.*, DecayRateEQ.*, SimdBiquad.h)
│     └─ Presets/    (ReverbParameters.*, ReverbParamMap.cpp)
├─ modules/          (FieldChain.*, FieldDualChain.h, FieldNodes/*, Mixing/*)
├─ processor/        (PluginProcessor.*, PluginEditor.*, BusesLayouts.h)
├─ ui/               (visual only)
├─ features/         (visual tabs only; no DSP)
├─ presets/
└─ tests/            (offline & perf)
```

---

## One-Shot Move Script (previewable)

> Run with `-n` for dry run, then remove `-n` to preserve history via `git mv`.

```bash
set -e
# Core
mkdir -p Source/core/{params,runtime,signal,telemetry,util}
git mv -n Source/shared/Core/SignalGraph.*              Source/core/signal/ || true
git mv -n Source/shared/Core/OversamplingStage.h        Source/core/signal/ || true
git mv -n Source/shared/Core/FrameAccumulator.h         Source/core/signal/ || true
git mv -n Source/shared/Core/LatencyManager.h           Source/core/runtime/ || true
git mv -n Source/shared/Core/{DebugTelemetry,GlitchHunt,LatencyProbe}.h Source/core/telemetry/ || true
git mv -n Source/shared/Core/FloatShim.h                Source/core/util/ || true

# Processor
mkdir -p Source/processor
git mv -n Source/shared/Core/PluginProcessor.*          Source/processor/ || true
git mv -n Source/shared/Core/PluginEditor.*             Source/processor/ || true

# Engines
mkdir -p Source/engines/{delay,dynamics,phase,reverb/{Core,DSP,Presets}}
# (add per-file moves as needed; see ledger below)
```

---

## Build System Scaffolding

**Root** `Source/CMakeLists.txt`

```cmake
add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(engines)
add_subdirectory(modules)
add_subdirectory(processor)
add_subdirectory(ui)
add_subdirectory(presets)
add_subdirectory(tests)
```

**CI Tripwires (example)**

```cmake
add_custom_target(ci_tripwire_legacy_includes ALL
  COMMAND ${CMAKE_COMMAND} -E echo "Scanning legacy includes..."
  COMMAND ${CMAKE_COMMAND} -E env LC_ALL=C
    grep -RInE "\\bDspRuntimeConfig\\b|#include\\s*\"shared/dsp/|#include\\s*\"features/.*/DSP/"
      ${CMAKE_SOURCE_DIR}/Source && exit 1 || exit 0
)
```

---

## Verification Matrix

| Area             | Check                                                      | Status                      |
| ---------------- | ---------------------------------------------------------- | --------------------------- |
| Build parity     | Standalone/AU/VST3 green                                   | ✅                           |
| Insert safety    | Ableton insert while playing (no pop/crash)                | ✅                           |
| Lifecycle        | Zero-buffer / zero-channel safe                            | ✅                           |
| Engine fencing   | `engines/**` never include `features/` or `shared/`        | ✅ (tripwire)                |
| Param discipline | No APVTS reads on audio thread                             | ✅ policy; sweep in progress |
| Reverb DSP SoT   | Only `engines/reverb/**` compiled; legacy headers poisoned | ✅                           |
| Tests            | Null-unity, latency smoke, golden swaps compile & pass     | ✅ (where enabled)           |

---

## Risk Register

* **Hidden UI↔DSP coupling** resurfaces via includes → *Mitigation:* CI tripwires, include-scope fencing.
* **Latency drift** across live swaps → *Mitigation:* `LatencyManager`, `LiveSwapPlanner`, offline probe tests.
* **Double/float parity** discrepancies → *Mitigation:* unified prepare, canonical wrap, debug invariants, DC guards.
* **Team local branches** re-introduce legacy includes → *Mitigation:* poison headers + grep in default build.

---

## Rollback Plan

* All work rides behind the `feature` branch.
* Emergency rollback: point CMake back to legacy paths and disable tripwire target (short-term only).
* Poison headers make misuse loud; removing poison restores the previous state if absolutely necessary.

---

## Ownership

* **Architecture & Processor:** @trail, @grant
* **DSP & Core:** @trail
* **UI & Features:** @grant

---

## Work Orders (WO) Ledger

> **Legend:** ✅ shipped • 🧪 tested • ☣️ poisoned (legacy) • 🔒 CI tripwire

### Foundation / Structure

* **WO-21/24/32/33** — Retire `shared/dsp`: move phase/delay artifacts into `engines/**`; remove legacy includes. ✅🔒
* **WO-43/57** — Include-scope hygiene & UI fence for engine scope. ✅🔒
* **WO-55** — Motion/Machine engine split; `MotionController` visual wrapper; engine code under `engines/{motion,machine}`. ✅

### Reverb Single Source of Truth

* **WO-22/34/40/41/46/56** — Consolidate Reverb under `engines/reverb/**`; **poison** legacy headers:
  `ReverbFDN.h`, `ReverbParamIDs.h`, `ReverbEQParamIDs.h`, `ReverbProcessorGlue.h`. ✅☣️🔒

### DSP Hygiene & Stability (FDN)

* **WO-23/25/26/27/28/29/30/31/44** — Canonical wrap, SIMD-safe pads, DC guards, feedback glide, stability pins, debug invariants, first-bad-sample capture (dev-only). ✅🧪

### Processor & Chain Safety

* **WO-49/50** — Ableton insert-safe; lifecycle & zero-buffer guards. ✅
* **WO-51/52/54** — FieldChain prepared/active stage list; Node_Meter hardened; unity-with-disabled-stages test. ✅🧪
* **WO-7/8/14/15/16/17/18/19/20** — Live-swap infra (same-latency), HUD (dev-only), offline goldens, latency smoke, processor glue tests. ✅🧪

### Runtime & Params

* **WO-11/12/13** — Param IDs & layout in `core/params/`, `ParamChangeBus`, prepare-time rebuild & latency/tail apply. ✅
* **WO-37/38/42** — **DspRuntimeConfig** soft → hard removal; header **poisoned**; `OSPhaseResolver` introduced; CI tripwires. ✅☣️🔒

---

### Appendix A — Minimal CMake Modules (example)

`core/CMakeLists.txt`

```cmake
add_library(field_core
  runtime/{LatencyManager.h,RebuildGate.h,OSPhaseResolver.h,HostPDCGuard.h,TailGuard.h,LiveSwapPlanner.h,DevHudFlag.h}
  signal/SignalGraph.cpp signal/{SignalGraph.h,CrossfadeRamp.h,OversamplingStage.h,FrameAccumulator.h,Sanitize.h,NullNode.h,Warmup.h}
  telemetry/{DebugTelemetry.h,GlitchHunt.h,StateSanity.h,LiveSwapHUD.h}
  params/{ParamIDs.h,ParamLayout.cpp,Snapshot.h}
  util/{FloatShim.h,DenormGuard.h}
)
target_include_directories(field_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(field_core PUBLIC juce_dsp)
```

`engines/CMakeLists.txt` (excerpt)

```cmake
add_library(field_engines
  reverb/Core/{ReverbEngine.cpp,ReverbEngine.h,FieldReverbConfig.h,ReverbTypes.h}
  reverb/DSP/{ReverbFDN.h,ReverbEQ.cpp,ReverbEQ.h,DecayRateEQ.cpp,DecayRateEQ.h,SimdBiquad.h}
  reverb/Presets/{ReverbParameters.cpp,ReverbParameters.h,ReverbParamMap.cpp}
  delay/{DelayEngine.h,DelayPresetLibrary.cpp,DelayPresetLibrary.h}
  phase/{PhaseAlignmentEngine.cpp,PhaseAlignmentEngine.h,PhaseModes.h,MinPhaseBankIntegration.{h,cpp}}
  dynamics/{DynamicEqState.{h,cpp},FilterFactory.h,Ducker.h}
)
target_include_directories(field_engines PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(field_engines PUBLIC field_core juce_dsp)
```

`modules/CMakeLists.txt` (excerpt)

```cmake
add_library(field_modules
  FieldChain.cpp FieldChain.h FieldDualChain.h FieldParamHooks.h
  FieldNodes/Node_{Reverb,Delay,DynEq,Phase,Imager}.h
  Mixing/Node_{Gain,MSMatrix,Meter}.h
  FieldNodes/NodeLatency.h
)
target_include_directories(field_modules PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(field_modules PUBLIC field_engines field_core)
```

`processor/CMakeLists.txt`

```cmake
add_library(field_processor
  PluginProcessor.{h,cpp}
  PluginEditor.{h,cpp}
  BusesLayouts.h
  LatencyTailCompute.h
)
target_include_directories(field_processor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(field_processor PUBLIC field_core field_modules juce_audio_processors)
```

---

### Appendix B — Selected Tripwire Patterns

* `\bDspRuntimeConfig\b`
* `#include\s*"shared/dsp/`
* `#include\s*"features/.*/DSP/`
* `getRawParameterValue\(.*\).*processBlock` *(blocks APVTS reads on audio thread)*

---

**End of Phase 1 Audit**
*This document is the authoritative reference for structure, guardrails, and the WO ledger for the architecture refactor. Keep it in-repo and update alongside the code.*
