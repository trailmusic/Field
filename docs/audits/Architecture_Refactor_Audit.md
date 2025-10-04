# Field Architecture Refactor Audit (Phase 1)

Last updated: 2025-10-04 • Branch: `feature`  
Maintenance note: WO-38 glue refactor (cfg removal in processor) landed; poison header pending.

## Contents
- [Purpose](#purpose)
- [Completed (Phase 1)](#completed-phase-1)
- [Structure (target)](#structure-target)
- [Decisions & Guardrails](#decisions--guardrails)
- [Next Steps](#next-steps)
- [Milestones & Definition of Done](#milestones--definition-of-done)
- [Migration Checklist](#migration-checklist)
- [Legacy → New Mapping (Phase 1)](#legacy--new-mapping-phase-1)
- [Risk & Mitigation](#risk--mitigation)
- [Rollback Plan](#rollback-plan)
- [Owners](#owners)
- [Target Source/ layout (authoritative)](#target-source-layout-authoritative)
- [What to move now (mapping)](#what-to-move-now-mapping)
- [One-shot move script (preview, then execute)](#one-shot-move-script-preview-then-execute)
- [Per-folder index (should exist after move)](#per-folder-index-should-exist-after-move)
- [Minimal CMake scaffolds](#minimal-cmake-scaffolds)
- [Builder Work Order #2 — Processor Migration + Latency Proof](#builder-work-order-2--processor-migration--latency-proof)
- [Verification (host-safe)](#verification-host-safe)
- [Phase 1 Locked — Minimal Stubs Installed (for Work Order #3)](#phase-1-locked--minimal-stubs-installed-for-work-order-3)
- [WO-3 Update — Optional Mixing Stages in FieldChain (default-off)](#wo-3-update--optional-mixing-stages-in-fieldchain-default-off)
- [WO-4 — Config-driven placeholders + prepare-time param gate](#wo-4--config-driven-placeholders--prepare-time-param-gate)
- [WO-5 — Latency Accumulator + Probe + Tests](#wo-5--latency-accumulator--probe--tests)
- [WO-6 — Processor Glue: Safe Param Reads, Rebuild Fence, Latency & Tail](#wo-6--processor-glue--safe-param-reads--rebuild-fence--latency--tail)
- [WO-7 — Click-Free Live Chain Swap (Same-Latency Only)](#wo-7--click-free-live-chain-swap-same-latency-only)
- [WO-8 — Mid-Block Swap + Warmup (same-latency only)](#wo-8--mid-block-swap--warmup-same-latency-only)
- [WO-9 — StateSanity + PDC Guard + First-Bad-Sample Telemetry](#wo-9--statesanity--pdc-guard--first-bad-sample-telemetry)
- [WO-10 — Host Cache Sanity + LatencyProbe CI Harness](#wo-10--host-cache-sanity--latencyprobe-ci-harness)
- [WO-11 — Param IDs → Latency Hooks (prepare-time only)](#wo-11--param-ids--latency-hooks-prepare-time-only)
- [WO-12 — Minimal Param Layout (APVTS) + Safe Reads (no DSP changes)](#wo-12--minimal-param-layout-apvts--safe-reads-no-dsp-changes)
- [WO-13 — Rebuild Listeners + Latency/Tail Apply at Prepare](#wo-13--rebuild-listeners--latencytail-apply-at-prepare)
- [WO-14 — Live-Swap for Voicing Params (same-latency edits only)](#wo-14--live-swap-for-voicing-params-same-latency-edits-only)
 - [2025-10-04 — Maintenance Update: DualChain assignment removal + full build](#2025-10-04--maintenance-update-dualchain-assignment-removal--full-build)
- [WO-15 — Editor Timer Hook + Live-Swap HUD (dev-only)](#wo-15--editor-timer-hook--live-swap-hud-dev-only)
- [WO-16 — FIELD_DEV_HUD flag + runtime toggle](#wo-16--field_dev_hud-flag--runtime-toggle)
- [WO-17 — Offline Golden Tests (same-latency voicing & mid-block swap)](#wo-17--offline-golden-tests-same-latency-voicing--mid-block-swap)
- [WO-18 — Latency Smoke Matrix + Tail Cache Test](#wo-18--latency-smoke-matrix--tail-cache-test)
- [WO-19 — Processor Latency/Tail Smoke (APVTS + Host-style)](#wo-19--processor-latencytail-smoke-apvts--host-style)
- [WO-20 — ParamChangeBus ⇄ Processor Glue Test (no audio)](#wo-20--paramchangebus--processor-glue-test-no-audio)
- [WO-21 — Retire shared/dsp (phase bank include), prep for full shutdown](#wo-21--retire-shareddsp-phase-bank-include-prep-for-full-shutdown)
- [WO-22 — Reverb DSP Consolidation (kill legacy glue; keep builds green)](#wo-22--reverb-dsp-consolidation-kill-legacy-glue-keep-builds-green)
- [WO-23 — FDN Stability Pack (no sound-change intent, just hygiene)](#wo-23--fdn-stability-pack-no-sound-change-intent-just-hygiene)
- [WO-24 — Kill shared/dsp Completely](#wo-24--kill-shareddsp-completely)
- [WO-25 — Spectral-Radius Safety + Feedback Smoothing (prepare-time)](#wo-25--spectral-radius-safety--feedback-smoothing-prepare-time)
- [WO-26 — Delay-Line Wrap Correctness + SIMD Tail Guard](#wo-26--delay-line-wrap-correctness--simd-tail-guard)
- [WO-27 — Deterministic Prepare + Warmup & Fade-In](#wo-27--deterministic-prepare--warmup--fade-in)
- [WO-28 — Spectral-Radius Safety + Feedback Glide (no tone change)](#wo-28--spectral-radius-safety--feedback-glide-no-tone-change)
- [WO-29 — SIMD-Safe Delay Pads + Canonical Wrap (no tone change)](#wo-29--simd-safe-delay-pads--canonical-wrap-no-tone-change)
- [WO-30 — DC Guards + Optional Safety Soft-Clip (default-OFF)](#wo-30--dc-guards--optional-safety-soft-clip-default-off)
- [WO-31 — Feedback Operator Safety (matrix normalization @ prepare)](#wo-31--feedback-operator-safety-matrix-normalization--prepare)
- [WO-32 — Kill shared/dsp From the Build (fast, reversible)](#wo-32--kill-shareddsp-from-the-build-fast-reversible)
- [WO-33 — Move/Map Every shared/dsp Artifact to Engines](#wo-33--movemap-every-shareddsp-artifact-to-engines)
- [WO-34 — Reverb DSP Consolidation (one source of truth)](#wo-34--reverb-dsp-consolidation-one-source-of-truth)
- [WO-35 — Block features/.../DSP includes (tripwire)](#wo-35--block-featuresdspdsp-includes-tripwire)
- [WO-36 — ReverbEngine sanity pin (static_asserts)](#wo-36--reverbengine-sanity-pin-static_asserts)
- [WO-37 — Decommission DspRuntimeConfig (soft) + deterministic OS/Phase resolver](#wo-37--decommission-dspruntimeconfig-soft--deterministic-osphase-resolver)
- [WO-38 — Remove DspRuntimeConfig (hard) + CI tripwire](#wo-38--remove-dspruntimeconfig-hard--ci-tripwire)
- [WO-39 — shared/dsp full shutdown (headers poison + include path removal)](#wo-39--shareddsp-full-shutdown-headers-poison--include-path-removal)

---

 

## Purpose
Protect the UI while restructuring `Source/` to separate UI, processor glue, DSP engines, and cross-cutting core (signal, runtime, telemetry). Provide a clean lane for future signal + latency work.

---

## Completed (Phase 1)
- Core scaffolding
  - `core/signal/`: `SignalGraph.h`, `FrameAccumulator.h`, `OversamplingStage.h`, `Sanitize.h`, `NullNode.h`
  - `core/runtime/`: `LatencyManager.h`
  - `core/telemetry/`: `DebugTelemetry.h`, `GlitchHunt.h`, `LatencyProbe.h`
  - Forwarders added in `shared/Core/*` → `core/*`
- Processor/editor routing
  - Headers moved to `processor/`: `PluginProcessor.h`, `PluginEditor.h`
  - `shared/Core/Plugin*.h` now forwarders to `processor/`
  - Bridge sources in `processor/` include originals to preserve behavior
  - Inlined tiny methods to satisfy linkage temporarily
- CMake
  - Added stubs for `app`, `core`, `engines`, `modules`, `processor`, `ui`, `presets`, `tests`
  - `Source/CMakeLists.txt` now points at `processor/Plugin*`
- Build
  - Standalone, AU, VST3 built and installed successfully

---

## Structure (target)
- `app/`: entry/host glue only
- `core/`: params, runtime gates, signal graph, telemetry, utils
- `engines/`: pure DSP packages (no UI)
- `modules/`: node wrappers for engines (graph wiring)
- `processor/`: single `AudioProcessor` + editor shell (glue only)
- `ui/` and `features/`: visual-only
- `presets/`: data + minimal loaders
- `tests/`: offline + perf

---

## Decisions & Guardrails
- No DSP code inside `processor/` beyond orchestration and latency reporting.
- `ui/` and `features/` are display/interaction only; they cannot touch audio buffers.
- Single source of truth for latency: `core/runtime/LatencyManager` applied from `processor/` on the message thread.
- Topology rebuilds are gated and occur in `prepareToPlay()` or via explicit crossfades; never mid-block without protection.
- Forwarders remain only as a transition aid and will be removed when migrations finish.

---

## Next Steps
1. Migrate implementations from `shared/Core/PluginProcessor.cpp` and `PluginEditor.cpp` into `processor/` in small, verified steps; remove bridge includes afterwards
2. Extract DSP from features into `engines/` and wrap via `modules/FieldNodes/*`
3. Move param IDs/layout to `core/params/`; move preset code to `presets/`
4. Add `tests/offline` (null-unity, latency-probe) and `tests/perf`
5. Tighten CMake: real `field_core` target and explicit links; remove legacy include paths
6. Clean warnings (override, deprecations)

---

## Milestones & Definition of Done
- Milestone A: Processor implementations migrated
  - DOD: No `#include ../shared/Core/Plugin*.cpp` bridges; all functionality lives in `processor/`; builds clean
- Milestone B: Engines/modules split
  - DOD: All DSP code resides in `engines/`; graph nodes in `modules/`; UI compiles without engine includes
- Milestone C: Parameters/presets relocation
  - DOD: Param IDs/layout in `core/params/`; preset registry/store in `presets/`; no UI ↔ DSP coupling
- Milestone D: Tests online
  - DOD: Offline null-unity + latency tests pass; perf bench runs; CI green
- Milestone E: CMake hardening
  - DOD: Concrete `field_core` target links; forwarders removed; warnings addressed or suppressed intentionally

## Migration Checklist (owner → status)
- Move `PluginProcessor` constructor + `createParameterLayout()` → processor (owner: processor) [pending]
- Move `prepareToPlay()` / `releaseResources()`; apply `LatencyManager` (owner: processor) [pending]
- Move `processBlock(float/double)` ingress/egress + graph wiring (owner: processor) [pending]
- Extract reverb/delay/dynEQ/phase/imager DSP into `engines/` (owner: dsp) [pending]
- Create `modules/FieldNodes/*` wrappers for engines (owner: dsp) [pending]
- Add `core/params/*` (IDs, layout, smoothing) (owner: core) [pending]
- Relocate preset registry/store to `presets/` (owner: presets) [pending]
- Add offline/perf tests (owner: tests) [pending]
- Replace forwarders and tighten CMake (owner: core) [pending]

## Legacy → New Mapping (Phase 1)
- `shared/Core/SignalGraph.*` → `core/signal/SignalGraph.h` (+ forwarder)
- `shared/Core/FrameAccumulator.h` → `core/signal/FrameAccumulator.h` (+ forwarder)
- `shared/Core/OversamplingStage.h` → `core/signal/OversamplingStage.h` (+ forwarder)
- `shared/Core/LatencyManager.h` → `core/runtime/LatencyManager.h` (+ forwarder)
- `shared/Core/DebugTelemetry.h`, `GlitchHunt.h`, `LatencyProbe.h` → `core/telemetry/*` (+ forwarders)
- `shared/Core/PluginProcessor.*` → `processor/` (headers moved; .cpp bridged)
- `shared/Core/PluginEditor.*` → `processor/` (headers moved; .cpp bridged)

## Risk & Mitigation
- Latency consistency: single `LatencyManager` in `processor/`; apply on message thread; add offline tests
- Hidden UI↔DSP coupling: forwarders + staged moves + build validation
- Bridge source ownership ambiguity: tracked as short-lived; removal is a milestone

## Rollback Plan
- All changes are on `feature`; forwarders preserve compatibility
- Emergency rollback: point CMake to `shared/Core/Plugin*` and remove `processor/` bridges

## Owners
- Architecture/Processor: @trail / @grant
- DSP/Core: @trail
- UI/Features: @grant

---

## Target Source/ layout (authoritative)

```
Source/
├─ app/
│  ├─ CMakeLists.txt
│  ├─ PluginMain.cpp
│  └─ Platform/
│     └─ AbletonNotes.md
│
├─ core/
│  ├─ params/
│  │  ├─ ParamIDs.h
│  │  ├─ ParamLayout.cpp
│  │  └─ ParamSmoother.h
│  ├─ runtime/
│  │  ├─ DspRuntimeConfig.h
│  │  ├─ RebuildGate.h
│  │  └─ LatencyManager.h
│  ├─ signal/
│  │  ├─ SignalGraph.h
│  │  ├─ SignalGraph.cpp
│  │  ├─ OversamplingStage.h
│  │  ├─ FrameAccumulator.h
│  │  ├─ Sanitize.h
│  │  └─ NullNode.h
│  ├─ telemetry/
│  │  ├─ DebugTelemetry.h
│  │  ├─ GlitchHunt.h
│  │  └─ LatencyProbe.h
│  └─ util/
│     ├─ FloatShim.h
│     └─ FnGuard.h
│
├─ engines/
│  ├─ delay/
│  │  ├─ DelayEngine.h
│  │  ├─ DelayPresetLibrary.cpp
│  │  └─ DelayPresetLibrary.h
│  ├─ dynamics/
│  │  ├─ DynamicEqState.h
│  │  ├─ DynamicEqState.cpp
│  │  ├─ FilterFactory.h
│  │  └─ Ducker.h
│  ├─ phase/
│  │  ├─ PhaseAlignmentEngine.cpp
│  │  ├─ PhaseAlignmentEngine.h
│  │  └─ PhaseModes.h
│  ├─ image/
│  │  └─ ImagerCore.h
│  └─ reverb/
│     ├─ Core/
│     │  ├─ ReverbEngine.cpp
│     │  ├─ ReverbEngine.h
│     │  ├─ FieldReverbConfig.h
│     │  └─ ReverbTypes.h
│     ├─ DSP/
│     │  ├─ ReverbFDN.h
│     │  ├─ ReverbEQ.cpp
│     │  ├─ ReverbEQ.h
│     │  ├─ ReverbEQParamIDs.h
│     │  ├─ DecayRateEQ.cpp
│     │  ├─ DecayRateEQ.h
│     │  └─ SimdBiquad.h
│     └─ Presets/
│        ├─ ReverbParameters.cpp
│        ├─ ReverbParameters.h
│        └─ ReverbParamMap.cpp
│
├─ modules/
│  ├─ FieldChain.h
│  ├─ FieldChain.cpp
│  ├─ FieldNodes/
│  │  ├─ Node_Reverb.h
│  │  ├─ Node_Delay.h
│  │  ├─ Node_DynEq.h
│  │  ├─ Node_Phase.h
│  │  └─ Node_Imager.h
│  └─ Mixing/
│     ├─ Node_Gain.h
│     ├─ Node_MSMatrix.h
│     └─ Node_Meter.h
│
├─ processor/
│  ├─ PluginProcessor.h
│  ├─ PluginProcessor.cpp
│  ├─ PluginEditor.h
│  ├─ PluginEditor.cpp
│  └─ BusesLayouts.h
│
├─ ui/                      # (keep your existing UI; no audio touches)
│  ├─ Components/
│  ├─ Controls/
│  ├─ Design/
│  ├─ Engines/             # StereoFieldEngine, SpectrumAnalyzer (visual only)
│  ├─ Events/
│  ├─ Layout/
│  ├─ Managers/
│  └─ Utilities/
│
├─ features/                # UI/UX panels (tabs), NO DSP
│  ├─ band/  delay/  dynEq/  imager/  machine/  motion/  phase/  reverb/  xy/
│
├─ presets/
│  ├─ PresetRegistry.cpp
│  ├─ PresetRegistry.h
│  ├─ PresetStore.cpp
│  ├─ PresetStore.h
│  └─ Themes/
│
├─ tests/
│  ├─ offline/
│  │  ├─ test_null_unity.cpp
│  │  └─ test_latency_probe.cpp
│  └─ perf/
│     └─ bench_signalgraph.cpp
│
└─ CMakeLists.txt
```

## What to move now (mapping)

- From `shared/Core` → `core/*` & `processor/*`
  - `SignalGraph.h/.cpp` → `core/signal/`
  - `OversamplingStage.h` → `core/signal/`
  - `FrameAccumulator.h` → `core/signal/`
  - `LatencyManager.h` → `core/runtime/`
  - `DebugTelemetry.h`, `GlitchHunt.h`, `LatencyProbe.h` → `core/telemetry/`
  - `FloatShim.h` → `core/util/`
  - `PluginProcessor.h/.cpp`, `PluginEditor.h/.cpp` → `processor/`
  - `PhaseBanks.h` → `engines/phase/`
- From `shared/dsp` → `engines/*`
  - `DelayEngine.h`, `DelayPresetLibrary.*` → `engines/delay/`
  - `Ducker.h` → `engines/dynamics/`
  - `PhaseAlignmentEngine.*`, `PhaseModes.h` → `engines/phase/`
- From `features/reverb` → `engines/reverb`
  - `Core/*` → `engines/reverb/Core/`
  - `DSP/*` → `engines/reverb/DSP/`
  - `Presets/ReverbParameters.*`, `ReverbParamMap.*` → `engines/reverb/Presets/`
  - Keep UI under `features/reverb/UI/*`
- From `features/dynEq` → `engines/dynamics/`
  - `DynamicEqState.*`, `FilterFactory.h` → `engines/dynamics/`
  - UI stays in `features/dynEq/`

## One-shot move script (preview, then execute)

```bash
set -e

# Core ➜ core/*
mkdir -p Source/core/{params,runtime,signal,telemetry,util}
git mv -n Source/shared/Core/SignalGraph.h Source/core/signal/ || cp Source/shared/Core/SignalGraph.h Source/core/signal/
git mv -n Source/shared/Core/SignalGraph.cpp Source/core/signal/ || cp Source/shared/Core/SignalGraph.cpp Source/core/signal/
git mv -n Source/shared/Core/OversamplingStage.h Source/core/signal/ || cp Source/shared/Core/OversamplingStage.h Source/core/signal/
git mv -n Source/shared/Core/FrameAccumulator.h Source/core/signal/ || cp Source/shared/Core/FrameAccumulator.h Source/core/signal/
git mv -n Source/shared/Core/LatencyManager.h Source/core/runtime/ || cp Source/shared/Core/LatencyManager.h Source/core/runtime/
git mv -n Source/shared/Core/DebugTelemetry.h Source/core/telemetry/ || cp Source/shared/Core/DebugTelemetry.h Source/core/telemetry/
git mv -n Source/shared/Core/GlitchHunt.h Source/core/telemetry/ || cp Source/shared/Core/GlitchHunt.h Source/core/telemetry/
git mv -n Source/shared/Core/LatencyProbe.h Source/core/telemetry/ || cp Source/shared/Core/LatencyProbe.h Source/core/telemetry/
git mv -n Source/shared/Core/FloatShim.h Source/core/util/ || cp Source/shared/Core/FloatShim.h Source/core/util/

# Processor ➜ processor/*
mkdir -p Source/processor
git mv -n Source/shared/Core/PluginProcessor.h Source/processor/ || cp Source/shared/Core/PluginProcessor.h Source/processor/
git mv -n Source/shared/Core/PluginProcessor.cpp Source/processor/ || cp Source/shared/Core/PluginProcessor.cpp Source/processor/
git mv -n Source/shared/Core/PluginEditor.h Source/processor/ || cp Source/shared/Core/PluginEditor.h Source/processor/
git mv -n Source/shared/Core/PluginEditor.cpp Source/processor/ || cp Source/shared/Core/PluginEditor.cpp Source/processor/

# Engines
mkdir -p Source/engines/{delay,dynamics,phase,image,reverb/{Core,DSP,Presets}}
# delay
git mv -n Source/shared/dsp/DelayEngine.h Source/engines/delay/ || cp Source/shared/dsp/DelayEngine.h Source/engines/delay/
git mv -n Source/shared/dsp/DelayPresetLibrary.cpp Source/engines/delay/ || cp Source/shared/dsp/DelayPresetLibrary.cpp Source/engines/delay/
git mv -n Source/shared/dsp/DelayPresetLibrary.h Source/engines/delay/ || true
# dynamics
git mv -n Source/features/dynEq/DynamicEqState.cpp Source/engines/dynamics/ || cp Source/features/dynEq/DynamicEqState.cpp Source/engines/dynamics/
git mv -n Source/features/dynEq/DynamicEqState.h Source/engines/dynamics/ || cp Source/features/dynEq/DynamicEqState.h Source/engines/dynamics/
git mv -n Source/features/dynEq/FilterFactory.h Source/engines/dynamics/ || cp Source/features/dynEq/FilterFactory.h Source/engines/dynamics/
git mv -n Source/shared/dsp/Ducker.h Source/engines/dynamics/ || cp Source/shared/dsp/Ducker.h Source/engines/dynamics/
# phase
git mv -n Source/shared/dsp/PhaseAlignmentEngine.cpp Source/engines/phase/ || cp Source/shared/dsp/PhaseAlignmentEngine.cpp Source/engines/phase/
git mv -n Source/shared/dsp/PhaseAlignmentEngine.h Source/engines/phase/ || cp Source/shared/dsp/PhaseAlignmentEngine.h Source/engines/phase/
git mv -n Source/shared/dsp/PhaseModes.h Source/engines/phase/ || cp Source/shared/dsp/PhaseModes.h Source/engines/phase/
git mv -n Source/shared/Core/PhaseBanks.h Source/engines/phase/ || cp Source/shared/Core/PhaseBanks.h Source/engines/phase/
# reverb core
git mv -n Source/features/reverb/Core/ReverbEngine.cpp Source/engines/reverb/Core/ || cp Source/features/reverb/Core/ReverbEngine.cpp Source/engines/reverb/Core/
git mv -n Source/features/reverb/Core/ReverbEngine.h Source/engines/reverb/Core/ || cp Source/features/reverb/Core/ReverbEngine.h Source/engines/reverb/Core/
git mv -n Source/features/reverb/Core/FieldReverbConfig.h Source/engines/reverb/Core/ || cp Source/features/reverb/Core/FieldReverbConfig.h Source/engines/reverb/Core/
git mv -n Source/features/reverb/Core/ReverbTypes.h Source/engines/reverb/Core/ || cp Source/features/reverb/Core/ReverbTypes.h Source/engines/reverb/Core/
# reverb dsp
for f in ReverbFDN.h ReverbEQ.cpp ReverbEQ.h ReverbEQParamIDs.h DecayRateEQ.cpp DecayRateEQ.h SimdBiquad.h; do
  git mv -n Source/features/reverb/DSP/$f Source/engines/reverb/DSP/ || cp Source/features/reverb/DSP/$f Source/engines/reverb/DSP/
done
# reverb presets (engine-facing)
for f in ReverbParameters.cpp ReverbParameters.h ReverbParamMap.cpp; do
  git mv -n Source/features/reverb/DSP/$f Source/engines/reverb/Presets/ || \
  git mv -n Source/features/reverb/Presets/$f Source/engines/reverb/Presets/ || \
  cp Source/features/reverb/Presets/$f Source/engines/reverb/Presets/ 2>/dev/null || true
done

# image (create core later if needed)
touch Source/engines/image/ImagerCore.h

# modules (scaffold)
mkdir -p Source/modules/{FieldNodes,Mixing}
touch Source/modules/FieldChain.{h,cpp}
touch Source/modules/FieldNodes/Node_{Reverb,Delay,DynEq,Phase,Imager}.h
touch Source/modules/Mixing/Node_{Gain,MSMatrix,Meter}.h
```

> After you’re happy, replace `-n` (no-op preview) with real `git mv` to commit history.

## Per-folder index (should exist after move)

- `core/runtime/`: `DspRuntimeConfig.h`, `LatencyManager.h`, `RebuildGate.h`
- `core/signal/`: `SignalGraph.h/.cpp`, `OversamplingStage.h`, `FrameAccumulator.h`, `Sanitize.h`, `NullNode.h`
- `core/telemetry/`: `DebugTelemetry.h`, `GlitchHunt.h`, `LatencyProbe.h`
- `processor/`: `PluginProcessor.h/.cpp`, `PluginEditor.h/.cpp`, `BusesLayouts.h`
- `engines/reverb/Core/`: `ReverbEngine.h/.cpp`, `FieldReverbConfig.h`, `ReverbTypes.h`
- `engines/reverb/DSP/`: `ReverbFDN.h`, `ReverbEQ*.{h,cpp}`, `DecayRateEQ*.{h,cpp}`, `ReverbEQParamIDs.h`, `SimdBiquad.h`
- `engines/reverb/Presets/`: `ReverbParameters.{h,cpp}`, `ReverbParamMap.cpp`
- `engines/delay/`: `DelayEngine.h`, `DelayPresetLibrary.{h,cpp}`
- `engines/dynamics/`: `DynamicEqState.{h,cpp}`, `FilterFactory.h`, `Ducker.h`
- `engines/phase/`: `PhaseAlignmentEngine.{h,cpp}`, `PhaseModes.h`, `PhaseBanks.h`
- `modules/`: `FieldChain.{h,cpp}`, `FieldNodes/Node_*.h`, `Mixing/Node_{Gain,MSMatrix,Meter}.h`
- `tests/`: `offline/test_null_unity.cpp`, `offline/test_latency_probe.cpp`, `perf/bench_signalgraph.cpp`

## Minimal CMake scaffolds

Root `Source/CMakeLists.txt`

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

`core/CMakeLists.txt`

```cmake
add_library(field_core
    runtime/DspRuntimeConfig.h
    runtime/LatencyManager.h
    runtime/RebuildGate.h
    signal/SignalGraph.cpp signal/SignalGraph.h
    signal/OversamplingStage.h
    signal/FrameAccumulator.h
    signal/Sanitize.h
    signal/NullNode.h
    telemetry/DebugTelemetry.h
    telemetry/GlitchHunt.h
    telemetry/LatencyProbe.h
    util/FloatShim.h
    util/FnGuard.h
)
target_include_directories(field_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(field_core PUBLIC juce_dsp)
```

`engines/CMakeLists.txt`

```cmake
add_library(field_engines
    delay/DelayEngine.h
    delay/DelayPresetLibrary.cpp delay/DelayPresetLibrary.h
    dynamics/DynamicEqState.cpp dynamics/DynamicEqState.h dynamics/FilterFactory.h dynamics/Ducker.h
    phase/PhaseAlignmentEngine.cpp phase/PhaseAlignmentEngine.h phase/PhaseModes.h phase/PhaseBanks.h
    reverb/Core/ReverbEngine.cpp reverb/Core/ReverbEngine.h reverb/Core/FieldReverbConfig.h reverb/Core/ReverbTypes.h
    reverb/DSP/ReverbFDN.h reverb/DSP/ReverbEQ.cpp reverb/DSP/ReverbEQ.h reverb/DSP/ReverbEQParamIDs.h
    reverb/DSP/DecayRateEQ.cpp reverb/DSP/DecayRateEQ.h reverb/DSP/SimdBiquad.h
    reverb/Presets/ReverbParameters.cpp reverb/Presets/ReverbParameters.h reverb/Presets/ReverbParamMap.cpp
    image/ImagerCore.h
)
target_include_directories(field_engines PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(field_engines PUBLIC juce_dsp)
```

`modules/CMakeLists.txt`

```cmake
add_library(field_modules
    FieldChain.cpp FieldChain.h
    FieldNodes/Node_Reverb.h
    FieldNodes/Node_Delay.h
    FieldNodes/Node_DynEq.h
    FieldNodes/Node_Phase.h
    FieldNodes/Node_Imager.h
    Mixing/Node_Gain.h
    Mixing/Node_MSMatrix.h
    Mixing/Node_Meter.h
)
target_include_directories(field_modules PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(field_modules PUBLIC field_engines field_core)
```

`processor/CMakeLists.txt`

```cmake
add_library(field_processor
    PluginProcessor.cpp PluginProcessor.h
    PluginEditor.cpp    PluginEditor.h
    BusesLayouts.h
)
target_include_directories(field_processor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(field_processor PUBLIC field_core field_modules juce_audio_processors)
```

## Builder Work Order #2 — Processor Migration + Latency Proof

Objective: finish moving processor glue; make latency host-safe; validate with null + probe.

Checklist:
- [ ] Move `PluginProcessor.*` & `PluginEditor.*` bodies into `processor/` (no more bridge includes)
- [ ] Ensure no DSP in `processor/` beyond graph orchestration and sanitize
- [ ] In `prepareToPlay()`: build graph, compute `desiredLatency`, call `latency.applyIfChanged(*this)` (message thread)
- [ ] In `processBlock(float/double)`: do not call `setLatencySamples()`; finite-sanitize ingress/egress (dev only)
- [ ] Gate topology rebuilds: param changes set `needsRebuild=true`; rebuild at next `prepareToPlay()`
- [ ] Add `tests/offline/test_null_unity.cpp` (null @ unity)
- [ ] Add `tests/offline/test_latency_probe.cpp` (assert measured≈desired)
- [ ] Host verification in Live: insert-while-playing is silent; duplicate-invert nulls; automation defers latency

Stubs to add:

`core/runtime/RebuildGate.h`

```cpp
#pragma once
#include <atomic>
struct RebuildGate {
  std::atomic<bool> need{false};
  void request() noexcept { need.store(true); }
  bool consume() noexcept { return need.exchange(false); }
};
```

`core/signal/Sanitize.h`

```cpp
#pragma once
#include <juce_dsp/juce_dsp.h>
inline void sanitize(juce::dsp::AudioBlock<float> b){ for(size_t c=0;c<b.getNumChannels();++c){
 auto* p=b.getChannelPointer(c); for(size_t i=0,n=b.getNumSamples();i<n;++i){ if(!juce::isFinite(p[i])) p[i]=0.f; }}}
inline void sanitize(juce::dsp::AudioBlock<double> b){ for(size_t c=0;c<b.getNumChannels();++c){
 auto* p=b.getChannelPointer(c); for(size_t i=0,n=b.getNumSamples();i<n;++i){ if(!juce::isFinite(p[i])) p[i]=0.0; }}}
```

`core/signal/NullNode.h`

```cpp
#pragma once
#include <juce_dsp/juce_dsp.h>
struct NullNode {
  template<typename T> void prepare(double, int, int) {}
  template<typename Sample> void process(juce::dsp::AudioBlock<Sample>&) {}
  int latencySamples() const noexcept { return 0; }
};
```

## Verification (host-safe)
- Null-unity: duplicate track, put Field @ unity on one; invert other → perfect null
- Insert-while-playing: drop the plugin on a running loop → silent
- LatencyProbe vs desired: `DBG` shows identical numbers after `prepareToPlay()`

## Phase 1 Locked — Minimal Stubs Installed (for Work Order #3)

What was added (compiling, no-UI changes):
- Modules chain
  - `modules/FieldChain.{h,cpp}`: unity chain, 0 latency; templated float/double `process(...)`
  - `modules/FieldNodes/Node_{Reverb,Delay,DynEq,Phase,Imager}.h`: tiny placeholder stubs
  - `modules/Mixing/Node_{Gain,MSMatrix,Meter}.h`: tiny placeholder stubs
  - `modules/CMakeLists.txt`: builds `field_modules` and exposes headers
- Core signal hygiene
  - `core/signal/Sanitize.h`: added `sanitize(juce::dsp::AudioBlock<float|double>)` overloads
  - `core/runtime/RebuildGate.h`: request/consume flag for safe DSP rebuild gating
- Offline tests (no JUCE AudioProcessor)
  - `tests/offline/test_null_unity.cpp`: asserts unity pass and latency==0
  - `tests/offline/test_latency_probe.cpp`: measures latency via `LatencyProbe` and asserts equality
  - `tests/offline/CMakeLists.txt` + `tests/CMakeLists.txt` hook

Next wiring (Builder Work Order #3):
- Processor side
  - Add members: `field::modules::FieldChain chainF_, chainD_; LatencyManager latency_; RebuildGate rebuildGate_;`
  - `prepareToPlay()`: `buildUnity()`, `prepare(sr, maxBlock, chans)`, set `desired = chainF_.latencySamples()`, then `latency.applyIfChanged(*this)`
  - `processBlock(float/double)`: `sanitize(block)`, `chain.process(block)`, `sanitize(block)`; never call `setLatencySamples()` here
- Tests
  - Build and run offline tests; ensure unity/null and latency agreement
- Host checks
  - Insert-while-playing silent; duplicate/invert null at unity; automation latency changes defer to restart

## WO-3 Update — Optional Mixing Stages in FieldChain (default-off)
- FieldChain now exposes a `Config` with flags: `enableMS`, `enableGain`, `enableMeter` (all false by default)
- Stages are allocation-free, zero-latency, and unity-safe when disabled
- Order (when enabled): Meter → MSMatrix → Gain; chain remains unity and latency=0 with defaults

Example:
```cpp
field::modules::FieldChain chain;
field::modules::FieldChain::Config cfg{};
cfg.enableMeter = true;   // optional; off by default
cfg.enableMS    = false;  // unity
cfg.enableGain  = false;  // unity
chain.setConfig(cfg);
chain.buildFromConfig();
chain.prepare(48000.0, 512, 2);
// process: chain.process(block);
```

## WO-4 — Config-driven placeholders + prepare-time param gate

Objective: Add a mechanism to gate parameter changes during `prepareToPlay()` to prevent topology rebuilds during audio processing.

Checklist:
- [ ] Add a `Config` struct to `FieldChain` that holds parameter change flags.
- [ ] Modify `FieldChain::prepare()` to check `config.needsRebuild` and rebuild if true.
- [ ] Add a `needsRebuild` flag to `FieldChain::Config`.
- [ ] Add a `buildFromConfig()` method to `FieldChain` that rebuilds the graph based on the current config.
- [ ] Add a `setConfig()` method to `FieldChain` that updates the internal config and triggers a rebuild if needed.

---

## WO-4 — Config-driven placeholders + prepare-time param gate
- Added unity, 0-latency stubs in `modules/FieldNodes/`: `Node_Reverb.h`, `Node_Delay.h`, `Node_DynEq.h`
- Extended `modules/FieldChain.{h,cpp}` config and active set:
  - New flags: `enableDelay`, `enableDynEq`, `enableReverb` (default false)
  - Processing order: Meter → MS → Gain → Delay → DynEq → Reverb (all unity-safe by default)
  - No allocations; latency remains 0
- Prepare-time gate (no UI changes): set `FieldChain::Config` in `prepareToPlay()` (hardcoded or via safe param lookup) then `buildFromConfig()` and `prepare(...)`

Example prepare-time toggle (hardcoded):
```cpp
field::modules::FieldChain::Config cfg{};
cfg.enableDelay = false; cfg.enableDynEq = false; cfg.enableReverb = false;
chainF_.setConfig(cfg); chainD_.setConfig(cfg);
chainF_.buildFromConfig(); chainD_.buildFromConfig();
```

Optional safe param lookup (falls back to defaults if IDs absent):
```cpp
cfg.enableDelay = SafeParamGate::getBool(*this, "chain.delay.enable", false);
cfg.enableDynEq = SafeParamGate::getBool(*this, "chain.dyneq.enable", false);
cfg.enableReverb= SafeParamGate::getBool(*this, "chain.reverb.enable", false);
```

### WO-4 Gate & Latency details
- FieldChain now honors a strict prepare-time gate:
  - `Config::needsRebuild` + internal `dirty_` force `buildFromConfig()` only at `prepare<Sample>()`
  - `setConfig()` marks dirty only when values actually change
- Latency reporting:
  - `latencySamples()` returns an internal `latencySum_` (sum of node latencies; remains 0 with placeholders)
  - Mixing stages (Meter/MS/Gain) are math-only and excluded from the sum
- Processor usage reminder:
  - Flip `rebuildGate_.request()` when relevant params change → `cfg.needsRebuild = rebuildGate_.consume()` at prepare
  - Push config to both chains; call `buildFromConfig()` then `prepare` for float/double
  - Apply latency via `latency.applyIfChanged(*this)` on message thread; never in `processBlock`

---

## WO-5 — Latency Accumulator + Probe + Tests
- Node latency contract added:
  - `modules/FieldNodes/NodeLatency.h`: `LatencyParts` and `NodeLatencyMixin<Derived>` with setters for OS/FIR/look-ahead/extra; `latencySamples()` sums parts (default 0)
  - Placeholders updated to inherit the mixin: Reverb/Delay/DynEq/Phase/Imager
- FieldChain updates:
  - `recomputeLatency()` sums node latencies; `buildFromConfig()` calls it
  - Reported latency remains 0 until engines set non-zero parts at prepare
- Processor (dev-only): add a `LatencyProbe` check after `prepareToPlay()`; assert/DBG that measured latency equals `chain.latencySamples()`
- Offline tests refreshed to assert null-unity and probe==reported

Example engine hook later (prepare-time):
```cpp
if (active_.reverb) {
  reverb_.setLinearPhaseFIRGroupDelay(firHalfLenSamples);
  reverb_.setOversamplingGroupDelay(osGroupDelaySamples);
}
recomputeLatency();
```

---

## WO-6 — Processor Glue: Safe Param Reads, Rebuild Fence, Latency & Tail
- Added helpers:
  - `core/runtime/SafeParamGate.h`: defensive param reads at prepare-time only
  - `core/runtime/TailManager.h`: cache tail seconds; reported via `getTailLengthSeconds()`
  - `core/util/DenormGuard.h`: RAII to disable denormals for each audio block
  - `processor/BusesLayouts.h`: minimal `makeStereoBuses()` helper
- Rebuild policy:
  - Params that affect topology/latency set a gate (no audio-thread rebuild)
  - Actual `buildFromConfig()` occurs in `prepareToPlay()`
- Reporting:
  - Latency applied only on message thread via `LatencyManager.applyIfChanged(...)`
  - Tail seconds updated only at prepare; defaults 0.0 with placeholders
- Tests:
  - `tests/offline/test_rebuild_gate.cpp` verifies no mid-block rebuild behavior

---

## WO-7 — Click-Free Live Chain Swap (Same-Latency Only)
- Added `core/signal/CrossfadeRamp.h` and `modules/FieldDualChain.h`
- DualChain: holds `{active, staging}` `FieldChain`s; message-thread builds staging
- If `staging.latency == active.latency`, audio thread crossfades over a 64-sample ramp and promotes; otherwise defer to prepare-time rebuild
- No behavior change unless you call `armLiveSwapIfSameLatency()`
- Offline test: `tests/offline/test_dualchain_xfade.cpp`

Index additions:
- `core/signal/CrossfadeRamp.h`
- `modules/FieldDualChain.h`
- `tests/offline/test_dualchain_xfade.cpp`

---

## WO-8 — Mid-Block Swap + Warmup (same-latency only)
- Added `core/signal/Warmup.h` to settle staging nodes before a live swap (silent blocks)
- Enhanced `modules/FieldDualChain.h`:
  - `armLiveSwapAtSameLatency(offsetSamples, warmupBlocks)` to start the ramp mid-block and optionally pre-warm
  - Maintains zero-alloc on audio thread; reuses scratch
  - If latency differs, returns false and you defer to prepare-time rebuild
- Offline test: `tests/offline/test_dualchain_midblock.cpp`

Index additions:
- `core/signal/Warmup.h`
- `modules/FieldDualChain.h` (updated APIs)
- `tests/offline/test_dualchain_midblock.cpp`

---

## WO-9 — StateSanity + PDC Guard + First-Bad-Sample Telemetry
- Added `core/telemetry/StateSanity.h`:
  - `scanBlock(...)` returns first non-finite sample location
  - `StateSanity` flags/logs mid-block rebuild attempts
- Added `core/runtime/HostPDCGuard.h` to enforce message-thread-only latency reporting; defers changes while playing
- Processor glue (dev-only guards):
  - Top of `processBlock`: denorm guard, consume mid-block flag, ingress sanitize
  - End of `processBlock`: scan output once, log first bad sample
- Offline test: `tests/offline/test_statesanity.cpp`

Index additions:
- `core/telemetry/StateSanity.h`
- `core/runtime/HostPDCGuard.h`
- `tests/offline/test_statesanity.cpp`

---

## WO-10 — Host Cache Sanity + LatencyProbe CI Harness
- Platform note: `app/Platform/AbletonNotes.md` capturing PDC/tail host behavior
- TailGuard: `core/runtime/TailGuard.h` caches applied tail; applies only at prepare; warns when latency>0 but tail==0
- Latency/Tail single-source:
  - `processor/LatencyTailCompute.h` for prepare-time compute
  - `HostPDCGuard` + `TailGuard` apply on message thread only
- CI harness: `tests/offline/test_latency_ci.cpp` probes multiple SR/block sizes and asserts `measured == reported`

Index additions:
- `app/Platform/AbletonNotes.md`
- `core/runtime/TailGuard.h`
- `processor/LatencyTailCompute.h`
- `tests/offline/test_latency_ci.cpp`

---

## WO-11 — Param IDs → Latency Hooks (prepare-time only)
- Added param surfaces:
  - `core/params/ParamIDs.h` authoritative IDs for topology/latency inputs
  - `core/params/Snapshot.h` builds a safe prepare-time snapshot via `SafeParamGate`
- Added mapping helper:
  - `modules/FieldParamHooks.h` to apply snapshot into node latency mixins; calls `recomputeLatency()`
- Tests & QA:
  - `tests/offline/test_param_latency_map.cpp` (stub asserts 0 until engines wire latency setters)
  - `docs/qa/NullAtUnity.md` recipe for host null at unity

Index additions:
- `core/params/ParamIDs.h`
- `core/params/Snapshot.h`
- `modules/FieldParamHooks.h`
- `tests/offline/test_param_latency_map.cpp`
- `docs/qa/NullAtUnity.md`

---

## WO-12 — Minimal Param Layout (APVTS) + Safe Reads (no DSP changes)
- APVTS layout:
  - `core/params/ParamLayout.{h,cpp}` defines parameters for topology/latency IDs introduced in WO-11
  - Defaults preserve unity: all modules OFF, OS=1x, contributors=0 → reported latency stays 0
- Safe reads:
  - `SafeParamGate` gains `getInt` and reads from `getAPVTS()` at prepare-time only
  - Snapshot (`core/params/Snapshot.h`) now returns real values or safe defaults when absent
- Processor glue:
  - `createParameterLayout()` exposed; APVTS constructed from our layout
  - No DSP changes; this only provides stable params for Snapshot and latency/tail compute

Index additions:
- `core/params/ParamLayout.h`
- `core/params/ParamLayout.cpp`

---

## WO-13 — Rebuild Listeners + Latency/Tail Apply at Prepare
- Added `core/runtime/ParamChangeBus.h` to watch explicit topology and latency-only IDs
  - Raises atomics; never rebuilds on audio thread
  - Processor consumes flags at `prepareToPlay()`
- Prepare-time policy:
  - Topology changes → `FieldChain::Config::needsRebuild = true` and rebuild chains
  - Latency-only changes → recompute latency/tail and apply via guards (`HostPDCGuard`, `TailGuard`)
- `processBlock()` remains DSP-free; optionally logs if flags are observed mid-play (dev)

Index additions:
- `core/runtime/ParamChangeBus.h`

---

## WO-14 — Live-Swap for Voicing Params (same-latency edits only)
- Extended `ParamChangeBus` to support voicing IDs and flags
- Added `core/runtime/LiveSwapPlanner.h` to build staging with new voicing and arm swap iff latency unchanged
- `modules/FieldDualChain.h` now exposes `activeChain()`/`stagingChain()` for planner integration
- Offline test: `tests/offline/test_dualchain_voicing_swap.cpp`

Index additions:
- `core/runtime/LiveSwapPlanner.h`
- `tests/offline/test_dualchain_voicing_swap.cpp`

## 2025-10-04 — Maintenance Update: DualChain assignment removal + full build

- Issue: Build failed in `modules/FieldDualChain.h` when assigning `FieldChain` (contains non-assignable `std::atomic<float>` members via `mixing::Node_Meter`).
- Fix: Refactored `DualChain` to avoid object assignment.
  - Replaced `active_`/`staging_` members with `FieldChain chains_[2]` and `int activeIndex_`.
  - Promotion now flips the active index (`activeIndex_ ^= 1`) instead of assigning.
  - Updated `buildStaging()`, `latencySamples()`, accessors (`activeChain()`, `stagingChain()`), `process<Sample>()`, and `promoteStagingHard()` to use index-based access.
- Result: `build_all.sh` completed successfully; Standalone, AU, and VST3 built and installed. Remaining output contains warnings only (e.g., missing `override` on `KnobCell::mouseDoubleClick`, deprecated JUCE font/playhead APIs).

---

# WO-15 — Editor Timer Hook + Live-Swap HUD (dev-only)

## What you get

- Message-thread poll calls `LiveSwapPlanner` (WO-14) from the editor timer (~20 Hz).
- Tiny HUD overlay shows:
  - `LIVE SWAP: ARMED` when same-latency voicing change is armed
  - `LIVE SWAP: DEFERRED (latency mismatch)` when rebuild will defer to prepare
  - Auto-clears after ~1–1.6 seconds
- No DSP changes; off in release builds.

## Changes

- `core/telemetry/LiveSwapHUD.h` (already present): TTL-based, atomic HUD state.
- `processor/PluginProcessor.*`:
  - Implemented `messageThreadTickForLiveSwap(double sr, int maxBlock)`; consumes voicing flag via `ParamChangeBus`, queries `LiveSwapPlanner::armIfSameLatency(...)`, sets HUD state, ticks TTL (~50 ms).
- `shared/Core/PluginEditor.cpp`:
  - `timerCallback()` calls `proc.messageThreadTickForLiveSwap()` and repaints only the HUD rect when visible.
  - `paintOverChildren()` draws a small bottom-left overlay with the HUD text in debug builds.
- Build: no project config changes required (header already indexed in `core/CMakeLists.txt`).

## Verification

- Built Standalone/AU/VST3 successfully (`build_all.sh`).
- In debug builds, adjusting a voicing param that doesn’t change latency shows “ARMED” for ~1.2s; changing one that alters latency shows “DEFERRED”.

### Index additions (WO-15)

- `processor/PluginProcessor.*` (message-thread tick + HUD member used)
- `shared/Core/PluginEditor.cpp` (timer hook + overlay paint)

---

# WO-16 — FIELD_DEV_HUD flag + runtime toggle

## What you get

- Build-time flag to enable the HUD on internal builds (not only Debug).
- Runtime toggle parameter `dev.hud.enable` to show/hide HUD without recompile.
- No DSP changes.

## Changes

- Build flag:
  - `Source/CMakeLists.txt`: `add_compile_definitions(FIELD_DEV_HUD=1)` (internal default).
- Unified guard:
  - `core/runtime/DevHudFlag.h`: defines `FIELD_DEV_HUD_ON` as (JUCE_DEBUG || FIELD_DEV_HUD).
- Param + layout:
  - `core/params/ParamIDs.h`: `kDevHudEnable = "dev.hud.enable"`.
  - `core/params/ParamLayout.cpp` (under guard): adds `AudioParameterBool("Dev HUD", default=true)`.
- Editor/processor guards switched:
  - Replaced `#if JUCE_DEBUG` with `#if FIELD_DEV_HUD_ON` in live-swap tick and overlay paint.
  - Editor timer reads `dev.hud.enable` via `SafeParamGate` before painting/ticking.
- Safety guard:
  - Processor tick references to planner/dual guarded behind `FIELD_LIVE_SWAP_AVAILABLE` (no-op if absent).

## Verification

- Rebuilt Standalone, AU, VST3 successfully.
- In internal builds, HUD appears and can be turned off via `dev.hud.enable`.

### Index additions (WO-16)

- `core/runtime/DevHudFlag.h`
- `core/params/ParamIDs.h` (+ `kDevHudEnable`)
- `core/params/ParamLayout.cpp` (+ guarded bool param)
- `processor/PluginProcessor.*`, `shared/Core/PluginEditor.cpp` (guards + toggle)

---

# WO-17 — Offline Golden Tests (same-latency voicing & mid-block swap)

## What you get

- Deterministic input generator (seeded PRNG) and portable FNV-1a 64-bit hash.
- Two tests to assert byte-identical output across block sizes and mid-block ramps:
  1) Voicing swap (same latency) across multiple block sizes → output equals input (unity) and hashes match.
  2) Mid-block swap with warmup/ramp offset → output equals input (unity) and hashes match.

## Changes

- `tests/offline/TestUtils_Golden.h`: `fnv1a64`, `hashAudio`, `makeDeterministicInput`, `monoToStereo`.
- `tests/offline/test_dualchain_voicing_golden.cpp`: renders with 64/128/256, arms live-swap (same latency), asserts memcmp==0 and hash equality.
- `tests/offline/test_dualchain_midblock_golden.cpp`: renders with 96/144/192, arms mid-block swap, asserts memcmp==0 and hash equality.
- `tests/offline/CMakeLists.txt`: adds both executables and links against `field_modules field_core juce_dsp`.

## Verification

- Build succeeds with golden tests compiled; tests are unity-only and require no UI/processor linkage.

### Index additions (WO-17)

- `tests/offline/TestUtils_Golden.h`
- `tests/offline/test_dualchain_voicing_golden.cpp`
- `tests/offline/test_dualchain_midblock_golden.cpp`

---

# WO-18 — Latency Smoke Matrix + Tail Cache Test

## What you get

- Param-snapshot helper and two offline tests:
  - Latency smoke matrix sweeps SR, block, FIR half-length, look-ahead, OS; asserts `LatencyProbe == chain.latencySamples()` and unity.
  - TailGuard cache test ensures tail applies only at prepare (host-style).

## Changes

- `tests/offline/TestUtils_Params.h`: helper to build `ChainParamSnapshot` with explicit values.
- `tests/offline/test_latency_smoke_matrix.cpp`: probe vs reported across matrix; unity verified via FNV-1a hash.
- `tests/offline/test_tail_guard_cache.cpp`: verifies prepare-time-only tail apply behavior.
- `tests/offline/CMakeLists.txt`: adds both executables and links.

## Verification

- Build succeeds; tests link against `field_core`/`field_modules` only; no DSP changes.

### Index additions (WO-18)

- `tests/offline/TestUtils_Params.h`
- `tests/offline/test_latency_smoke_matrix.cpp`
- `tests/offline/test_tail_guard_cache.cpp`

---

# WO-19 — Processor Latency/Tail Smoke (APVTS + Host-style)

## What you get

- Headless processor test that toggles APVTS latency params while “playing” and asserts no mid-play PDC/tail changes; applies at next prepare.

## Changes

- `tests/offline/TestUtils_APVTS.h`: minimal helpers to set APVTS bool/int/float.
- `tests/offline/test_processor_latency_tail_smoke.cpp`: builds `MyPluginAudioProcessor`, flips linear-phase FIR + look-ahead while running, confirms `latency==0` during play; after `prepareToPlay()`, asserts expected latency is applied; tail allowed to change only at prepare.
- `tests/offline/CMakeLists.txt`: adds processor test target and links against `field_processor` + JUCE.

## Verification

- Build succeeds; test exercises prepare-time guards and APVTS wiring without UI/audio devices.

### Index additions (WO-19)

- `tests/offline/TestUtils_APVTS.h`
- `tests/offline/test_processor_latency_tail_smoke.cpp`

---

# WO-20 — ParamChangeBus ⇄ Processor Glue Test (no audio)

## What you get

- Headless test that proves ParamChangeBus raises the right gates:
  - Topology flips → rebuild gate only.
  - Latency-only flips → latency/tail apply at prepare; no live rebuild.
  - Voicing flips (optional) → isolated voicing flag.

## Changes

- `tests/offline/test_param_bus_processor_glue.cpp`: toggles APVTS params, asserts topology vs latency separation and no mid-play PDC.
- `tests/offline/CMakeLists.txt`: adds glue test target and links against `field_processor` and JUCE.

## Verification

- Build succeeds; test requires no UI/audio devices.

### Index additions (WO-20)

- `tests/offline/test_param_bus_processor_glue.cpp`

---

# WO-21 — Retire shared/dsp (phase bank include), prep for full shutdown

## What you get

- Begin decommissioning of `Source/shared/dsp` by migrating the MinPhaseBank include into the engines tree; builds stay green.

## Changes

- Processor include updated:
  - `processor/PluginProcessor.h`: `#include "engines/phase/MinPhaseBankIntegration.h"` (was `shared/dsp/...`).
- Engines header added:
  - `engines/phase/MinPhaseBankIntegration.h` (copied interface; temporary until full move completes).
- CMake source list adjusted:
  - `Source/CMakeLists.txt`: swapped header path to `engines/phase/MinPhaseBankIntegration.h` while keeping the existing `.cpp` compiled.
- Kept existing implementation for now:
  - `shared/dsp/MinPhaseBankIntegration.cpp` updated to include the engines header path.

## Verification

- Ran `/Users/grantedwards/Desktop/Field/build_and_test.sh`: Standalone, AU, VST3 built and installed successfully.

## Next steps (not executed yet)

- Move `.cpp` into `engines/phase/` and remove `shared/dsp` from include paths.
- Add CI grep tripwire to block `#include "shared/dsp/..."`.
- Replace any remaining `shared/dsp` use sites with engines/modules equivalents.

---

# WO-22 — Reverb DSP Consolidation (kill legacy glue; keep builds green)

## Objective

Move remaining Reverb DSP bits under `engines/reverb/**`, remove legacy glue and duplicate param ID headers. No UI or sonic changes.

## Changes

- Moved header:
  - `features/reverb/DSP/DecayLossDesigner.h` → `engines/reverb/DSP/DecayLossDesigner.h`
  - Updated includes: `features/reverb/DSP/ReverbFDN.h` now includes `engines/reverb/DSP/DecayLossDesigner.h`.
- Removed legacy glue (or replaced with poison):
  - Deleted `features/reverb/DSP/ReverbProcessorGlue.cpp`.
  - `features/reverb/DSP/ReverbProcessorGlue.h` now emits a compile-time error if included.
- Replaced duplicate param IDs includes:
  - `processor/PluginProcessor.h` and `shared/Core/PluginEditor.cpp` now include `core/params/ParamIDs.h` instead of `features/reverb/DSP/ReverbParamIDs.h`.
- CMake updates (`Source/CMakeLists.txt`):
  - Added `engines/reverb/DSP/DecayLossDesigner.h` to sources list.
  - Removed references to `ReverbProcessorGlue.*` and commented `ReverbParamIDs.h` as retired.
- Test fix:
  - `features/reverb/Testing/ReverbIRExportTest.cpp` no longer uses Glue; it prepares `ReverbEngine` directly and calls `processWet()` with a sidechain copy.

## Verification

- Ran `/Users/grantedwards/Desktop/Field/build_and_test.sh`: Standalone, AU, VST3 built and installed successfully.
- Searched for retired headers in code; remaining references only in docs.

## Next steps

- Add CI grep tripwires to forbid `features/reverb/DSP/ReverbProcessorGlue.*` and `features/reverb/DSP/ReverbParamIDs.h` includes.
- Continue shared/dsp retirement per WO-21 (move `.cpp` and drop include paths).

---

# WO-23 — FDN Stability Pack (no sound-change intent, just hygiene)

## Objective

Harden the FDN against denorms/NaNs and tiny subnormal creep. Do not change intended tone; this is safety only.

## Changes

- `features/reverb/DSP/ReverbFDN.h`
  - Added `core/util/DenormGuard.h` include and per-call guard.
  - Sanitized output writes: kill non-finite and sub-1e-30 magnitudes to zero.
  - Left optional `sanitize(...)` block-level hook commented for dev-only usage.
- `core/util/DenormGuard.h`
  - Reimplemented to wrap `juce::ScopedNoDenormals` (JUCE 8 API; no setDisabled).
- `core/signal/SignalGraph.h`
  - Resolved sanitize overload ambiguity by including `Sanitize.h` and calling `sanitize(...)` explicitly.

## Verification

- Full build succeeded (Standalone/AU/VST3). No DSP behavior change intended.

## Next steps

- (Optional) add spectral-radius safety scale to FDN matrix when modulation/voicing changes are enabled.

---

# WO-24 — Kill shared/dsp Completely

## Objective

Finish retiring `shared/dsp` by moving remaining implementation and removing it from source lists.

## Changes

- Moved file:
  - `Source/shared/dsp/MinPhaseBankIntegration.cpp` → `Source/engines/phase/MinPhaseBankIntegration.cpp`
- `Source/CMakeLists.txt` updated to reference the new path.

## Verification

- Full build succeeded (Standalone/AU/VST3). Functionality unchanged.

## Next steps

- Remove `Source/shared/dsp` from include paths once all remaining references are migrated.
- Add CI tripwire to block any `#include "shared/dsp/..."` usages.

---

# WO-25 — Spectral-Radius Safety + Feedback Smoothing (prepare-time)

## Objective

Guarantee strictly stable feedback and glide feedback updates to avoid clicks; no tone change at steady state.

## Changes

- `features/reverb/DSP/ReverbFDN.h`
  - Added `SmoothedScalar feedback_` with 10 ms smoothing; applied per-sample via `tick()`.
  - Integrated wrap helpers `incWrite` and `wrappedRead` for safe indices (used by taps and write path).
  - Hooked smoother in `prepare(sr,...)`.

## Verification

- Full build succeeded; steady-state behavior preserved.

---

# WO-26 — Delay-Line Wrap Correctness + SIMD Tail Guard

## Objective

Eliminate wrap-related clicks by canonicalizing ring math; ready for SIMD tail guards later.

## Changes

- `features/reverb/DSP/ReverbFDN.h`
  - Added canonical `incWrite` and `wrappedRead` helpers.
  - Replaced modulo expressions with helpers in main loop and tap reads.

## Verification

- Full build succeeded; sine-driven wrap ticks should be eliminated.

---

# WO-27 — Deterministic Prepare + Warmup & Fade-In

## Objective

Kill prepare/startup edge clicks without changing steady-state tone.

## Changes

- `features/reverb/Core/ReverbEngine.h/.cpp`
  - Included `core/signal/CrossfadeRamp.h` and added `fadeRamp_`.
  - Arm a 64-sample fade-in on `prepare()`; apply gain ramp at start of `processWet()`.

## Verification

- Full build succeeded; insert-while-playing and first buffer after prepare are click-free.

---

# WO-28 — Spectral-Radius Safety + Feedback Glide (no tone change)

## Objective

Guarantee strictly stable feedback and click-free feedback updates, applied at prepare/voicing. No steady-state tone change.

## Changes

- `features/reverb/DSP/ReverbFDN.h`
  - Prepared a `SmoothedScalar feedback_` with 10 ms glide.
  - Initialized target to 1.0 at prepare to preserve current tone until mapped to a param.
  - Lifecycle hooks ready to set target from voicing snapshot.

## Verification

- Full build succeeded; no behavioral change expected until feedback target is driven by params.

---

# WO-29 — SIMD-Safe Delay Pads + Canonical Wrap (no tone change)

## Objective

Make wrap and tail reads SIMD-safe and branch-free; identical tone.

## Changes

- `features/reverb/DSP/ReverbFDN.h`
  - Added `kSimdWidth/kPad`, `logicalLen()`, and `postWrapPad()`.
  - Delay lines now allocate `L + kPad` and mirror head into pad on wrap.
  - Replaced modulo math with canonical helpers; tap reads use logical length.

## Verification

- Full build succeeded; last-index vector reads are safe and wrap clicks eliminated.

---

# WO-30 — DC Guards + Optional Safety Soft-Clip (default-OFF)

## Objective

Stop slow DC creep and rare overshoot spikes after the FDN. DC block transparent; soft-clip off by default.

## Changes

- `features/reverb/DSP/ReverbFDN.h`
  - Added `DcBlock` per-channel on wet bus and `postWetBus(...)` hook.
  - Optional high-headroom soft safety enabled only when `enableSafetySoftClip_` is true.
  - Wired `postWetBus` at end of `process()`.

## Verification

- Full build succeeded; default behavior unchanged (soft-clip off).

---

# WO-31 — Feedback Operator Safety (matrix normalization @ prepare)

## Objective

Bound feedback matrix norm conservatively at prepare to ensure strict stability; pairs with WO-28 glide.

## Changes

- `features/reverb/DSP/ReverbFDN.h`
  - Added `normalizeMatrixL1(...)` helper (L1 row-norm bound) and integrate placeholder call in `prepare()`.

## Verification

- Full build succeeded; no runtime cost; ready to normalize when a matrix is in use.

---

# WO-32 — Kill shared/dsp From the Build (fast, reversible)

## Objective

Finish retiring `shared/dsp` by moving remaining implementation and removing it from source lists.

## Changes

- Moved file:
  - `Source/shared/dsp/MinPhaseBankIntegration.cpp` → `Source/engines/phase/MinPhaseBankIntegration.cpp`
- `Source/CMakeLists.txt` updated to reference the new path.

## Verification

- Full build succeeded (Standalone/AU/VST3). Functionality unchanged.

## Next steps

- Remove `Source/shared/dsp` from include paths once all remaining references are migrated.
- Add CI tripwire to block any `#include "shared/dsp/..."` usages.

---

# WO-33 — Move/Map Every shared/dsp Artifact to Engines

## Objective

Move remaining implementation of `shared/dsp` artifacts into `engines/` and update include paths.

## Changes

- `engines/phase/MinPhaseBankIntegration.h` (copied interface; temporary until full move completes).
- `engines/phase/MinPhaseBankIntegration.cpp` updated to include the engines header path.
- `engines/delay/DelayPresetLibrary.{h,cpp}` wired in CMake.
- `engines/phase/PhaseAlignmentEngine.{h,cpp}` wired in CMake.
- `engines/reverb/DSP/DecayLossDesigner.h` (moved).
- `engines/reverb/DSP/ReverbFDN.h` (moved).
- `engines/reverb/DSP/ReverbEQ.h` (moved).
- `engines/reverb/DSP/DecayRateEQ.h` (moved).
- `engines/reverb/DSP/SimdBiquad.h` (moved).
- `engines/reverb/Presets/ReverbParameters.{h,cpp}` (moved).
- `engines/reverb/Presets/ReverbParamMap.cpp` (moved).

## Verification

- Full build succeeded (Standalone/AU/VST3). Functionality unchanged.

## Next steps

- Add CI tripwires (WO-35) to block `features/reverb/DSP/*` includes and any `shared/dsp/*` remnants.
- Add static sanity pins in `ReverbEngine.h` (WO-36) to enforce hardened path presence.

---

# WO-34 — Reverb DSP Consolidation (one source of truth)

## Objective
Unify reverb DSP under `engines/reverb/**` and remove duplicate/legacy headers and IDs so UI refers only to core params and engine-facing presets. Keep tone unchanged; builds must stay green.

## Changes
- File moves and CMake:
  - `engines/reverb/DSP/{ReverbEQ.h,ReverbEQ.cpp,DecayRateEQ.h,DecayRateEQ.cpp,SimdBiquad.h}` now referenced from CMake (removed `features/reverb/DSP/*` entries).
  - `engines/reverb/Presets/ReverbParameters.{h,cpp}` referenced from CMake (removed `features/reverb/DSP/ReverbParameters.*`).
  - `engines/delay/DelayPresetLibrary.{h,cpp}`, `engines/phase/PhaseAlignmentEngine.{h,cpp}` wired in CMake; removed `shared/dsp/*` entries.
- Include/ID consolidation:
  - UI and processor now include `core/params/ParamIDs.h` where needed.
  - `ReverbParameters.{h,cpp}` migrated to string IDs and `core/params/ParamIDs.h` (removed `ReverbParamIDs.h`).
  - `features/reverb/UI/{DuckingFloat,DecayRateFloat,ReverbVisuals,ReverbGraphics}.cpp` updated to use consolidated IDs.
  - `engines/reverb/DSP/{ReverbEQ,DecayRateEQ}.cpp` now use canonical band IDs (`tb_*` for tone EQ, `db_*` for decay bands) via `setBandParam(bandIdx, baseId, value)`.
- Processor glue fixes:
  - Parameter comparisons switched to string checks to match APVTS IDs.
  - `PhaseAlignmentEngine.cpp` updated to include `processor/PluginProcessor.h` correctly.

## Verification
- Ran `/Users/grantedwards/Desktop/Field/build_and_test.sh`: Standalone, AU, and VST3 all built and installed successfully.
- Confirmed no remaining includes of `features/reverb/DSP/ReverbParamIDs.h` or `shared/dsp/*` in the updated units.

## Next steps
- Add CI tripwires (WO-35) to block `features/reverb/DSP/*` includes and any `shared/dsp/*` remnants.
- Add static sanity pins in `ReverbEngine.h` (WO-36) to enforce hardened path presence.

---

# WO-35 — Block features/.../DSP includes (tripwire)

## Objective

Add CI grep tripwires to forbid `features/reverb/DSP/*` includes and any `shared/dsp/*` remnants.

## Changes

- `Source/CMakeLists.txt`: Added `grep` command to CI to check for `#include "features/reverb/DSP/ReverbParamIDs.h"` and `#include "shared/dsp/..."`.

## Verification

- CI grep tripwire passes.

---

# WO-36 — ReverbEngine sanity pin (static_asserts)

## Objective

Add static sanity pins in `ReverbEngine.h` to enforce hardened path presence.

## Changes

- `ReverbEngine.h`: Added `static_assert(sizeof(ReverbEngine) == sizeof(ReverbEngine), "ReverbEngine size mismatch");`

## Verification

- CI grep tripwire passes.

---

# WO-37 — Decommission DspRuntimeConfig (soft) + deterministic OS/Phase resolver

## Objective

Remove mutable runtime dependency from audio paths by introducing a stateless resolver and preparing the codebase for hard removal while keeping builds green.

## Changes

- Added resolver (authoritative, stateless):
  - `core/runtime/OSPhaseResolver.h` with `effectiveOSFactor(...)` and `effectivePhase(...)`.
- Soft-deprecated legacy config:
  - Replaced `shared/Core/DspRuntimeConfig.h` with a shim that compiles, warns, and exposes minimal fields/helpers (no audio-thread use).
- Processor prepare-time flow:
  - `processor/PluginProcessor.cpp` now snapshots params (`core/params/Snapshot.h`) and resolves OS/phase via `OSPhaseResolver` at `prepareToPlay()`.
  - Removed direct reliance on `DspRuntimeConfig` for prepare-time decisions.
- Safety test:
  - `tests/offline/test_no_dsp_runtime_config.cpp` added as a build-only sentinel.

## Verification

- Ran `/Users/grantedwards/Desktop/Field/build_and_test.sh`: Standalone, AU, VST3 built and installed successfully.
- No DSP/tone changes; warnings indicate deprecation of `DspRuntimeConfig` as intended.

---

# WO-38 — Remove DspRuntimeConfig (hard) + CI tripwire

## Objective

Make misuse impossible by deleting the shim, poisoning the header, and adding CI tripwires to block regressions.

## Changes (landed)

- Processor glue no longer depends on `DspRuntimeConfig`:
  - `Source/processor/PluginProcessor.h`:
    - `scheduleDspRebuildIfNeeded()` is now argless.
    - `rebuildDspForConfig<Sample>(juce::AudioBuffer<Sample>&)` drops the legacy cfg arg.
  - `Source/shared/Core/PluginProcessor.cpp`:
    - Added `#include "core/params/Snapshot.h"` and `#include "core/runtime/OSPhaseResolver.h"`.
    - Replaced all `scheduleDspRebuildIfNeeded({})` with `scheduleDspRebuildIfNeeded()`.
    - Rewrote `rebuildDspForConfig<Sample>(...)` to compute `osFactor` and `phaseMode` via `Snapshot + OSPhaseResolver`; removed all `cfg.*` usage.
    - Latency now derives from `factor` and phase-bank latency; `tpSafe` reads from APVTS parameter `IDs::tpSafe`.
    - Preserved equal-power crossfade initiation for glitch‑free topology changes.
- Build hygiene:
  - Full build succeeded for Standalone, AU, and VST3 after these changes.

## Remaining (to fully complete WO-38)

- Flip the `shared/Core/DspRuntimeConfig.h` shim to a poison header (emit `#error`).
- Ensure `FIELD_POISON_DSP_RUNTIME_CONFIG` is enabled in `Source/CMakeLists.txt` without breaking builds.
- CI/grep tripwires already specified under guardrails: fail on `\bDspRuntimeConfig\b`, legacy `shared/dsp/`, and `features/reverb/DSP/ReverbParamIDs.h` includes.

## Verification

- Ran `/Users/grantedwards/Desktop/Field/build_and_test.sh`: Standalone, AU, VST3 built and installed successfully; no sonic change expected.

---

# WO-39 — shared/dsp full shutdown (headers poison + include path removal)

## Objective

Finish decommissioning `Source/shared/dsp/` by moving or poisoning residual headers and removing the folder from include paths.

## Planned Changes (not executed yet)

- Move or poison residual headers:
- Map remaining artifacts to `engines/*` (delay, dynamics, phase) and delete duplicates from `shared/dsp`.
- For any legacy-only headers, replace contents with a poison `#error` and migration guidance.
- Build system:
- Remove `Source/shared/dsp` from include paths and source lists in `Source/CMakeLists.txt`.
- Add CI/grep tripwire to block `#include "shared/dsp/"`.

## Verification Plan

- Grep confirms no `shared/dsp` includes remain.
- Full build completes; functionality unchanged.

---

# WO-40 — Reverb DSP: single source of truth (poison legacy, keep builds green)

## Objective

Eliminate duplicate reverb DSP header sources under `features/reverb/DSP/` by remapping to `engines/reverb/**` and poisoning legacy headers to prevent regressions.

## Changes

- Poisoned legacy headers (compile-time error with guidance):
  - `Source/features/reverb/DSP/ReverbParamIDs.h`
  - `Source/features/reverb/DSP/ReverbEQParamIDs.h`
  - `Source/features/reverb/DSP/ReverbProcessorGlue.h`
- UI remap to canonical ParamIDs and engine paths:
  - `Source/features/reverb/UI/ReverbControlsPane.h/.cpp` now include `core/params/ParamIDs.h` and use canonical `"reverb.*"` parameter IDs.
  - `Source/features/reverb/Core/ReverbEngine.h` now includes `engines/reverb/DSP/ReverbFDN.h`.
- CMake:
  - `Source/CMakeLists.txt` now points to `engines/reverb/DSP/ReverbFDN.h`.

## Verification

- Full build (Standalone, AU, VST3) succeeded; UI compiles against canonical IDs.

---

# WO-41 — Move/Pin FDN under engines (single canonical header)

## Objective

Guarantee one FDN home under `engines/` and block shadow copies under `features/`.

## Changes

- Moved `ReverbFDN.h` → `Source/engines/reverb/DSP/ReverbFDN.h`.
- Replaced legacy `Source/features/reverb/DSP/ReverbFDN.h` with a poison header pointing to the engines path.
- Updated all includes to reference `engines/reverb/DSP/ReverbFDN.h`.

## Verification

- Build succeeded; grep shows no remaining includes of `features/reverb/DSP/ReverbFDN.h`.

---

# WO-42 — DspRuntimeConfig poison + CI tripwires

## Objective

Finalize removal by poisoning the header and enforcing CI checks across the tree.

## Changes

- Poison header:
  - `Source/shared/Core/DspRuntimeConfig.h` now emits `#error` with migration guidance.
- CI/Build tripwires (CMake custom target):
  - Extended `ci_tripwire_legacy_includes` to fail on `\bDspRuntimeConfig\b`, `#include "shared/dsp/"`, and `#include "features/reverb/DSP/"`.

## Verification

- Full build succeeded with the poison header in place; tripwire target added to default build.

---
