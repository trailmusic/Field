# Field Architecture Refactor Audit (Phase 1)

Last updated: 2025-10-03 • Branch: `feature`

## Purpose
Protect the UI while restructuring `Source/` to separate UI, processor glue, DSP engines, and cross-cutting core (signal, runtime, telemetry). Provide a clean lane for future signal + latency work.

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

## Structure (target)
- `app/`: entry/host glue only
- `core/`: params, runtime gates, signal graph, telemetry, utils
- `engines/`: pure DSP packages (no UI)
- `modules/`: node wrappers for engines (graph wiring)
- `processor/`: single `AudioProcessor` + editor shell (glue only)
- `ui/` and `features/`: visual-only
- `presets/`: data + minimal loaders
- `tests/`: offline + perf

## Decisions & Guardrails
- No DSP code inside `processor/` beyond orchestration and latency reporting.
- `ui/` and `features/` are display/interaction only; they cannot touch audio buffers.
- Single source of truth for latency: `core/runtime/LatencyManager` applied from `processor/` on the message thread.
- Topology rebuilds are gated and occur in `prepareToPlay()` or via explicit crossfades; never mid-block without protection.
- Forwarders remain only as a transition aid and will be removed when migrations finish.

## Next Steps
1. Migrate implementations from `shared/Core/PluginProcessor.cpp` and `PluginEditor.cpp` into `processor/` in small, verified steps; remove bridge includes afterwards
2. Extract DSP from features into `engines/` and wrap via `modules/FieldNodes/*`
3. Move param IDs/layout to `core/params/`; move preset code to `presets/`
4. Add `tests/offline` (null-unity, latency-probe) and `tests/perf`
5. Tighten CMake: real `field_core` target and explicit links; remove legacy include paths
6. Clean warnings (override, deprecations)

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
