# Field Audio Plugin - Design System Rules (2025)

## 🎯 Design Philosophy

**From**: Ad-hoc, hand-computed coordinates + one-off sizes sprinkled through constructors and paint  
**To**: Tokenized, responsive, state-driven layout system

### Core Principles

1. **Tokens Everywhere**: All sizes/spacing/radii/line widths come from a single design system
2. **One Place Does Layout**: Components positioned only in `resized()`, never in `paint()` or style helpers
3. **Responsive by Design**: Scale factor → density-independent pixels; breakpoint logic decides stacking
4. **State Drives Structure**: Split/stereo flips `setVisible()` and grid reflows; no coordinate recomputation
5. **Paint is Cosmetic**: Drawing reads bounds + tokens to decorate; never mutates geometry
6. **Micro-controls Attach to Parents**: Tiny sliders/labels placed relative to parent bounds via helpers

---

## 📐 Layout & UX Patterns

### ✅ Current Implementation
- Using `juce::Grid` for main layout sections
- `Layout::dp()` helper for scale-aware sizing
- Breakpoint logic for responsive behavior
- State-driven visibility management

### 🔄 Required Upgrades
- [ ] **Remove all `setSize()` from `styleSlider()` / `styleMainSlider()`**
- [ ] **Replace remaining manual math with `juce::Grid`**
- [ ] **Implement FlexBox for complex nested layouts**
- [ ] **Add proper container hierarchy with individual `resized()` methods**

### 📋 Rules
```cpp
// ✅ DO: Use tokens for all measurements
const int knobSize = Layout::dp(Layout::KNOB, scaleFactor);
const int gap = Layout::dp(Layout::GAP, scaleFactor);

// ❌ DON'T: Hard-code sizes
const int knobSize = 70; // Magic number
```

---

## ⚡ Performance & Rendering

### ✅ Current Implementation
- Basic scaling system in place
- Grid-based layout reduces manual calculations

### 🔄 Required Upgrades
- [ ] **Cache woodgrain texture to `juce::Image`**
- [ ] **Implement partial repaints for XYPad**
- [ ] **Preallocate Path space for spectral responses**
- [ ] **Decimate x-samples (0.5-1.0px steps)**
- [ ] **Convert vector allocations to member buffers**

### 📋 Rules
```cpp
// ✅ DO: Cache expensive textures
class CachedTexture {
    juce::Image woodgrainTexture;
    void updateTexture() {
        // Rebuild only when theme/size changes
    }
};

// ❌ DON'T: Rebuild textures every paint
void paint(juce::Graphics& g) {
    // Expensive woodgrain generation every frame
}
```

---

## 🔒 Real-Time Safety

### ⚠️ Critical Issues
- [ ] **Audio thread → UI thread data race in waveform sample pushing**
- [ ] **Direct component memory access from audio thread**

### 🔄 Required Fixes
- [ ] **Implement lock-free FIFO (`juce::AbstractFifo`)**
- [ ] **Use atomic write indices**
- [ ] **Move UI updates to message thread via timer**

### 📋 Rules
```cpp
// ✅ DO: Use lock-free communication
juce::AbstractFifo audioToUI;
std::atomic<int> writeIndex{0};

// Audio thread
void audioCallback() {
    audioToUI.write(samples, numSamples);
}

// UI thread (timer)
void timerCallback() {
    audioToUI.read(samples, numSamples);
    pad.pushWaveformSamples(samples);
}

// ❌ DON'T: Direct audio thread → UI access
void audioCallback() {
    pad.pushWaveformSample(sample); // Data race!
}
```

---

## 🎛️ State & APVTS Hygiene

### ⚠️ Current Issues
- [ ] **Duplicate parameter attachments (`bass_db`, `scoop`, `depth`)**
- [ ] **Timer writes to APVTS every tick (automation noise)**
- [ ] **No change detection for parameter updates**

### 🔄 Required Fixes
- [ ] **Deduplicate all parameter attachments**
- [ ] **Implement change detection with deadband**
- [ ] **Gate writes to actual value changes**
- [ ] **Add gesture-aware parameter updates**

### 📋 Rules
```cpp
// ✅ DO: Check for actual changes
void updateParameter(float newValue) {
    if (std::abs(newValue - currentValue) > deadband) {
        apvts.setParameter(parameterId, newValue, nullptr);
        currentValue = newValue;
    }
}

// ❌ DON'T: Write every tick
void timerCallback() {
    apvts.setParameter(parameterId, value, nullptr); // Always writes
}
```

---

## ♿ Accessibility & Keyboard

### 🔄 Required Implementation
- [ ] **Add `getAccessibilityHandler()` for all interactive controls**
- [ ] **Implement keyboard navigation (Tab order)**
- [ ] **Add keyboard increments for knobs (arrows, PageUp/Down)**
- [ ] **Provide screen reader labels and roles**

