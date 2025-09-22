## Field UI Performance & Consistency Audit

Last updated: 2025-09-22
Scope: Validate responsiveness, consistency, theme-compliance, and lifecycle rules across all UI.

### Initial static-scan findings (triage)
- Hardcoded colours present (hex and `Colours::`), notably in `PluginEditor.*`; replace with `FieldLNF::theme`.
- Legacy caption/value-label placement found (`placeLabelBelow` lambda and KnobCell comments); standardize on `slider.setName(...)` + `KnobCell::ValueLabelMode::Managed`.
- Conditional add/remove in layout paths; verify no creation/reparenting in timers/drag.
- Excessive timer rates: `startTimerHz(60)` in several components; target 15–30 Hz per rules.
- Opaqueness: defer changes to visuals. Identify candidates only (components that fully paint their backgrounds) and revisit later.
- Widespread `reduced(...)` use; ensure Group 2 panels do not use outer reductions and grids remain gapless.
- Texture/paint cost review needed in metallic/gradient-heavy paths; cache images where needed.

### How to verify per component
- Captions/labels: `setName(...)` used; Managed value-label mode; correct precision rules.
- Attachments: created once and owned long-term; none created in `resized/timer/drag`.
- Paint: no heavy per-pixel/random work; textures cached; repaint minimal region; `setOpaque(true)` when fully painting.
- Timers: 15–30 Hz; no layout or heavy work in callbacks.
- Layout: flat grids; zero gaps; no outer `reduced(...)` on Group 2; DUCK strip metrics match.
- Theme: no hardcoded hex/`Colours::`; colours derived from `FieldLNF::theme`; correct metallic scope and border flags.

---

### Component checklists

#### Top-level and LNF
- `Source/PluginEditor.h`
- `Source/PluginEditor.cpp`
- `Source/FieldLookAndFeel.h`
- `Source/FieldLookAndFeel.cpp`

Checklist:
- [ ] Captions via `setName(...)`; Managed value labels; precision per type
- [ ] One long-lived attachment per control; not in `resized/timer/drag`
- [ ] No hardcoded colours; all from `FieldLNF::theme`
- [ ] No heavy/random allocations in `paint()`; cache textures; minimal repaints
- [ ] Timers 15–30 Hz; no layout in timers
- [ ] Flat grids; zero gaps; no outer `reduced(...)` on Group 2
- [ ] `setOpaque(true)` if fully painting background
- [ ] Remove legacy `placeLabelBelow` usage

#### Core cells
- `Source/KnobCell.h`
- `Source/KnobCell.cpp`
- `Source/KnobCellDual.*`
- `Source/KnobCellQuad.*`
- `Source/KnobCellMini.h`

Checklist:
- [ ] Default to `ValueLabelMode::Managed`; label gap set; metrics consistent
- [ ] No allocations/randomization in `paint()`; cache any heavy assets
- [ ] Minimal repaint regions; set opaque if fully painted

#### Reverb UI
- `Source/reverb/ui/ReverbPanel.*`
- `Source/reverb/ui/ReverbControlsPanel.h`
- `Source/reverb/ui/ReverbEQComponent.*`
- `Source/reverb/ui/ReverbScopeComponent.*`
- `Source/reverb/ui/DecayCurveComponent.*`

Checklist:
- [ ] Abbreviations per spec (ER WID, TL WID, ER DEN, ...)
- [ ] DUCK strip metrics match main knobs
- [ ] Managed value labels; captions and precision correct
- [ ] Theme-only colours; no metallic tint on Group 2
- [ ] Timers within 15–30 Hz; no layout in callbacks

#### Motion UI
- `Source/motion/MotionPanel.*`

Checklist:
- [ ] Lives in Group 1 only; flat grid; zero gaps
- [ ] Theme: `motionPanelTop/motionPanelBot/motionBorder`; migrate to `motionPurpleBorder`
- [ ] Managed value labels; captions present for LNF rendering
- [ ] No hardcoded colours; cache heavy paints

#### Delay/Imager/Stereo/Meters
- `Source/ui/delay/DelayVisuals.h`
- `Source/ui/ImagerPane.h`
- `Source/ui/StereoFieldEngine.*`
- `Source/ui/SpectrumAnalyzer.*`
- `Source/ui/ProcessedSpectrumPane.h`

Checklist:
- [ ] Managed labels where using KnobCell; captions set
- [ ] Theme-only colours; no random per-paint
- [ ] Timers 15–30 Hz (Spectrum/Scopes may justify higher; measure); minimal repaints

#### Machine panes and helpers
- `Source/ui/MachinePane.*`
- `Source/ui/machine/MachinePane.*`
- `Source/ui/machine/WidthDesignerPanel.*`
- `Source/ui/machine/ProposalCard.*`
- `Source/ui/machine/MachineEngine.*`
- `Source/ui/machine/MachineHelpersJUCE.h`
- `Source/ui/machine/ParamPatch.h`

Checklist:
- [ ] Flat layouts; no outer `reduced(...)` on Group 2 screens
- [ ] No add/remove/reparent in timers/drag; toggle visibility instead
- [ ] Theme-only colours; set opaque where fully painted

#### Supporting UI
- `Source/PresetCommandPalette.*`
- `Source/ui/PaneManager.h`

Checklist:
- [ ] Theme-only colours; timers 15–30 Hz; no layout in timers
- [ ] No hardcoded hex; minimal repaint regions

---

### Action items queue (current)
- [ ] Replace hardcoded colours in `PluginEditor.*` with `FieldLNF::theme` lookups
- [ ] Remove `placeLabelBelow` path; enforce Managed labels in all `KnobCell` usages
- [x] Normalize timer rates to 15–30 Hz where feasible (Motion/Delay visuals at 60 Hz require profiling justification)
- [ ] Audit and set `setOpaque(true)` where applicable
- [ ] Verify Group 2 layouts have no outer `reduced(...)`; keep zero gaps
- [ ] Ensure texture caching for metallic/brush/noise where used

