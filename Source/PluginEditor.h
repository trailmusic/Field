#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "FieldLookAndFeel.h"
#include "IconSystem.h"
#include "PresetSystem.h"

/*==============================================================================
    DEV NOTES – OVERVIEW
    - This header keeps your visual design as-is while removing duplication.
    - Rotary drawing is centralized via ui::paintRotaryWithLNF.
    - Icon-style buttons share a single base: ThemedIconButton (consistent states).
    - “Green mode” is inferred from FieldLNF::theme.accent instead of per-control flags.
    - XYPad public API is preserved to avoid .cpp breakage.
    - Attachment aliases reduce type noise.
==============================================================================*/

//------------------------------------------------------------------------------
// Shared UI helpers
//------------------------------------------------------------------------------
namespace ui
{
    // DEV NOTE: One-line helper so all rotaries render identically through FieldLNF.
    inline void paintRotaryWithLNF (juce::Graphics& g, juce::Slider& s, juce::Rectangle<float> bounds)
    {
        if (auto* lf = dynamic_cast<FieldLNF*>(&s.getLookAndFeel()))
        {
            const double minV = s.getMinimum();
            const double maxV = s.getMaximum();
            const float  pos01 = (maxV > minV) ? (float) ((s.getValue() - minV) / (maxV - minV)) : 0.0f;

            lf->drawRotarySlider(g, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                 pos01,
                                 juce::MathConstants<float>::pi,
                                 juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                                 s);
        }
    }
}

//------------------------------------------------------------------------------
// Attachment aliases (cuts down type verbosity)
//------------------------------------------------------------------------------
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

//------------------------------------------------------------------------------
// WaveformDisplay class removed - functionality integrated into XYPad background
//------------------------------------------------------------------------------

//==============================================================================
// XYPad (kept API to avoid .cpp breakage)
// DEV NOTE: Hover timer retained; if you want instant hover, remove Timer and
//   update mouseEnter/Exit to just repaint immediately.
//==============================================================================
class XYPad : public juce::Component, public juce::Timer
{
public:
    std::function<void (float x01, float y01)> onChange; // x=pan, y=depth
    std::function<void (float leftX01, float rightX01, float y01)> onSplitChange; // for split mode
    std::function<void (int ballIndex, float x01, float y01)> onBallChange; // individual ball control
    
    void setPoint01 (float x, float y) { pt = { juce::jlimit(0.f,1.f,x), juce::jlimit(0.f,1.f,y) }; repaint(); }
    void setSplitPoints (float leftX, float rightX, float y) { leftPt = leftX; rightPt = rightX; pt.second = y; repaint(); }
    void setBallPosition (int ballIndex, float x, float y); // optional: only keep if used in .cpp
    std::pair<float,float> getPoint01() const { return pt; }
    std::pair<float,float> getSplitPoints() const { return {leftPt, rightPt}; }
    std::pair<float,float> getBallPosition (int ballIndex) const; // optional: only keep if used
    
    void setSplitMode (bool split)
    { 
        isSplitMode = split; 
        if (split && isLinked) { leftPt = 0.5f; rightPt = 0.5f; }
        repaint(); 
    }
    bool getSplitMode() const { return isSplitMode; }
    
    void setLinked (bool linked)
    { 
        isLinked = linked; 
        if (linked && isSplitMode) { leftPt = 0.5f; rightPt = 0.5f; }
        repaint(); 
    }
    bool getLinked() const { return isLinked; }
    
    // Visual links (XYPad paints reactively; these are “hints” from parameters)
    void setGainValue (float gainDb)       { gainValue  = gainDb; repaint(); }
    void setWidthValue (float widthPercent){ widthValue = widthPercent; repaint(); }
    void setTiltValue (float tiltDegrees)  { tiltValue  = tiltDegrees; repaint(); }
    void setMixValue (float mix01)         { mixValue   = mix01; repaint(); }
    void setDriveValue (float driveDb)     { driveValue = driveDb; repaint(); }
    void setAirValue (float airDb)         { airValue   = airDb; repaint(); }
    void setBassValue (float bassDb)       { bassValue  = bassDb; repaint(); }
    void setScoopValue (float scoopDb)     { scoopValue = scoopDb; repaint(); } // scoop EQ
    void setHPValue (float hpHz)           { hpValue    = hpHz; repaint(); }
    void setLPValue (float lpHz)           { lpValue    = lpHz; repaint(); }
    void setPanValue (float pan)           { panValue   = pan; repaint(); }
    void setMonoValue (float monoHz)       { monoHzValue= monoHz; repaint(); }
    void setSpaceValue (float depth)       { spaceValue = depth; repaint(); }
    void setSpaceAlgorithm (int algorithm) { spaceAlgorithm = algorithm; repaint(); }
    void setGreenMode (bool enabled)       { isGreenMode = enabled; repaint(); } // kept for .cpp compatibility
    
