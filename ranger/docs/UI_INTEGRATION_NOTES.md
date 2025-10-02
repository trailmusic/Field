# Field Ranger UI Integration Notes (Look & Feel + Rendering)

**Targets:** `FieldLNF`, `FieldRendering`, EQ views, ComboBox (chevron/abbr), metallic buttons, theme propagation
**Status:** Ready to implement (no external deps)
**Purpose:** Seamless integration of Field Ranger with Field's existing Look & Feel system

---

## 1) What You Already Have ✅

* A consolidated **`FieldLNF`** (`LookAndFeel_V4 + ChangeBroadcaster`) with a full `FieldTheme` palette.
* Centralized `FieldRendering::*` hooks for **buttons, sliders, combo boxes, labels**, and **tab pills**.
* A dedicated set of **EQ ColourIds** (label/border/zero/grid/handles/analyzer).
* A **metallic** visual subsystem (via `MetallicRenderer`) integrated into buttons & combos.
* Early support for **abbreviation mode** + **chevron-only** combos and **two-line label wrapping**.

This is a strong base. With a few small extensions you'll get predictable theme switching, perfect ComboBox behavior, and cheaper paints.

---

## 2) Gaps / Risks to Address 🛠️

1. **Theme propagation:**
   You broadcast theme change by calling `FieldLNF::broadcastThemeChanged(root)`. Great—but components that **cache colors** or keep cached `Image`s should also listen for `FieldLNF` changes.

2. **EQ Colors not guaranteed:**
   Ensure EQ components **always** read colors via `findColour(FieldLNF::<eqColourId>, true)` at paint time (no stored copies).

3. **Abbreviation mapping duplicated in rendering:**
   You currently inline "General→G, Vocal→V…" logic in `drawComboBox`. Move that to a **shared mapper** so editor code, lists, and rendering all agree.

4. **ComboBox text bounds + chevron overlap:**
   In abbreviation mode you set label width to `70%`. On narrow UIs the chevron can still touch text. Reserve a **fixed px inset**.

5. **DropShadow cost:**
   Rendering DropShadows every paint (esp. multiple layers) can be expensive. Cache to `juce::Image` (per DPI) or use a simple **lazy raster**.

6. **Accessibility hints:**
   Expose accessible names/roles to VoiceOver/Narrator for custom cells (esp. metallic buttons with icons only).

---

## 3) Drop-in Patches (Copy/Paste)

### 3.1 Theme Listener Helper (subscribe to LNF changes)

**File:** `FieldLookAndFeel.h` (add a small RAII helper)

```cpp
// Listens to FieldLNF::ChangeBroadcaster and calls a callback
struct LNFThemeSubscription : juce::ChangeListener
{
    FieldLNF* lnf = nullptr;
    std::function<void()> onThemeChanged;

    void attach(juce::LookAndFeel& lf, std::function<void()> cb)
    {
        detach();
        onThemeChanged = std::move(cb);
        if (auto* fld = dynamic_cast<FieldLNF*>(&lf))
        {
            lnf = fld;
            lnf->addChangeListener(this);
        }
    }
    void detach()
    {
        if (lnf) { lnf->removeChangeListener(this); lnf = nullptr; }
    }
    ~LNFThemeSubscription() { detach(); }

    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        if (onThemeChanged) onThemeChanged();
    }
};
```

**Usage in a component:**

```cpp
class ReverbToneEQ : public juce::Component
{
    LNFThemeSubscription themeListen;
    void lookAndFeelChanged() override
    {
        themeListen.attach(getLookAndFeel(), [this]{ repaint(); });
        repaint();
    }
};
```

> Now **any** palette swap through `FieldLNF::applyTheme()` triggers repaints without manual wiring.

---

### 3.2 EQ Colour Access (no stale colors)

**In each EQ `paint()`** (tone & decay), **replace any cached colour** with:

```cpp
const auto border   = findColour(FieldLNF::eqBorderColourId, true);
const auto zeroLine = findColour(FieldLNF::eqZeroLineColourId, true);
const auto grid     = findColour(FieldLNF::eqGridLineColourId, true);
const auto handle   = findColour(FieldLNF::eqBandHandleColourId, true);
const auto handleOn = findColour(FieldLNF::eqBandHandleActiveId, true);
const auto trace    = findColour(FieldLNF::eqAnalyzerTraceColourId, true);
```

