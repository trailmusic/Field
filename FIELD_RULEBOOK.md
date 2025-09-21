# FIELD GUI + CODE RULEBOOK (v1)

## 0) non-negotiables (musts)

- **Single sources of truth**
  - **Theme/colors**: only from `FieldLNF::theme` (e.g., `theme.panel`, `theme.text`, `theme.accent`, `theme.hl`, `theme.sh`, `theme.shadowDark`, `theme.shadowLight`). No raw hex colours in components.
  - **Layout metrics**: only from tokens in `Source/Layout.h` (`Layout::*` and `Layout::Knob`, `Layout::knobPx`, helpers like `sizeKnob/sizeMicro`) and `Layout::dp(px, scaleFactor)`. No magic numbers in `resized()` or `paint()`.
  - **Angles & mapping**: use the control’s own parameters (e.g., `slider.getRotaryParameters()` or `valueToProportionOfLength`). Do not hardcode π spans.
- **Sizing only in `resized()`**
  - Never call `setBounds`/`setSize` in constructors (except the top-level editor’s initial `setSize`).
  - `paint()` must not call layout/sizing functions.
- **No global mouse hacks**
  - Hover/active visuals use `isMouseOverOrDragging()` and `isMouseButtonDown()` on the control itself.
  - If a custom draw function lacks the control reference, pass an `isHovered`/`isActive` flag; don’t query `Desktop` for hit tests.
- **APVTS discipline**
  - All parameter I/O goes through attachments or explicit `beginChangeGesture` / `setValueNotifyingHost` / `endChangeGesture`.
  - Never set parameters in `paint()` or `resized()`.
- **Green/Ocean modes**
  - Mode drives only theme values and `setGreenMode(bool)` on components; no conditional hardcoded colors in components.

---

## 1) project structure & naming

- **Files**
  - Look & feel: `FieldLookAndFeel.h/.cpp` (all drawing and palette).
  - Editor: `PluginEditor.h/.cpp` (layout, wiring, component ownership).
  - Processor: `PluginProcessor.h/.cpp` (audio, parameters).
  - Custom components each in their own files (e.g., `ToggleSwitch`, `XYPad`, `ControlContainer`, `PresetCombo`).
- **Namespaces & names**
  - Layout constants live in `namespace Layout { ... }`.
  - Class names: PascalCase; members: camelCase; constants: ALL_CAPS only inside `Layout`.
  - No `using namespace` in headers.

---

## 2) layout & scaling

- **Scale-aware**
  - All pixel values pass through `Layout::dp(px, scaleFactor)`.
  - `scaleFactor` is computed from the smaller of width/height ratios: `min(getWidth()/baseWidth, getHeight()/baseHeight)` and clamped (current floor: 0.5; ceiling: 2.0).
  - Define rhythm in `Layout` (see `Source/Layout.h`): `PAD`, `GAP`, knob sizes via `Layout::Knob::{S,M,L,XL}`, micro sizes, and breakpoints.
- **Where layout happens**
  - The editor owns grid/flow; containers arrange their children only (no sibling knowledge).
  - Use `juce::Grid` for rows/columns; don’t manually sprinkle `setBounds` everywhere. Use layout tokens to size items prior to `performLayout()`.
- **Breakpoints**
  - Use `getWidth()` vs `Layout::BP_WIDE` to switch grid templates (e.g., collapse “split pan” when narrow).
  - Minimum width floor uses `Layout::BP_WIDE` (wide breakpoint) or calculated content minimum, whichever is larger. Initial size prefers baseWidth/baseHeight over content min.

- **Containers (Editor-level)**
  - `leftContentContainer`: holds panes (top) and both control groups (rows) below. All Group 1/2 controls are parented here and use container-local coordinates starting at x=0.
  - `metersContainer`: sibling at right; meters are children here (no overlap with left content). Width is derived from grid metrics; heights are local to the container.
  - Stacking: panes at the top of the left container, then 4 uniform control rows directly below; no left padding beyond container border.

---

## 3) drawing rules (LNF-only visuals)

