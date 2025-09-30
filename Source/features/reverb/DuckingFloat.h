#pragma once
#include <JuceHeader.h>

class DuckingFloat : public juce::Component
{
public:
    DuckingFloat(juce::AudioProcessorValueTreeState& apvts);
    ~DuckingFloat() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    
    // State management
    void setExpanded(bool expanded);
    bool isExpanded() const { return expanded; }
    
    // GR meter updates
    void updateGrMeter(float grDb);
    
    // Visibility control
    void setVisible(bool shouldBeVisible) override;
    
    // State management for greyed out mode
    void setActive(bool active);
    bool isActive() const { return active; }
    void setGreyedOut(bool greyedOut);
    bool isGreyedOut() const { return greyedOut; }

private:
    void setupComponents();
    void updateLayout();
    void paintCollapsed(juce::Graphics& g);
    void paintExpanded(juce::Graphics& g);
    void paintGrMeter(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    // UI Components
    juce::Label grLabel;
    juce::Slider grMeter;
    
    // Mode selection
    juce::ComboBox modeSelector;
    juce::ComboBox detectorSelector;
    
    // Ducking controls
    juce::Slider depthSlider, thresholdSlider, ratioSlider, kneeSlider;
    juce::Slider attackSlider, releaseSlider;
    juce::Slider bandFreqSlider, bandQSlider;
    
    // Labels
    juce::Label depthLabel, thresholdLabel, ratioLabel, kneeLabel;
    juce::Label attackLabel, releaseLabel;
    juce::Label bandFreqLabel, bandQLabel;
    juce::Label modeLabel, detectorLabel;
    
    // APVTS attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> expandAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> detectorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandQAttachment;
    
    // State
    bool expanded = false;
    bool active = true;
    bool greyedOut = false;
    float currentGrDb = 0.0f;
    
    // Constants
    static constexpr float COLLAPSED_HEIGHT = 40.0f;
    static constexpr float EXPANDED_HEIGHT = 200.0f;
    static constexpr float PILL_CORNER_RADIUS = 20.0f;
    static constexpr float GR_METER_WIDTH = 60.0f;
    static constexpr float GR_METER_HEIGHT = 20.0f;
};