> This guarantees 100% theme compliance even after LNF swaps.

---

### 3.3 Central Abbreviation Mapper (single source of truth)

**File:** `FieldRendering.h` (or a new header `FieldTextMaps.h`)

```cpp
inline juce::String mapAbbrev(const juce::String& full)
{
    static const std::map<juce::String, juce::String, juce::StringComparator> k {
        { "General", "G" }, { "Vocal", "V" }, { "DrumBus", "DB" },
        { "Guitar", "GT" }, { "Keys", "K" }, { "Dry", "D" },
        { "ER", "ER" }, { "Tail", "TL" }, { "Wet Sum", "WS" }
    };
    auto it = k.find(full);
    return it != k.end() ? it->second : full;
}
```

**In `FieldRendering::drawComboBox()`** replace the if/else ladder:

```cpp
if (abbreviationMode)
{
    if (box.getSelectedId() > 0)
        displayText = mapAbbrev(box.getItemText(box.getSelectedItemIndex()));
    else
        displayText = box.getText();
}
```

> Anywhere else in UI (headers, mini labels) can now share the same mapping.

---

### 3.4 ComboBox Text Bounds (reserve chevron lane)

**In `positionComboBoxText()`** replace the `70%` trick with fixed padding:

```cpp
else if (abbreviationMode)
{
    auto bounds = box.getLocalBounds();
    const int chevronLane = juce::jmax(18, bounds.getHeight() / 3); // ~icon lane
    bounds.removeFromRight(chevronLane + 6); // keep space for the triangle + pad
    label.setBounds(bounds);
}
```

**In `drawComboBox()` chevron draw**, use the reserved lane:

```cpp
const int chevronLane = juce::jmax(18, height / 3);
juce::Rectangle<float> lane(r.getRight() - chevronLane, r.getY(), chevronLane, r.getHeight());

juce::Path p;
auto cx = lane.getCentreX(), cy = lane.getCentreY();
p.addTriangle(cx - 4, cy - 2, cx + 4, cy - 2, cx, cy + 2);
g.setColour(chevronOnly ? theme.accent : theme.accent.withAlpha(0.9f));
g.fillPath(p);
```

> Prevents overlap in tight layouts and ensures consistent chevron positioning.

---

### 3.5 Cheap Shadows (cache per DPI)

**File:** `FieldRendering.h` add a tiny cache:

```cpp
struct ShadowCache {
    juce::Image img;
    juce::Rectangle<int> lastRect;
    float lastCorner = 0.f;

    void render(juce::Graphics& g, juce::Rectangle<float> r, float corner, juce::Colour col)
    {
        auto ir = r.getSmallestIntegerContainer();
        if (!img.isValid() || img.getBounds() != ir || lastCorner != corner)
        {
            img = juce::Image(juce::Image::ARGB, ir.getWidth(), ir.getHeight(), true);
            juce::Graphics gg(img);
            gg.setColour(col);
            gg.fillRoundedRectangle(r.translated(-ir.getX(), -ir.getY()), corner);
            // simple blur (you can swap for your blur kernel)
            juce::ImageConvolutionKernel blur(3);
            blur.setPixel(0,0,1); blur.setPixel(1,1,4); blur.setPixel(2,2,1); // placeholder
            blur.applyToImage(img, ir, ir, 1);
            lastRect = ir; lastCorner = corner;
        }
        g.drawImageAt(img, ir.getX(), ir.getY());
    }
};
```

**Use in `drawToggleButton` / `drawButtonBackground`** where you previously created multiple `DropShadow`s:

```cpp
static ShadowCache sc;
sc.render(g, r, 8.0f, theme.shadowDark.withAlpha(0.25f));
```

> This keeps the *look* but removes the per-frame DropShadow cost.

---

### 3.6 Accessible Names (icons-only metallics)

Add once, where you set a button's properties:

```cpp
button.setAccessible(true);
button.getAccessibilityHandler()->setName(button.getButtonText().isNotEmpty()
    ? button.getButtonText()
    : button.getProperties().getWithDefault("a11yName", "Button").toString());
button.getAccessibilityHandler()->setRole(juce::AccessibilityRole::button);
```

