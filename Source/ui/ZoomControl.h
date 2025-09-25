#pragma once
#include <JuceHeader.h>
#include "ZoomState.h"
#include "../Core/IconSystem.h"

// Zoom control with slider, presets, and reset button
class ZoomControl : public juce::Component, private juce::Timer
{
public:
    explicit ZoomControl (ZoomState& zoomState)
    : zoomState (zoomState)
    {
        // Preset buttons with icons
        for (int i = 0; i < 5; ++i)
        {
            presetButtons[i].setButtonText (""); // Use icons instead
            presetButtons[i].setClickingTogglesState (false);
            presetButtons[i].setColour (juce::TextButton::buttonColourId, juce::Colours::black.withAlpha (0.3f));
            presetButtons[i].setColour (juce::TextButton::buttonOnColourId, juce::Colours::white.withAlpha (0.2f));
            presetButtons[i].setColour (juce::TextButton::textColourOnId, juce::Colours::white);
            presetButtons[i].onClick = [this, i] { setPreset (i); };
            addAndMakeVisible (presetButtons[i]);
        }
        
        // Zoom slider
        zoomSlider.setSliderStyle (juce::Slider::LinearVertical);
        zoomSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        zoomSlider.setRange (6.0, 36.0, 0.1);
        zoomSlider.setValue (18.0);
        zoomSlider.onValueChange = [this] { onSliderChanged(); };
        zoomSlider.onDragStart = [this] { onSliderDragStart(); };
        zoomSlider.onDragEnd = [this] { onSliderDragEnd(); };
        addAndMakeVisible (zoomSlider);
        
        // Reset button with icon
        resetButton.setButtonText (""); // Use icon instead
        resetButton.setColour (juce::TextButton::buttonColourId, juce::Colours::black.withAlpha (0.3f));
        resetButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::white.withAlpha (0.2f));
        resetButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        resetButton.onClick = [this] { handleReset(); };
        addAndMakeVisible (resetButton);
        
        // Start animation timer
        startTimerHz (60);
    }
    
    ~ZoomControl() override
    {
        stopTimer();
    }
    
    // Callbacks
    std::function<void()> onZoomChanged;
    std::function<void()> onAutoToggled;
    std::function<void()> onReset;
    
    // Public interface
    void setAuto (bool on)
    {
        zoomState.setAuto (on);
        updatePresetButtons();
        if (onAutoToggled) onAutoToggled();
    }
    
    void setHalfRangeDb (float halfRangeDb)
    {
        zoomState.setHalfRangeDb (halfRangeDb);
        updateSliderFromState();
        updatePresetButtons();
        if (onZoomChanged) onZoomChanged();
    }
    
    void wheelZoom (float delta, float anchorY)
    {
        const float current = zoomState.getCurrent();
        const float range = zoomState.getMax() - zoomState.getMin();
        const float sensitivity = 0.1f;
        const float newValue = juce::jlimit (zoomState.getMin(), zoomState.getMax(), 
                                           current + delta * range * sensitivity);
        
        setHalfRangeDb (newValue);
    }
    
    
    void resized() override
    {
        auto r = getLocalBounds().reduced (4);
        
        // Preset buttons (top to bottom)
        const int buttonHeight = 20;
        const int buttonSpacing = 2;
        
        for (int i = 0; i < 5; ++i)
        {
            auto buttonArea = r.removeFromTop (buttonHeight);
            presetButtons[i].setBounds (buttonArea);
            r.removeFromTop (buttonSpacing);
        }
        
        r.removeFromTop (8);
        
        // Zoom slider (takes most space)
        auto sliderArea = r.removeFromTop (r.getHeight() - 30);
        zoomSlider.setBounds (sliderArea);
        
        r.removeFromTop (8);
        
        // Reset button (bottom)
        resetButton.setBounds (r.removeFromTop (24));
    }
    
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        
        // Draw preset tick marks on slider
        drawPresetTicks (g, zoomSlider.getBounds().toFloat());
        
        // Draw icons on buttons
        drawButtonIcon (g, resetButton, IconSystem::Reset);
        
        // Draw preset icons
        for (int i = 0; i < 5; ++i)
        {
            IconSystem::IconType iconType;
            switch (i)
            {
                case 0: iconType = IconSystem::ZoomOut; break; // ±6 dB
                case 1: iconType = IconSystem::ZoomOut; break; // ±12 dB  
                case 2: iconType = IconSystem::ZoomIn; break;  // ±18 dB
                case 3: iconType = IconSystem::ZoomIn; break;  // ±24 dB
                case 4: iconType = IconSystem::ZoomIn; break;  // ±36 dB
                default: iconType = IconSystem::ZoomIn; break;
            }
            drawButtonIcon (g, presetButtons[i], iconType);
        }
    }
    
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        const float anchorY = (float) e.position.y / (float) getHeight();
        wheelZoom (wheel.deltaY, anchorY);
    }
    
