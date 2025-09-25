#pragma once
#include <JuceHeader.h>
#include "ZoomState.h"
#include "ZoomControl.h"

// Side rail container for Dynamic EQ zoom control
class DynEqZoomSideRail : public juce::Component
{
public:
    explicit DynEqZoomSideRail (ZoomState& zoomState)
    : zoomState (zoomState), zoomControl (zoomState)
    {
        setInterceptsMouseClicks (true, true);
        setWantsKeyboardFocus (true);
        
        // Header label (rotated)
        header.setText ("ZOOM", juce::dontSendNotification);
        header.setJustificationType (juce::Justification::centredLeft);
        header.setInterceptsMouseClicks (false, false);
        header.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.8f));
        addAndMakeVisible (header);
        
        // Mode LEDs
        addAndMakeVisible (ledAuto);
        addAndMakeVisible (ledManual);
        
        ledAuto.setButtonText ("AUTO");
        ledAuto.setClickingTogglesState (true);
        ledAuto.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        ledAuto.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFF5AA9E6)); // Accent color
        ledAuto.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        ledAuto.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.6f));
        
        ledManual.setButtonText ("MAN");
        ledManual.setClickingTogglesState (true);
        ledManual.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        ledManual.setColour (juce::TextButton::buttonOnColourId, juce::Colours::white.withAlpha (0.2f));
        ledManual.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        ledManual.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.6f));
        
        // Set initial state
        ledManual.setToggleState (true, juce::dontSendNotification);
        
        ledAuto.onClick = [this]
        {
            const bool on = ledAuto.getToggleState();
            ledManual.setToggleState (!on, juce::dontSendNotification);
            if (on) zoomControl.setAuto (true);
        };
        
        ledManual.onClick = [this]
        {
            const bool on = ledManual.getToggleState();
            ledAuto.setToggleState (!on, juce::dontSendNotification);
            if (on) zoomControl.setAuto (false);
        };
        
        // Zoom control
        addAndMakeVisible (zoomControl);
        
        // Reset button
        resetButton.setButtonText ("↺");
        resetButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        resetButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::white.withAlpha (0.2f));
        resetButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        resetButton.onClick = [this] { onReset(); };
        addAndMakeVisible (resetButton);
        
        // Wire up zoom control callbacks
        zoomControl.onZoomChanged = [this] { if (onZoomChanged) onZoomChanged(); };
        zoomControl.onAutoToggled = [this] { if (onAutoToggled) onAutoToggled(); };
    }
    
    // Callbacks
    std::function<void()> onZoomChanged;
    std::function<void()> onAutoToggled;
    std::function<void()> onReset;
    
    // Public interface
    void setAuto (bool on)
    {
        ledAuto.setToggleState (on, juce::dontSendNotification);
        ledManual.setToggleState (!on, juce::dontSendNotification);
        zoomControl.setAuto (on);
    }
    
    void setHalfRangeDb (float halfRangeDb)
    {
        zoomControl.setHalfRangeDb (halfRangeDb);
    }
    
    float getCurrentHalfRangeDb() const
    {
        return zoomState.getCurrent();
    }
    
    bool isAutoMode() const
    {
        return ledAuto.getToggleState();
    }
    
    // Forward wheel zoom
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        const float anchorY = juce::jlimit (0.f, 1.f, (float) e.position.y / (float) getHeight());
        zoomControl.wheelZoom (wheel.deltaY * 8.f, anchorY);
    }
    
    void mouseUp (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            showContextMenu();
        }
    }
    
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        
        // Panel background
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (r.reduced (1), 6.f);
        
        // Right-edge separator shadow
        juce::DropShadow ds { juce::Colours::black.withAlpha (0.35f), 12, { 2, 0 } };
        ds.drawForRectangle (g, getLocalBounds().translated (getWidth()-2, 0));
        
        // Focus ring
        if (hasKeyboardFocus (true))
        {
            g.setColour (juce::Colour (0xFF5AA9E6).withAlpha (0.5f));
            g.drawRoundedRectangle (r.reduced (1), 6.f, 2.f);
        }
    }
    
    void resized() override
    {
        auto r = getLocalBounds().reduced (6);
        
        // Header (rotated)
        auto headerArea = r.removeFromTop (24);
        header.setBounds (headerArea);
        
        // Rotate header text
        header.setTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi,
                                                              (float) headerArea.getCentreX(),
                                                              (float) headerArea.getCentreY()));
        
        r.removeFromTop (4);
        
        // Mode LEDs
        auto modeArea = r.removeFromTop (48);
        ledAuto.setBounds (modeArea.removeFromTop (22));
        modeArea.removeFromTop (4);
        ledManual.setBounds (modeArea.removeFromTop (22));
        
        r.removeFromTop (6);
        
        // Zoom control body (takes most space)
        auto body = r;
        zoomControl.setBounds (body.removeFromTop (body.getHeight() - 28));
        
        // Reset at bottom
        resetButton.setBounds (r.removeFromBottom (24));
    }
    
    bool keyPressed (const juce::KeyPress& key) override
    {
        if (key.getKeyCode() == '+' || key.getKeyCode() == '=')
        {
            const float current = zoomState.getCurrent();
            const float newValue = juce::jlimit (zoomState.getMin(), zoomState.getMax(), current + 3.0f);
            setHalfRangeDb (newValue);
            return true;
        }
        else if (key.getKeyCode() == '-')
        {
            const float current = zoomState.getCurrent();
            const float newValue = juce::jlimit (zoomState.getMin(), zoomState.getMax(), current - 3.0f);
            setHalfRangeDb (newValue);
            return true;
        }
        else if (key.getKeyCode() == '0')
        {
            setHalfRangeDb (18.0f);
            return true;
        }
        
        return false;
    }
    
    // Access underlying ZoomControl
    ZoomControl& getZoomControl() noexcept { return zoomControl; }
    
private:
    ZoomState& zoomState;
    ZoomControl zoomControl;
    
    juce::Label header;
    juce::ToggleButton ledAuto, ledManual;
    juce::TextButton resetButton;
    
    void showContextMenu()
    {
        juce::PopupMenu m;
        m.addItem (1, "Auto Zoom", true, ledAuto.getToggleState());
        m.addSeparator();
        m.addItem (2, "±6 dB");
        m.addItem (3, "±12 dB");
        m.addItem (4, "±18 dB");
        m.addItem (5, "±24 dB");
        m.addItem (6, "±36 dB");
        m.addSeparator();
        m.addItem (7, "Reset (±18, Manual)");
        
        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
            [this](int result)
            {
                if (result == 1)
                {
                    setAuto (!ledAuto.getToggleState());
                }
                else if (result >= 2 && result <= 6)
                {
                    const float presets[] = { 6.0f, 12.0f, 18.0f, 24.0f, 36.0f };
                    setAuto (false);
                    setHalfRangeDb (presets[result - 2]);
                }
                else if (result == 7)
                {
                    setAuto (false);
                    setHalfRangeDb (18.0f);
                    if (onReset) onReset();
                }
            });
    }
};
