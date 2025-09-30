#pragma once

#include <JuceHeader.h>

class ControlContainer : public juce::Component, public juce::Timer
{
public:
    ~ControlContainer() override { stopTimer(); }
    
    ControlContainer();
    
    void setTitle(const juce::String& title);
    void setShowBorder(bool show) { showBorder = show; }
    void setBorderColour(juce::Colour colour) { borderColour = colour; useCustomBorderColour = true; }
    void setBackgroundColour(juce::Colour colour) { backgroundColour = colour; useCustomBackgroundColour = true; }
    
    void paint(juce::Graphics& g) override;
    void mouseEnter(const juce::MouseEvent&) override { hovered = true; hoverActive = true; stopTimer(); repaint(); }
    void mouseExit(const juce::MouseEvent&) override { hovered = false; startTimer(hoverOffDelayMs); }
    void timerCallback() override { hoverActive = false; stopTimer(); repaint(); }
    
private:
    juce::String containerTitle;
    bool hovered = false;
    bool hoverActive = false;
    bool showBorder = true;
    bool useCustomBorderColour = false;
    juce::Colour borderColour = juce::Colours::transparentBlack;
    bool useCustomBackgroundColour = false;
    juce::Colour backgroundColour = juce::Colours::transparentBlack;
    const int hoverOffDelayMs = 160;
};