    // Frequency controls for EQ viz
    void setTiltFreqValue (float f)  { tiltFreqValue  = f; repaint(); }
    void setScoopFreqValue (float f) { scoopFreqValue = f; repaint(); }
    void setBassFreqValue (float f)  { bassFreqValue  = f; repaint(); }
    void setAirFreqValue (float f)   { airFreqValue   = f; repaint(); }
    
    void pushWaveformSample (double sampleL, double sampleR); // for background waveform
    void setSampleRate (double fs) { vizSampleRate = fs > 0.0 ? fs : 48000.0; }

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
    float leftPt = 0.5f, rightPt = 0.5f; // Start both balls centered
    bool isSplitMode = false;            // single ball by default
    bool isLinked = true;                // default linked mode
    int  activeBall = 0;                 // 0=center, 1=left, 2=right
    bool snapEnabled = false;
    bool hoverActive = false;
    const int hoverOffDelayMs = 160;

    // Visual state mirrors of params
    float gainValue = 0.0f;
    float widthValue = 50.0f;
    float tiltValue = 0.0f;
    float mixValue = 0.5f;
    float driveValue = 0.0f;
    float airValue = 0.0f;
    float bassValue = 0.0f;
    float scoopValue = 0.0f;
    float hpValue = 20.0f;
    float lpValue = 20000.0f;
    float panValue = 0.0f;
    float monoHzValue = 0.0f;
    float spaceValue = 0.0f;
    int   spaceAlgorithm = 0; // 0=Inner, 1=Outer, 2=Deep
    bool  isGreenMode = false; // kept for compatibility (XYPad paint may read this)
    
    // EQ frequency positions (for drawing only)
    float tiltFreqValue = 500.0f;
    float scoopFreqValue = 800.0f;
    float bassFreqValue = 150.0f;
    float airFreqValue = 8000.0f;
    
    // Waveform buffer
    static constexpr int waveformBufferSize = 512;
    std::array<double, waveformBufferSize> waveformL{}, waveformR{};
    int  waveformWriteIndex = 0;
    bool hasWaveformData = false;
    double vizSampleRate = 48000.0; // for EQ magnitude rendering
    
    // internals
    void drag (const juce::MouseEvent& e);
    void drawGrid (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawBalls (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawWaveformBackground (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawEQCurves (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawFrequencyRegions (juce::Graphics& g, juce::Rectangle<float> bounds);
    void analyzeSpectralResponse (std::vector<float>& response, float width); // optional; keep if used
    int  getBallAtPosition (juce::Point<float> pos, juce::Rectangle<float> bounds);
};

//==============================================================================
// ControlContainer (kept hover timer; purely cosmetic “soft fade” on hover)
//==============================================================================
class ControlContainer : public juce::Component, public juce::Timer
{
public:
    ControlContainer();
    
    void setTitle (const juce::String& title);
    void setShowBorder (bool show) { showBorder = show; }
    void paint (juce::Graphics& g) override;
    void mouseEnter (const juce::MouseEvent&) override { hovered = true;  hoverActive = true; stopTimer(); repaint(); }
    void mouseExit  (const juce::MouseEvent&) override { hovered = false; startTimer (hoverOffDelayMs); }
    void timerCallback() override { hoverActive = false; stopTimer(); repaint(); }
    
private:
    juce::String containerTitle;
    bool hovered = false;
    bool hoverActive = false;
    bool showBorder = true;
    const int hoverOffDelayMs = 160;
};

//==============================================================================
// ToggleSwitch (kept smoothing for handle; hover timer only for subtle fade)
//==============================================================================
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

//==============================================================================
// BypassButton (kept blink behavior, reads theme accent automatically)
//==============================================================================
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
        if (getToggleState())
            repaint(); // Trigger repaint for blinking effect
    }
    
private:
    class CustomLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
            
            // Read color mode from LNF accent 
            juce::Colour accent = juce::Colour(0xFF2196F3);
            if (auto* lf = dynamic_cast<FieldLNF*>(&button.getLookAndFeel()))
                accent = lf->theme.accent;

            juce::Colour baseColour;
            if (button.getToggleState())
            {
                baseColour = juce::Colour(0xFFE53935); // red when bypassed
                auto now = juce::Time::getMillisecondCounter();
                if ((now / 250) % 2 == 0) baseColour = baseColour.darker(0.3f); // blink
                g.setColour(juce::Colour(0x40FFEB3B)); // glow
                g.fillRoundedRectangle(bounds.expanded(4.0f), 6.0f);
            }
            else
            {
                baseColour = accent;
            }

            if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
                baseColour = baseColour.brighter(0.1f);
            
            // shadow
            g.setColour(juce::Colour(0x40000000));
            g.fillRoundedRectangle(bounds.translated(2.0f, 2.0f), 4.0f);
            
            // bg + border
            g.setColour(baseColour);
            g.fillRoundedRectangle(bounds, 4.0f);
            g.setColour(baseColour.darker(0.3f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        }
        
        void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                            bool, bool) override
        {
            auto bounds = button.getLocalBounds().toFloat();
            juce::Colour iconColour = button.getToggleState() ? juce::Colours::white
                                                              : juce::Colour(0xFF1565C0);
            if (auto* lf = dynamic_cast<FieldLNF*>(&button.getLookAndFeel()))
                if (!button.getToggleState()) iconColour = lf->theme.textMuted;

            IconSystem::drawIcon(g, IconSystem::Power, bounds.reduced(4.0f), iconColour);
        }
    };
    
    CustomLookAndFeel customLookAndFeel;
};

//==============================================================================
// ThemedIconButton – base for all small icon buttons (Options/Link/Snap/etc.)
// DEV NOTE: This consolidates shared drawing (panel gradient, accent-on, borders).
//==============================================================================
class ThemedIconButton : public juce::TextButton
{
public:
    enum class Style { SolidAccentWhenOn, GradientPanel };
    struct Options
    {
        IconSystem::IconType icon = IconSystem::CogWheel;
        bool toggleable = false;
        Style style = Style::GradientPanel;
        float corner = 4.0f;
        float pad = 4.0f;
        bool glowWhenOn = false;
    };

