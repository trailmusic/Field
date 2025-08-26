#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "FieldLookAndFeel.h"
#include "IconSystem.h"
#include "PresetSystem.h"

// WaveformDisplay class removed - functionality integrated into XYPad background

class XYPad : public juce::Component, public juce::Timer
{
public:
    std::function<void (float x01, float y01)> onChange; // x=pan, y=depth
    std::function<void (float leftX01, float rightX01, float y01)> onSplitChange; // for split mode
    std::function<void (int ballIndex, float x01, float y01)> onBallChange; // for individual ball control
    
    void setPoint01 (float x, float y) { pt = { juce::jlimit(0.f,1.f,x), juce::jlimit(0.f,1.f,y) }; repaint(); }
    void setSplitPoints (float leftX, float rightX, float y) { leftPt = leftX; rightPt = rightX; pt.second = y; repaint(); }
    void setBallPosition (int ballIndex, float x, float y); // New: set individual ball position
    std::pair<float,float> getPoint01() const { return pt; }
    std::pair<float,float> getSplitPoints() const { return {leftPt, rightPt}; }
    std::pair<float,float> getBallPosition (int ballIndex) const; // New: get individual ball position
    
    void setSplitMode (bool split) { 
        isSplitMode = split; 
        if (split && isLinked) {
            // When entering split mode with linking, ensure both balls are at center
            leftPt = 0.5f;
            rightPt = 0.5f;
        }
        repaint(); 
    }
    bool getSplitMode() const { return isSplitMode; }
    
    void setLinked (bool linked) { 
        isLinked = linked; 
        if (linked && isSplitMode) {
            // When linking in split mode, move both balls to center
            leftPt = 0.5f;
            rightPt = 0.5f;
        }
        repaint(); 
    }
    bool getLinked() const { return isLinked; }
    
    void setGainValue (float gainDb) { gainValue = gainDb; repaint(); }
    void setWidthValue (float widthPercent) { widthValue = widthPercent; repaint(); }
    void setTiltValue (float tiltDegrees) { tiltValue = tiltDegrees; repaint(); }
    void setMixValue (float mix01) { mixValue = mix01; repaint(); }
    void setDriveValue (float driveDb) { driveValue = driveDb; repaint(); }
    void setAirValue (float airDb) { airValue = airDb; repaint(); }
    void setBassValue (float bassDb) { bassValue = bassDb; repaint(); }
    void setScoopValue (float scoopDb) { scoopValue = scoopDb; repaint(); } // NEW: Scoop value setter
    void setHPValue (float hpHz) { hpValue = hpHz; repaint(); }
    void setLPValue (float lpHz) { lpValue = lpHz; repaint(); }
    void setPanValue (float pan) { panValue = pan; repaint(); }
    void setMonoValue (float monoHz) { monoHzValue = monoHz; repaint(); }
    void setSpaceValue (float spaceDepth) { spaceValue = spaceDepth; repaint(); }
    void setSpaceAlgorithm (int algorithm) { spaceAlgorithm = algorithm; repaint(); }
    void setGreenMode (bool enabled) { isGreenMode = enabled; repaint(); }
    
    // NEW: Frequency control setters for EQ visualization
    void setTiltFreqValue (float freq) { tiltFreqValue = freq; repaint(); }
    void setScoopFreqValue (float freq) { scoopFreqValue = freq; repaint(); }
    void setBassFreqValue (float freq) { bassFreqValue = freq; repaint(); }
    void setAirFreqValue (float freq) { airFreqValue = freq; repaint(); }
    
    void pushWaveformSample (double sampleL, double sampleR); // New: for background waveform

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override { drag(e); }
    void mouseDrag (const juce::MouseEvent& e)  override { drag(e); }
    void mouseUp (const juce::MouseEvent&) override { activeBall = 0; }
    void mouseEnter (const juce::MouseEvent&) override { hoverActive = true; stopTimer(); repaint(); }
    void mouseExit  (const juce::MouseEvent&) override { startTimer (hoverOffDelayMs); }
    void timerCallback() override { hoverActive = false; stopTimer(); repaint(); }
    
    void setSnapEnabled (bool shouldSnap) { snapEnabled = shouldSnap; }
    bool getSnapEnabled () const { return snapEnabled; }

private:
    std::pair<float,float> pt { 0.5f, 0.2f };
    float leftPt = 0.5f, rightPt = 0.5f; // Start both balls in center
    bool isSplitMode = false; // Default to single ball
    bool isLinked = true; // Default to linked mode
    int activeBall = 0; // 0 = center, 1 = left, 2 = right
    bool snapEnabled = false;
    bool hoverActive = false;
    const int hoverOffDelayMs = 160;
    float gainValue = 0.0f; // For visual feedback
    float widthValue = 50.0f; // For visual feedback
    float tiltValue = 0.0f; // For visual feedback
    float mixValue = 0.5f; // For waveform opacity control
    float driveValue = 0.0f; // For drive visual feedback
    float airValue = 0.0f; // For air band visual feedback
    float bassValue = 0.0f; // For bass shelf visual feedback
    float scoopValue = 0.0f; // NEW: For scoop EQ visual feedback
    float hpValue = 20.0f; // For HP filter visual feedback
    float lpValue = 20000.0f; // For LP filter visual feedback
    float panValue = 0.0f; // For pan visual feedback
    float monoHzValue = 0.0f; // For mono indicator
    float spaceValue = 0.0f; // For space depth visual feedback
    int spaceAlgorithm = 0; // For space algorithm visual feedback (0=Inner, 1=Outer, 2=Deep)
    bool isGreenMode = false; // For green color mode
    
    // NEW: Frequency control values for EQ visualization
    float tiltFreqValue = 500.0f;
    float scoopFreqValue = 800.0f;
    float bassFreqValue = 150.0f;
    float airFreqValue = 8000.0f;
    
    // Waveform data for background display
    static constexpr int waveformBufferSize = 512;
    std::array<double, waveformBufferSize> waveformL, waveformR;
    int waveformWriteIndex = 0;
    bool hasWaveformData = false;
    
