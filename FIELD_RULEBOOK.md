# FIELD GUI + CODE RULEBOOK (v1)

## 0) non-negotiables (musts)

- **Single sources of truth**
  - **Theme/colors**: only from `FieldLNF::theme` (e.g., `theme.panel`, `theme.text`, `theme.accent`, `theme.hl`, `theme.sh`, `theme.shadowDark`, `theme.shadowLight`). No raw hex colours in components.
  - **Layout metrics**: only from `Layout::*` and `Layout::dp(px, scaleFactor)`. No magic numbers in `resized()` or `paint()`.
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
  - Define rhythm in `Layout`: `PAD`, `GAP`, `KNOB`, `KNOB_L`, `MICRO_W/H`, breakpoints (`BP_WIDE`, etc.).
- **Where layout happens**
  - The editor owns grid/flow; containers arrange their children only (no sibling knowledge).
  - Use `juce::Grid` for rows/columns; don’t manually sprinkle `setBounds` everywhere.
- **Breakpoints**
  - Use `getWidth()` vs `Layout::BP_WIDE` to switch grid templates (e.g., collapse “split pan” when narrow).

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

If you want, we can turn this into a short lint checklist comment to paste at the top of each new file.
