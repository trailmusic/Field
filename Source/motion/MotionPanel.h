#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "MotionIDs.h"
#include "MotionVisual.h"
#include "../ui/Design.h"
namespace motion {

// Legacy visual state structure for backward compatibility
struct LegacyVisualState {
    int pathType = 0;
    float rate = 0.5f;
    float depth = 0.5f;
    float spread = 1.0f;
    float elevationBias = 0.0f;
    float frontBias = 0.0f;
    float swing = 0.2f;
    float inertia = 120.0f;
    bool anchor = false;
    int quantizeDiv = 0;
    int mode = 0;
    bool retrig = false;
    float holdMs = 0.0f;
    float motionSend = 0.0f;
};

class MotionPanel : public juce::Component, private juce::Timer {
public:
    MotionPanel(juce::AudioProcessorValueTreeState& s, juce::UndoManager* um = nullptr);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    
    // Update visual state from motion engine (passive view)
    void setVisualState(const VisualState& state);
    void visibilityChanged() override;
    
private:
    void timerCallback() override { repaint(); }
    
    // Visual rendering methods
    void drawOrbBackground(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawPathPreview(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawPannerDots(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawElevationRings(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawBassFloorRing(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawAnchorCircle(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawStatusIndicators(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawQuantizeGrid(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawInertiaTrail(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawSwingGrid(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawOcclusionEffect(juce::Graphics& g, const juce::Rectangle<float>& orb);
    void drawEnableIndicator(juce::Graphics& g, const juce::Rectangle<float>& orb);
    
    // Helper methods
    juce::Point<float> polarToCartesian(float azimuth, float radius, const juce::Rectangle<float>& orb);
    juce::Colour getElevationColor(float elevation);
    juce::Colour getPathColor(int pathType);
    
    juce::Rectangle<int> orbBounds;
    juce::AudioProcessorValueTreeState& state;
    VisualState visualState;
    LegacyVisualState legacyState; // For backward compatibility with existing draw methods
    
    // Animation state
    float animationTime = 0.0f;
    juce::Array<juce::Point<float>> pathPoints;
    int pathPointIndex = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MotionPanel)
};
}