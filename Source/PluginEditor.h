#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "FieldLookAndFeel.h"
#include "KnobCell.h"
#include "KnobCellDual.h"
#include "KnobCellQuad.h"
#include "IconSystem.h"
#include "PresetRegistry.h"
#include "PresetCommandPalette.h"
#include "PresetManager.h"
// MegaMenu and old preset system removed

/*==============================================================================
    DEV NOTES – OVERVIEW
    - This header keeps your visual design as-is while removing duplication.
    - Rotary drawing is centralized via ui::paintRotaryWithLNF.
    - Icon-style buttons share a single base: ThemedIconButton (consistent states).
    - "Green mode" is inferred from FieldLNF::theme.accent instead of per-control flags.
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
        // Guard: avoid degenerate drawing when bounds are tiny/invalid
        if (bounds.getWidth() <= 2.0f || bounds.getHeight() <= 2.0f)
            return;
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
    
    // Visual links (XYPad paints reactively; these are "hints" from parameters)
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
    void setMonoSlopeDbPerOct (int slope)  { monoSlopeDbPerOct = slope; repaint(); }
    void setSpaceValue (float depth)       { spaceValue = depth; repaint(); }
    void setSpaceAlgorithm (int algorithm) { spaceAlgorithm = algorithm; repaint(); }
    void setGreenMode (bool enabled)       { isGreenMode = enabled; repaint(); } // kept for .cpp compatibility
    
    // Frequency controls for EQ viz
    void setTiltFreqValue (float f)  { tiltFreqValue  = f; repaint(); }
    void setScoopFreqValue (float f) { scoopFreqValue = f; repaint(); }
    void setBassFreqValue (float f)  { bassFreqValue  = f; repaint(); }
    void setAirFreqValue (float f)   { airFreqValue   = f; repaint(); }
    
    // EQ S/Q shaping and links for visualization
    void setShelfShapeS (float s)     { shelfShapeS = juce::jlimit (0.25f, 1.50f, s); repaint(); }
    void setQLink       (bool on)     { qLink = on; repaint(); }
    void setFilterQ     (float q)     { filterQGlobal = juce::jlimit (0.5f, 1.2f, q); repaint(); }
    void setHPQ         (float q)     { hpQ = juce::jlimit (0.5f, 1.2f, q); if (!qLink) repaint(); }
    void setLPQ         (float q)     { lpQ = juce::jlimit (0.5f, 1.2f, q); if (!qLink) repaint(); }
    void setTiltUseS    (bool on)     { tiltUsesShelfS = on; repaint(); }

    // Imaging/shuffler overlays state
    void setXoverLoHz   (float hz) { xoverLoHz = juce::jlimit (40.0f, 400.0f, hz); repaint(); }
    void setXoverHiHz   (float hz) { xoverHiHz = juce::jlimit (800.0f, 6000.0f, hz); if (xoverHiHz <= xoverLoHz) xoverHiHz = juce::jlimit (xoverLoHz + 10.0f, 6000.0f, xoverHiHz); repaint(); }
    void setRotationDeg (float d)  { rotationDeg = juce::jlimit (-45.0f, 45.0f, d); repaint(); }
    void setAsymmetry   (float a)  { asym = juce::jlimit (-1.0f, 1.0f, a); repaint(); }
    void setShuffler    (float loPct, float hiPct, float xHz)
    {
        shufLoPct = loPct; shufHiPct = hiPct; shufXHz = juce::jlimit (150.0f, 2000.0f, xHz); repaint();
    }
    
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
    int   monoSlopeDbPerOct = 12;
    float spaceValue = 0.0f;
    int   spaceAlgorithm = 0; // 0=Room, 1=Plate, 2=Hall
    bool  isGreenMode = false; // kept for compatibility (XYPad paint may read this)
    
    // EQ frequency positions (for drawing only)
    float tiltFreqValue = 500.0f;
    float scoopFreqValue = 800.0f;
    float bassFreqValue = 150.0f;
    float airFreqValue = 8000.0f;
    
    // EQ S/Q state for drawing (mirrors APVTS)
    float shelfShapeS    = 0.90f;
    bool  qLink          = true;
    float filterQGlobal  = 0.7071f;
    float hpQ            = 0.7071f;
    float lpQ            = 0.7071f;
    bool  tiltUsesShelfS = true;

    // Imaging/shuffler overlay state (mirrors APVTS)
    float xoverLoHz = 150.0f;
    float xoverHiHz = 2000.0f;
    float rotationDeg = 0.0f;
    float asym = 0.0f; // -1..+1
    float shufLoPct = 100.0f;
    float shufHiPct = 100.0f;
    float shufXHz   = 700.0f;
    
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
    void drawImagingOverlays (juce::Graphics& g, juce::Rectangle<float> bounds);
    void analyzeSpectralResponse (std::vector<float>& response, float width); // optional; keep if used
    int  getBallAtPosition (juce::Point<float> pos, juce::Rectangle<float> bounds);
};

//==============================================================================
// ControlContainer (kept hover timer; purely cosmetic "soft fade" on hover)
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
        setClickingTogglesState(true);
        startTimerHz(20); // High refresh so blink is obvious
    }
    
    ~BypassButton() override
    {
        stopTimer();
        setLookAndFeel(nullptr);
    }
    
    void timerCallback() override
    {
        // Always repaint when visible to drive blink smoothly when toggled
        if (isShowing()) repaint();
    }
    
private:
    class CustomLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
            
            // Read current theme
            juce::Colour accent = juce::Colour(0xFF2196F3);
            juce::Colour textGrey = juce::Colour(0xFFB8BDC7);
            juce::Colour panel = juce::Colour(0xFF3A3D45);
            {
                const juce::Component* c = &button;
                while (c)
                {
                    if (auto* lf = dynamic_cast<FieldLNF*>(&c->getLookAndFeel()))
                    {
                        accent = lf->theme.accent;
                        textGrey = lf->theme.textMuted; // theme font grey
                        panel = lf->theme.panel;
                        break;
                    }
                    c = c->getParentComponent();
                }
            }

            juce::Colour baseColour;
            if (button.getToggleState())
            {
                // When bypassed: use theme font grey for the button body
                baseColour = textGrey;
                auto now = juce::Time::getMillisecondCounter();
                const bool phase = ((now / 250) % 2) == 0;
                // Blink by alternating brightness noticeably
                baseColour = phase ? baseColour.darker(0.35f) : baseColour.brighter(0.05f);
                g.setColour(baseColour.withAlpha(phase ? 0.35f : 0.18f)); // pulsing glow
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
            
            // bg + border with animated stroke width when bypassed
            g.setColour(baseColour);
            g.fillRoundedRectangle(bounds, 4.0f);
            auto now2 = juce::Time::getMillisecondCounter();
            const float pulse = (float) ((now2 / 200) % 2 == 0 ? 2.0f : 1.0f);
            g.setColour(baseColour.darker(0.45f));
            g.drawRoundedRectangle(bounds, 4.0f, pulse);
        }
        
        void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                            bool, bool) override
        {
            auto bounds = button.getLocalBounds().toFloat();
            FieldLNF* lf = nullptr;
            {
                const juce::Component* c = &button;
                while (c)
                {
                    if (auto* l = dynamic_cast<FieldLNF*>(&c->getLookAndFeel())) { lf = l; break; }
                    c = c->getParentComponent();
                }
            }
            juce::Colour textGrey = lf ? lf->theme.textMuted : juce::Colour(0xFFB8BDC7);
            // Ensure the bypassed X is very visible against grey background
            juce::Colour iconColour = button.getToggleState() ? juce::Colours::black : textGrey;

            auto icon = button.getToggleState() ? IconSystem::X : IconSystem::Power;
            auto pad = button.getToggleState() ? 6.0f : 4.0f;
            IconSystem::drawIcon(g, icon, bounds.reduced(pad), iconColour);
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

        const bool invert = (bool) getProperties().getWithDefault ("invertActive", false);
        const bool on = invert ? (! getToggleState()) : getToggleState();

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
        const bool invert = (bool) getProperties().getWithDefault ("invertActive", false);
        const bool on = invert ? (! getToggleState()) : getToggleState();

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

class ColorModeButton  : public ThemedIconButton
{
public:
    ColorModeButton() : ThemedIconButton(Options{ IconSystem::ColorPalette, true, ThemedIconButton::Style::GradientPanel, 4.0f, 4.0f, false }) {}
    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        drawBackground(g, r, over, down);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            IconSystem::drawIcon(g, IconSystem::ColorPalette, r.reduced(4.0f), lf->theme.accent);
        }
        else
        {
            IconSystem::drawIcon(g, IconSystem::ColorPalette, r.reduced(4.0f), juce::Colour(0xFF5AA9E6));
        }
    }
};

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
                                     public juce::ComboBox::Listener,
                                     public juce::Button::Listener,
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
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void buttonClicked(juce::Button* button) override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;

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
    
    bool layoutReady { false };
    
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
            auto b = getLocalBounds().toFloat().reduced(2.0f);
            if (hovered || active) b = b.expanded(1.5f);
            // base rotary
            ui::paintRotaryWithLNF(g, *this, b);

            // Secondary arc for current ducking amount (gain reduction)
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                const float grDb = currentGrDb;                 // 0..~24
                const float maxDb = 24.0f;                      // visualize up to 24 dB
                const float t = juce::jlimit (0.0f, 1.0f, grDb / maxDb);
                if (t > 0.001f)
                {
                    const float start = juce::MathConstants<float>::pi;
                    const float end   = start + juce::MathConstants<float>::twoPi * t;
                    juce::Path arc;
                    auto ring = b.reduced (b.getWidth() * 0.06f);
                    arc.addArc (ring.getX(), ring.getY(), ring.getWidth(), ring.getHeight(), start, end, true);
                    // use a secondary accent (textMuted) for contrast
                    g.setColour (lf->theme.textMuted.withAlpha (0.85f));
                    g.strokePath (arc, juce::PathStrokeType (3.0f));
                }

                // Muted overlay ring to indicate parent (Reverb) off state controlling this knob
                if (muted)
                {
                    auto inner = b.reduced (6.0f); // match rotary painter's reduced bounds for tight ring
                    g.setColour (lf->theme.panel.withAlpha (0.35f));
                    g.fillEllipse (inner);
                    g.setColour (lf->theme.textMuted.withAlpha (0.85f));
                    g.drawEllipse (inner, 1.5f);
                }
            }
        }
        void setCurrentGrDb (float db) { currentGrDb = db; }
        void setMuted (bool m) { muted = m; repaint(); }
    private:
        bool hovered = false, active = false;
        float currentGrDb = 0.0f;
        bool muted = false;
    };

    // Generic duck parameter slider that supports a muted overlay state
    class DuckParamSlider : public juce::Slider
    {
    public:
        DuckParamSlider() : juce::Slider(RotaryHorizontalVerticalDrag, NoTextBox) {}
        void setMuted (bool m) { muted = m; repaint(); }
        bool isMuted() const { return muted; }

        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat().reduced(2.0f);
            ui::paintRotaryWithLNF(g, *this, r);
            if (muted)
            {
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                {
                    auto inner = r.reduced (6.0f); // align with LNF rotary bounds for no-gap ring
                    // Soft wash to grey-out arcs
                    g.setColour (lf->theme.panel.withAlpha (0.35f));
                    g.fillEllipse (inner);
                    // Thin muted ring on top for clarity
                    g.setColour (lf->theme.textMuted.withAlpha (0.85f));
                    g.drawEllipse (inner, 1.5f);
                }
            }
        }
    private:
        bool muted { false };
    };

    // Duck ratio slider with stepped snapping and custom tick dots for allowed ratios only
    class DuckRatioSlider : public juce::Slider
    {
    public:
        DuckRatioSlider() : juce::Slider(RotaryHorizontalVerticalDrag, NoTextBox)
        {
            setMouseDragSensitivity(100);
        }
        void setMuted (bool m) { muted = m; repaint(); }
        bool isMuted() const { return muted; }

        double snapValue (double attemptedValue, DragMode) override
        {
            // Nearest value from allowed ratios
            double best = allowed[0];
            double bestErr = std::abs(attemptedValue - best);
            for (double v : allowed)
            {
                const double err = std::abs(attemptedValue - v);
                if (err < bestErr) { bestErr = err; best = v; }
            }
            return best;
        }

        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat().reduced(2.0f);
            // Use FieldLNF rotary rendering (single source of truth for quarter ticks)
            ui::paintRotaryWithLNF(g, *this, r);
            if (muted)
            {
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                {
                    auto inner = r.reduced (6.0f); // align with LNF rotary bounds for no-gap ring
                    g.setColour (lf->theme.panel.withAlpha (0.35f));
                    g.fillEllipse (inner);
                    g.setColour (lf->theme.textMuted.withAlpha (0.85f));
                    g.drawEllipse (inner, 1.5f);
                }
            }
        }

    private:
        std::array<double,5> allowed { 2.0, 4.0, 8.0, 12.0, 20.0 };
        bool muted { false };
    };
    
    // Lightweight container cell for non-knob components (e.g., switches)
    class SwitchCell : public juce::Component
    {
    public:
        explicit SwitchCell(juce::Component& contentToHost) : content(contentToHost)
        {
            setOpaque(false);
            caption.setJustificationType (juce::Justification::centred);
            caption.setInterceptsMouseClicks (false, false);
            addAndMakeVisible (caption);
            // If content is a ToggleButton, clear text to prefer icon-only LNF drawing
            if (auto* tb = dynamic_cast<juce::ToggleButton*>(&content)) tb->setButtonText("");
        }
        void setMetrics (int /*knobPx*/, int /*valuePx*/, int /*gapPx*/) { resized(); }
        void setShowBorder (bool show) { showBorder = show; repaint(); }
        void setDelayTheme (bool on) { isDelayTheme = on; repaint(); }
        void setCaption (const juce::String& text)
        {
            captionText = text;
            caption.setText (captionText, juce::dontSendNotification);
            repaint();
        }
        void resized() override
        {
            if (content.getParentComponent() != this)
                addAndMakeVisible (content);
            auto b = getLocalBounds().reduced (6); // inset to reveal panel border fully
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            // Caption height
            const int capH = captionText.isNotEmpty() ? 14 : 0;
            if (captionText.isNotEmpty())
            {
                caption.setBounds (b.removeFromTop (capH));
                if (lf) caption.setColour (juce::Label::textColourId, lf->theme.textMuted);
            }
            content.setBounds (b);
        }
        void paint (juce::Graphics& g) override
        {
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                if (isDelayTheme)
                {
                    auto r = getLocalBounds().toFloat().reduced (3.0f);
                    auto panel  = lf->theme.panel.brighter (0.10f);
                    auto border = lf->theme.text; // use default font grey for border
                    g.setColour (panel);  g.fillRoundedRectangle (r, 8.0f);
                    if (showBorder) { g.setColour (border); g.drawRoundedRectangle (r, 8.0f, 1.5f); }
                }
                else
                {
                    lf->paintCellPanel (g, *this, showBorder, isMouseOverOrDragging() || hoverActive);
                }
            }
        }
        void visibilityChanged() override
        {
            if (isVisible())
            {
                if (content.getParentComponent() != this)
                    addAndMakeVisible (content);
                resized();
                content.setVisible (true);
                repaint();
            }
        }
        void mouseEnter (const juce::MouseEvent&) override { hoverActive = true;  repaint(); }
        void mouseExit  (const juce::MouseEvent&) override { hoverActive = false; repaint(); }
    private:
        juce::Component& content;
        juce::Label caption;
        juce::String captionText;
        bool showBorder { true };
        bool hoverActive { false };
        bool isDelayTheme { false };
    };
    
    // Compact 3-segment control bound to an APVTS choice parameter (0..2)
    class Segmented3Control : public juce::Component
    {
    public:
        Segmented3Control (juce::AudioProcessorValueTreeState& state, const juce::String& paramID,
                           const juce::StringArray& labels)
            : apvts (state), id (paramID)
        {
            for (int i = 0; i < 3; ++i)
            {
                buttons[i].setClickingTogglesState (false);
                buttons[i].onClick = [this, i]{ setIndexFromUI (i); };
                if (i < labels.size()) buttons[i].setButtonText (labels[i]);
                // assign painter: i=0 Straight, 1 Dotted, 2 Triplet
                buttons[i].painter = [this, i](juce::Graphics& g, juce::Rectangle<float> area, bool over, bool on)
                {
                    auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel());
                    juce::Colour accent = lf2 ? lf2->theme.accent : juce::Colour (0xFF2196F3);
                    juce::Colour text   = lf2 ? lf2->theme.text   : juce::Colours::white;
                    juce::Colour iconCol = on ? accent : (over ? text.withAlpha (0.80f) : text.withAlpha (0.65f));
                    // Shadowed, slightly heavier icon
                    auto inner = area.reduced (2.0f);
                    g.setColour (juce::Colours::black.withAlpha (0.18f));
                    drawFeelIcon (g, inner.translated (0.6f, 0.9f), i, iconCol);
                    g.setColour (iconCol);
                    drawFeelIcon (g, inner, i, iconCol);
                };
                addAndMakeVisible (buttons[i]);
            }
            applyTheme();
            if (auto* p = apvts.getParameter (id))
            {
                attachment = std::make_unique<juce::ParameterAttachment>(*dynamic_cast<juce::RangedAudioParameter*>(p),
                    [this](float newVal)
                    {
                        const int idx = (int) juce::roundToInt (newVal);
                        updateButtons (idx);
                    }, nullptr);
                updateButtons ((int) juce::roundToInt (p->getValue()));
            }
        }
        void resized() override
        {
            auto b = getLocalBounds().reduced (4);
            int w = b.getWidth() / 3;
            for (int i = 0; i < 3; ++i)
            {
                auto cell = b.removeFromLeft (w);
                auto inner = cell.reduced (2);
                buttons[i].setBounds (inner);
                buttons[i].setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
            }
        }
        void lookAndFeelChanged() override { applyTheme(); }
        void setLabels (const juce::StringArray& labels)
        {
            for (int i = 0; i < 3 && i < labels.size(); ++i) buttons[i].setButtonText (labels[i]);
        }
    private:
        static void drawFeelIcon (juce::Graphics& g, juce::Rectangle<float> r, int feelIndex, juce::Colour c)
        {
            g.setColour (c);

            auto cx = r.getCentreX();
            auto baseY = r.getCentreY() + r.getHeight() * 0.10f;

            const float headW = juce::jmin (10.0f, r.getWidth() * 0.8f);
            const float headH = headW * 0.68f;
            const float stemH = juce::jmin (22.0f, r.getHeight() * 0.42f);
            const float stemX = cx + headW * 0.45f;
            const float headY = baseY - headH * 0.5f;

            juce::Path head;
            head.addEllipse (cx - headW * 0.5f, headY, headW, headH);
            auto tilt = juce::AffineTransform::rotation (juce::degreesToRadians (-12.0f), cx, headY + headH * 0.5f);
            head.applyTransform (tilt);
            g.fillPath (head);

            juce::Path stem;
            stem.startNewSubPath (stemX, headY + headH * 0.15f);
            stem.lineTo (stemX, headY - stemH);
            g.strokePath (stem, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            if (feelIndex == 1)
            {
                float dotR = 2.0f;
                float dotX = stemX + 3.0f;
                float dotY = headY + headH * 0.50f;
                if (r.getWidth() < 16.0f) { dotX = stemX; dotY = headY + headH * 0.15f - 4.0f; }
                g.fillEllipse (dotX - dotR, dotY - dotR, dotR * 2, dotR * 2);
            }
            else if (feelIndex == 2)
            {
                g.setFont (juce::Font (10.0f, juce::Font::bold));
                float tx = stemX + 2.0f;
                float ty = headY - 6.0f;
                g.drawSingleLineText ("3", (int) tx, (int) ty);
            }
        }
        void applyTheme()
        {
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            juce::Colour accent = lf ? lf->theme.accent : juce::Colour (0xFF2196F3);
            juce::Colour text   = lf ? lf->theme.text   : juce::Colours::white;
            juce::Colour muted  = lf ? lf->theme.textMuted : juce::Colour (0xFFB8BDC7);
            for (int i = 0; i < 3; ++i)
            {
                auto& b = buttons[i];
                b.setColour (juce::TextButton::buttonOnColourId, accent);
                b.setColour (juce::TextButton::textColourOnId,   text);
                b.setColour (juce::TextButton::textColourOffId,  muted);
            }
        }
        void setIndexFromUI (int i)
        {
            if (! attachment) return;
            attachment->setValueAsCompleteGesture ((float) i);
            updateButtons (i);
        }
        void updateButtons (int idx)
        {
            for (int i = 0; i < 3; ++i)
                buttons[i].setToggleState (i == idx, juce::dontSendNotification);
            repaint();
        }
        juce::AudioProcessorValueTreeState& apvts;
        juce::String id;
        // Custom button subclass to draw icon
        struct IconButton : public juce::TextButton
        {
            std::function<void(juce::Graphics&, juce::Rectangle<float>, bool, bool)> painter;
            void paintButton (juce::Graphics& g, bool over, bool down) override
            {
                auto r = getLocalBounds().toFloat();
                auto rr = r.reduced (1.0f);
                float cr = 4.0f;
                auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel());
                auto panel  = lf2 ? lf2->theme.panel  : juce::Colour (0xFF3A3D45);
                auto border = lf2 ? lf2->theme.sh     : juce::Colour (0xFF2A2A2A);
                juce::Colour accent = lf2 ? lf2->theme.accent : juce::Colour (0xFF2196F3);
                juce::Colour text   = lf2 ? lf2->theme.text   : juce::Colours::white;

                g.setColour (panel);
                g.fillRoundedRectangle (rr, cr);
                g.setColour (border);
                g.drawRoundedRectangle (rr, cr, 1.0f);

                if (painter)
                {
                    bool on = getToggleState();
                    juce::Colour iconCol = on ? accent : (over ? text.withAlpha (0.85f) : text.withAlpha (0.75f));
                    auto inner = rr.reduced (3.0f);
                    // Shadow pass
                    g.setColour (juce::Colours::black.withAlpha (0.18f));
                    painter (g, inner.translated (0.6f, 0.9f), over, on);
                    // Main pass
                    g.setColour (iconCol);
                    painter (g, inner, over, on);
                }
            }
        };
        IconButton buttons[3];
        std::unique_ptr<juce::ParameterAttachment> attachment;
    };
    
    // Resize handle functionality
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

    // Layout management
    void performLayout();

