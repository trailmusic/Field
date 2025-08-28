#pragma once
#include <JuceHeader.h>
#include <vector>

// Forward-declare your LookAndFeel so we can theme nicely without including extra headers
class FieldLNF;

/**
 * KnobCell
 *
 * A lightweight container that:
 *  - Hosts a rotary knob (juce::Slider) you already own
 *  - Optionally hosts its value Label (you already own it) — you’re positioning it via placeLabelBelow()
 *  - Optionally hosts a mini control (Linear slider) under the knob (e.g., frequency minis)
 *
 * Notes:
 *  - This class does **not** own the controls; it only reparents them when laid out.
 *  - It cooperates with your existing value-label placement (placeLabelBelow), so it
 *    doesn't move the label; it just ensures the label can live inside the same parent.
 *  - Call setMetrics(...) before layout to update target sizes.
 *  - Call setMini(...) if the cell should include a mini control.
 */
class KnobCell : public juce::Component
{
public:
    KnobCell(juce::Slider& knobToHost,
             juce::Label& valueLabelToHost,
             const juce::String& caption = {});
    ~KnobCell() override = default;

    /// Knob diameter (px), value-label band height (px), gap between sections (px), optional mini height (px)
    void setMetrics (int knobPx, int valuePx, int gapPx, int miniPx = 0);

    /// Attach/detach an optional mini slider (e.g. frequency). Pass nullptr to detach.
    /// Backwards-compatible helper; internally uses the aux area.
    void setMini (juce::Slider* miniSlider, int miniHeightPx);

    /// Set arbitrary auxiliary components to render in the mini area under the knob.
    /// Components are non-owned; they will be reparented to this cell when laid out.
    void setAuxComponents (const std::vector<juce::Component*>& components, int miniHeightPx);

 

    /// Draw outer border + hover halo (default: true).
    void setShowBorder (bool shouldDrawBorder) { showBorder = shouldDrawBorder; repaint(); }

    /// External hover flag to keep halo visible while interacting elsewhere.
    void setHoverActive (bool on) { hoverActive = on; repaint(); }

    /// Place the mini/aux area to the right of the knob instead of below (default: false = below)
    void setMiniPlacementRight (bool right) { miniOnRight = right; resized(); repaint(); }

    /// Set thickness (height in px) of the right-side mini bar when miniOnRight = true
    void setMiniThicknessPx (int px) { miniThicknessPx = juce::jmax (6, px); }

    // Managed value-label layout (optional)
    enum class ValueLabelMode { External, Managed };
    void setValueLabelMode (ValueLabelMode m) { valueLabelMode = m; resized(); }
    void setValueLabelGap  (int px)           { valueLabelGap  = juce::jmax (0, px); resized(); }

    // juce::Component
    void paint   (juce::Graphics& g) override;
    void resized() override;

private:
    // Adopt children if they're not already our children
    void ensureChildrenAreHere();

    // Simple helpers
    juce::Colour getPanelColour() const;
    juce::Colour getTextColour()  const;
    juce::Colour getAccentColour()const;
    juce::Colour getShadowDark()  const;
    juce::Colour getShadowLight() const;
    juce::Colour getRimColour()   const;

    juce::Slider& knob;
    juce::Label&  valueLabel;     // not positioned here; you do it via placeLabelBelow(...)
    juce::Component* mini { nullptr }; // legacy single-mini path
    std::vector<juce::Component*> auxComponents;     // arbitrary aux components (non-owned)

    // Layout targets (pixels)
    int K = 84;   // knob diameter
    int V = 14;   // value label band height (space reservation)
    int G = 4;    // gap between elements
    int M = 0;    // mini height (0 = none)

    bool showBorder  = true;
    bool hoverActive = false;
    bool miniOnRight = false;
    int  miniThicknessPx { 12 };
    ValueLabelMode valueLabelMode { ValueLabelMode::External };
    int valueLabelGap { 4 };
};