    void drag (const juce::MouseEvent& e);
    void drawGrid (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawBalls (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawWaveformBackground (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawEQCurves (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawFrequencyRegions (juce::Graphics& g, juce::Rectangle<float> bounds);
    void analyzeSpectralResponse (std::vector<float>& response, float width);
    int getBallAtPosition (juce::Point<float> pos, juce::Rectangle<float> bounds);
};

class ControlContainer : public juce::Component, public juce::Timer
{
public:
    ControlContainer();
    
    void setTitle (const juce::String& title);
    void setShowBorder (bool show) { showBorder = show; }
    void paint (juce::Graphics& g) override;
    void mouseEnter (const juce::MouseEvent&) override { hovered = true; hoverActive = true; stopTimer(); repaint(); }
    void mouseExit  (const juce::MouseEvent&) override { hovered = false; startTimer (hoverOffDelayMs); }
    void timerCallback() override { hoverActive = false; stopTimer(); repaint(); }
    
private:
    juce::String containerTitle;
    bool hovered = false;
    bool hoverActive = false;
    bool showBorder = true;
    const int hoverOffDelayMs = 160;
};

class ToggleSwitch : public juce::Component, public juce::Timer
{
public:
    ToggleSwitch();
    
    void setToggleState (bool shouldBeOn, juce::NotificationType notification = juce::dontSendNotification);
    bool getToggleState() const { return isOn; }
    
    void setLabels (const juce::String& offLabel, const juce::String& onLabel);
    
    std::function<void(bool)> onToggleChange;
    
protected:
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent&) override { hovered = true; hoverActive = true; stopTimer(); repaint(); }
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

class BypassButton : public juce::TextButton, public juce::Timer
{
public:
    BypassButton() : juce::TextButton("")
    {
        setLookAndFeel(&customLookAndFeel);
        startTimerHz(2); // Blink at 2Hz when bypassed
    }
    
    ~BypassButton() override
    {
        stopTimer();
        setLookAndFeel(nullptr);
    }
    
    void timerCallback() override
    {
        if (getToggleState()) {
            repaint(); // Trigger repaint for blinking effect
        }
    }
    
private:
    class CustomLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
            
            // Determine button color based on state and green mode
            juce::Colour baseColour;
            bool isGreenMode = false;
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&button.getLookAndFeel())) {
                // Check if we're in green mode by looking at the accent color
                isGreenMode = (lookAndFeel->theme.accent == juce::Colour(0xFF5AA95A));
            }
            
            if (button.getToggleState()) {
                // Active (bypassed) - red with blinking effect and yellow glow
                baseColour = juce::Colour(0xFFE53935); // Material Design Red
                
                // Blinking effect - alternate between red and darker red
                auto currentTime = juce::Time::getMillisecondCounter();
                if ((currentTime / 250) % 2 == 0) { // Blink every 250ms
                    baseColour = baseColour.darker(0.3f);
                }
                
                // Draw yellow glow behind button when bypassed
                g.setColour(juce::Colour(0x40FFEB3B)); // Semi-transparent yellow
                g.fillRoundedRectangle(bounds.expanded(4.0f), 6.0f);
            } else {
                // Inactive (processing) - green or blue accent color based on mode
                baseColour = isGreenMode ? juce::Colour(0xFF5AA95A) : juce::Colour(0xFF2196F3);
            }
            
            // Add highlight effect if needed
            if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted) {
                baseColour = baseColour.brighter(0.1f);
            }
            
            // Draw raised shadow effect
            g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
            g.fillRoundedRectangle(bounds.translated(2.0f, 2.0f), 4.0f);
            
            // Draw main button background
            g.setColour(baseColour);
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Draw border
            g.setColour(baseColour.darker(0.3f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        }
        
        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat();
            
            // Icon color based on state and green mode
            juce::Colour iconColour;
            bool isGreenMode = false;
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&button.getLookAndFeel())) {
                // Check if we're in green mode by looking at the accent color
                isGreenMode = (lookAndFeel->theme.accent == juce::Colour(0xFF5AA95A));
            }
            
            if (button.getToggleState()) {
                // Active (bypassed) - white icon on red
                iconColour = juce::Colours::white;
            } else {
                // Inactive (processing) - green or blue icon based on mode
                iconColour = isGreenMode ? juce::Colour(0xFF2E7D32) : juce::Colour(0xFF1565C0);
            }
            
            // Draw power icon
            IconSystem::drawIcon(g, IconSystem::Power, bounds.reduced(4.0f), iconColour);
        }
    };
    
    CustomLookAndFeel customLookAndFeel;
};

class MyPluginAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer, public juce::Slider::Listener, public juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit MyPluginAudioProcessorEditor (MyPluginAudioProcessor&);
    ~MyPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sliderValueChanged(juce::Slider* slider) override;
    void mouseEnter (const juce::MouseEvent& e) override { 
        // Only activate if mouse is in header area
        auto headerBounds = getLocalBounds().removeFromTop(static_cast<int>(60 * scaleFactor));
        if (headerBounds.contains(e.position.toInt())) {
            headerHovered = true; 
            headerHoverActive = true; 
            stopTimer(); 
            repaint(); 
        }
    }
    void mouseExit  (const juce::MouseEvent& e) override { 
        // Check if mouse is still in header area
        auto headerBounds = getLocalBounds().removeFromTop(static_cast<int>(60 * scaleFactor));
        if (!headerBounds.contains(e.position.toInt())) {
            headerHovered = false; 
            startTimer (headerHoverOffDelayMs); 
        }
    }
    
    void setScaleFactor (float newScale) override;
    
    // Audio sample callback for waveform display
    // pushAudioSample removed - waveform display integrated into XYPad
    void syncXYPadWithParameters();
    void setupTooltips();
    
    // Custom gain slider component
    class GainSlider : public juce::Slider
    {
    public:
        GainSlider() : juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox) {}
        
        void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
        void mouseExit  (const juce::MouseEvent&) override { hovered = false; repaint(); }
        void mouseDown  (const juce::MouseEvent& e) override { active = true; juce::Slider::mouseDown(e); repaint(); }
        void mouseUp    (const juce::MouseEvent& e) override { active = false; juce::Slider::mouseUp(e); repaint(); }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            if (hovered || active) bounds = bounds.expanded(2.0f);
            
            // Get current gain value
            float gainDb = static_cast<float>(getValue());
            
            // Use custom drawing function
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                const double minV = getMinimum();
                const double maxV = getMaximum();
                float pos01 = (maxV > minV) ? static_cast<float>((getValue() - minV) / (maxV - minV)) : 0.0f;
                lookAndFeel->drawGainSlider(g, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                           pos01, juce::MathConstants<float>::pi,
                                           juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi, gainDb);
            }
        }
    private:
        bool hovered = false;
        bool active = false;
    };
    
    // Custom pan slider component with split percentage visualization
    class PanSlider : public juce::Slider
    {
    public:
        PanSlider() : juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox) {}
        
        void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
        void mouseExit  (const juce::MouseEvent&) override { hovered = false; repaint(); }
        void mouseDown  (const juce::MouseEvent& e) override { active = true; juce::Slider::mouseDown(e); repaint(); }
        void mouseUp    (const juce::MouseEvent& e) override { active = false; juce::Slider::mouseUp(e); repaint(); }

        void setSplitPercentage(float leftPercent, float rightPercent)
        {
            splitLeftPercent = leftPercent;
            splitRightPercent = rightPercent;
            repaint();
        }
        
        void setLabel(const juce::String& label)
        {
            knobLabel = label;
            repaint();
        }

        void setOverlayEnabled (bool enabled)
        {
            overlayEnabled = enabled;
            repaint();
        }
        
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            // Slight grow when hovered/active
            if (hovered || active) bounds = bounds.expanded(2.0f);
            
            // Draw the normal slider first
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                const double minV = getMinimum();
                const double maxV = getMaximum();
                float pos01 = (maxV > minV) ? static_cast<float>((getValue() - minV) / (maxV - minV)) : 0.0f;
                lookAndFeel->drawRotarySlider(g, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                             pos01, juce::MathConstants<float>::pi,
                                             juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi, *this);
            }
            
            if (overlayEnabled)
            {
                // Draw blue border indication based on value (50L to 50R range)
                float normalizedValue = (getValue() + 1.0f) * 0.5f; // Convert -1..1 to 0..1
                float borderThickness = 3.0f;
                
                juce::Path valueBorder;
                float valueAngle = juce::jmap(normalizedValue, 0.0f, 1.0f, 
                                            juce::MathConstants<float>::pi, 
                                            juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
                valueBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                 juce::MathConstants<float>::pi, valueAngle, true);
                g.setColour(juce::Colour(0xFF5AA9E6).withAlpha(0.8f)); // Blue accent
                g.strokePath(valueBorder, juce::PathStrokeType(borderThickness));
            }
            
            // Draw split percentage border if in split mode
            if (overlayEnabled && splitLeftPercent >= 0.0f && splitRightPercent >= 0.0f)
            {
                float borderThickness = 3.0f;
                // Draw left channel border (blue)
                juce::Path leftBorder;
                float leftAngle = juce::jmap(splitLeftPercent, 0.0f, 100.0f, 
                                           juce::MathConstants<float>::pi, 
                                           juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
                leftBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                 juce::MathConstants<float>::pi, leftAngle, true);
                g.setColour(juce::Colour(0xFF5AA9E6).withAlpha(0.8f)); // Blue for left
                g.strokePath(leftBorder, juce::PathStrokeType(borderThickness));
                
                // Draw right channel border (red)
                juce::Path rightBorder;
                float rightAngle = juce::jmap(splitRightPercent, 0.0f, 100.0f, 
                                            juce::MathConstants<float>::pi, 
                                            juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
                rightBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                  leftAngle, rightAngle, true);
                g.setColour(juce::Colour(0xFFFF6B6B).withAlpha(0.8f)); // Red for right
                g.strokePath(rightBorder, juce::PathStrokeType(borderThickness));
            }
            
            // Draw L/R label if set
            if (knobLabel.isNotEmpty())
            {
                g.setColour(juce::Colour(0xFFF0F2F5));
                g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
                g.drawText(knobLabel, bounds, juce::Justification::centred);
            }
        }
        
    private:
        float splitLeftPercent = -1.0f;  // -1 means not in split mode
        float splitRightPercent = -1.0f;
        bool hovered = false;
        bool active = false;
        bool overlayEnabled = false;
        juce::String knobLabel;
    };
    
    // Custom ducking slider component with smaller design
    class DuckingSlider : public juce::Slider
    {
    public:
        DuckingSlider() : juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox) {}
        
        void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
        void mouseExit  (const juce::MouseEvent&) override { hovered = false; repaint(); }
        void mouseDown  (const juce::MouseEvent& e) override { active = true; juce::Slider::mouseDown(e); repaint(); }
        void mouseUp    (const juce::MouseEvent& e) override { active = false; juce::Slider::mouseUp(e); repaint(); }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(4.0f); // Smaller than standard
            if (hovered || active) bounds = bounds.expanded(1.5f);
            
            // Draw the normal slider with smaller size
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                const double minV = getMinimum();
                const double maxV = getMaximum();
                float pos01 = (maxV > minV) ? static_cast<float>((getValue() - minV) / (maxV - minV)) : 0.0f;
                lookAndFeel->drawRotarySlider(g, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                             pos01, juce::MathConstants<float>::pi,
                                             juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi, *this);
            }
        }
    private:
        bool hovered = false;
        bool active = false;
    };
    
    // Resize handle functionality
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

