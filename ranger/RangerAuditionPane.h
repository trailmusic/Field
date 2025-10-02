#pragma once

#include <JuceHeader.h>

class RangerAuditionPane : public juce::Component
{
public:
    RangerAuditionPane();
    ~RangerAuditionPane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // View controls
    juce::ToggleButton impulseButton;
    juce::ToggleButton stepButton;
    juce::ToggleButton magnitudeButton;
    
    // A/B comparison
    juce::ToggleButton baselineButton;
    juce::ToggleButton candidateButton;
    
    // Display options
    juce::ToggleButton overlayButton;
    juce::ToggleButton deltaButton;
    
    // Normalization
    juce::ComboBox normComboBox;
    juce::Label normLabel;
    
    // Magnitude options
    juce::ToggleButton dbButton;
    juce::ToggleButton smoothButton;
    juce::ToggleButton logFreqButton;
    
    // Action buttons
    juce::TextButton generateButton;
    juce::TextButton exportButton;
    juce::TextButton copyTapsButton;
    juce::TextButton verifyTPButton;
    
    // Status
    juce::Label statusLabel;
    juce::Label tpStatusLabel;
    
    // Plot area
    juce::Component plotArea;
    
    // Current view data
    enum class ViewType { Impulse, Step, Magnitude };
    ViewType currentView = ViewType::Impulse;
    bool showOverlay = false;
    bool showDelta = false;
    
    void setupControls();
    void generateAudition();
    void updatePlot();
    void drawPlot(juce::Graphics& g);
    
    // Event handlers
    void onViewChanged();
    void onABChanged();
    void onOptionsChanged();
    void onGenerateClicked();
    void onExportClicked();
    void onCopyTapsClicked();
    void onVerifyTPClicked();
    
    // Utility functions
    juce::Colour getBaselineColor() const;
    juce::Colour getCandidateColor() const;
    juce::Colour getDeltaColor() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerAuditionPane)
};