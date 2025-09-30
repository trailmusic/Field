#pragma once
#include <JuceHeader.h>

//==============================================================================
// ToggleSwitch (kept smoothing for handle; hover timer only for subtle fade)
//==============================================================================
class ToggleSwitch : public juce::Component, public juce::Timer
{
public:
    ~ToggleSwitch() override { stopTimer(); }
    
    ToggleSwitch();
    
    void setToggleState (bool shouldBeOn, juce::NotificationType notification = juce::dontSendNotification);
    bool getToggleState() const { return isOn; }
    
    void setLabels (const juce::String& offLabel, const juce::String& onLabel);
    
    std::function<void(bool)> onToggleChange;
    
protected:
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent&) override { hovered = true;  hoverActive = true; stopTimer(); repaint(); }
    void mouseExit  (const juce::MouseEvent&) override { hovered = false; startTimer (hoverOffDelayMs); }
    void timerCallback() override { hoverActive = false; stopTimer(); repaint(); }
    
private:
    bool isOn = false;
    bool isMouseDown = false;
    bool hovered = false;
    bool hoverActive = false;
    const int hoverOffDelayMs = 160;
    juce::String offText, onText;
    juce::SmoothedValue<float> sliderValue { 0.0f };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleSwitch)
};