- All visual tokens originate in `FieldLNF::theme`. Components never invent colors.
- **Shadows/glows**
  - Use `juce::DropShadow` in LNF or a component’s `paint()`; alpha in theme; radius minimal (<= 20).
- **Rotary sliders**
  - Angles: always from the slider’s rotary params (`start`, `end`, `stopAtEnd`).
  - Progress ring: background ring in `theme.base.darker(0.2f)`, value ring in `theme.accent`.
  - Tick marks: compute as fractions of arcSpan (0, .25, .5, .75, 1.0); never assume ±π.
  - Hover “raise”: expand the local draw bounds by a few dp if `isMouseOverOrDragging()`.
- **Linear sliders (micro)**
  - Position with `slider.valueToProportionOfLength` (skew-safe).
  - Clamp thumb visuals into the track; progress fill uses `theme.accent` gradient.
- **Text**
  - Fonts: use `juce::FontOptions(size).withStyle("Bold")` for titles/knob labels.
  - Colors: `theme.text` / `theme.textMuted`. No hardcoded whites.

---

## 4) component behavior

- **State → visuals only**
  - `paint()` must never mutate state or parameters.
  - Animations: use `juce::SmoothedValue` or a `Timer` updating a local visual property; call `repaint()`.
- **Hover/active**
  - Only via `isMouseOverOrDragging()` and `isMouseButtonDown()` of the component.
  - For helper draw fns that lack the component, add parameters (e.g., `drawGainSlider(..., bool isHovered, bool isActive)`).
- **XYPad**
  - Public API: `setSplitMode(bool)`, `setLinked(bool)`, `setSnapEnabled(bool)`, `setPoint01(x, y)`, `getPoint01()`, `getBallPosition(i)`, `setBallPosition(i,x,y)`.
  - Events: `onChange(x,y)`, `onSplitChange(lx, rx, y)`, `onBallChange(index,x,y)`.
  - Hit tests use local bounds only; snap rounds to fixed divisions; all clamping via `jlimit`.
- **ToggleSwitch**
  - Animation via `SmoothedValue`; no external `Desktop` querying; border/hover handled locally.
- **Containers**
  - Draw borders/headers in `paint()`. No parameter logic inside containers.

---

## 5) parameters & attachments (APVTS)

- **Create once, attach once**
  - Each parameter has at most one `SliderAttachment`/`ButtonAttachment` per control.
  - If a control is shown/hidden (e.g., split vs stereo), still attach statically; only toggle visibility.
- **Host sync**
  - Gestures wrap every manual change:

```cpp
if (auto* p = apvts.getParameter("pan")) { p->beginChangeGesture(); p->setValueNotifyingHost(v); p->endChangeGesture(); }
```

- **Listeners**
  - Use listeners only when you must react to host-driven changes (e.g., `space_algo` to UI switch).
- **Preset manager**
  - Parameter getter/setter are declared once and shared; no duplicates.
  - When adding a new parameter: update APVTS layout, presets getter/setter, and UI attachment in a single commit.

---

## 6) performance & safety

- **Paint cost**
  - Avoid per-pixel loops in `paint()`; precompute paths where possible.
  - Heavy visuals (waveforms) gated by `hasWaveformData`; keep buffers bounded.
- **No allocations in hot paths**
  - Reserve vectors or keep them as members; do not allocate in high-frequency `paint()` unless trivial.
- **Threading**
  - UI thread only touches components; audio thread never accesses UI. Data passed via atomics/copies.

---

## 7) interaction & UX

- **Cursor policy**
  - Interactive comps: `PointingHandCursor`; everything else default.
- **Tooltips**
  - Use a shared `TooltipWindow`; short, actionable strings.
- **Accessibility**
  - Provide labels/`setName` for controls (used for central knob labels and screen readers).

---

## 8) code quality gates

- **Includes**
  - Add `<vector>`, `<cmath>` etc. where used; no hidden transitive reliance.
- **C++ hygiene**
  - `override` on all virtual overrides; no naked `new`; RAII for attachments.
  - No `static` state in components unless const/immutable.
