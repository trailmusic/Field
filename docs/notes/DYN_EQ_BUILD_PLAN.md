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

#### Curve Rendering & Colour System

- Curve taxonomy:
  - Band Contribution Curves: per-band responses rendered as light paths (code: `bandPaths`) with optional area fills (`bandAreas`) for dynamic/spectral states.
  - Macro EQ Curve: composite response of all active bands (code: `eqPath`), visually more prominent.
- Theme-driven palette (no hard-coded colours):
  - All colours derive from `FieldLNF::theme`.
  - Macro curve colour = `theme.accent` with slight prominence (thicker stroke).
  - Band colours: generated per-band by hue-cycling around `theme.accent` using a golden-ratio offset; saturation/brightness clamped to readable ranges.
  - Channel variants: per-band base colour tinted by channel selection — Stereo/Mid=base, Side=increased saturation/brightness, Left=slight negative hue shift, Right=slight positive hue shift.
- Dynamic/Spectral visualization:
  - When Dynamics or Spectral is ON for a band, a subtle vertical gradient fill under that band’s path is shown (area path in `bandAreas`).
  - Fill alpha is slightly higher for Dynamic than Spectral to improve legibility while respecting UI performance rules.
- Dynamic range path & handle:
  - Each band draws a secondary “dynamic range” path (`bandDynPaths`) indicating the max compression/expansion envelope relative to its Band Contribution Curve.
  - A central handle sits at the band’s center frequency; dragging it adjusts `dynRangeDb` (respects `dynMode` Up/Down), Gaussian-weighted by Q so the shape mirrors the band.
  - Visuals are theme-tinted (slightly darker variant of the band colour); optional dashed stroke in polish phase.
- Units & grid:
  - Horizontal dB lines and labels are drawn by the pane (not the analyzer).
  - Vertical Hz ticks from analyzer remain; pane adds Hz labels at major points (20, 50, 100, 200, 500, 1k, 2k, 5k, 10k, 20k).

#### Overlay (Floating Per-Band Panel)

- Positioning: anchored near bottom of pane at a fixed Y; X follows the selected band’s latitude. Panel stays out of the EQ curves.
- Drag behavior: during overlay slider drags (Gain/Q/Freq), the panel position is frozen to prevent the Freq slider from chasing the mouse as the band moves.
- Controls: Gain (dB), Q, Freq (log 20–20k), Type, Phase, Channel, Dynamic/Spectral toggles. Small curve icon mirrors current Type.

#### Interaction Details

- Predictive type on add: HP auto-selects below 50 Hz; LP auto-selects above 10 kHz. Otherwise Bell.
- Wheel adjusts Q; Shift+wheel for faster adjustment. Double-click deletes band. Right-click offers per-band quick actions.

Implementation references:
- `DynEqTab.h`: colour helpers `bandColourFor(bandIdx)`, `applyChannelTint(colour, channel)`, and `macroColour()`.
- `FieldLookAndFeel.h`: `FieldLNF::theme` supplies all base colours including `theme.accent` and EQ palette.

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