    explicit ThemedIconButton (Options o) : options(std::move(o))
    {
        setClickingTogglesState(options.toggleable);
        setButtonText(juce::String());
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        drawBackground(g, r, over, down);
        drawIcon(g, r.reduced(options.pad));
    }

protected:
    Options options;

    void drawBackground (juce::Graphics& g, juce::Rectangle<float> r, bool over, bool down)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto panel = lf ? lf->theme.panel : juce::Colour(0xFF3A3D45);
        auto accent = lf ? lf->theme.accent : juce::Colour(0xFF2196F3);
        auto sh = lf ? lf->theme.sh : juce::Colour(0xFF2A2A2A);
        auto hl = lf ? lf->theme.hl : juce::Colour(0xFF4A4A4A);

        const bool on = getToggleState();

        auto drawGradient = [&] {
            juce::Colour top = panel.brighter(0.10f), bot = panel.darker(0.10f);
            juce::ColourGradient grad(top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(r, options.corner);
            g.setColour(down ? sh : (over ? hl : sh));
            g.drawRoundedRectangle(r, options.corner, 1.0f);
        };

        if (options.style == Style::SolidAccentWhenOn && on)
        {
            auto bg = down ? accent.darker(0.30f) : (over ? accent.brighter(0.10f) : accent);
            g.setColour(bg);
            g.fillRoundedRectangle(r, options.corner);
            g.setColour(bg.darker(0.30f));
            g.drawRoundedRectangle(r, options.corner, 1.0f);
            if (options.glowWhenOn)
                g.setColour(bg.withAlpha(0.30f)), g.drawRoundedRectangle(r.expanded(1.0f), options.corner+1.0f, 2.0f);
        }
        else
        {
            drawGradient();
        }

        // subtle elevation shadow
        g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(r.translated(1.5f, 1.5f), options.corner);
    }

    void drawIcon (juce::Graphics& g, juce::Rectangle<float> r)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto textMuted = lf ? lf->theme.textMuted : juce::Colour(0xFF888888);
        auto onCol = juce::Colours::white;
        const bool on = getToggleState();

        IconSystem::drawIcon(g, options.icon, r, on ? onCol : textMuted);
    }
};

//------------------------------------------------------------------------------
// Concrete icon buttons (tiny classes = tiny maintenance)
//------------------------------------------------------------------------------
class OptionsButton    : public ThemedIconButton { public: OptionsButton()
: ThemedIconButton(Options{ IconSystem::CogWheel, false, ThemedIconButton::Style::GradientPanel, 3.0f, 4.0f, false }) {} };

class LinkButton       : public ThemedIconButton { public: LinkButton()
: ThemedIconButton(Options{ IconSystem::Link, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, true }) {} };