### 📋 Rules
```cpp
// ✅ DO: Implement accessibility
class AccessibleKnob : public juce::Slider {
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override {
        return std::make_unique<juce::SliderAccessibilityHandler>(*this);
    }
};

// ✅ DO: Add keyboard support
void keyPressed(const juce::KeyPress& key) override {
    if (key == juce::KeyPress::upKey) {
        setValue(getValue() + getInterval());
    }
}
```

---

## 🎨 Hi-DPI, Theming & Contrast

### 🔄 Required Implementation
- [ ] **Mark heavy components `setOpaque(true)`**
- [ ] **Verify WCAG contrast ratios**
- [ ] **Support theme swap without reallocation**
- [ ] **Cache colors in LookAndFeel**

### 🌈 Gradients (Buttons)
- **Angle**: Top-to-bottom (vertical). Origin = top edge, destination = bottom edge.
- **Stops**:
  - Top: `base.brighter(0.15)`
  - Bottom: `base.darker(0.15)`
- **Base colors**:
  - Standard header buttons: `#3A3D45`
  - Active green mode buttons: `#4CAF50`
- **Implementation**:
```cpp
// All header buttons use the same gradient spec
auto bounds = getLocalBounds().toFloat().reduced(2.0f);
juce::Colour top = base.brighter(0.15f);
juce::Colour bot = base.darker(0.15f);
juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
g.setGradientFill(grad);
g.fillRoundedRectangle(bounds, 4.0f);
```

### 📋 Rules
```cpp
// ✅ DO: Support theme changes
class ThemeManager {
    void updateTheme(const Theme& newTheme) {
        cachedColors.clear();
        // Invalidate only dependent caches
    }
};

// ✅ DO: Use proper contrast
const juce::Colour textColor = theme.text.contrasting(theme.background);
```

---

## 🏗️ Modern C++ Hygiene (C++20/23)

### 🔄 Required Implementation
- [ ] **Use `enum class` for parameter IDs**
- [ ] **Replace string IDs with constants**
- [ ] **Add `[[nodiscard]]` to important getters**
- [ ] **Mark non-virtual overrides `final`**
- [ ] **Use `= default` destructors**

### 📋 Rules
```cpp
// ✅ DO: Use modern C++
enum class ParameterID {
    Gain,
    Drive,
    Mix
};

[[nodiscard]] float getValue() const noexcept { return value; }

class ModernComponent final : public juce::Component {
    ~ModernComponent() = default;
};
```

---

## 🧪 Testability & Presets

### 🔄 Required Implementation
- [ ] **Snapshot tests for layout regression detection**
- [ ] **Round-trip preset I/O validation**
- [ ] **CI bitmap comparison with tolerance**
- [ ] **A/B state validation**

### 📋 Rules
```cpp
// ✅ DO: Test preset round-trips
TEST_CASE("Preset round-trip") {
    auto preset = savePreset();
    loadPreset(preset);
    REQUIRE(currentState == originalState);
}

// ✅ DO: Snapshot testing
TEST_CASE("Layout snapshots") {
    renderToBitmap();
    compareWithBaseline(tolerance);
}
```

---

## 🚀 Quick Wins Implementation Order

### Phase 1: Critical Safety & Performance
1. [ ] **Fix audio thread data race with FIFO**
2. [ ] **Remove `setSize()` from style helpers**
3. [ ] **Cache woodgrain texture**
4. [ ] **Deduplicate parameter attachments**

### Phase 2: Layout Modernization
1. [ ] **Complete Grid migration for all sections**
2. [ ] **Implement FlexBox for complex layouts**
3. [ ] **Add proper container hierarchy**

### Phase 3: Accessibility & Polish
1. [ ] **Add accessibility handlers**
2. [ ] **Implement keyboard navigation**
3. [ ] **Add WCAG contrast validation**

### Phase 4: Testing & Validation
1. [ ] **Add snapshot tests**
2. [ ] **Implement preset round-trip validation**
3. [ ] **Add CI regression detection**

---

## 📊 Success Metrics

### Performance Targets
- **Frame Rate**: 60fps at 4K resolution
- **Audio Latency**: <1ms additional latency
- **Memory Usage**: <50MB total plugin memory
- **CPU Usage**: <5% on modern systems

### Code Quality Targets
- **Zero magic numbers** in layout code
- **100% Grid/FlexBox usage** for positioning
- **Zero data races** between audio and UI threads
- **100% accessibility coverage** for interactive elements

### User Experience Targets
- **Responsive scaling** from 50% to 200%
- **Keyboard navigation** for all controls
- **Screen reader compatibility**
- **High contrast mode support**

---

## 🔄 Continuous Improvement

### Weekly Reviews
- [ ] **Performance profiling** results
- [ ] **Accessibility audit** findings
- [ ] **Code quality metrics** (clang-tidy, sanitizers)
- [ ] **User feedback** integration

### Monthly Goals
- [ ] **Complete one major phase** per month
- [ ] **Add one new accessibility feature**
- [ ] **Improve one performance metric**
- [ ] **Update design tokens** based on usage

---

*This document serves as the authoritative guide for all UI/UX decisions in the Field Audio Plugin. All changes must align with these principles to maintain the modern, responsive, and accessible design system.*
