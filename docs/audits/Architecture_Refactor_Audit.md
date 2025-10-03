# Field Architecture Refactor Audit (Phase 1)

Last updated: 2025-10-03 • Branch: `feature`

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