class SnapButton       : public ThemedIconButton { public: SnapButton()
: ThemedIconButton(Options{ IconSystem::Snap, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, false }) {} };

class FullScreenButton : public ThemedIconButton
{
public:
    FullScreenButton() : ThemedIconButton(Options{ IconSystem::FullScreen, true, ThemedIconButton::Style::GradientPanel, 3.0f, 4.0f, false }) {}
    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        drawBackground(g, r, over, down);
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto iconColor = lf ? lf->theme.textMuted : juce::Colour(0xFF888888);
        auto icon = getToggleState() ? IconSystem::ExitFullScreen : IconSystem::FullScreen;
        IconSystem::drawIcon(g, icon, r.reduced(4.0f), iconColor);
    }
};

class ColorModeButton  : public ThemedIconButton { public: ColorModeButton()
: ThemedIconButton(Options{ IconSystem::ColorPalette, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, false }) {} };

class CopyButton       : public ThemedIconButton { public: CopyButton()
: ThemedIconButton(Options{ IconSystem::Save, false, ThemedIconButton::Style::GradientPanel, 3.0f, 4.0f, false }) {} };

class LockButton       : public ThemedIconButton
{
public:
    LockButton() : ThemedIconButton(Options{ IconSystem::Lock, true, ThemedIconButton::Style::GradientPanel, 4.0f, 4.0f, false }) {}
    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        drawBackground(g, r, over, down);
        auto icon = getToggleState() ? IconSystem::Lock : IconSystem::Unlock;
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto col = getToggleState() ? (lf ? lf->theme.accent : juce::Colour(0xFF5AA9E6))
                                    : (lf ? lf->theme.textMuted : juce::Colour(0xFF888888));
        IconSystem::drawIcon(g, icon, r.reduced(4.0f), col);
    }
};

// NOTE: PresetArrowButton kept custom drawing (half-circle motif) to preserve your unique look
class PresetArrowButton : public juce::TextButton
{
public:
    explicit PresetArrowButton(bool isLeft) : juce::TextButton(""), leftArrow(isLeft) {}

    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // shadow
        g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 3.0f);

        // panel gradient
        juce::Colour base = juce::Colour(0xFF3A3D45);
        juce::Colour top  = base.brighter(0.10f);
        juce::Colour bot  = base.darker(0.10f);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            base = lf->theme.panel; top = base.brighter(0.10f); bot = base.darker(0.10f);
        }
        juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 3.0f);

        // border
        auto borderColor = juce::Colour(0xFF2A2A2A);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            borderColor = isButtonDown ? lf->theme.sh : (isMouseOver ? lf->theme.hl : lf->theme.sh);
        g.setColour(borderColor);
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        // half-circle motif
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto accent = lf ? lf->theme.accent : juce::Colour(0xFF2196F3);
        auto defaultColor = lf ? lf->theme.text : juce::Colour(0xFFF0F2F5);

        auto c = bounds.getCentre();
        float r = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.25f;
        juce::Rectangle<float> circle (c.x - r, c.y - r, 2*r, 2*r);

        if (leftArrow)
        {
            g.setColour(accent);       g.fillEllipse(circle.getX(), circle.getY(), circle.getWidth(), circle.getHeight() * 0.5f);       // top
            g.setColour(defaultColor); g.fillEllipse(circle.getX(), circle.getCentreY(), circle.getWidth(), circle.getHeight() * 0.5f); // bottom
        }
        else
        {
            g.setColour(defaultColor); g.fillEllipse(circle.getX(), circle.getY(), circle.getWidth(), circle.getHeight() * 0.5f);       // top
            g.setColour(accent);       g.fillEllipse(circle.getX(), circle.getCentreY(), circle.getWidth(), circle.getHeight() * 0.5f); // bottom
        }

        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawEllipse(circle, 1.0f);
    }

private:
    bool leftArrow;
};

