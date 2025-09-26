#pragma once
#include <JuceHeader.h>
#include <vector>

// Forward-declare your LookAndFeel so we can theme nicely without including extra headers
class FieldLNF;

/**
 * KnobCellWithAux
 *
 * A template for double-wide cells that have:
 *  - A main knob on the left
 *  - Auxiliary components on the right (switches, buttons, sliders, etc.)
 *
 * This follows the same pattern as DoubleKnobCell but is more flexible
 * for different types of auxiliary components.
 */
class KnobCellWithAux : public juce::Component
{
public:
    KnobCellWithAux(juce::Slider& mainKnob,
                    juce::Label& mainLabel,
                    const std::vector<juce::Component*>& auxComponents,
                    const std::vector<float>& auxWeights = {});
    ~KnobCellWithAux() override = default;

    /// Knob diameter (px), value-label band height (px), gap between sections (px)
    void setMetrics (int knobPx, int valuePx, int gapPx);

    /// Set the auxiliary area height (px)
    void setAuxHeight (int auxHeightPx);

    /// Set relative weights for auxiliary components (defaults to equal weights)
    void setAuxWeights (const std::vector<float>& weights);

    /// Draw outer border + hover halo (default: true).
    void setShowBorder (bool shouldDrawBorder) { showBorder = shouldDrawBorder; repaint(); }

    /// Show/hide the cell's panel background (true by default).
    void setShowPanel (bool shouldDrawPanel) { showPanel = shouldDrawPanel; repaint(); }

    // juce::Component
    void paint   (juce::Graphics& g) override;
    void resized() override;

private:
    void ensureChildren();
    void layoutAuxComponents (juce::Rectangle<int> auxArea);

    juce::Slider& mainKnob;
    juce::Label&  mainLabel;
    std::vector<juce::Component*> auxComponents;
    std::vector<float> auxWeights;

    // Layout targets (pixels)
    int K = 88;   // knob diameter
    int V = 14;   // value label band height
    int G = 4;    // gap between elements
    int A = 40;   // aux area height

    bool showBorder = true;
    bool showPanel  = true;
};
