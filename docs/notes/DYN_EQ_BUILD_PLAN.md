## Dynamic EQ (DynEQ) Build Plan

Last updated: 2025-09-23

### Scope

Replace the former Spectrum tab with a precision-first Dynamic/Spectral EQ pane (in-pane experience). Tab 2 is now Dynamic EQ throughout code and state. The module interops with XY, Motion, Band/Imager, Reverb, and Delay. No Sketch Mode.

### Constraints & Compliance

- FIELD_UI_RULES: 2×16 flat grids; Managed labels; theme-only colours; zero gaps; timers 15–30 Hz; setOpaque(true) when fully painting; avoid add/remove in timers; no heavy UI on message thread.
- UI Performance Audit: reuse analyzer mapping and path caching; minimal repaint regions; editor timer bursts allowed but component timers ≤30 Hz and visibility-gated.
- Build: Standalone, AU, VST3 always built together via scripts.
- Naming: use DynEQ; state key is `dyneq` (migrate from legacy `spec`).

### Signal Chain Placement

- Default: Input → Utility/Trim → XY Pad EQ → Dynamic EQ → Band/Imager/Motion → Saturation → Reverb/Delay → Output.
- Per-band detector tap: PreXY / PostXY / External (Motion/Rev/Delay).
- Optional alternate placements via per-band tap only (no global reorder).

### Parameter Schema (APVTS)

- Instance: enabled, qualityMode (Zero/Natural/Linear), oversample (1x/2x/4x/8x), analyzerOn, analyzerPrePost, latencyMs (readout), unmaskEnable, unmaskTargetBus.
- Bands (24): active, type, slope, channel, phase, freqHz, gainDb, q.
  - Dynamics: dynOn, dynMode, dynRangeDb, dynThreshDb, dynAtkMs, dynRelMs, dynHoldMs, dynLookAheadMs (0/3/6), dynDetectorSrc (Pre/Post/Ext), dynDetHPHz, dynDetLPHz, dynDetQ, dynTAmount, dynSAmount.
  - Spectral: specOn, specSelect, specResol (Low/Med/High), specAdaptive.
  - Character: character (Clean/Subtle/Warm), charAmt.
  - Constellations: constOn, constRoot (Auto/Pitch/Note/Hz), constHz, constNote, constCount, constSpread, constWeights (blob), constOddEven, constTrack, constGlideMs, constFormant.

### State Model

- `DynamicEqState` mirrors APVTS; serialised as `ValueTree("DynamicEQ")` with 24 `band_i` children.
- Band weights stored as `MemoryBlock` (count + floats) for compactness.
- Migration: accept `ui_activePane == "spec"` and map to `"dyneq"`.

### DSP Architecture

- Engine: `DynamicEqDSP` maintains 24 `DynBandDSP` instances.
- Filters: start with peak/shelf stubs; later add full Bell/Shelf/HP/LP/Notch/BP coefficient factory, slopes 6–96 dB/oct; phase: Zero/Natural/Linear.
- Dynamics: per-band detector with HP/LP/Q; block-level mapping initially; later sample-accurate envelope with look-ahead and T/S split.
- Spectral framework: FFT per band (lazy/shared) with selectivity and resolution presets; apply per-bin gains within band pass.
- Latency: aggregate per-band look-ahead (+ later Linear phase) → instance PDC; expose ms in header.
- Constellations: pitch tracker stub (YIN/ACF later); generate harmonic child frequencies/weights; optional M/S routing bias.

### UI Architecture

- Tab 2: `DynEqTab` hosts visuals and editor; setOpaque(true); 30 Hz timer gated by visibility.
- Visuals: analyzer canvas (pre/post), DynEQ curve overlay, spectral “glitter”, constellation markers; colours from `FieldLNF::theme`.
- Interaction: click add point, drag (freq/gain), wheel Q; double-click delete; mini menu per point (Type, Slope, Channel, Phase, Dynamics, Spectral, Character, Tap, Solo/Delta, Constellations).
- Inspector: bottom 2×16 grid using `KnobCell` with Managed labels; precision: Hz 0 decimals, dB 1 decimal, ms 0–2 decimals.
- Hz mapping: reuse log 20–20k helpers consistent with XY/Analyzer.

### Interop Hooks

- XY: "Adopt XY node" (one-shot copy + optional soft link); hold-⌥ to temporarily bypass overlapping XY bands.
- Motion/Reverb/Delay: expose side-chain taps for per-band detector.
- Unmask: shared 256-bin perceptual loudness bus; allows suggestions and commit flow (later).

### Milestones & Tasks

1) Foundation (this PR)
   - Rename Spectrum → DynEQ (PaneID::DynEQ, paneKey `dyneq`, migrate `spec`).
   - Add `DynEqTab` scaffold and wire into PaneManager.

2) Parameters & State
   - Implement `DynamicEqParamIDs` + `addDynamicEqParameters(apvts, 24)`.
   - Implement `DynamicEqState` with `ValueTree` serialisation.

3) DSP Integration
   - Add `DynamicEqDSP` scaffolding; place post-XY; provide Pre/Post XY taps and optional external side-chain.
   - Latency aggregation for look-ahead; host PDC update.

4) UI Editor

   - Analyzer canvas with theme colours and visibility-gated 30 Hz timer.
   - Point editor (add/drag/wheel/delete) and band mini-menu.
   - 2×16 inspector grid (KnobCell) with Managed labels and correct precision.

5) Spectral & Constellations
   - Spectral bins within band; presets for resolution/selectivity.
   - Pitch tracker replacement; Harmonic Constellations with odd/even bias, glide, formant-safe.

6) Interop & Quality
   - XY adopt/cross-listen; Motion/Reverb/Delay side-chain hooks.
   - Unmask bus scaffolding (publish/fetch), suggestion UI (later pass).

7) Performance & Polish
   - Audit timers, repaints, allocations; adhere to FIELD_UI_RULES.
   - Theming via `FieldLNF::theme`; no hardcoded colours.

### Acceptance Criteria

- Tab 2 labeled “DYNAMIC EQ” renders an analyzer + draggable point editor and a functional 2×16 inspector grid.
- APVTS exposes 24 bands and instance parameters; state save/restore round-trips.
- DSP stage processes post-XY; per-band detector taps work; latency readout reflects look-ahead.
- UI meets performance rules (30 Hz components, minimal repaints), labels managed via KnobCell, correct precision.

### Risks & Follow-ups

- Linear phase/anti-pre-ring requires careful latency handling and potential blending.
- Spectral dynamics cost: consider shared FFT pool and decimation.
- Inter-instance Unmask suggestions need UX iteration before shipping.


