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
 *
 * ## Metallic Template System Integration
 *
 * KnobCellWithAux fully supports the Field metallic template system, providing
 * sophisticated brushed-metal backgrounds with multiple variants:
 *
 * ### Supported Metallic Variants
 * - **Grey Metallic** (`metallic` property): Neutral steel for XY controls
 * - **Reverb Metallic** (`reverbMetallic` property): Burnt orange for reverb
 * - **Delay Metallic** (`delayMetallic` property): Light yellowish-green for delay
 * - **Band Metallic** (`bandMetallic` property): Metallic blue for band controls
 * - **Motion Metallic** (`motionGreenBorder` property): Motion panel colors
 *
 * ### Metallic Visual Effects
 * - **Brushed-metal gradient**: Realistic metal surface appearance
 * - **Horizontal brushing lines**: Subtle texture for authenticity
 * - **Fine grain noise overlay**: Low-alpha noise for realism
 * - **Diagonal micro-scratches**: Random scratches for worn metal look
 * - **Vignette effects**: Edge darkening for depth perception
 *
 * ### Usage Example
 * ```cpp
 * // Create KnobCellWithAux with metallic styling
 * auto cell = std::make_unique<KnobCellWithAux>(mainKnob, mainLabel, auxComponents);
 * 
 * // Apply grey metallic background (for XY controls)
 * cell->getProperties().set("metallic", true);
 * 
 * // Apply reverb metallic background
 * cell->getProperties().set("reverbMetallic", true);
 * 
 * // Apply delay metallic background  
 * cell->getProperties().set("delayMetallic", true);
 * ```
 *
 * ### Layout System
 * - **Split Layout**: 2/3 space for main knob, 1/3 for auxiliary components
 * - **Weighted Auxiliary Components**: Supports relative weights for vertical sizing
 * - **Flexible Component Hosting**: Can host any juce::Component as auxiliary elements
 * - **Consistent Styling**: Matches visual theme of other cells
 *
 * ### Integration with Field Architecture
 * - Uses same metallic template logic as KnobCell for consistency
 * - Supports all Field theme variants through properties system
 * - Maintains visual consistency across all cell types
 * - No hardcoded backgrounds - fully template-driven
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

    /// Get auxiliary components for metallic styling
    const std::vector<juce::Component*>& getAuxComponents() const { return auxComponents; }

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