- **PR checklist (builder must confirm)**
  1. No raw colours or magic numbers outside `FieldLNF::theme` and `Layout`.
  2. No layout calls outside `resized()`; no param writes in `paint()`/`resized()`.
  3. Hover states use local component APIs; no `Desktop` hit tests.
  4. Rotary/linear mapping uses control params / `valueToProportionOfLength`.
  5. Attachments exist exactly once per control; listener usage justified.
  6. Build warnings = 0; files include what they use.
  7. Green/Ocean visuals switch via LNF theme only.
  8. New parameters added to APVTS, attachments, preset I/O, and UI in the same change.
  9. `resized()` deterministic (no temporaries that depend on repaint order).
  10. `paint()` pure (no side effects).

---

## 9) “prompt contract” for codegen tasks

When asking the builder (or LLM) for code, start your prompt with this contract:

> Follow the FIELD RULEBOOK v1 strictly.
>
> - Use `FieldLNF::theme` for all colours; use `Layout::dp` and `Layout` constants for sizes.
> - All sizing in `resized()`; no parameter writes in `paint()`/`resized()`.
> - Hover via `isMouseOverOrDragging()`; no global `Desktop` queries.
> - Rotary arcs use provided rotary params; linear mapping uses `valueToProportionOfLength`.
> - Attach parameters using APVTS attachments; one attachment per control.
> - If a draw helper needs hover/active, add boolean arguments (don’t infer).
> - Output complete, compilable `.h/.cpp` snippets with includes and no external magic.

Also include in the prompt:

- What you’re adding/changing (component name & purpose).
- Which parameters it binds to (names).
- Where it lives (parent container & approximate grid cell).
- Acceptance tests (what should visibly happen).

---

## 10) acceptance checklist per component

- Visuals match theme (no hexes), hover/active correct.
- Resizes cleanly at 0.6×–2.0× scale; no overlaps/clipping at breakpoints.
- Keyboard/mouse interaction works; cursor correct.
- Parameter round-trip verified (UI → APVTS → UI).
- No warnings; includes present.

---

## 11) tiny examples (patterns to copy)

**Skew-safe micro slider:**

```cpp
void FieldLNF::drawLinearSlider(..., juce::Slider& s) {
  const float t = juce::jlimit(0.f, 1.f, (float) s.valueToProportionOfLength(s.getValue()));
  // use t for progress + thumbX
}
```

**Rotary using provided angles:**

```cpp
auto [start, end, stopAtEnd] = s.getRotaryParameters();
const float angle = start + proportion * (end - start);
```

**Hover in helper draw:**

```cpp
void drawKnob(Graphics& g, const Rectangle<float>& r, bool isHovered, bool isActive) {
  auto bounds = isHovered || isActive ? r.expanded(Layout::dp(2, scale)) : r;
  // ...
}
```

**Parameter write (gesture-safe):**

```cpp
if (auto* p = apvts.getParameter("width")) {
  p->beginChangeGesture(); p->setValueNotifyingHost(normalised); p->endChangeGesture();
}
```

---

## 12) Group 2 panel system

- **Panel Architecture**
  - Group 2 panel (`bottomAltPanel`) is a sliding overlay that covers the four control rows (inside `leftContentContainer`) when activated.
  - Panel uses rounded corners (6px radius) for modern appearance via `g.fillRoundedRectangle()`.
  - Bounds are aligned to the exact 4-row rectangle of Group 1 (container-local); top aligns to y=0 of the panel’s local space.
  - Panel slides in/out with smooth animation using `bottomAltSlide01` progress value; shown only when slide progress > 0.

- **Group Separation**
  - **Group 1**: Main flat grid (4×16) of controls; always visible below panes.
  - **Group 2**: Delay + Reverb flat grids (8 columns each) presented in the sliding panel; shares the same rows rectangle as Group 1.
  - Motion Engine lives only in Group 1’s grid; it is not duplicated in Group 2.