//==============================================================================
// Editor
//==============================================================================
class MyPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     public juce::Timer,
                                     public juce::Slider::Listener,
                                     public juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit MyPluginAudioProcessorEditor (MyPluginAudioProcessor&);
    ~MyPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sliderValueChanged(juce::Slider* slider) override;

    // Header hover (kept timer; cosmetic)
    void mouseEnter (const juce::MouseEvent& e) override
    { 
        auto headerBounds = getLocalBounds().removeFromTop(static_cast<int>(60 * scaleFactor));
        if (headerBounds.contains(e.position.toInt()))
        {
            headerHovered = true; 
            headerHoverActive = true; 
            stopTimer(); 
            repaint(); 
        }
    }
    void mouseExit  (const juce::MouseEvent& e) override
    { 
        auto headerBounds = getLocalBounds().removeFromTop(static_cast<int>(60 * scaleFactor));
        if (!headerBounds.contains(e.position.toInt()))
        {
            headerHovered = false; 
            startTimer (headerHoverOffDelayMs); 
        }
    }
    
    void setScaleFactor (float newScale) override;
    
    // Waveform is now drawn behind XYPad; no explicit push from editor
    void syncXYPadWithParameters();
    void setupTooltips();
    
    //--- custom sliders --------------------------------------------------------
    class GainSlider : public juce::Slider
    {
    public:
        GainSlider() : Slider(RotaryHorizontalVerticalDrag, NoTextBox) {}
        void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
        void mouseExit  (const juce::MouseEvent&) override { hovered = false; repaint(); }
        void mouseDown  (const juce::MouseEvent& e) override { active = true;  Slider::mouseDown(e); repaint(); }
        void mouseUp    (const juce::MouseEvent& e) override { active = false; Slider::mouseUp(e);   repaint(); }

        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced(2.0f);
            if (hovered || active) b = b.expanded(2.0f);
            ui::paintRotaryWithLNF(g, *this, b);
        }
    private:
        bool hovered = false, active = false;
    };
    
    class PanSlider : public juce::Slider
    {
    public:
        PanSlider() : Slider(RotaryHorizontalVerticalDrag, NoTextBox) {}
        void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
        void mouseExit  (const juce::MouseEvent&) override { hovered = false; repaint(); }
        void mouseDown  (const juce::MouseEvent& e) override { active = true;  Slider::mouseDown(e); repaint(); }
        void mouseUp    (const juce::MouseEvent& e) override { active = false; Slider::mouseUp(e);   repaint(); }

        void setSplitPercentage(float leftPercent, float rightPercent) { splitLeftPercent = leftPercent; splitRightPercent = rightPercent; repaint(); }
        void setLabel(const juce::String& label) { knobLabel = label; repaint(); }
        void setOverlayEnabled (bool enabled) { overlayEnabled = enabled; repaint(); }
        
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            if (hovered || active) bounds = bounds.expanded(2.0f);
            
            // base rotary
            ui::paintRotaryWithLNF(g, *this, bounds);
            
            if (overlayEnabled)
            {
                // current pan indicator arc
                const float normalizedValue = (getValue() + 1.0f) * 0.5f; // -1..1 -> 0..1
                const float borderThickness = 3.0f;
                juce::Path valueBorder;
                const float valueAngle = juce::jmap(normalizedValue, 0.0f, 1.0f, 
                                            juce::MathConstants<float>::pi, 
                                            juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
                valueBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                 juce::MathConstants<float>::pi, valueAngle, true);
                g.setColour(juce::Colour(0xFF5AA9E6).withAlpha(0.8f)); // blue accent
                g.strokePath(valueBorder, juce::PathStrokeType(borderThickness));
            }
            
            // split arcs (L: blue, R: red)
            if (overlayEnabled && splitLeftPercent >= 0.0f && splitRightPercent >= 0.0f)
            {
                const float borderThickness = 3.0f;

                juce::Path leftBorder;
                const float leftAngle = juce::jmap(splitLeftPercent, 0.0f, 100.0f, 
                                           juce::MathConstants<float>::pi, 
                                           juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
                leftBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                 juce::MathConstants<float>::pi, leftAngle, true);
                g.setColour(juce::Colour(0xFF5AA9E6).withAlpha(0.8f));
                g.strokePath(leftBorder, juce::PathStrokeType(borderThickness));
                
                juce::Path rightBorder;
                const float rightAngle = juce::jmap(splitRightPercent, 0.0f, 100.0f, 
                                            juce::MathConstants<float>::pi, 
                                            juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
                rightBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                  leftAngle, rightAngle, true);
                g.setColour(juce::Colour(0xFFFF6B6B).withAlpha(0.8f));
                g.strokePath(rightBorder, juce::PathStrokeType(borderThickness));
            }
            
            if (knobLabel.isNotEmpty())
            {
                g.setColour(juce::Colour(0xFFF0F2F5));
                g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
                g.drawText(knobLabel, bounds, juce::Justification::centred);
            }
        }
        
    private:
        float splitLeftPercent = -1.0f;  // -1 = not in split mode
        float splitRightPercent = -1.0f;
        bool hovered = false, active = false, overlayEnabled = false;
        juce::String knobLabel;
    };
    
    class DuckingSlider : public juce::Slider
    {
    public:
        DuckingSlider() : Slider(RotaryHorizontalVerticalDrag, NoTextBox) {}
        void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
        void mouseExit  (const juce::MouseEvent&) override { hovered = false; repaint(); }
        void mouseDown  (const juce::MouseEvent& e) override { active = true;  Slider::mouseDown(e); repaint(); }
        void mouseUp    (const juce::MouseEvent& e) override { active = false; Slider::mouseUp(e);   repaint(); }

        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced(4.0f); // smaller footprint
            if (hovered || active) b = b.expanded(1.5f);
            ui::paintRotaryWithLNF(g, *this, b);
        }
    private:
        bool hovered = false, active = false;
    };
    
    // Resize handle functionality
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

