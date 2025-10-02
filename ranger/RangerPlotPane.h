#pragma once

#include <JuceHeader.h>

class RangerPlotPane : public juce::Component
{
public:
    RangerPlotPane();
    ~RangerPlotPane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Plot data
    void setFilterData(const juce::Array<float>& taps);
    void setFrequencyResponse(const juce::Array<float>& magnitude, const juce::Array<float>& phase);

private:
    // Plot controls
    juce::Label plotTitleLabel;
    juce::ComboBox plotTypeCombo;
    juce::Label plotTypeLabel;
    
    juce::TextButton generatePlotButton;
    juce::TextButton exportPlotButton;
    
    // Audition controls
    juce::Label auditionLabel;
    juce::ToggleButton impulseViewButton;
    juce::ToggleButton stepViewButton;
    juce::ToggleButton magnitudeViewButton;
    juce::ComboBox normalizationCombo;
    juce::TextButton generateAuditionButton;
    juce::TextButton exportAuditionButton;
    
    // Plot area
    juce::Component plotArea;
    
    // Plot data
    juce::Array<float> filterTaps;
    juce::Array<float> magnitudeResponse;
    juce::Array<float> phaseResponse;
    juce::Array<float> impulseResponse;
    
    // Plot parameters
    float sampleRate = 44100.0f;
    int plotType = 0; // 0=freq, 1=impulse, 2=phase, 3=group delay
    
    // Methods
    void generatePlot();
    void exportPlot();
    void drawPlot(juce::Graphics& g);
    void drawFrequencyResponse(juce::Graphics& g, const juce::Rectangle<int>& bounds);
    void drawImpulseResponse(juce::Graphics& g, const juce::Rectangle<int>& bounds);
    void drawPhaseResponse(juce::Graphics& g, const juce::Rectangle<int>& bounds);
    void drawGroupDelay(juce::Graphics& g, const juce::Rectangle<int>& bounds);
    
    // Audition methods
    void generateAudition();
    void exportAudition();
    void setupAuditionControls();
    
    // Utility functions
    juce::Array<float> calculateFrequencyResponse(const juce::Array<float>& taps);
    juce::Array<float> calculatePhaseResponse(const juce::Array<float>& taps);
    juce::Array<float> calculateGroupDelay(const juce::Array<float>& taps);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerPlotPane)
};