### Runtime verification steps
- Repaint highlighting on (Debug): verify child-only repaint during drags
- Interaction sweep: fast drags; confirm no creation/reparenting in logs
- Timer sweep: disable meters/animations → baseline; re-enable at 15–30 Hz; confirm stability
- Adaptive burst sweep: begin dragging any control; confirm editor timer rises to ~60 Hz during interaction and returns to ~30 Hz within ~150 ms after release; ensure CPU drops back accordingly.

---

## Component findings: PluginEditor

Evidence and initial actions for `Source/PluginEditor.*`.

- Hardcoded colours in multiple paint paths (replace with theme):
```318:320:Source/PluginEditor.h
            juce::Colour accent = juce::Colour(0xFF2196F3);
            juce::Colour textGrey = juce::Colour(0xFFB8BDC7);
            juce::Colour panel = juce::Colour(0xFF3A3D45);
```
```28:33:Source/PluginEditor.cpp
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xFF2C2F35), r.getTopLeft(), juce::Colour (0xFF24272B), r.getBottomRight(), false));
        g.fillRect (r);
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawRoundedRectangle (r.reduced (1.0f), 5.0f, 1.0f);
```

- Legacy label placement helper still present; migrate to Managed labels only:
```2429:2437:Source/PluginEditor.cpp
    auto placeLabelBelow = [&] (juce::Label& label, juce::Component& target, int yOffset)
    {
        if (auto* parent = target.getParentComponent())
        {
            if (label.getParentComponent() != parent)
            {
                if (auto* oldParent = label.getParentComponent())
                    oldParent->removeChildComponent (&label);
                parent->addAndMakeVisible (label);
            }
```
Action: remove this pathway for `KnobCell`-managed controls; ensure all such labels are set to `ValueLabelMode::Managed` and positioned in `KnobCell::resized()`.

- Timer frequencies review: several are at or above targets; ensure justification or reduce:
```293:296:Source/PluginEditor.h
            startTimerHz(20); // High refresh so blink is obvious
```
```2241:2241:Source/PluginEditor.h
            startTimerHz(60);
```
Action: keep 15–30 Hz unless profiling shows need; if 60 Hz required (e.g., animation), measure and confine repaint area.

- Conditional add/remove during layout: ensure not executed in drag/timers; okay in `resized/performLayout` only:
```2490:2492:Source/PluginEditor.cpp
    if (headerLeftGroup.getParentComponent() != this) addAndMakeVisible (headerLeftGroup);
    if (bypassButton.getParentComponent() != &headerLeftGroup) headerLeftGroup.addAndMakeVisible (bypassButton);
```
Action: confirm `performLayout` is called only on size/layout changes, not during high-frequency interactions.

- Managed value labels present in many cells (good); continue unifying:
```3321:3326:Source/PluginEditor.cpp
            bassCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            if (kc) { kc->setMetrics (lPx, valuePx, labelGap); kc->setValueLabelMode (KnobCell::ValueLabelMode::Managed); kc->setValueLabelGap (labelGap); }
```

Planned fixes for PluginEditor:
- [x] Replace hardcoded colours with `lf->theme.*` with fallbacks removed
- [ ] Remove `placeLabelBelow` use; ensure all `KnobCell` use Managed labels
- [x] Audit timers; reduce to 15–30 Hz where acceptable
- [ ] Verify `performLayout` call sites; avoid during drag/timer paths
- [ ] Consider `setOpaque(true)` for containers fully painting their background


### Completed actions (2025-09-22)
- Theme colours in Pan overlay and labels
  - `Source/PluginEditor.h` → `PanSlider::paint`: overlay arcs now use `FieldLNF::theme.accent` with fallback to `Colours::lightblue`; label text uses `theme.text` with fallback.
  - Removes dependency on hardcoded blue (`0xFF5AA9E6`) in paint path.
- Editor heartbeat timer normalized
  - `Source/PluginEditor.cpp` → editor ctor: added `startTimerHz(30)`. Existing `timerCallback` retains internal throttling (e.g., heavy work ~10 Hz; modal-aware skip). This balances smoothness and CPU.
- Paint-path allocation reductions
  - `Source/PluginEditor.cpp` → `XYPad::drawEQCurves`: preallocate `juce::Path` storage (`preallocateSpace`) based on sample count to prevent per-frame reallocations.
 - Adaptive site-wide refresh burst (interaction-driven)
  - `Source/PluginEditor.*`: editor runs at 30 Hz baseline and automatically bursts to 60 Hz while the user is interacting (mouse down/drag/wheel) and for ~150 ms after, then returns to 30 Hz.
  - Implemented via a child-propagating MouseListener proxy and a timer Hz adjustment in `timerCallback`. This keeps idle cost low while making drags feel crisp.

- Component timer normalization and visibility gating
  - `Source/motion/MotionPanel.*`: reduced to 30 Hz and added `visibilityChanged()` gating (start at 30 Hz when visible, stop when hidden).
  - `Source/PluginEditor.h` → `ShadeOverlay`: reduced to 30 Hz and added `visibilityChanged()` gating.
  - Policy clarified in `FIELD_UI_RULES` and `FIELD_RULEBOOK.md`: editor may burst to 60 Hz; components stay ≤30 Hz unless profiled; timers off when hidden.

Notes:
- Build succeeded for Standalone/AU/VST3. Several warnings remain (deprecated `Font`, unused vars). Track in Analyzer/Machine/Imager cleanup passes; visuals preserved.


