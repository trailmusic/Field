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

## Next Steps
1. Migrate implementations from `shared/Core/PluginProcessor.cpp` and `PluginEditor.cpp` into `processor/` in small, verified steps; remove bridge includes afterwards
2. Extract DSP from features into `engines/` and wrap via `modules/FieldNodes/*`
3. Move param IDs/layout to `core/params/`; move preset code to `presets/`
4. Add `tests/offline` (null-unity, latency-probe) and `tests/perf`
5. Tighten CMake: real `field_core` target and explicit links; remove legacy include paths
6. Clean warnings (override, deprecations)

## Risk & Mitigation
- Latency consistency: single `LatencyManager` in `processor/`; apply on message thread; add offline tests
- Hidden UI↔DSP coupling: forwarders + staged moves + build validation
- Bridge source ownership ambiguity: tracked as short-lived; removal is a milestone

## Owners
- Architecture/Processor: @trail / @grant
- DSP/Core: @trail
- UI/Features: @grant