private:
    MyPluginAudioProcessor& proc;
    FieldLNF lnf;
    
    // UI Components
    GainSlider gain;
    juce::Slider width, tilt, monoHz, hpHz, lpHz, satDrive, satMix, air, bass, scoop; // Added scoop knob
    PanSlider panKnob;  // Custom pan slider with split visualization
    PanSlider panKnobLeft, panKnobRight;  // Split mode pan controls
    DuckingSlider duckingKnob;  // Custom ducking slider with smaller design
    juce::ComboBox osSelect;

    PresetComboBox presetCombo;
    SavePresetButton savePresetButton;
    BypassButton bypassButton;
    ToggleSwitch splitToggle;
    
    // NEW: Frequency control sliders
    juce::Slider tiltFreqSlider, scoopFreqSlider, bassFreqSlider, airFreqSlider;
    
    // Custom icon buttons
    class OptionsButton : public juce::TextButton
    {
    public:
        OptionsButton() : juce::TextButton("") {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Background with light gradient (design system, toned down)
            juce::Colour base = juce::Colour(0xFF3A3D45);
            juce::Colour top  = base.brighter(0.10f);
            juce::Colour bot  = base.darker(0.10f);
            juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, 3.0f);
            
            // Cog wheel icon - color based on mode
            juce::Colour iconColor = juce::Colour(0xFF888888); // Default grey
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                iconColor = lookAndFeel->theme.textMuted;
            }
            IconSystem::drawIcon(g, IconSystem::CogWheel, bounds.reduced(4.0f), iconColor);
        }
    };
    
    OptionsButton optionsButton;
    
    class LinkButton : public juce::TextButton
    {
    public:
        LinkButton() : juce::TextButton("") {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw raised shadow effect for depth
            g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
            g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 4.0f);
            
            // Background - accent color when active, grey when inactive
            juce::Colour bgColour;
            bool isActive = getToggleState();
            
            if (isActive) {
                // Active state - color based on mode
                juce::Colour activeColor = juce::Colour(0xFF2196F3); // Default blue
                if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                    activeColor = lookAndFeel->theme.accent;
                }
                bgColour = isButtonDown ? activeColor.darker(0.3f) : 
                          isMouseOver ? activeColor.brighter(0.1f) : 
                          activeColor;
            } else {
                // Inactive state - grey theme
                bgColour = isButtonDown ? juce::Colour(0xFF4A4A4A) : 
                          isMouseOver ? juce::Colour(0xFF5A5A5A) : 
                          juce::Colour(0xFF3A3A3A);
            }
            
            g.setColour(bgColour);
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Border for definition
            g.setColour(bgColour.darker(0.3f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
            
            // Link icon - white when active, grey when inactive
            juce::Colour iconColour = isActive ? juce::Colours::white : juce::Colour(0xFF888888);
            IconSystem::drawIcon(g, IconSystem::Link, bounds.reduced(4.0f), iconColour);
        }
    };
    
    LinkButton linkButton;
    
    class SnapButton : public juce::TextButton
    {
    public:
        SnapButton() : juce::TextButton("") {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw raised shadow effect for depth
            g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
            g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 3.0f);
            
            // Background - accent color when active, FullScreenButton gradient when inactive
            bool isActive = getToggleState();
            
            if (isActive) {
                // Active state - color based on mode
                juce::Colour activeColor = juce::Colour(0xFF2196F3); // Default blue
                if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                    activeColor = lookAndFeel->theme.accent;
                }
                g.setColour(activeColor);
                g.fillRoundedRectangle(bounds, 4.0f);
                
                // Border for definition
                g.setColour(activeColor.darker(0.3f));
                g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
            } else {
                // Inactive state - FullScreenButton gradient style
                // Get theme colors from look and feel
                juce::Colour base, top, bot, borderColor;
                if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                    base = lookAndFeel->theme.panel;
                    top = base.brighter(0.10f);
                    bot = base.darker(0.10f);
                    borderColor = isButtonDown ? lookAndFeel->theme.sh : 
                                 isMouseOver ? lookAndFeel->theme.hl : 
                                 lookAndFeel->theme.sh;
                } else {
                    base = juce::Colour(0xFF3A3D45);
                    top = base.brighter(0.10f);
                    bot = base.darker(0.10f);
                    borderColor = isButtonDown ? juce::Colour(0xFF2A2A2A) : 
                                 isMouseOver ? juce::Colour(0xFF4A4A4A) : 
                                 juce::Colour(0xFF2A2A2A);
                }
                
                // Background with light gradient (design system, toned down)
                juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(bounds, 3.0f);
                
                // Border for definition
                g.setColour(borderColor);
                g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
            }
            
            // Snap icon - white when active, grey when inactive
            juce::Colour iconColour = isActive ? juce::Colours::white : juce::Colour(0xFF888888);
            IconSystem::drawIcon(g, IconSystem::Snap, bounds.reduced(4.0f), iconColour);
        }
    };
    
    SnapButton snapButton;
    
    // Custom Space knob (now just a regular knob without algorithm functionality)
    class SpaceKnob : public juce::Slider
    {
    public:
        SpaceKnob() : juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox) {}
        
        void setGreenMode(bool enabled)
        {
            isGreenMode = enabled;
            repaint();
        }
        
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw the normal slider (with built-in blue indicator)
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                const double minV = getMinimum();
                const double maxV = getMaximum();
                float pos01 = (maxV > minV) ? static_cast<float>((getValue() - minV) / (maxV - minV)) : 0.0f;
                lookAndFeel->drawRotarySlider(g, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                             pos01, juce::MathConstants<float>::pi,
                                             juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi, *this);
            }
        }
        
    private:
        bool isGreenMode = false;
    };
    
    SpaceKnob spaceKnob;
    
    // 3-way button switch for Space algorithms (Inner, Outer, Deep)
    class SpaceAlgorithmSwitch : public juce::Component
    {
    public:
        SpaceAlgorithmSwitch() : currentPosition(0) // 0=Inner, 1=Outer, 2=Deep
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }
        
        void setAlgorithm(int algorithm)
        {
            currentPosition = algorithm;
            repaint();
        }
        
        void setAlgorithmFromParameter(int algorithm)
        {
            currentPosition = algorithm;
            repaint();
        }
        
        int getAlgorithm() const { return currentPosition; }
        
        void setGreenMode(bool enabled)
        {
            isGreenMode = enabled;
            repaint();
        }
        
        std::function<void(int)> onAlgorithmChange;
        
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            
            
            
            // Background removed per request
            
            // Calculate positions for three stacked buttons with vertical spacing
            const float spacing = 6.0f;
            const float availableH = juce::jmax(0.0f, bounds.getHeight() - 2.0f * spacing);
            const float buttonHeight = availableH / 3.0f;
            
            juce::Rectangle<float> topButton(bounds.getX(), bounds.getY(), bounds.getWidth(), buttonHeight); // Deep
            juce::Rectangle<float> midButton(bounds.getX(), bounds.getY() + buttonHeight + spacing, bounds.getWidth(), buttonHeight); // Outer
            juce::Rectangle<float> bottomButton(bounds.getX(), bounds.getY() + 2.0f * (buttonHeight + spacing), bounds.getWidth(), buttonHeight); // Inner

            // Draw the three buttons with old school compressor style
            drawButton(g, topButton, 2, "Deep");
            drawButton(g, midButton, 1, "Outer");
            drawButton(g, bottomButton, 0, "Inner");
        }
        
        void mouseDown(const juce::MouseEvent& e) override
        {
            // Right-click: show a popup menu for reliable selection
            if (e.mods.isPopupMenu())
            {
                juce::PopupMenu m;
                m.addItem(1, "Inner", true, currentPosition == 0);
                m.addItem(2, "Outer", true, currentPosition == 1);
                m.addItem(3, "Deep",  true, currentPosition == 2);
                
                m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
                    [this](int result)
                    {
                        if (result > 0)
                        {
                            int newPosition = result - 1; // 1->0, 2->1, 3->2
                            currentPosition = newPosition;
                            repaint();
                            if (onAlgorithmChange) {
                                onAlgorithmChange(currentPosition);
                            }
                        }
                    });
                return;
            }

            // Left-click: respect vertical spacing between buttons
            auto bounds = getLocalBounds().toFloat();
            const float spacing = 6.0f;
            const float availableH = juce::jmax(0.0f, bounds.getHeight() - 2.0f * spacing);
            const float buttonHeight = availableH / 3.0f;
            const float y = e.y;

            const float topEnd = buttonHeight;
            const float midStart = buttonHeight + spacing;
            const float midEnd = midStart + buttonHeight;
            const float bottomStart = midEnd + spacing;
            
            int newPosition;
            if (y <= topEnd)                      newPosition = 2; // Deep (top)
            else if (y >= midStart && y <= midEnd) newPosition = 1; // Outer (middle)
            else if (y >= bottomStart)            newPosition = 0; // Inner (bottom)
            else                                   return; // Clicked in a gap; ignore
            
            currentPosition = newPosition;
            repaint();
            
            if (onAlgorithmChange) {
                onAlgorithmChange(currentPosition);
            }
        }
        
    private:
        int currentPosition;
        bool isGreenMode = false;
        
        void drawButton(juce::Graphics& g, juce::Rectangle<float> buttonBounds, int buttonIndex, const juce::String& label)
        {
            bool isActive = (currentPosition == buttonIndex);
            
            // Button background with grey shading
            juce::Colour buttonColour = isActive ? getActiveColour(buttonIndex) : juce::Colour(0xFF2A2C30);
            g.setColour(buttonColour);
            g.fillRoundedRectangle(buttonBounds, 6.0f);
            
            // Thin border
            g.setColour(juce::Colour(0xFF1A1C20));
            g.drawRoundedRectangle(buttonBounds, 6.0f, 1.0f);
            
            // 3D effect - pressed down when active, raised when inactive - consistent top-left light
            if (isActive) {
                // Pressed down effect - darker inner shadow
                g.setColour(juce::Colour(0x40000000));
                g.fillRoundedRectangle(buttonBounds.reduced(1.0f), 5.0f);
            } else {
                // Raised effect - lighter highlight from top-left
                g.setColour(juce::Colour(0x20FFFFFF));
                g.fillRoundedRectangle(buttonBounds.reduced(1.0f).translated(-0.5f, -0.5f), 5.0f);
            }
            
            // Draw label (bigger font, theme text color)
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                g.setColour(lookAndFeel->theme.text);
            else
                g.setColour(juce::Colour(0xFFF0F2F5));
            g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
            g.drawText(label, buttonBounds, juce::Justification::centred);
        }
        
        juce::Colour getActiveColour(int algorithm)
        {
            if (isGreenMode) {
                switch (algorithm) {
                    case 0: return juce::Colour(0xFF5AA95A); // Inner - Green
                    case 1: return juce::Colour(0xFF7ACF95); // Outer - Mint green (more distinct)
                    case 2: return juce::Colour(0xFF4C8F4C); // Deep - Darker green
                    default: return juce::Colour(0xFF5AA95A);
                }
            } else {
                switch (algorithm) {
                    case 0: return juce::Colour(0xFF5AA9E6); // Inner - Blue
                    case 1: return juce::Colour(0xFF2EC4B6); // Outer - Teal (distinct from blue)
                    case 2: return juce::Colour(0xFF2A1B3D); // Deep - Dark blue/purple
                    default: return juce::Colour(0xFF5AA9E6);
                }
            }
        }
    };
    
    SpaceAlgorithmSwitch spaceAlgorithmSwitch;
    
    class FullScreenButton : public juce::TextButton
    {
    public:
        FullScreenButton() : juce::TextButton("") {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Get theme colors from look and feel
            juce::Colour base, top, bot, borderColor, iconColor;
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                base = lookAndFeel->theme.panel;
                top = base.brighter(0.10f);
                bot = base.darker(0.10f);
                borderColor = isButtonDown ? lookAndFeel->theme.sh : 
                             isMouseOver ? lookAndFeel->theme.hl : 
                             lookAndFeel->theme.sh;
                iconColor = lookAndFeel->theme.textMuted;
            } else {
                base = juce::Colour(0xFF3A3D45);
                top = base.brighter(0.10f);
                bot = base.darker(0.10f);
                borderColor = isButtonDown ? juce::Colour(0xFF2A2A2A) : 
                             isMouseOver ? juce::Colour(0xFF4A4A4A) : 
                             juce::Colour(0xFF2A2A2A);
                iconColor = juce::Colour(0xFF888888);
            }
            
            // Background with light gradient (design system, toned down)
            juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, 3.0f);
            
            // Border for definition
            g.setColour(borderColor);
            g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
            
            // Full screen icon (grey like options button)
            auto iconType = getToggleState() ? IconSystem::ExitFullScreen : IconSystem::FullScreen;
            IconSystem::drawIcon(g, iconType, bounds.reduced(4.0f), iconColor);
        }
    };
    
    FullScreenButton fullScreenButton;
    
    class ColorModeButton : public juce::TextButton
    {
    public:
        ColorModeButton() : juce::TextButton("") {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw raised shadow effect for depth
            g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
            g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 4.0f);
            
            // Background - green when active, standard grey when inactive
            juce::Colour bgColour;
            bool isActive = getToggleState();
            
            if (isActive) {
                // Active state - keep green color
                bgColour = isButtonDown ? juce::Colour(0xFF2E7D32) : 
                          isMouseOver ? juce::Colour(0xFF388E3C) : 
                          juce::Colour(0xFF4CAF50);
            } else {
                // Inactive state - FullScreenButton gradient style
                // Get theme colors from look and feel
                juce::Colour base, top, bot, borderColor, iconColor;
                if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                    base = lookAndFeel->theme.panel;
                    top = base.brighter(0.10f);
                    bot = base.darker(0.10f);
                    borderColor = isButtonDown ? lookAndFeel->theme.sh : 
                                 isMouseOver ? lookAndFeel->theme.hl : 
                                 lookAndFeel->theme.sh;
                    iconColor = lookAndFeel->theme.textMuted;
                } else {
                    base = juce::Colour(0xFF3A3D45);
                    top = base.brighter(0.10f);
                    bot = base.darker(0.10f);
                    borderColor = isButtonDown ? juce::Colour(0xFF2A2A2A) : 
                                 isMouseOver ? juce::Colour(0xFF4A4A4A) : 
                                 juce::Colour(0xFF2A2A2A);
                    iconColor = juce::Colour(0xFF888888);
                }
                
                // Background with light gradient (design system, toned down)
                juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(bounds, 3.0f);
                
                // Border for definition
                g.setColour(borderColor);
                g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
                
                // Color palette icon
                auto iconType = IconSystem::ColorPalette;
                IconSystem::drawIcon(g, iconType, bounds.reduced(4.0f), iconColor);
                return; // Skip the rest of the painting for inactive state
            }
            
            g.setColour(bgColour);
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Border for definition
            g.setColour(bgColour.darker(0.3f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
            
            // Color palette icon
            auto iconType = IconSystem::ColorPalette;
            juce::Colour iconColour = isActive ? juce::Colours::white : juce::Colour(0xFF888888);
            IconSystem::drawIcon(g, iconType, bounds.reduced(4.0f), iconColour);
        }
    };
    
    ColorModeButton colorModeButton;
    
    class ABButton : public juce::TextButton
    {
    public:
        ABButton(bool isAButton) : juce::TextButton(isAButton ? "A" : "B"), isA(isAButton) {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw raised shadow effect for depth
            g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
            g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 4.0f);
            
            // Get green mode state from look and feel
            bool isGreenMode = false;
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&this->getLookAndFeel())) {
                // Check if we're in green mode by looking at the accent color
                isGreenMode = (lookAndFeel->theme.accent == juce::Colour(0xFF5AA95A));
            }
            
            // Background - blue for A, green for B, with active state
            juce::Colour bgColour;
            bool isActive = getToggleState();
            
            if (isA) {
                // A button - blue theme (match bypass blue) or green theme in green mode
                if (isActive) {
                    if (isGreenMode) {
                        bgColour = isButtonDown ? juce::Colour(0xFF2E7D32) : 
                                  isMouseOver ? juce::Colour(0xFF388E3C) : 
                                  juce::Colour(0xFF4CAF50);
                    } else {
                        bgColour = isButtonDown ? juce::Colour(0xFF1565C0) : 
                                  isMouseOver ? juce::Colour(0xFF1976D2) : 
                                  juce::Colour(0xFF2196F3);
                    }
                } else {
                    // Inactive state - use FullScreenButton gradient style
                    // Get theme colors from look and feel
                    juce::Colour base, top, bot, borderColor, textColor;
                    if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                        base = lookAndFeel->theme.panel;
                        top = base.brighter(0.10f);
                        bot = base.darker(0.10f);
                        borderColor = isButtonDown ? lookAndFeel->theme.sh : 
                                     isMouseOver ? lookAndFeel->theme.hl : 
                                     lookAndFeel->theme.sh;
                        textColor = lookAndFeel->theme.textMuted;
                    } else {
                        base = juce::Colour(0xFF3A3D45);
                        top = base.brighter(0.10f);
                        bot = base.darker(0.10f);
                        borderColor = isButtonDown ? juce::Colour(0xFF2A2A2A) : 
                                     isMouseOver ? juce::Colour(0xFF4A4A4A) : 
                                     juce::Colour(0xFF2A2A2A);
                        textColor = juce::Colour(0xFF888888);
                    }
                    
                    // Background with light gradient (design system, toned down)
                    juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
                    g.setGradientFill(grad);
                    g.fillRoundedRectangle(bounds, 3.0f);
                    
                    // Border for definition
                    g.setColour(borderColor);
                    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
                    
                    // Text
                    g.setColour(textColor);
                    g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
                    g.drawText("A", bounds, juce::Justification::centred);
                    return; // Skip the rest of the painting for inactive state
                }
            } else {
                // B button - green theme (or blue theme in non-green mode)
                if (isActive) {
                    if (isGreenMode) {
                        bgColour = isButtonDown ? juce::Colour(0xFF2E7D32) : 
                                  isMouseOver ? juce::Colour(0xFF388E3C) : 
                                  juce::Colour(0xFF4CAF50); // Bright green when active
                    } else {
                        bgColour = isButtonDown ? juce::Colour(0xFF1565C0) : 
                                  isMouseOver ? juce::Colour(0xFF1976D2) : 
                                  juce::Colour(0xFF2196F3); // Blue when active in non-green mode
                    }
                } else {
                    // Inactive state - use FullScreenButton gradient style
                    // Get theme colors from look and feel
                    juce::Colour base, top, bot, borderColor, textColor;
                    if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                        base = lookAndFeel->theme.panel;
                        top = base.brighter(0.10f);
                        bot = base.darker(0.10f);
                        borderColor = isButtonDown ? lookAndFeel->theme.sh : 
                                     isMouseOver ? lookAndFeel->theme.hl : 
                                     lookAndFeel->theme.sh;
                        textColor = lookAndFeel->theme.textMuted;
                    } else {
                        base = juce::Colour(0xFF3A3D45);
                        top = base.brighter(0.10f);
                        bot = base.darker(0.10f);
                        borderColor = isButtonDown ? juce::Colour(0xFF2A2A2A) : 
                                     isMouseOver ? juce::Colour(0xFF4A4A4A) : 
                                     juce::Colour(0xFF2A2A2A);
                        textColor = juce::Colour(0xFF888888);
                    }
                    
                    // Background with light gradient (design system, toned down)
                    juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
                    g.setGradientFill(grad);
                    g.fillRoundedRectangle(bounds, 3.0f);
                    
                    // Border for definition
                    g.setColour(borderColor);
                    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
                    
                    // Text
                    g.setColour(textColor);
                    g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
                    g.drawText("B", bounds, juce::Justification::centred);
                    return; // Skip the rest of the painting for inactive state
                }
            }
            
            g.setColour(bgColour);
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Border for definition
            g.setColour(bgColour.darker(0.3f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
            
            // Active state indicator (glow effect)
            if (isActive) {
                g.setColour(bgColour.withAlpha(0.3f));
                g.drawRoundedRectangle(bounds.expanded(1.0f), 5.0f, 2.0f);
            }
            
            // Text
            g.setColour(isActive ? juce::Colours::white : juce::Colour(0xFF888888));
            g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
            g.drawText(isA ? "A" : "B", bounds, juce::Justification::centred);
        }
        
    private:
        bool isA;
    };
    
    class PresetArrowButton : public juce::TextButton
    {
    public:
        PresetArrowButton(bool isLeft) : juce::TextButton(""), leftArrow(isLeft) {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw raised shadow effect for depth
            g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
            g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 3.0f);
            
            // Get theme colors from look and feel
            juce::Colour base, top, bot, borderColor;
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                base = lookAndFeel->theme.panel;
                top = base.brighter(0.10f);
                bot = base.darker(0.10f);
                borderColor = isButtonDown ? lookAndFeel->theme.sh : 
                             isMouseOver ? lookAndFeel->theme.hl : 
                             lookAndFeel->theme.sh;
            } else {
                base = juce::Colour(0xFF3A3D45);
                top = base.brighter(0.10f);
                bot = base.darker(0.10f);
                borderColor = isButtonDown ? juce::Colour(0xFF2A2A2A) : 
                             isMouseOver ? juce::Colour(0xFF4A4A4A) : 
                             juce::Colour(0xFF2A2A2A);
            }
            
            // Background with light gradient (design system, toned down)
            juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, 3.0f);
            
            // Border for definition
            g.setColour(borderColor);
            g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
            
            // Draw circle with blue half and default font color half
            auto centerX = bounds.getCentreX();
            auto centerY = bounds.getCentreY();
            float circleRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.25f; // 25% of button size
            
            // Get green mode state from look and feel
            bool isGreenMode = false;
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                // Check if we're in green mode by looking at the accent color
                isGreenMode = (lookAndFeel->theme.accent == juce::Colour(0xFF5AA95A));
            }
            
            // Get default font color from look and feel
            juce::Colour defaultColor = juce::Colour(0xFF888888); // Default grey
            if (auto* lookAndFeel2 = dynamic_cast<FieldLNF*>(&this->getLookAndFeel())) {
                defaultColor = lookAndFeel2->theme.text;
            }
            
            // Blue color (match bypass button) or green in green mode
            juce::Colour accentColor = isGreenMode ? juce::Colour(0xFF5AA95A) : juce::Colour(0xFF2196F3);
            
            // Draw the circle
            juce::Rectangle<float> circleBounds(centerX - circleRadius, centerY - circleRadius, 
                                              circleRadius * 2.0f, circleRadius * 2.0f);
            
            if (leftArrow) {
                // Up arrow: accent color on top half, default color on bottom half
                // Top half (accent color)
                g.setColour(accentColor);
                g.fillEllipse(circleBounds.getX(), circleBounds.getY(), 
                             circleBounds.getWidth(), circleBounds.getHeight() * 0.5f);
                
                // Bottom half (default color)
                g.setColour(defaultColor);
                g.fillEllipse(circleBounds.getX(), circleBounds.getCentreY(), 
                             circleBounds.getWidth(), circleBounds.getHeight() * 0.5f);
            } else {
                // Down arrow: default color on top half, accent color on bottom half
                // Top half (default color)
                g.setColour(defaultColor);
                g.fillEllipse(circleBounds.getX(), circleBounds.getY(), 
                             circleBounds.getWidth(), circleBounds.getHeight() * 0.5f);
                
                // Bottom half (accent color)
                g.setColour(accentColor);
                g.fillEllipse(circleBounds.getX(), circleBounds.getCentreY(), 
                             circleBounds.getWidth(), circleBounds.getHeight() * 0.5f);
            }
            
            // Draw circle border
            g.setColour(juce::Colour(0xFF2A2A2A));
            g.drawEllipse(circleBounds, 1.0f);
        }
        
    private:
        bool leftArrow;
    };
    
    class CopyButton : public juce::TextButton
    {
    public:
        CopyButton() : juce::TextButton("") {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw raised shadow effect for depth
            g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
            g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 3.0f);
            
            // Background with light gradient (design system, toned down)
            juce::Colour base = juce::Colour(0xFF3A3D45);
            juce::Colour top  = base.brighter(0.10f);
            juce::Colour bot  = base.darker(0.10f);
            juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, 3.0f);
            
            // Border for definition
            g.setColour(isButtonDown ? juce::Colour(0xFF2A2A2A) : 
                       isMouseOver ? juce::Colour(0xFF4A4A4A) : 
                       juce::Colour(0xFF2A2A2A));
            g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
            
            // Get green mode state from look and feel
            bool isGreenMode = false;
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                // Check if we're in green mode by looking at the accent color
                isGreenMode = (lookAndFeel->theme.accent == juce::Colour(0xFF5AA95A));
            }
            
            // Copy icon (two overlapping rectangles) - grey color or green in green mode
            juce::Colour iconColor = isGreenMode ? juce::Colour(0xFF5AA95A) : juce::Colour(0xFF888888);
            g.setColour(iconColor);
            float scale = bounds.getWidth() / 16.0f;
            
            // Source document
            g.fillRect(bounds.getX() + 3 * scale, bounds.getY() + 5 * scale, 6 * scale, 8 * scale);
            g.setColour(juce::Colour(0xFF3A3A3A));
            g.fillRect(bounds.getX() + 4 * scale, bounds.getY() + 6 * scale, 4 * scale, 6 * scale);
            
            // Copy document (offset)
            g.setColour(iconColor);
            g.fillRect(bounds.getX() + 7 * scale, bounds.getY() + 3 * scale, 6 * scale, 8 * scale);
            g.setColour(juce::Colour(0xFF3A3A3A));
            g.fillRect(bounds.getX() + 8 * scale, bounds.getY() + 4 * scale, 4 * scale, 6 * scale);
        }
    };
    
    class LockButton : public juce::TextButton
    {
    public:
        LockButton() : juce::TextButton("") {}
        
        void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw raised shadow effect for depth
            g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
            g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 4.0f);
            
            // Background
            g.setColour(isButtonDown ? juce::Colour(0xFF4A4A4A) : 
                       isMouseOver ? juce::Colour(0xFF5A5A5A) : 
                       juce::Colour(0xFF3A3A3A));
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Border for definition
            g.setColour(isButtonDown ? juce::Colour(0xFF2A2A2A) : 
                       isMouseOver ? juce::Colour(0xFF4A4A4A) : 
                       juce::Colour(0xFF2A2A2A));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
            
            // Lock/Unlock icon based on state
            auto iconType = getToggleState() ? IconSystem::Lock : IconSystem::Unlock;
            juce::Colour iconColour;
            if (auto* lookAndFeel = dynamic_cast<FieldLNF*>(&getLookAndFeel())) {
                iconColour = getToggleState() ? lookAndFeel->theme.accent : juce::Colour(0xFF7A7D85);
            } else {
                iconColour = getToggleState() ? juce::Colour(0xFF5AA9E6) : juce::Colour(0xFF7A7D85);
            }
            IconSystem::drawIcon(g, iconType, bounds.reduced(4.0f), iconColour);
        }
    };
    
    // XY Pad
    XYPad pad;
    
    // Preset system
    PresetManager presetManager;
    
        // Control Containers
    ControlContainer mainControlsContainer, volumeContainer, eqContainer;
    ControlContainer spaceKnobContainer, panKnobContainer;
    
    // Waveform Display
    // WaveformDisplay removed - now integrated into XYPad background
    
    // Visual feedback colors

    
    // Numerical indicators (recessed text fields)
    juce::Label leftIndicator, rightIndicator;
    juce::Label gainValue, widthValue, tiltValue, monoValue, hpValue, lpValue, satDriveValue, satMixValue, airValue, bassValue, scoopValue; // Added scoopValue
    juce::Label panValue, panValueLeft, panValueRight, spaceValue, duckingValue;  // Main user control values
    
    // NEW: Frequency control value labels
    juce::Label tiltFreqValue, scoopFreqValue, bassFreqValue, airFreqValue;
    
    // Text labels for knobs
    juce::Label gainL, widthL, tiltL, monoL, hpL, lpL, satDriveL, satMixL;
    juce::Label panL, spaceL, duckingL;  // Main user control labels
    
    // Locks removed
    
    // Parameter attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    
    // Scaling
    float scaleFactor = 1.0f;
    const int baseWidth = 1500;  // Increased width for better proportions
    const int baseHeight = 1000; // Reduced height for better aspect ratio
    const int standardKnobSize = 80; // Increased minimum knob size
    
    // Helper functions
    void styleSlider (juce::Slider& s);
    void styleMainSlider (juce::Slider& s);
    void updateParameterLocks();
    // applyPreset function removed - now using PresetManager system
    void drawRecessedLabel (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text, bool isActive = true);
    void drawKnobWithIntegratedValue (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& knobName, const juce::String& value, bool isActive = true);
    
    // Resize handle
    bool isResizing = false;
    juce::Point<int> resizeStart;
    juce::Rectangle<int> originalBounds;
    
    // Full screen state
    juce::Rectangle<int> savedBounds;

    // A/B System components
    ABButton abButtonA{true}, abButtonB{false};
    CopyButton copyButton;
    PresetArrowButton prevPresetButton{true}, nextPresetButton{false};
    // Split-pan container placeholder for grid cell (no painting, no mouse)
    juce::Component panSplitContainer;
    
    // A/B state storage
    std::map<juce::String, float> stateA, stateB;
    bool isStateA = true;
    bool isGreenMode = false; // Color mode state
    std::map<juce::String, float> clipboardState; // For copy/paste functionality
    int currentAlgorithm = 0; // 0=Inner, 1=Outer, 2=Deep
    juce::String presetNameA = "Default", presetNameB = "Default"; // Store preset names for A/B states
    
    // A/B System functions
    void saveCurrentState();
    void loadState(bool loadStateA);
    void toggleABState();
    void copyState(bool copyFromA);
    void pasteState(bool pasteToA);
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void updatePresetDisplay(); // Update dropdown to show current preset name
    
    // Header hover system
    bool headerHovered = false;
    bool headerHoverActive = false;
    const int headerHoverOffDelayMs = 160;

    // Cached layout for dividers
    juce::Rectangle<int> dividerVolBounds;

    // Global cursor policy
    void applyGlobalCursorPolicy();
    void childrenChanged() override { juce::AudioProcessorEditor::childrenChanged(); applyGlobalCursorPolicy(); }

    // Shade overlay for XYPad (block-vision control)
    class ShadeOverlay : public juce::Component, private juce::Timer
    {
    public:
        explicit ShadeOverlay (FieldLNF& lnfRef) : lnf(lnfRef)
        {
            setAlwaysOnTop(true);
            setInterceptsMouseClicks(true, true);
            amount.reset(0.0, 0.12);
            amount.setCurrentAndTargetValue(0.0f);
            startTimerHz(60);
        }

        void setAmount (float a, bool animate = true)
        {
            a = juce::jlimit(0.f, 1.f, a);
            animate ? amount.setTargetValue(a)
                    : amount.setCurrentAndTargetValue(a);
            if (onAmountChanged) onAmountChanged(getAmount());
            repaint();
        }
        float getAmount() const { return amount.getCurrentValue(); }
        void toggle(bool animate = true) { setAmount(getAmount() > 0.5f ? 0.f : 1.f, animate); }

        std::function<void(float)> onAmountChanged;

        bool hitTest (int x, int y) override
        {
            auto edge = juce::jlimit (0.0f, (float) getHeight(), shadeEdgeY());
            if (y <= edge) return true; // covered area blocks
            // allow interaction on the handle even when fully open
            return getHandle().contains ((float) x, (float) y);
        }

        void paint (juce::Graphics& g) override
        {
            const auto r = getLocalBounds().toFloat();
            const float coveredH = r.getHeight() * getAmount();
            const auto cover = r.withHeight(coveredH);

            if (coveredH > 0.001f)
            {
                g.setColour(lnf.theme.panel.withAlpha(0.92f));
                g.fillRect(cover);

                g.setColour(lnf.theme.sh.withAlpha(0.07f));
                for (int yy = 0; yy < (int)coveredH; yy += 3)
                    g.drawHorizontalLine(yy, cover.getX(), cover.getRight());

                g.setColour(lnf.theme.sh.withAlpha(0.85f));
                g.fillRect(juce::Rectangle<float>(cover.getX(), cover.getBottom()-1.0f, cover.getWidth(), 1.0f));
                juce::DropShadow(juce::Colours::black.withAlpha(0.5f), 12, {0,2})
                    .drawForRectangle(g, cover.getSmallestIntegerContainer());
            }

            // Always draw handle so users can discover the control even when open
            drawHandle(g, getHandle());
        }

        void mouseDown (const juce::MouseEvent& e) override { dragStartY = e.y; startAmt = amount.getTargetValue(); }
        void mouseDrag (const juce::MouseEvent& e) override
        {
            const float dy = (float)(e.y - dragStartY);
            setAmount(juce::jlimit(0.f, 1.f, startAmt + dy / (float)getHeight()));
        }
        void mouseDoubleClick (const juce::MouseEvent&) override { toggle(); }
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wd) override
        {
            setAmount(juce::jlimit(0.f, 1.f, getAmount() - wd.deltaY * 0.5f));
        }

    private:
        FieldLNF& lnf;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amount;
        int   dragStartY = 0;
        float startAmt   = 0.f;

        float shadeEdgeY () const { return (float)getHeight() * getAmount(); }

        juce::Rectangle<float> getHandle () const
        {
            auto r = getLocalBounds().toFloat();
            float tabW = juce::jmin(120.0f, r.getWidth() * 0.6f);
            float tabH = 22.0f;
            float edge = juce::jlimit (0.0f, r.getHeight(), shadeEdgeY());
            float y = juce::jlimit (0.0f + tabH * 0.5f, r.getHeight() - tabH * 0.5f, edge);
            return { r.getCentreX() - tabW * 0.5f, y - tabH * 0.5f, tabW, tabH };
        }

        void drawHandle (juce::Graphics& g, juce::Rectangle<float> tab) const
        {
            g.setColour(lnf.theme.base);
            g.fillRoundedRectangle(tab, 8.0f);
            g.setColour(lnf.theme.hl.withAlpha(0.6f));
            g.drawRoundedRectangle(tab, 8.0f, 1.0f);

            // Center four grip dashes within the tab
            const int numBars = 4;
            const float barW = 10.0f;
            const float barH = 6.0f;
            const float gap  = 14.0f;
            const float totalW = numBars * barW + (numBars - 1) * gap;
            float startX = tab.getCentreX() - totalW * 0.5f;
            float y = tab.getCentreY() - barH * 0.5f;

            g.setColour(lnf.theme.textMuted);
            for (int i = 0; i < numBars; ++i)
            {
                juce::Rectangle<float> r (startX + i * (barW + gap), y, barW, barH);
                g.fillRoundedRectangle(r, 2.0f);
            }
        }

        void timerCallback() override
        {
            if (amount.isSmoothing()) repaint();
        }
    };

    std::unique_ptr<ShadeOverlay> xyShade;

    // Small vertical dividers near split toggle
    class VerticalDivider : public juce::Component {
    public:
        VerticalDivider(FieldLNF& l) : lnf(l) {}
        void paint(juce::Graphics& g) override {
            g.fillAll(juce::Colours::transparentBlack);
            g.setColour(lnf.theme.hl.withAlpha(0.6f));
            auto b = getLocalBounds().toFloat();
            g.drawLine(b.getCentreX(), b.getY(), b.getCentreX(), b.getBottom(), 1.0f);
        }
    private:
        FieldLNF& lnf;
    };

    VerticalDivider splitDivider{lnf};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessorEditor)
}; 