private:
    void drawButtonIcon (juce::Graphics& g, juce::Button& button, IconSystem::IconType iconType)
    {
        if (!button.isVisible()) return;
        
        auto bounds = button.getBounds().toFloat();
        auto iconSize = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.6f;
        auto iconBounds = bounds.withSizeKeepingCentre (iconSize, iconSize);
        
        // Get appropriate color based on button state
        juce::Colour iconColour;
        if (button.getToggleState())
            iconColour = juce::Colours::white;
        else
            iconColour = juce::Colours::white.withAlpha (0.6f);
        
        IconSystem::drawIcon (g, iconType, iconBounds, iconColour);
    }
    
    ZoomState& zoomState;
    juce::Slider zoomSlider;
    juce::TextButton presetButtons[5];
    juce::TextButton resetButton;
    
    void timerCallback() override
    {
        zoomState.update();
        updateSliderFromState();
    }
    
    void onSliderChanged()
    {
        const float value = (float) zoomSlider.getValue();
        zoomState.setHalfRangeDb (value);
        updatePresetButtons();
        if (onZoomChanged) onZoomChanged();
    }
    
    void onSliderDragStart()
    {
        zoomState.setMode (ZoomState::Mode::Manual);
    }
    
    void onSliderDragEnd()
    {
        // Snap to nearest preset if close
        const float value = (float) zoomSlider.getValue();
        if (zoomState.isNearPreset (value, 1.5f))
        {
            const float preset = zoomState.getClosestPreset (value);
            setHalfRangeDb (preset);
        }
    }
    
    void handleReset()
    {
        setHalfRangeDb (18.0f);
        if (onReset) onReset();
    }
    
    void setPreset (int index)
    {
        const float presets[] = { 6.0f, 12.0f, 18.0f, 24.0f, 36.0f };
        setHalfRangeDb (presets[index]);
    }
    
    void updateSliderFromState()
    {
        zoomSlider.setValue (zoomState.getCurrent(), juce::dontSendNotification);
    }
    
    void updatePresetButtons()
    {
        const float current = zoomState.getCurrent();
        const float presets[] = { 6.0f, 12.0f, 18.0f, 24.0f, 36.0f };
        
        for (int i = 0; i < 5; ++i)
        {
            const bool isActive = std::abs (current - presets[i]) < 0.5f;
            presetButtons[i].setToggleState (isActive, juce::dontSendNotification);
        }
    }
    
    juce::String getPresetLabel (int index) const
    {
        const juce::String labels[] = { "±6", "±12", "±18", "±24", "±36" };
        return labels[index];
    }
    
    void drawPresetTicks (juce::Graphics& g, juce::Rectangle<float> sliderBounds)
    {
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        
        const float presets[] = { 6.0f, 12.0f, 18.0f, 24.0f, 36.0f };
        const float min = zoomState.getMin();
        const float max = zoomState.getMax();
        
        for (int i = 0; i < 5; ++i)
        {
            const float normalized = (presets[i] - min) / (max - min);
            const float y = sliderBounds.getY() + normalized * sliderBounds.getHeight();
            
            // Draw tick mark
            g.drawLine (sliderBounds.getX() + 2, y, sliderBounds.getX() + 6, y, 1.0f);
        }
    }
};