private:
    MyPluginAudioProcessor& proc;
    FieldLNF lnf;
    XYPad pad;
    // Multi-pane visual dock (XY, Spectrum, Imager)
    std::unique_ptr<class PaneManager> panes;
    std::unique_ptr<juce::KeyListener> keyListener;
    
    // Resize constraints
    int minWidth = 0;
    int minHeight = 0;
    int maxWidth = 3000;
    int maxHeight = 2000;
    
    // UI Components
    GainSlider   gain;
    juce::Slider width, tilt, monoHz, hpHz, lpHz, satDrive, satMix, air, bass, scoop; // includes Scoop
    juce::ComboBox  monoSlopeChoice;
    // AUD audition: custom-styled toggle button (non-checkbox)
    class AuditionButton : public ThemedIconButton, public juce::Timer
    {
    public:
        AuditionButton() : ThemedIconButton(Options{ IconSystem::Mono, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, true })
        {
            setButtonText("AUD");
            startTimerHz (6); // ~6 Hz repaint; blink phase derived from time
        }
        ~AuditionButton() override { stopTimer(); }
        void timerCallback() override { if (isShowing()) repaint(); }
        void paintButton(juce::Graphics& g, bool over, bool down) override
        {
            auto r = getLocalBounds().toFloat().reduced (2.0f);
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            auto panel  = lf ? lf->theme.panel  : juce::Colour (0xFF3A3D45);
            auto accent = lf ? lf->theme.accent : juce::Colour (0xFF2196F3);
            auto sh     = lf ? lf->theme.sh     : juce::Colour (0xFF2A2A2A);
            auto hl     = lf ? lf->theme.hl     : juce::Colour (0xFF4A4A4A);

            // Elevation shadow first
            g.setColour (juce::Colour (0x40000000));
            g.fillRoundedRectangle (r.translated (1.5f, 1.5f), 4.0f);

            const bool on = getToggleState();
            if (on)
            {
                // Blink between two blue tones
                auto now = juce::Time::getMillisecondCounter();
                const bool phase = ((now / 280) % 2) == 0; // ~3.6 Hz blink
                auto bg = phase ? accent : accent.darker (0.35f);
                if (down || over) bg = bg.brighter (0.10f);
                g.setColour (bg);
                g.fillRoundedRectangle (r, 4.0f);
                g.setColour (bg.darker (0.30f));
                g.drawRoundedRectangle (r, 4.0f, 1.0f);
            }
            else
            {
                // Dark when not engaged (gradient panel)
                juce::Colour top = panel.brighter (0.10f), bot = panel.darker (0.10f);
                juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
                g.setGradientFill (grad);
                g.fillRoundedRectangle (r, 4.0f);
                g.setColour (down ? sh : (over ? hl : sh));
                g.drawRoundedRectangle (r, 4.0f, 1.0f);
            }

            // Text
            auto textCol = on ? juce::Colours::white : (lf ? lf->theme.textMuted : juce::Colour (0xFF888888));
            g.setColour (textCol);
            g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
            g.drawText ("AUD", r, juce::Justification::centred);
        }
    };
    AuditionButton monoAuditionButton;
    // Imaging controls
    juce::Slider widthLo, widthMid, widthHi;
    juce::Slider xoverLoHz, xoverHiHz;
    juce::Slider rotationDeg, asymmetry;
    juce::Slider shufLoPct, shufHiPct, shufXHz;
    PanSlider    panKnob;
    PanSlider    panKnobLeft, panKnobRight;  // split mode pan
    DuckingSlider duckingKnob;
    DuckParamSlider duckAttack, duckRelease, duckThreshold; // Ducking advanced (UI knobs)
    DuckRatioSlider duckRatio;
    juce::ComboBox osSelect;

    // Old preset combo & save button removed
    BypassButton     bypassButton;
    ToggleSwitch     splitToggle;
    
    // Frequency control sliders
    juce::Slider tiltFreqSlider, scoopFreqSlider, bassFreqSlider, airFreqSlider;
    // EQ shape/Q controls
    juce::Slider shelfShapeS, filterQ;
    juce::ToggleButton tiltLinkSButton, qLinkButton;
    juce::Slider hpQSlider, lpQSlider;
    // Q-cluster dummy hosts (no visible knob in cluster)
    juce::Slider qClusterDummySlider;
    juce::Label  qClusterDummyValue;
    
    // Delay controls
    juce::Slider delayTime, delayFeedback, delayWet, delaySpread, delayWidth, delayModRate, delayModDepth, delayWowflutter, delayJitter;
    juce::Slider delayHp, delayLp, delayTilt, delaySat, delayDiffusion, delayDiffuseSize;
    juce::Slider delayDuckDepth, delayDuckAttack, delayDuckRelease, delayDuckThreshold, delayDuckRatio, delayDuckLookahead;
    juce::ComboBox delayMode, delayTimeDiv, delayDuckSource, delayGridFlavor;
    juce::ToggleButton delayEnabled, delaySync, delayKillDry, delayFreeze, delayPingpong, delayDuckPost, delayDuckLinkGlobal;
    
    // Icon buttons (shared base)
    OptionsButton    optionsButton;
    LinkButton       linkButton;
    SnapButton       snapButton;
    FullScreenButton fullScreenButton;
    ColorModeButton  colorModeButton;
    class HelpButton : public ThemedIconButton { public: HelpButton()
    : ThemedIconButton(Options{ IconSystem::Help, false, ThemedIconButton::Style::GradientPanel, 3.0f, 4.0f, false }) {} };
    HelpButton       helpButton;
    CopyButton       copyButton;
    LockButton       lockButton;

    // Global Wet Only (Kill Dry) UI toggle (no param binding per instructions)
    juce::ToggleButton wetOnlyToggle;

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
    // Placeholder for mono slope switch definition (defined after SpaceAlgorithmSwitch)
    class MonoSlopeSwitch;
    std::unique_ptr<MonoSlopeSwitch> monoSlopeSwitch;
    
    // 3-way segmented switch (Room / Plate / Hall) with horizontal or vertical layout.
    class SpaceAlgorithmSwitch : public juce::Component
    {
    public:
        enum class Orientation { Horizontal, Vertical };

        SpaceAlgorithmSwitch()
        {
            items.add ("Room");
            items.add ("Plate");
            items.add ("Hall");
        }

        void setGreenMode (bool) {}

        // Orientation / labels
        void setOrientation (Orientation o)          { orientation = o; repaint(); }
        void setLabels (const juce::StringArray& ls) { items = ls; currentIndex = juce::jlimit (0, items.size() - 1, currentIndex); repaint(); }

        // State
        void setAlgorithm (int i)              { currentIndex = juce::jlimit (0, items.size() - 1, i); repaint(); }
        void setAlgorithmFromParameter (int i) { setAlgorithm (i); }
        int  getAlgorithm () const             { return currentIndex; }
        void setMuted (bool m)                 { muted = m; repaint(); }

        std::function<void (int)> onAlgorithmChange;

        void setSpacing (float px) { spacing = juce::jmax (0.0f, px); repaint(); }
        void setDrawOwnPanel (bool on) { drawOwnPanel = on; repaint(); }

        void paint (juce::Graphics& g) override
        {
            const int n = items.size();
            if (n == 0) return;

            auto b = getLocalBounds().toFloat();

            // Draw own panel only when not hosted in SwitchCell
            if (drawOwnPanel)
            {
                if (auto* lfPanel = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                {
                    const float rad = 8.0f;
                    auto panel = b.reduced (3.0f);
                    g.setColour (lfPanel->theme.panel);
                    g.fillRoundedRectangle (panel, rad);
                    g.setColour (lfPanel->theme.sh.withAlpha (0.18f));
                    g.drawRoundedRectangle (panel.reduced (1.0f), rad - 1.0f, 0.8f);
                }
            }
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            const auto outline = juce::Colour (0xFF1A1C20);
            const float corner = 6.0f;

            if (orientation == Orientation::Horizontal)
            {
                const float s = spacing;
                // DEV NOTE: 4x larger feel – rely on wider column and use most of height/width
                const float side = juce::jmin (b.getHeight() - 8.0f, (b.getWidth() - (n - 1) * s - 8.0f) / (float) n);
                const float totalW = side * n + s * (n - 1);
                juce::Rectangle<float> r (b.getX() + (b.getWidth() - totalW) * 0.5f,
                                          b.getY() + (b.getHeight() - side) * 0.5f,
                                          side, side);

                for (int i = 0; i < n; ++i)
                {
                    const bool on = (currentIndex == i);
                    drawButton (g, r, i, on, items[i], lf, outline, corner, muted);
                    r.setX (r.getX() + side + s);
                }
            }
            else
            {
                const float s = spacing;
                const float availableH = juce::jmax (0.0f, b.getHeight() - (n - 1) * s - 8.0f);
                const float h = availableH / (float) n;

                juce::Rectangle<float> r (b.getX(), b.getY() + 4.0f, b.getWidth(), h);
                for (int i = n - 1; i >= 0; --i)
                {
                    const bool on = (currentIndex == i);
                    drawButton (g, r, i, on, items[i], lf, outline, corner, muted);
                    r.setY (r.getY() + h + s);
                }
            }
        }

        void mouseDown (const juce::MouseEvent& e) override
        {
            if (e.mods.isPopupMenu())
            {
                juce::PopupMenu m;
                for (int i = 0; i < items.size(); ++i)
                    m.addItem (i + 1, items[i], true, currentIndex == i);

                m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                                 [this](int r){ if (r > 0) { currentIndex = r - 1; repaint(); if (onAlgorithmChange) onAlgorithmChange (currentIndex); }});
                return;
            }

            const int n = items.size();
            if (n == 0) return;

            auto b = getLocalBounds().toFloat();
            const float s = spacing;

            if (orientation == Orientation::Horizontal)
            {
                const float side = juce::jmin (b.getHeight(), (b.getWidth() - (n - 1) * s) / (float) n);
                const float totalW = side * n + s * (n - 1);
                juce::Rectangle<float> r (b.getX() + (b.getWidth() - totalW) * 0.5f,
                                          b.getY() + (b.getHeight() - side) * 0.5f,
                                          side, side);

                for (int i = 0; i < n; ++i)
                {
                    if (r.contains ((float) e.x, (float) e.y))
                    {
                        if (currentIndex != i) { currentIndex = i; repaint(); if (onAlgorithmChange) onAlgorithmChange (currentIndex); }
                        return;
                    }
                    r.setX (r.getX() + side + s);
                }
            }
            else
            {
                const float availableH = juce::jmax (0.0f, b.getHeight() - (n - 1) * s);
                const float h = availableH / (float) n;
                juce::Rectangle<float> r (b.getX(), b.getY(), b.getWidth(), h);

                for (int i = n - 1; i >= 0; --i)
                {
                    if (r.contains ((float) e.x, (float) e.y))
                    {
                        if (currentIndex != i) { currentIndex = i; repaint(); if (onAlgorithmChange) onAlgorithmChange (currentIndex); }
                        return;
                    }
                    r.setY (r.getY() + h + s);
                }
            }
        }

    private:
        int currentIndex { 0 };
        juce::StringArray items;
        Orientation orientation { Orientation::Vertical };
        float spacing { 6.0f };
        bool  drawOwnPanel { true };
        bool  muted { false };

        static juce::Colour activeColour (int idx, FieldLNF* lf)
        {
            if (lf != nullptr)
            {
                auto a = lf->theme.accent;
                switch (idx) {
                    case 0: return a;                 // Room
                    case 1: return a.brighter (0.35f);// Plate
                    case 2: return a.darker   (0.35f);// Hall
                }
            }
            switch (idx) { case 0: return juce::Colour (0xFF5AA9E6); case 1: return juce::Colour (0xFF2EC4B6); case 2: return juce::Colour (0xFF2A1B3D); }
            return juce::Colour (0xFF5AA9E6);
        }

        static void drawButton (juce::Graphics& g, juce::Rectangle<float> r, int idx, bool on, const juce::String& label,
                                FieldLNF* lf, juce::Colour outline, float corner, bool muted)
        {
            juce::Colour base = juce::Colour (0xFF2A2C30);
            juce::Colour fill = on ? activeColour (idx, lf) : base;

            g.setColour (fill);
            g.fillRoundedRectangle (r, corner);

            g.setColour (outline);
            g.drawRoundedRectangle (r, corner, 1.0f);

            juce::Colour textCol = lf ? lf->theme.text : juce::Colour (0xFFF0F2F5);
            if (muted) textCol = textCol.withAlpha (0.45f);
            g.setColour (textCol);
            g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
            g.drawText (label, r, juce::Justification::centred);
        }
    };
    
    SpaceAlgorithmSwitch spaceAlgorithmSwitch;
    std::unique_ptr<SpaceAlgorithmSwitch> monoSlopeSegmentSwitch;
    std::unique_ptr<SwitchCell> spaceSwitchCell;
    std::unique_ptr<SwitchCell> wetOnlyCell;

    // Dedicated Mono Slope Switch (6/12/24) with independent drawing but same visual language
    class MonoSlopeSwitch : public juce::Component
    {
    public:
        MonoSlopeSwitch() = default;
        void setIndex (int idx) { current = juce::jlimit (0, 2, idx); repaint(); if (onChange) onChange (current); }
        int  getIndex () const { return current; }
        std::function<void(int)> onChange;
        void paint (juce::Graphics& g) override
        {
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            auto accent = lf ? lf->theme.eq.hp : juce::Colour (0xFF5AA9E6);
            auto panel  = lf ? lf->theme.panel  : juce::Colour (0xFF2A2C30);
            auto sh     = lf ? lf->theme.sh     : juce::Colour (0xFF1A1C20);
            auto hl     = lf ? lf->theme.hl     : juce::Colour (0xFF4A4A4A);
            auto text   = lf ? lf->theme.text   : juce::Colours::white;

            auto b = getLocalBounds().toFloat();
            const float spacing = 6.0f;
            const float availableH = juce::jmax (0.0f, b.getHeight() - 2.0f * spacing);
            const float h = availableH / 3.0f;

            auto draw = [&](juce::Rectangle<float> r, int idx, const juce::String& lbl)
            {
                const bool on = (current == idx);
                // Elevation shadow like AUD
                g.setColour (juce::Colour (0x40000000));
                g.fillRoundedRectangle (r.translated (1.5f, 1.5f), 6.0f);

                if (on)
                {
                    juce::Colour bg = accent;
                    if (idx == 0) bg = accent.brighter (0.25f);    // 6 dB
                    else if (idx == 2) bg = accent.darker (0.25f); // 24 dB
                    g.setColour (bg);
                    g.fillRoundedRectangle (r, 6.0f);
                    g.setColour (bg.darker (0.30f));
                    g.drawRoundedRectangle (r, 6.0f, 1.0f);
                }
                else
                {
                    // Gradient panel like ThemedIconButton::GradientPanel
                    juce::Colour top = panel.brighter (0.10f), bot = panel.darker (0.10f);
                    juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
                    g.setGradientFill (grad);
                    g.fillRoundedRectangle (r, 6.0f);
                    g.setColour (sh);
                    g.drawRoundedRectangle (r, 6.0f, 1.0f);
                }

                g.setColour (text);
                g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
                g.drawText (lbl, r, juce::Justification::centred);
            };

            draw ({ b.getX(), b.getY(),                     b.getWidth(), h },                 0, "6");
            draw ({ b.getX(), b.getY() + h + spacing,       b.getWidth(), h },                 1, "12");
            draw ({ b.getX(), b.getY() + 2*(h + spacing),   b.getWidth(), h },                 2, "24");
        }
        void mouseDown (const juce::MouseEvent& e) override
        {
            const float spacing = 6.0f;
            const float availableH = juce::jmax (0.0f, (float)getHeight() - 2.0f * spacing);
            const float h = availableH / 3.0f; const float y = (float) e.y;
            int idx = (y <= h) ? 0 : (y <= h * 2 + spacing ? 1 : 2);
            if (idx != current) { current = idx; repaint(); if (onChange) onChange (current); }
        }
    private:
        int current { 1 }; // default to 12 dB/oct
    };
    
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
    juce::TextButton presetField; // clickable field to open palette and display current preset
    juce::Label presetNameLabel;
    juce::Component headerLeftGroup; // container for bypass (no logo)

    // Split-pan container placeholder for grid cell (no painting, no mouse)
    juce::Component panSplitContainer;
    
    // Preset system
    PresetStore presetStore;
    NewPresetManager presetManager;
    
    // Containers
    ControlContainer mainControlsContainer, volumeContainer;
    ControlContainer delayContainer;
    ControlContainer metersContainer;
    ControlContainer spaceKnobContainer, panKnobContainer;
    
    // Width grouping (Image row): large WIDTH + small W LO/MID/HI
    ControlContainer widthGroupContainer;
    juce::Component widthGroupSlot1, widthGroupSlot2, widthGroupSlot3; // grid placeholders to claim columns
    
    // Gain+Drive+Mix grouping (Volume row): invisible container spanning three columns (right side)
    ControlContainer gainMixGroupContainer;
    juce::Component gainMixSlot1, gainMixSlot2;

    // Ducking group container (Depth, Attack, Release, Threshold)
    ControlContainer duckGroupContainer;
    juce::Component duckSlot1, duckSlot2, duckSlot3;
    
    // Volume row unified grouping
    ControlContainer volGroupContainer;
    ControlContainer eqGroupContainer;
    ControlContainer imgGroupContainer;
    ControlContainer volGroupContainer2;
    ControlContainer monoGroupContainer;
 
    juce::Component volSlot1, volSlot2, volSlot3, volSlot4, volSlot5, volSlot6, volSlot7;

    

    // Value indicators (if you keep them)
    juce::Label leftIndicator, rightIndicator;
    juce::Label gainValue, widthValue, tiltValue, monoValue, hpValue, lpValue, satDriveValue, satMixValue, airValue, bassValue, scoopValue;
    juce::Label shelfShapeValue, filterQValue;
    juce::Label monoSlopeName, monoAudName;
    juce::Label panValue, panValueLeft, panValueRight, spaceValue, duckingValue;
    juce::Label duckAttackValue, duckReleaseValue, duckThresholdValue, duckRatioValue;
    juce::Label tiltFreqValue, scoopFreqValue, bassFreqValue, airFreqValue;
    juce::Label widthLoValue, widthMidValue, widthHiValue, xoverLoValue, xoverHiValue, rotationValue, asymValue, shufLoValue, shufHiValue, shufXValue;
    juce::Label delayTimeValue, delayFeedbackValue, delayWetValue, delaySpreadValue, delayWidthValue, delayModRateValue, delayModDepthValue, delayWowflutterValue, delayJitterValue;
    juce::Label delayHpValue, delayLpValue, delayTiltValue, delaySatValue, delayDiffusionValue, delayDiffuseSizeValue;
    juce::Label delayDuckDepthValue, delayDuckAttackValue, delayDuckReleaseValue, delayDuckThresholdValue, delayDuckRatioValue, delayDuckLookaheadValue;
    // Imaging knob name labels (third row)
    juce::Label widthLoName, widthMidName, widthHiName;
    juce::Label xoverLoName, xoverHiName;
    juce::Label rotationName, asymName;
    juce::Label shufLoName, shufHiName, shufXName;
    
    // Delay name labels
    juce::Label delayTimeName, delayFeedbackName, delayWetName, delaySpreadName, delayWidthName, delayModRateName, delayModDepthName, delayWowflutterName, delayJitterName;
    juce::Label delayHpName, delayLpName, delayTiltName, delaySatName, delayDiffusionName, delayDiffuseSizeName;
    juce::Label delayDuckDepthName, delayDuckAttackName, delayDuckReleaseName, delayDuckThresholdName, delayDuckRatioName, delayDuckLookaheadName;
    
    // Text labels
    juce::Label gainL, widthL, tiltL, monoL, hpL, lpL, satDriveL, satMixL;
    juce::Label panL, spaceL, duckingL;

    // Row 1 cells
    std::unique_ptr<KnobCell> panCell;
    std::unique_ptr<KnobCell> widthCell, widthLoCell, widthMidCell, widthHiCell;
    std::unique_ptr<KnobCell> gainCell, satDriveCell, satMixCell, monoCell;

    // EQ row cells (knob + value + optional mini)
    std::unique_ptr<KnobCell> bassCell;
    std::unique_ptr<KnobCell> airCell;
    std::unique_ptr<KnobCell> tiltCell;
    std::unique_ptr<KnobCell> scoopCell;
    std::unique_ptr<KnobCell> hpCell;
    std::unique_ptr<KnobCell> lpCell;

    // Reverb/Duck cells
    std::unique_ptr<KnobCell> spaceCell;
    std::unique_ptr<KnobCell> duckCell;
    std::unique_ptr<KnobCell> duckAttCell;
    std::unique_ptr<KnobCell> duckRelCell;
    std::unique_ptr<KnobCell> duckThrCell;
    std::unique_ptr<KnobCell> duckRatCell;

    // Imaging (row 4) cells
    std::unique_ptr<KnobCell> xoverLoCell;
    std::unique_ptr<KnobCell> xoverHiCell;
    std::unique_ptr<KnobCell> rotationCell;
    std::unique_ptr<KnobCell> asymCell;
    std::unique_ptr<KnobCell> shufLoCell;
    std::unique_ptr<KnobCell> shufHiCell;
    std::unique_ptr<KnobCell> shufXCell;
    std::unique_ptr<KnobCell> shelfShapeCell;
    std::unique_ptr<KnobCell> filterQCell;
    std::unique_ptr<KnobCell> qClusterCell;

    // Delay cells (knob + value)
    std::unique_ptr<KnobCell> delayTimeCell;
    std::unique_ptr<KnobCell> delayFeedbackCell;
    std::unique_ptr<KnobCell> delayWetCell;
    std::unique_ptr<KnobCell> delaySpreadCell;
    std::unique_ptr<KnobCell> delayWidthCell;
    std::unique_ptr<KnobCell> delayModRateCell;
    std::unique_ptr<KnobCell> delayModDepthCell;
    std::unique_ptr<KnobCell> delayWowflutterCell;
    std::unique_ptr<KnobCell> delayJitterCell;
    std::unique_ptr<KnobCell> delayHpCell;
    std::unique_ptr<KnobCell> delayLpCell;
    std::unique_ptr<KnobCell> delayTiltCell;
    std::unique_ptr<KnobCell> delaySatCell;
    std::unique_ptr<KnobCell> delayDiffusionCell;
    std::unique_ptr<KnobCell> delayDiffuseSizeCell;
    std::unique_ptr<KnobCell> delayDuckDepthCell;
    std::unique_ptr<KnobCell> delayDuckAttackCell;
    std::unique_ptr<KnobCell> delayDuckReleaseCell;
    std::unique_ptr<KnobCell> delayDuckThresholdCell;
    std::unique_ptr<KnobCell> delayDuckLookaheadCell;

    // Delay control cells (buttons/combos, styled like KnobCell panels)
    std::unique_ptr<SwitchCell> delayEnabledCell;
    std::unique_ptr<SwitchCell> delayModeCell;
    std::unique_ptr<SwitchCell> delaySyncCell;
    std::unique_ptr<SwitchCell> delayGridFlavorCell;
    std::unique_ptr<SwitchCell> delayFreezeCell;
    std::unique_ptr<SwitchCell> delayKillDryCell;
    std::unique_ptr<SwitchCell> delayPingpongCell;
    std::unique_ptr<SwitchCell> delayDuckSourceCell;
    std::unique_ptr<SwitchCell> delayDuckPostCell;

    std::unique_ptr<Segmented3Control> delayGridFlavorSegments;

    std::unique_ptr<class DoubleKnobCell> hpLpCell;
    std::unique_ptr<class QuadKnobCell> hpLpQClusterCell;
    void buildCells();

    // Attachments
    std::vector<std::unique_ptr<SliderAttachment>>  attachments;
    std::vector<std::unique_ptr<ButtonAttachment>>  buttonAttachments;
    std::vector<std::unique_ptr<ComboAttachment>>   comboAttachments;
    
    // Scaling
    float scaleFactor = 1.0f;
    const int baseWidth  = 2000; // increased to accommodate delay + motion grid by default
    const int baseHeight = 1250;
    const int standardKnobSize = 80;
    bool resizingRowGuard = false;
    
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
        CorrelationMeter (MyPluginAudioProcessor& p, FieldLNF& l) : proc (p), lnf (l) { startTimerHz (15); }
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
            g.drawText ("CORR", r.reduced (4).withHeight (16), juce::Justification::centredTop);

            // Minimal units: show -1, 0, +1 under the bar
            g.setColour (lnf.theme.textMuted.withAlpha (0.9f));
            g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")));
            const float labelY = barY + barH + 2.0f;
            juce::Rectangle<float> unitsArea (r.getX() + 6.0f, labelY, r.getWidth() - 12.0f, 12.0f);
            auto leftR  = juce::Rectangle<float> (unitsArea.getX(), unitsArea.getY(), unitsArea.getWidth() * 0.33f, unitsArea.getHeight());
            auto midR   = juce::Rectangle<float> (unitsArea.getX() + unitsArea.getWidth() * 0.33f, unitsArea.getY(), unitsArea.getWidth() * 0.34f, unitsArea.getHeight());
            auto rightR = juce::Rectangle<float> (unitsArea.getX() + unitsArea.getWidth() * 0.67f, unitsArea.getY(), unitsArea.getWidth() * 0.33f, unitsArea.getHeight());
            g.drawText ("-1", leftR.toNearestInt(), juce::Justification::centredLeft);
            g.drawText ("0",  midR.toNearestInt(),  juce::Justification::centred);
            g.drawText ("+1", rightR.toNearestInt(), juce::Justification::centredRight);
        }
        void timerCallback() override { repaint(); }
        void visibilityChanged() override { if (isVisible()) startTimerHz (15); else stopTimer(); }
    private:
        MyPluginAudioProcessor& proc;
        FieldLNF& lnf;
    };

    CorrelationMeter corrMeter { proc, lnf };

    // Horizontal L/R meters (RMS + Peak overlays)
    class HorizontalLRMeters : public juce::Component, public juce::Timer
    {
    public:
        HorizontalLRMeters (MyPluginAudioProcessor& p, FieldLNF& l) : proc(p), lnf(l) { startTimerHz (20); }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto top = r.removeFromTop (r.getHeight() * 0.5f).reduced (2.0f);
            auto bot = r.reduced (2.0f);

            auto drawBar = [&] (juce::Rectangle<float> b, float rms, float peak, const juce::String& label)
            {
                g.setColour (lnf.theme.panel);
                g.fillRoundedRectangle (b, 4.0f);
                // Track
                g.setColour (lnf.theme.hl.withAlpha (0.35f));
                g.fillRoundedRectangle (b.reduced (1.0f), 3.5f);
                // Border
                g.setColour (lnf.theme.sh);
                g.drawRoundedRectangle (b, 4.0f, 1.0f);

                // scale 0..1 across width
                auto wRms  = juce::jlimit (0.0f, 1.0f, rms ) * b.getWidth();
                auto wPeak = juce::jlimit (0.0f, 1.0f, peak) * b.getWidth();
                // RMS fill with color shift near clipping
                const bool nearClip = peak >= 0.98f || rms >= 0.90f;
                auto rmsCol = nearClip ? juce::Colour (0xFFE57373) /*red-ish*/ : lnf.theme.accent;
                g.setColour (rmsCol.withAlpha (0.35f));
                g.fillRoundedRectangle (juce::Rectangle<float> (b.getX(), b.getY(), wRms, b.getHeight()), 3.0f);
                // Peak line
                g.setColour (nearClip ? juce::Colour (0xFFE53935) : lnf.theme.accent);
                g.fillRect (juce::Rectangle<float> (b.getX() + wPeak - 1.0f, b.getY(), 2.0f, b.getHeight()));

                // Label
                g.setColour (lnf.theme.textMuted);
                g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
                g.drawText (label, b.reduced (4.0f), juce::Justification::centredLeft);

                // dB tick marks (approx) across the bar: -24, -12, -6, -3, -1 dBFS
                auto drawTick = [&] (float db, const char* text)
                {
                    // map dB to linear magnitude (0..1). For UI, assume 0 = -inf, 1 = 0 dBFS
                    const float lin = juce::Decibels::decibelsToGain (db, -60.0f);
                    const float x   = b.getX() + juce::jlimit (0.0f, 1.0f, lin) * b.getWidth();
                    g.setColour (lnf.theme.hl.withAlpha (0.6f));
                    g.fillRect (juce::Rectangle<float> (x, b.getY(), 1.0f, b.getHeight()));
                    g.setColour (lnf.theme.textMuted.withAlpha (0.8f));
                    g.drawText (text, juce::Rectangle<int> ((int) x - 16, (int) (b.getBottom() + 1), 32, 12), juce::Justification::centredTop);
                };
                drawTick (-24.0f, "-24");
                drawTick (-12.0f, "-12");
                drawTick (-6.0f,  "-6");
                drawTick (-3.0f,  "-3");
                drawTick (-1.0f,  "-1");

                // Minimal units label: show "dBFS" once (on top/L bar)
                if (label == "L")
                {
                    g.setColour (lnf.theme.textMuted.withAlpha (0.9f));
                    g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")));
                    g.drawText ("dBFS",
                                juce::Rectangle<int> ((int) (b.getRight() - 36.0f), (int) (b.getY() - 12.0f), 36, 12),
                                juce::Justification::centredRight);
                }
            };

            drawBar (top, proc.getRmsL(), proc.getPeakL(), "L");
            drawBar (bot, proc.getRmsR(), proc.getPeakR(), "R");
        }
        void timerCallback() override { if (isShowing()) repaint(); }
        void visibilityChanged() override { if (isVisible()) startTimerHz (20); else stopTimer(); }
    private:
        MyPluginAudioProcessor& proc;
        FieldLNF& lnf;
    };

    HorizontalLRMeters lrMeters { proc, lnf };

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
    void updatePresetDisplay();
    
    // Header hover
    bool headerHovered = false;
    bool headerHoverActive = false;
    const int headerHoverOffDelayMs = 160;

    // Cached layout
    juce::Rectangle<int> dividerVolBounds;

    // Cursor policy
    void applyGlobalCursorPolicy();
    void updateMutedKnobVisuals();
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
        void mouseMove (const juce::MouseEvent& e) override
        {
            const bool over = getHandle().contains (e.position.toFloat());
            if (over != hoverHandle)
            {
                hoverHandle = over;
                repaint();
            }
            setMouseCursor (over ? juce::MouseCursor::UpDownResizeCursor : juce::MouseCursor::NormalCursor);
        }
        void mouseExit (const juce::MouseEvent&) override
        {
            if (hoverHandle)
            {
                hoverHandle = false;
                repaint();
            }
            setMouseCursor (juce::MouseCursor::NormalCursor);
        }

    private:
        FieldLNF& lnf;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amount;
        int   dragStartY = 0;
        float startAmt   = 0.f;
        bool  hoverHandle = false;

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
            // Panel styling similar to Machine dropdown, with subtle hover glow
            g.setColour (lnf.theme.base);
            g.fillRoundedRectangle (tab, 8.0f);
            g.setColour (lnf.theme.hl.withAlpha (hoverHandle ? 0.85f : 0.6f));
            g.drawRoundedRectangle (tab, 8.0f, 1.2f);
            if (hoverHandle)
            {
                for (int i = 1; i <= 3; ++i)
                {
                    const float t = (float) i / 3.0f;
                    const float expand = 2.0f + t * 6.0f;
                    g.setColour (lnf.theme.accent.withAlpha ((1.0f - t) * 0.18f));
                    g.drawRoundedRectangle (tab.expanded (expand), 8.0f + expand * 0.35f, 1.8f);
                }
            }

            const int numBars = 4;
            const float barW = 10.0f, barH = 6.0f, gap = 14.0f;
            const float totalW = numBars * barW + (numBars - 1) * gap;
            float startX = tab.getCentreX() - totalW * 0.5f;
            float y = tab.getCentreY() - barH * 0.5f;

            g.setColour (hoverHandle ? lnf.theme.accent : lnf.theme.textMuted);
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
            auto b = getLocalBounds().toFloat();
            // Thicker divider with subtle insets for spacing
            const float w = juce::jmax (2.0f, b.getWidth());
            const float x = b.getX();
            // Outer soft lines
            g.setColour(lnf.theme.sh.withAlpha(0.25f));
            g.fillRect (juce::Rectangle<float> (x, b.getY(), w, b.getHeight()));
            // Inner highlight
            g.setColour(lnf.theme.hl.withAlpha(0.65f));
            g.fillRect (juce::Rectangle<float> (x + w * 0.5f - 0.5f, b.getY(), 1.0f, b.getHeight()));
        }
    private:
        FieldLNF& lnf;
    };

    VerticalDivider splitDivider{lnf}, eqDivLpMono{lnf}, eqDivScoopHp{lnf};
    VerticalDivider volDivPanSpace{lnf}, volDivDuckRight{lnf}, delayDivider{lnf}, motionDivider{lnf};
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

    // Motion cells (4x4 blank knobs on far right)
    std::array<std::unique_ptr<KnobCell>, 16> motionCells;
    std::array<juce::Slider, 16> motionDummies;
    std::array<juce::Label,  16> motionValues;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessorEditor)
}; 
