# Tabs-Driven Controls Migration (Final Direction)

Last updated: 2025-09-23

## 1) Goal
- Make tabs the single driver of the UI. Each tab shows its own controls only.
- Replace legacy Group 1/Group 2 model with per-tab, dedicated control grids.
- Preserve all visual style, control behavior, and parameter wiring.

## 2) Scope & Definitions
- Tabs (via `ui/PaneManager.h`): XY, Dynamic EQ (Spectrum), Imager, Band, Motion, Machine, Reverb, Delay.
- Each tab gets a dedicated 2×16 (32-slot) flat grid for its controls.
- Instant switching: no sliding overlays; visibility toggle only.
- Ownership: controls are owned and mounted inside their tab/pane component.
- Empty cells: use a styled empty `KnobCell` when fewer than 32 controls are needed.

## 3) Final Direction & Principles
- Tabs drive visibility; no shared global rows. The old Group 1/2 distinction is removed.
- Build controls once, attach once; never create/destroy/reparent during interaction.
- Use `FieldLNF::theme` and `Layout::dp` exclusively (no hardcoded colours, no magic px).
- Captions via `slider.setName(...)`; Managed value labels (`KnobCell::ValueLabelMode::Managed`).
- Timer policy: editor ~30 Hz baseline with short bursts on interaction; component timers ≤30 Hz and off when hidden.

## 4) Per-tab 2×16 control grids

### Delay (32 items)
- Row A (1–16): Enable, Mode, Sync, Feel, Ping-Pong, Freeze, Filter, Wet Only, TIME, FB, WET, RATE, DEPTH, SPREAD, WIDTH, PRE
- Row B (17–32): SAT, DIFF, SIZE, HP, LP, TILT, WOW, JITTER, Duck Source, Post, THR, DEPTH, ATT, REL, LA, RAT

Notes: Derived from the existing 4×8 layout in `Source/PluginEditor.cpp` (rows 1–4 → concatenated left-to-right, top-to-bottom).

### Reverb (32 items)
- Row A (1–16): Enable, PRE, ER LVL, Algo, ER DEN, ER WID, Wet Only, SIZE, DIF, MOD DEP, MOD RATE, HP, LP, TILT, EQ MIX, DEC XO LO
- Row B (17–32): ER→TAIL, LOW×, MID×, HIGH×, WIDTH, WET, DECAY, BLOOM, DISTANCE, FREEZE, DUCK, ATT, REL, THR, RAT, DEC XO HI

Notes: Derived from `ReverbControlsPanel` + top-level additions (Enable/Algo/WetOnly + new SIZE/BLOOM/DIST/DynEQ XO) in `Source/PluginEditor.cpp`.

### Motion (24 + 8 blanks)
- Row A (1–16): Enable, Panner, Path, Rate, Depth, Phase, Spread, Elev, Bounce, Jitter, Quant, Swing, Mode, Retrig, Hold, Sens
- Row B (17–24): Offset, Inertia, Front, Doppler, Send, Anchor, Bass, Occlusion, [8× Empty KnobCell]

Notes: Matches the existing Motion set (24). All visuals/labels managed by `KnobCell` and Switch/Combo wrappers.

### Imager (stereo field + placement)
- Row A (1–16): WIDTH, WIDTH LO, WIDTH MID, WIDTH HI, ROTATION, ASYM, SHUF LO, SHUF HI, SHUF X, MONO, PAN, [5× Empty]
- Row B (17–32): GAIN, SAT MIX, DRIVE, WET ONLY, [12× Empty]

Notes: Groups imaging and placement controls for discoverability with the Imager visuals. MONO includes its aux as currently styled.

### Dynamic EQ (Center/Tone)
- Row A (1–16): BASS, HP, LP, Q, Q LINK, AIR, TILT, SCOOP, SHELF SHAPE, XO LO, XO HI, [5× Empty]
- Row B (17–32): PUNCH AMT, PUNCH MODE, PHASE REC, PHASE AMT, CNTR LOCK, PROM, FOCUS LO, FOCUS HI, [8× Empty]

Notes: Consolidates tone and center controls that currently live in the middle/left of the legacy Group 1 grid.

### Machine
- Row A/B: [32× Empty KnobCell] initially, or populate with machine-specific controls when available.

### XY (pad) & Spectrum (Dynamic EQ visuals)
- Their canvases remain the pane content. Their 2×16 grids are empty unless later assigned tooling controls.

## 5) Ownership & Switching
- Each pane owns its controls and the 2×16 grid. Controls are created and attached once in the pane’s constructor, added to that pane only.
- On tab change (`PaneManager::setActive`), only `setVisible(true/false)` toggles; no reparenting. Reflow only on size/scale changes.

## 6) Implementation steps (high-level)
1. Create per-pane 2×16 grid containers (DelayPaneControls, ReverbPaneControls, MotionPaneControls, ImagerPaneControls, DynamicEqPaneControls, MachinePaneControls).
2. Move existing controls into their pane containers; keep captions, Managed value labels, metrics, theme flags.
3. Keep `AudioProcessorValueTreeState` attachments 1:1 and long-lived; build once in pane constructors.
4. Replace legacy Group 1/2 layout with pane-owned grids. Remove overlay/viewport coupling for controls.
5. Wire `PaneManager` to show/hide the correct pane content instantly (no slide), preserving canvases (DelayVisuals, ReverbCanvas, etc.).
6. Fill unused grid slots with styled Empty `KnobCell` to maintain strict 2×16 geometry.
7. Respect timer and repaint policies (visibility gating; ≤30 Hz on components; 30 Hz editor baseline with short bursts on interaction).

## 7) Preservation of behavior & visuals
- No parameter logic changes. All control IDs, gestures, and attachments remain identical.
- LNF-only visuals: colours from `FieldLNF::theme`; no new hardcoded hex values introduced.
- Captioning and value-label precision per `FIELD_UI_RULES`.
- Metrics: `Layout::dp`, `Layout::Knob::{S,M,L,XL}`, uniform row heights; zero gaps.

## 8) Performance & safety commitments
- No allocations in hot paths; no add/remove/reparent during interaction or timers.
- Reflow only on size/scale change (dirty flag). Minimal repaint regions.
- Component timers off when hidden; editor timer policy unchanged.

## 9) Open items
- PAN/MONO placement: default in Imager per above. If a distinct Center tab is preferred, move Center cluster there.
- DynEQ/EQ editors and canvases remain in their panes (Reverb/Delay/Imager/Spectrum) unchanged.

## 10) Acceptance criteria
- Tabs switch instantly; exactly one pane’s controls are visible and interactive.
- Delay/Reverb tabs show 32 controls each; others respect 2×16 with styled empties.
- Visuals match current theme and metrics; captions and label precision correct.
- Parameter round-trip intact (UI ↔ APVTS ↔ UI); no duplicate attachments.
- No layout or reparent in timers; no performance regressions in drag tests; repaint regions minimal.