For icon-only buttons, set:

```cpp
button.getProperties().set("a11yName", "Enable");
```

---

## 4) Integration Checklist ✅

* [ ] Swap EQ components to **always** query `findColour(FieldLNF::eq*ColourId, true)` in `paint()`.
* [ ] Add **`LNFThemeSubscription`** to any component that caches theme-derived assets.
* [ ] Replace ComboBox abbreviation ladder with **`mapAbbrev()`**.
* [ ] Reserve a **chevron lane** in `positionComboBoxText()` and draw into it.
* [ ] Replace heavy `DropShadow` stacks with **`ShadowCache`** for metallic/raised elements.
* [ ] Set **Accessible names** for icon-only controls.
* [ ] Unit-test: switch `ThemeVariant` at runtime → **everything repaints** and colors change.
* [ ] Visual QA at 100%/150%/200% DPI (cached shadows regenerate correctly).

---

## 5) Quick Test Matrix

| Test             | Steps                                       | Expected                                                                     |
| ---------------- | ------------------------------------------- | ---------------------------------------------------------------------------- |
| Theme swap       | Cycle `applyTheme()` among Ocean/Green/Pink | All EQ lines/labels/handles recolor; buttons/combos restyle; no stale colors |
| Combo abbr       | Turn on abbreviation + chevronOnly          | Abbreviated text centered; chevron aligned right; no overlap                 |
| Metallic buttons | Hover, press, toggle                        | Borders/glow update; shadow stable; perf steady (no jank)                    |
| EQ Zero/Grid     | Toggle light/dark                           | Zero line/grid read from ColourIds; contrast remains readable                |
| Accessibility    | Toggle icon-only buttons                    | VO/Narrator reads proper control names/roles                                 |

---

## 6) Tiny Dev Glossary (A-Z)

**Abbreviation Mode** – Render short token ("G", "V", "DB") while the menu still shows full names.
**Chevron-only** – Combo button shows only the down-triangle when no selection or by design.
**ColourIds** – Stable integer keys to theme colors so components don't hardcode `juce::Colours`.
**DropShadow cache** – Pre-rasterized shadow image to avoid CPU-heavy shadow draws every frame.
**EQ ColourIds** – Dedicated ids: label, border, zero line, grid, handle, handle-active, analyzer trace.
**FieldLNF** – Your global Look&Feel + theme broadcaster; sends change messages on palette swap.
**MetallicRenderer** – Your brushed/metal gradients used for special buttons & combos.
**Theme Variant** – Named palettes (Ocean/Green/…) applied across the app at runtime.

---

## 7) Notes for the Reverb Tab (bonus)

* For the **visualization area**, read colors from `theme.meters.*` (or define new ColourIds like `visRayColourId`, `visWaterfallLow/High`, etc.) and **don't** cache them.
* On **Enable** off, dim visuals using the component's `greyedOut/greyoutAlpha` properties you already support in `FieldRendering`.

---

### Theme invariants
- Components must query colors at paint-time via `findColour(FieldLNF::<id>, true)`.
- No color caching in members (only cache geometry/path).
- On theme change, `FieldLNF::applyTheme()` → `sendChangeMessage()` must trigger:
  - `ReverbToneEQ`/`DecayRateEQ` repaint
  - Combo/Buttons re-layout (chevron lane)
  - Visualization area repaint (uses theme.meters)

### ComboBox policy
- Abbreviation visible in closed state, full names in popup.
- Right chevron lane reserved: `max(18px, height/3)`.
- Shared mapping: `FieldRendering::mapAbbrev()`.

### Ranger Export Integration
- **Ranger Export**: When talking about `MinPhaseBank.h` generation
- **Field Integration**: Seamless theming with Field's Look & Feel system
- **Namespace**: Use `trail::field::ranger` for Ranger-specific components
- **Component Classes**: `RangerWindow`, `RangerDesigner`, `RangerPlotPane`
- **Settings**: `RangerPrefs`, `RangerRecentFiles`

---

If you want, I can also package these patches as **diffs against your files** (by path/line) or provide a **minimal test harness** (`Component` with combos/buttons) that toggles abbreviation/chevrons, flips themes, and screenshots states for QA.