private:
    MyPluginAudioProcessor& proc;
    FieldLNF lnf;
    XYPad pad;
    
    // UI Components
    GainSlider   gain;
    juce::Slider width, tilt, monoHz, hpHz, lpHz, satDrive, satMix, air, bass, scoop; // includes Scoop
    juce::ComboBox  monoSlopeChoice;
    juce::ToggleButton monoAuditionButton;
    // Imaging controls
    juce::Slider widthLo, widthMid, widthHi;
    juce::Slider xoverLoHz, xoverHiHz;
    juce::Slider rotationDeg, asymmetry;
    juce::Slider shufLoPct, shufHiPct, shufXHz;
    PanSlider    panKnob;
    PanSlider    panKnobLeft, panKnobRight;  // split mode pan
    DuckingSlider duckingKnob;
    juce::ComboBox osSelect;

    PresetComboBox   presetCombo;
    SavePresetButton savePresetButton;
    BypassButton     bypassButton;
    ToggleSwitch     splitToggle;
    
    // Frequency control sliders
    juce::Slider tiltFreqSlider, scoopFreqSlider, bassFreqSlider, airFreqSlider;
    
    // Icon buttons (shared base)
    OptionsButton    optionsButton;
    LinkButton       linkButton;
    SnapButton       snapButton;
    FullScreenButton fullScreenButton;
    ColorModeButton  colorModeButton;
    CopyButton       copyButton;
    LockButton       lockButton;

    // Space controls
    class SpaceKnob : public juce::Slider
    {
    public:
        SpaceKnob() : juce::Slider(RotaryHorizontalVerticalDrag, NoTextBox) {}
        void setGreenMode (bool) {}
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            ui::paintRotaryWithLNF(g, *this, bounds);
        }
    };
    SpaceKnob spaceKnob;
    
    // 3-way algorithm switch (colors derived from current LNF accent)
    class SpaceAlgorithmSwitch : public juce::Component
    {
    public:
        SpaceAlgorithmSwitch() = default; // 0=Inner, 1=Outer, 2=Deep
        void setGreenMode (bool) {}
        
        void setAlgorithm(int algorithm)             { currentPosition = juce::jlimit(0, 2, algorithm); repaint(); }
        void setAlgorithmFromParameter(int algorithm){ setAlgorithm(algorithm); }
        int  getAlgorithm() const                    { return currentPosition; }
        
        std::function<void(int)> onAlgorithmChange;
        
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            const float spacing = 6.0f;
            const float availableH = juce::jmax(0.0f, bounds.getHeight() - 2.0f * spacing);
            const float h = availableH / 3.0f;
            
            drawButton(g, {bounds.getX(), bounds.getY(), bounds.getWidth(), h},                            2, "Deep");
            drawButton(g, {bounds.getX(), bounds.getY() + h + spacing, bounds.getWidth(), h},              1, "Outer");
            drawButton(g, {bounds.getX(), bounds.getY() + 2.0f * (h + spacing), bounds.getWidth(), h},     0, "Inner");
        }
        
        void mouseDown(const juce::MouseEvent& e) override
        {
            if (e.mods.isPopupMenu())
            {
                juce::PopupMenu m;
                m.addItem(1, "Inner", true, currentPosition == 0);
                m.addItem(2, "Outer", true, currentPosition == 1);
                m.addItem(3, "Deep",  true, currentPosition == 2);
                m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
                    [this](int result)
                    {
                        if (result > 0) { currentPosition = result - 1; repaint(); if (onAlgorithmChange) onAlgorithmChange(currentPosition); }
                    });
                return;
            }

            const float spacing = 6.0f;
            const float availableH = juce::jmax(0.0f, (float)getHeight() - 2.0f * spacing);
            const float h = availableH / 3.0f;
            const float y = (float)e.y;

            int newPosition = (y <= h) ? 2 : (y <= h * 2 + spacing ? 1 : 0);
            if (newPosition != currentPosition) { currentPosition = newPosition; repaint(); if (onAlgorithmChange) onAlgorithmChange(currentPosition); }
        }
        
    private:
        int currentPosition = 0;

        juce::Colour activeColour (int algorithm) const
        {
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                auto a = lf->theme.accent;
                switch (algorithm) { case 0: return a; case 1: return a.brighter(0.2f); case 2: return a.darker(0.2f); }
            }
            switch (algorithm) { case 0: return juce::Colour(0xFF5AA9E6); case 1: return juce::Colour(0xFF2EC4B6); case 2: return juce::Colour(0xFF2A1B3D); }
            return juce::Colour(0xFF5AA9E6);
        }

        void drawButton(juce::Graphics& g, juce::Rectangle<float> r, int idx, const juce::String& label)
        {
            const bool on = (currentPosition == idx);

            g.setColour(on ? activeColour(idx) : juce::Colour(0xFF2A2C30));
            g.fillRoundedRectangle(r, 6.0f);
            g.setColour(juce::Colour(0xFF1A1C20));
            g.drawRoundedRectangle(r, 6.0f, 1.0f);

            if (on)  { g.setColour(juce::Colour(0x40000000)); g.fillRoundedRectangle(r.reduced(1.0f), 5.0f); }
            else     { g.setColour(juce::Colour(0x20FFFFFF)); g.fillRoundedRectangle(r.reduced(1.0f).translated(-0.5f, -0.5f), 5.0f); }

            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                g.setColour(lf->theme.text);
            else
                g.setColour(juce::Colour(0xFFF0F2F5));

            g.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
            g.drawText(label, r, juce::Justification::centred);
        }
    };
    
    SpaceAlgorithmSwitch spaceAlgorithmSwitch;
    
    // A/B & presets
    class ABButton : public ThemedIconButton
    {
    public:
        explicit ABButton (bool isAButton)
        : ThemedIconButton({ IconSystem::ColorPalette /*unused*/, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, true })
        , isA(isAButton) { setButtonText(isA ? "A" : "B"); }

        void paintButton(juce::Graphics& g, bool over, bool down) override
        {
            auto r = getLocalBounds().toFloat().reduced(2.0f);
            drawBackground(g, r, over, down);
            g.setColour(getToggleState() ? juce::Colours::white : juce::Colour(0xFF888888));
                    g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
            g.drawText(isA ? "A" : "B", r, juce::Justification::centred);
        }
    private:
        bool isA;
    };
    
    ABButton abButtonA{true}, abButtonB{false};
    PresetArrowButton prevPresetButton{true}, nextPresetButton{false};

    // Split-pan container placeholder for grid cell (no painting, no mouse)
    juce::Component panSplitContainer;
    
    // Preset manager
    PresetManager presetManager;
    
    // Containers
    ControlContainer mainControlsContainer, volumeContainer, eqContainer;
    ControlContainer imageContainer, metersContainer;
    ControlContainer spaceKnobContainer, panKnobContainer;
    
    // Value indicators (if you keep them)
    juce::Label leftIndicator, rightIndicator;
    juce::Label gainValue, widthValue, tiltValue, monoValue, hpValue, lpValue, satDriveValue, satMixValue, airValue, bassValue, scoopValue;
    juce::Label monoSlopeName, monoAudName;
    juce::Label panValue, panValueLeft, panValueRight, spaceValue, duckingValue;
    juce::Label tiltFreqValue, scoopFreqValue, bassFreqValue, airFreqValue;
    juce::Label widthLoValue, widthMidValue, widthHiValue, xoverLoValue, xoverHiValue, rotationValue, asymValue, shufLoValue, shufHiValue, shufXValue;
    // Imaging knob name labels (third row)
    juce::Label widthLoName, widthMidName, widthHiName;
    juce::Label xoverLoName, xoverHiName;
    juce::Label rotationName, asymName;
    juce::Label shufLoName, shufHiName, shufXName;
    
    // Text labels
    juce::Label gainL, widthL, tiltL, monoL, hpL, lpL, satDriveL, satMixL;
    juce::Label panL, spaceL, duckingL;

    // Attachments
    std::vector<std::unique_ptr<SliderAttachment>>  attachments;
    std::vector<std::unique_ptr<ButtonAttachment>>  buttonAttachments;
    std::vector<std::unique_ptr<ComboAttachment>>   comboAttachments;
    
    // Scaling
    float scaleFactor = 1.0f;
    const int baseWidth  = 1500;
    const int baseHeight = 1200;
    const int standardKnobSize = 80;
    
    // Helpers
    void styleSlider (juce::Slider& s);
    void styleMainSlider (juce::Slider& s);
    void updateParameterLocks();
    void drawRecessedLabel (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text, bool isActive = true);
    void drawKnobWithIntegratedValue (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& knobName, const juce::String& value, bool isActive = true);
    
    // Resize handle
    bool isResizing = false;
    juce::Point<int> resizeStart;
    juce::Rectangle<int> originalBounds;
    
    // Full screen
    juce::Rectangle<int> savedBounds;

    // Correlation meter mini component
    class CorrelationMeter : public juce::Component, public juce::Timer
    {
    public:
        CorrelationMeter (MyPluginAudioProcessor& p, FieldLNF& l) : proc (p), lnf (l) {}
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            g.setColour (lnf.theme.panel);
            g.fillRoundedRectangle (r, 6.0f);
            g.setColour (lnf.theme.sh);
            g.drawRoundedRectangle (r, 6.0f, 1.0f);

            const float corr = juce::jlimit (-1.0f, 1.0f, proc.getCorrelation());
            const float midX = r.getCentreX();
            const float barY = r.getY() + r.getHeight() * 0.35f;
            const float barH = r.getHeight() * 0.30f;

            // background track
            g.setColour (lnf.theme.hl.withAlpha (0.35f));
            g.fillRoundedRectangle ({ r.getX()+6.0f, barY, r.getWidth()-12.0f, barH }, 4.0f);

            if (corr >= 0.0f)
            {
                const float w = (r.getWidth()-12.0f) * corr * 0.5f;
                g.setColour (juce::Colour (0xFF66BB6A));
                g.fillRoundedRectangle ({ midX, barY, w, barH }, 3.0f);
            }
            else
            {
                const float w = (r.getWidth()-12.0f) * (-corr) * 0.5f;
                g.setColour (juce::Colour (0xFFE57373));
                g.fillRoundedRectangle ({ midX - w, barY, w, barH }, 3.0f);
            }

            g.setColour (lnf.theme.textMuted);
            g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
            g.drawText ("Corr", r.reduced (4).withHeight (16), juce::Justification::centredTop);
        }
        void timerCallback() override { repaint(); }
        void visibilityChanged() override { if (isVisible()) startTimerHz (15); else stopTimer(); }
    private:
        MyPluginAudioProcessor& proc;
        FieldLNF& lnf;
    };

    CorrelationMeter corrMeter { proc, lnf };

    // A/B state
    std::map<juce::String, float> stateA, stateB;
    bool isStateA = true;
    bool isGreenMode = false; // global color mode (your .cpp likely toggles LNF accent)
    std::map<juce::String, float> clipboardState;
    int  currentAlgorithm = 0; // 0=Inner, 1=Outer, 2=Deep
    juce::String presetNameA = "Default", presetNameB = "Default";
    
    // A/B logic
    void saveCurrentState();
    void loadState(bool loadStateA);
    void toggleABState();
    void copyState(bool copyFromA);
    void pasteState(bool pasteToA);
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void updatePresetDisplay();
    
    // Header hover
    bool headerHovered = false;
    bool headerHoverActive = false;
    const int headerHoverOffDelayMs = 160;

    // Cached layout
    juce::Rectangle<int> dividerVolBounds;

    // Cursor policy
    void applyGlobalCursorPolicy();
    void childrenChanged() override { juce::Component::childrenChanged(); applyGlobalCursorPolicy(); }

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
            animate ? amount.setTargetValue(a) : amount.setCurrentAndTargetValue(a);
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

            const int numBars = 4;
            const float barW = 10.0f, barH = 6.0f, gap = 14.0f;
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

    // Mini vertical divider near split toggle
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
    VerticalDivider eqDivLpMono{lnf}, eqDivScoopHp{lnf};
    // Horizontal dividers between rows
    class HorizontalDivider : public juce::Component {
    public:
        HorizontalDivider(FieldLNF& l) : lnf(l) {}
        void paint(juce::Graphics& g) override {
            g.fillAll(juce::Colours::transparentBlack);
            g.setColour(lnf.theme.sh.withAlpha(0.4f));
            auto b = getLocalBounds().toFloat();
            g.fillRect(juce::Rectangle<float>(b.getX(), b.getCentreY()-0.5f, b.getWidth(), 1.0f));
        }
    private:
        FieldLNF& lnf;
    };

    HorizontalDivider rowDivVol{lnf}, rowDivEQ{lnf};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessorEditor)
}; 