- **Grid Fit (Group 2)**
  - Cell width is derived from available width: `cellW = min(cellWTarget, availableWidth / 16)` so Delay (8) + Reverb (8) columns fit the panel without horizontal scroll.
  - Delay and Reverb grids use zero column/row gaps; metrics mirror Group 1: `knobPx = Layout::knobPx(L)`, `valuePx`, `labelGap` via `Layout::dp`.

- **Layout Rules**
  - Group 2 panel bounds: `bottomAltPanel.getLocalBounds()` (no extra padding). Delay group at `(x=0,y=0)`, Reverb group immediately to the right.
  - Panel overlays the same four-row height as Group 1; no top gap between panel and first control row.
  - All positioning uses `Layout::dp()` for scale awareness; rows are uniform height: `rowH = knobPx + labelGap + valuePx`.

- **Z-Order Management**
  - Panel is a child of `leftContentContainer` (above the row controls); meters live in `metersContainer` (sibling), preventing overlap.
  - Panel intercepts mouse clicks when active: `setInterceptsMouseClicks(true, false)`; hidden otherwise.

- **Animation & State**
  - Panel visibility controlled by `bottomAltTargetOn` boolean; bottom toggle sets the target.
  - Smooth slide animation with cosine easing: `0.5f - 0.5f * cos(π * t0)`; animation rate ≈ 0.28 for brisk smoothness.
  - Panel appears only when `effSlide > 0.001f` to prevent flicker; starts hidden on load (`bottomAltTargetOn=false`).

---

## 13) Control rows (uniform metrics)

- Four uniform rows beneath panes, with row height:
  - `rowH = knobPx(L) + labelGap + valuePx` (all via `Layout::dp(scaleFactor)`).
- Zero column and row gaps inside all control grids (Group 1 and Group 2), consistent with UI rules.
- Group 1 uses a flat 4×16 grid; Group 2 uses two 8‑column grids (Delay, Reverb) that fit the panel width.

---

## 14) Responsiveness & min sizes

- Minimum width uses a conservative floor (≥ `Layout::BP_WIDE`) combined with calculated content minimum.
- Initial size prefers base sizes (current defaults: baseWidth/baseHeight set in `PluginEditor.h`).
- Width shrinking reduces `scaleFactor` (via min of width/height ratios) and compresses grid cell width to avoid clipping.

---

If you want, we can turn this into a short lint checklist comment to paste at the top of each new file.

---

## 13) Motion Dual‑Panner rules (P1/P2/Link)

- **Default & visuals**
  - Panner Combo defaults to **P1**. Use LNF properties `forceSelectedText` and `defaultTextWhenEmpty` so the control renders "P1" without a chevron.
  - Motion Panel dots (P1/P2/Link) must switch `motion.panner_select` via `setValueNotifyingHost` with proper gesture wrapping when user‑initiated.

- **Independence & defaults**
  - P1 and P2 are truly independent parameter sets (no hidden copy/seed). Both initialize from the same factory defaults.
  - Link mode writes to P1 and mirrors to P2 (policy A). UI binds to P1 while linked.

- **Attachments (critical)**
  - Maintain separate motion‑only attachment buckets: `motionSliderAttachments`, `motionButtonAttachments`, `motionComboAttachments`.
  - On panner change, **clear and rebuild** only these buckets. Do not erase from global vectors by count.
  - Rebind on the message thread using an `AsyncUpdater` (`motionBinding`) to avoid races.

- **UI freshness**
  - After rebind, call `refreshMotionControlValues()` to push current parameter values into labels/knobs immediately.
  - Pull fresh `motion::VisualState` and update the pane visuals in the same step; repaint.

- **Events**
  - `parameterChanged(motion.panner_select)` triggers the async rebind; ComboBox `onChange` redundantly triggers to handle host timing quirks.
  - Motion Panel `mouseDown` hit‑tests P1/P2/Link dots and sets the selection parameter.

- **Do not**
  - Do not keep stale attachments around; do not rely on “erase last N attachments.”
  - Do not let the Panner ComboBox show the default chevron; enforce the default text behavior via LNF.
