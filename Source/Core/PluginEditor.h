#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "Core/FieldLookAndFeel.h"
#include "Core/FieldTheme.h"
#include "Core/FieldMetallic.h"
#include "ui/Components/KnobCell.h"
#include "Core/IconSystem.h"
#include "ui/Components/BypassButton.h"
#include "ui/Components/ThemedIconButton.h"
#include "ui/Components/AuditionButton.h"
#include "ui/Components/ABButton.h"
#include "ui/Components/PhaseModeButton.h"
#include "ui/Components/QualityButton.h"
#include "ui/Components/TooltipsButton.h"
#include "ui/Components/HelpButton.h"
#include "ui/Components/TooltipBubble.h"
#include "ui/Components/VerticalDivider.h"
#include "ui/Components/HorizontalDivider.h"
#include "ui/Components/XYPad.h"
#include "ui/Components/GainSlider.h"
#include "ui/Components/PanSlider.h"
#include "ui/Components/ControlContainer.h"
#include "ui/Components/UIHelpers.h"
#include "ui/Layout/LayoutManager.h"
#include "ui/Events/EventManager.h"
#include "Presets/PresetRegistry.h"
#include "Presets/PresetCommandPalette.h"
#include "Presets/PresetManager.h"
#include "ui/Engines/StereoFieldEngine.h"
#include "ui/Panes/ImagerPane.h"
#include "ui/Managers/PaneManager.h"
#include "ui/delay/DelayVisuals.h"

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

//==============================================================================
// ControlContainer (kept hover timer; purely cosmetic "soft fade" on hover)
//==============================================================================

//==============================================================================
// VerticalSlider3D - Beautiful 3D vertical slider with metallic treatment
//==============================================================================
class VerticalSlider3D : public juce::Slider
{
public:
    VerticalSlider3D();
    ~VerticalSlider3D() override = default;
    
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    
    void setSliderStyle (SliderStyle newStyle);
    
private:
    void draw3DHandle (juce::Graphics& g, juce::Rectangle<float> handleRect);
    void drawMetallicTrack (juce::Graphics& g, juce::Rectangle<float> trackRect);
    void drawMetallicBackground (juce::Graphics& g, juce::Rectangle<float> backgroundRect);
    void drawMarkers (juce::Graphics& g, juce::Rectangle<float> trackRect);
    
    bool isDragging = false;
    juce::Point<float> lastMousePos;
};

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



//------------------------------------------------------------------------------
// Concrete icon buttons (tiny classes = tiny maintenance)
//------------------------------------------------------------------------------
class OptionsButton    : public ThemedIconButton { public: OptionsButton()
: ThemedIconButton(Options{ IconSystem::CogWheel, false, ThemedIconButton::Style::SolidAccentWhenOn, 3.0f, 4.0f, false }) {} };

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
        // Check for metallic properties first - if found, delegate to FieldLNF
        auto metallicKind = metallicFromProps(getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Delegate to FieldLNF for metallic buttons
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                lf->drawButtonBackground(g, *this, juce::Colour(), over, down);
                return;
            }
        }
        
        // Fall back to custom rendering for non-metallic buttons
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
        // Check for metallic properties first - if found, delegate to FieldLNF
        auto metallicKind = metallicFromProps(getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Delegate to FieldLNF for metallic buttons
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                lf->drawButtonBackground(g, *this, juce::Colour(), over, down);
                return;
            }
        }
        
        // Fall back to custom rendering for non-metallic buttons
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        drawBackground(g, r, over, down);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            IconSystem::drawIcon(g, IconSystem::ColorPalette, r.reduced(4.0f), lf->theme.accent);
        }
        else
        {
            IconSystem::drawIcon(g, IconSystem::ColorPalette, r.reduced(4.0f), lf ? lf->theme.accent : juce::Colour(0xFF5AA9E6));
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
        // Check for metallic properties first - if found, delegate to FieldLNF
        auto metallicKind = metallicFromProps(getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Delegate to FieldLNF for metallic buttons
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                lf->drawButtonBackground(g, *this, juce::Colour(), over, down);
                return;
            }
        }
        
        // Fall back to custom rendering for non-metallic buttons
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
        // Check for metallic properties first - if found, delegate to FieldLNF
        auto metallicKind = metallicFromProps(getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Delegate to FieldLNF for metallic buttons
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                lf->drawButtonBackground(g, *this, juce::Colour(), isMouseOver, isButtonDown);
                return;
            }
        }
        
        // Fall back to custom rendering for non-metallic buttons
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // shadow
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) g.setColour(lf->theme.shadowDark.withAlpha (0.25f)); else g.setColour(juce::Colour(0x40000000));
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
    
    // Motion parameter management removed - now handled by MotionControlsPane

    void paint (juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void drawHeaderFieldLogo (juce::Graphics& g, juce::Rectangle<float> area) const;
    void timerCallback() override;
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void buttonClicked(juce::Button* button) override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    // Site-wide interaction tracking (proxy captures child events; we patch explicit handlers in cpp)

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
    void mouseMove (const juce::MouseEvent& e) override;
    
    void setScaleFactor (float newScale) override;
    
    // Waveform is now drawn behind XYPad; no explicit push from editor
    void syncXYPadWithParameters();
    void setupTooltips();
    
    bool layoutReady { false };
    
    //--- custom sliders --------------------------------------------------------
    
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
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                    g.setColour(lf->theme.accent.withAlpha(0.8f));
                else
                    g.setColour(juce::Colours::lightblue.withAlpha(0.8f));
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
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                    g.setColour(lf->theme.accent.withAlpha(0.8f));
                else
                    g.setColour(juce::Colours::lightblue.withAlpha(0.8f));
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
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                    g.setColour(lf->theme.text);
                else
                    g.setColour(juce::Colours::ghostwhite);
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
                const bool metallic = (bool) getProperties().getWithDefault ("metallic", false);
                const bool motionGreen = (bool) getProperties().getWithDefault ("motionPurpleBorder", (bool) getProperties().getWithDefault ("motionGreenBorder", false));
                const bool reverbMaroon = (bool) getProperties().getWithDefault ("reverbMaroonBorder", false);
                
                if (isDelayTheme)
                {
                    auto r = getLocalBounds().toFloat().reduced (3.0f);
                    auto panel  = lf->theme.panel.brighter (0.10f);
                    auto border = lf->theme.text; // use default font grey for border
                    g.setColour (panel);  g.fillRoundedRectangle (r, 8.0f);
                    if (showBorder) { g.setColour (border); g.drawRoundedRectangle (r, 8.0f, 1.5f); }
                }
                else if (motionGreen)
                {
                    // Custom paint for Motion cells with deep blue/purple border
                    auto r = getLocalBounds().toFloat();
                    const float rad = 8.0f;
                    const bool metallicOn = (bool) getProperties().getWithDefault ("metallic", false);

                    auto rr = r.reduced (3.0f);
                    if (metallicOn)
                    {
                        // Bluish-purple metallic gradient from theme
                        juce::Colour top = lf->theme.motionPanelTop;
                        juce::Colour bot = lf->theme.motionPanelBot;
                        juce::ColourGradient grad (top, rr.getX(), rr.getY(), bot, rr.getX(), rr.getBottom(), false);
                        g.setGradientFill (grad);
                        g.fillRoundedRectangle (rr, rad);

                        // Subtle vignette for depth
                        juce::ColourGradient vg (juce::Colours::transparentBlack, rr.getCentreX(), rr.getCentreY(),
                                                 juce::Colours::black.withAlpha (0.22f), rr.getCentreX(), rr.getCentreY() - rr.getHeight() * 0.6f, true);
                        g.setGradientFill (vg);
                        g.fillRoundedRectangle (rr, rad);
                    }
                    else
                    {
                        g.setColour (lf->theme.panel);
                        g.fillRoundedRectangle (rr, rad);
                    }

                    juce::DropShadow ds1 (lf->theme.shadowDark.withAlpha (0.35f), 12, { -1, -1 });
                    juce::DropShadow ds2 (lf->theme.shadowLight.withAlpha (0.25f),  6, { -1, -1 });
                    ds1.drawForRectangle (g, rr.getSmallestIntegerContainer());
                    ds2.drawForRectangle (g, rr.getSmallestIntegerContainer());
                    
                    g.setColour (lf->theme.sh.withAlpha (0.18f));
                    g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);
                    
                    if (showBorder)
                    {
                        auto border = r.reduced (2.0f);
                        g.setColour (lf->theme.motionBorder); // purple border from theme
                        if (isMouseOverOrDragging() || hoverActive)
                        {
                            for (int i = 1; i <= 6; ++i)
                            {
                                const float t = (float) i / 6.0f;
                                const float expand = 2.0f + t * 8.0f;
                                g.setColour (lf->theme.motionBorder.withAlpha ((1.0f - t) * 0.22f));
                                g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.35f, 2.0f);
                            }
                        }
                        g.setColour (lf->theme.motionBorder);
                        g.drawRoundedRectangle (border, rad, 1.5f);
                    }
                }
                else if (metallic)
                {
                    auto r = getLocalBounds().toFloat().reduced (3.0f);
                    const float rad = 8.0f;
                    // Darker metallic panel (Ocean-harmonized neutral steel)
                    juce::Colour top = juce::Colour (0xFF9CA4AD);
                    juce::Colour bot = juce::Colour (0xFF6E747C);
                    juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
                    g.setGradientFill (grad);
                    g.fillRoundedRectangle (r, rad);

                    // Brushing lines
                    g.setColour (juce::Colours::white.withAlpha (0.045f));
                    const int step = 1;
                    for (int y = (int) r.getY() + step; y < r.getBottom(); y += step)
                        g.fillRect (juce::Rectangle<int> ((int) r.getX() + 4, y, (int) r.getWidth() - 8, 1));

                    // Static metallic texture (no randomization for performance)
                    {
                        g.setColour (juce::Colours::black.withAlpha (0.040f));
                        const int noiseRows = juce::jmax (1, (int) r.getHeight() / 4);
                        for (int i = 0; i < noiseRows; ++i)
                        {
                            const int y = (int) r.getY() + 2 + i * 4;
                            const int w = juce::jmax (8, (int) r.getWidth() - 12);
                            const int x = (int) r.getX() + 6;
                            g.fillRect (juce::Rectangle<int> (x, y, w, 1));
                        }
                    }

                    // Static diagonal micro-scratches (no randomization for performance)
                    {
                        const int scratches = juce::jmax (6, (int) r.getWidth() / 22);
                        g.setColour (juce::Colours::white.withAlpha (0.035f));
                        for (int i = 0; i < scratches; ++i)
                        {
                            float sx = r.getX() + 6 + std::fmod (i * 3.7f, r.getWidth() - 12);
                            float sy = r.getY() + 6 + std::fmod (i * 2.3f, r.getHeight() - 12);
                            float len = 14.0f;
                            float dx = len * 0.86f;
                            float dy = len * 0.50f;
                            g.drawLine (sx, sy, sx + dx, sy + dy, 1.0f);
                        }
                        g.setColour (juce::Colours::black.withAlpha (0.025f));
                        for (int i = 0; i < scratches; ++i)
                        {
                            float sx = r.getX() + 6 + std::fmod (i * 4.1f, r.getWidth() - 12);
                            float sy = r.getY() + 6 + std::fmod (i * 3.1f, r.getHeight() - 12);
                            float len = 11.0f;
                            float dx = len * -0.80f;
                            float dy = len * 0.58f;
                            g.drawLine (sx, sy, sx + dx, sy + dy, 1.0f);
                        }
                    }

                    // Vignette
                    {
                        juce::ColourGradient vg (juce::Colours::transparentBlack, r.getCentreX(), r.getCentreY(),
                                                 juce::Colours::black.withAlpha (0.16f), r.getCentreX(), r.getCentreY() - r.getHeight() * 0.6f, true);
                        g.setGradientFill (vg);
                        g.fillRoundedRectangle (r, rad);
                    }

                    // Subtle rim
                    g.setColour (lf->theme.sh.withAlpha (0.14f));
                    g.drawRoundedRectangle (r.reduced (1.0f), rad - 1.0f, 0.8f);

                    if (showBorder)
                    {
                        auto border = r.reduced (2.0f);
                        g.setColour (juce::Colour (0xFF5A5F66));
                        if (isMouseOverOrDragging() || hoverActive)
                        {
                            for (int i = 1; i <= 6; ++i)
                            {
                                const float t = (float) i / 6.0f;
                                const float expand = 2.0f + t * 8.0f;
                                g.setColour (juce::Colour (0xFF5A5F66).withAlpha ((1.0f - t) * 0.22f));
                                g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.35f, 2.0f);
                            }
                        }
                        g.setColour (juce::Colour (0xFF51565D));
                        g.drawRoundedRectangle (border, rad, 1.5f);
                    }
                }
                else if (reverbMaroon)
                {
                    // Custom paint for Reverb cells with vintage orange/red maroon border
                    auto r = getLocalBounds().toFloat();
                    const float rad = 8.0f;

                    g.setColour (lf->theme.panel);
                    g.fillRoundedRectangle (r.reduced (3.0f), rad);

                    juce::DropShadow ds1 (lf->theme.shadowDark.withAlpha (0.35f), 12, { -1, -1 });
                    juce::DropShadow ds2 (lf->theme.shadowLight.withAlpha (0.25f),  6, { -1, -1 });
                    ds1.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());
                    ds2.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());

                    g.setColour (lf->theme.sh.withAlpha (0.18f));
                    g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);

                    if (showBorder)
                    {
                        auto border = r.reduced (2.0f);
                        const juce::Colour maroon = juce::Colour (0xFF8E3A2F);
                        if (isMouseOverOrDragging() || hoverActive)
                        {
                            for (int i = 1; i <= 6; ++i)
                            {
                                const float t = (float) i / 6.0f;
                                const float expand = 2.0f + t * 8.0f;
                                g.setColour (maroon.withAlpha ((1.0f - t) * 0.22f));
                                g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.35f, 2.0f);
                            }
                        }
                        g.setColour (maroon);
                        g.drawRoundedRectangle (border, rad, 1.5f);
                    }
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
                // Check for metallic properties first - if found, delegate to FieldLNF
                auto metallicKind = metallicFromProps(getProperties());
                if (metallicKind != MetallicKind::None)
                {
                    // Delegate to FieldLNF for metallic buttons
                    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                    {
                        lf->drawButtonBackground(g, *this, juce::Colour(), over, down);
                        return;
                    }
                }
                
                // Fall back to custom rendering for non-metallic buttons
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
    void layoutMeters(juce::Rectangle<int> metersArea, float s, float sv);

private:
    // Forward declaration for nested divider type used earlier in members
    MyPluginAudioProcessor& proc;
public:
    FieldLNF lnf;
private:
    // XYPad moved into XYTab - no longer direct member
    // Multi-pane visual dock (XY, Spectrum, Imager)
    std::unique_ptr<class PaneManager> panes;
    std::unique_ptr<juce::KeyListener> keyListener;
    
    // Layout and Event Management
    std::unique_ptr<LayoutManager> layoutManager;
    std::unique_ptr<EventManager> eventManager;
    
    // Resize constraints
    int minWidth = 0;
    int minHeight = 0;
    int maxWidth = 3000;
    int maxHeight = 2000;
    
    // UI Components
    GainSlider   gain;
    juce::Slider width, tilt, monoHz, hpHz, lpHz, satDrive, satMix, air, bass, scoop; // includes Scoop
    juce::ComboBox  monoSlopeChoice;
    // Center group controls moved to XYControlsPane (complete implementation there)
    // AUD audition: custom-styled toggle button (non-checkbox)
    AuditionButton monoAuditionButton;
    // Imaging controls
    juce::Slider widthLo, widthMid, widthHi;
    juce::Slider xoverLoHz, xoverHiHz;
    juce::Slider rotationDeg, asymmetry;
    // SHUF sliders moved to Band tab
    PanSlider    panKnob;
    PanSlider    panKnobLeft, panKnobRight;  // split mode pan
    DuckingSlider duckingKnob;
    DuckParamSlider duckAttack, duckRelease, duckThreshold; // Ducking advanced (UI knobs)
    DuckRatioSlider duckRatio;
    juce::ComboBox osSelect;

    // Old preset combo & save button removed
public:
    BypassButton     bypassButton;
private:
public:
    ToggleSwitch     splitToggle;
private:
    
    // Unified controls viewport (stacks Group 1 and Group 2 vertically)
    juce::Viewport controlsViewport;
    juce::Component controlsContent;     // content for the viewport
    juce::Component group1Container;     // holds Group 1 controls (flat 4x16 grid)
    juce::Component group2Container;     // holds Group 2 controls (Delay/Reverb grids)
    
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
    juce::Slider delayTime, delayFeedback, delayWet, delaySpread, delayWidth, delayModRate, delayModDepth, delayWowflutter, delayJitter, delayPreDelay;
    juce::Slider delayHp, delayLp, delayTilt, delaySat, delayDiffusion, delayDiffuseSize;
    juce::Slider delayDuckDepth, delayDuckAttack, delayDuckRelease, delayDuckThreshold, delayDuckRatio, delayDuckLookahead;
    juce::ComboBox delayMode, delayTimeDiv, delayDuckSource, delayGridFlavor, delayFilterType;
    juce::ToggleButton delayEnabled, delaySync, delayKillDry, delayFreeze, delayPingpong, delayDuckPost, delayDuckLinkGlobal;
    
    // Icon buttons (shared base)
public:
    OptionsButton    optionsButton;
private:
public:
    LinkButton       linkButton;
    SnapButton       snapButton;
private:
public:
    FullScreenButton fullScreenButton;
    ColorModeButton  colorModeButton;
    TooltipsButton   tooltipsButton; // Wrench icon (Options) toggles tooltip assistant
    // History and undo/redo removed
    HelpButton       helpButton;
private:
public:
    CopyButton       copyButton;
private:
    LockButton       lockButton;

public:
    bool tooltipAssistantOn_ { false }; // Header wrench toggle state
private:

    // Lightweight tooltip bubble shown when tooltip assistant is ON
public:
    TooltipBubble tooltipBubble;
private:
    juce::Component* lastTooltipTarget { nullptr };

    // Global Wet Only (Kill Dry) UI toggle (no param binding per instructions)
    juce::ToggleButton wetOnlyToggle;

    // Reverb controls
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
    
    // OLD REVERB SYSTEM REMOVED - Now using new reverb system in Source/reverb/

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
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) g.setColour (lf->theme.shadowDark.withAlpha (0.25f)); else g.setColour (juce::Colour (0x40000000));
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
    
public:
    ABButton abButtonA{true}, abButtonB{false};
    PresetArrowButton prevPresetButton{true}, nextPresetButton{false};
    juce::TextButton presetField; // clickable field to open palette and display current preset
    juce::Label presetNameLabel;
private:
public:
    juce::Label transportClockLabel;
private:
public:
    juce::Component headerLeftGroup; // container for bypass (no logo)
private:

    // Split-pan container placeholder for grid cell (no painting, no mouse)
    juce::Component panSplitContainer;
    
    // Preset system
    PresetStore presetStore;
    NewPresetManager presetManager;
    
    // Containers
    ControlContainer mainControlsContainer, volumeContainer;
    ControlContainer delayContainer;
    ControlContainer metersContainer;
    ControlContainer MainContentContainer;
    ControlContainer rightSlidersContainer;  // Container for sliders on the right
    
    // 3D Vertical Sliders for Input, Output, Mix
    VerticalSlider3D inputSlider, outputSlider, mixSlider;
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
    juce::Label widthLoValue, widthMidValue, widthHiValue, xoverLoValue, xoverHiValue, rotationValue, asymValue;
    juce::Label delayTimeValue, delayFeedbackValue, delayWetValue, delaySpreadValue, delayWidthValue, delayModRateValue, delayModDepthValue, delayWowflutterValue, delayJitterValue, delayPreDelayValue;
    juce::Label delayHpValue, delayLpValue, delayTiltValue, delaySatValue, delayDiffusionValue, delayDiffuseSizeValue;
    juce::Label delayDuckDepthValue, delayDuckAttackValue, delayDuckReleaseValue, delayDuckThresholdValue, delayDuckRatioValue, delayDuckLookaheadValue;
    // Imaging knob name labels (third row)
    juce::Label widthLoName, widthMidName, widthHiName;
    juce::Label xoverLoName, xoverHiName;
    juce::Label rotationName, asymName;
    juce::Label shufLoName, shufHiName, shufXName;
    
    // Delay name labels
    juce::Label delayTimeName, delayFeedbackName, delayWetName, delaySpreadName, delayWidthName, delayModRateName, delayModDepthName, delayWowflutterName, delayJitterName, delayPreDelayName;
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
    // SHUF cells moved to Band tab
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
    std::unique_ptr<KnobCell> delayPreDelayCell;
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
    std::unique_ptr<KnobCell> delayDuckRatioCell;

    // Delay control cells (buttons/combos, styled like KnobCell panels)
    std::unique_ptr<SwitchCell> delayEnabledCell;
    std::unique_ptr<SwitchCell> delayModeCell;
    std::unique_ptr<SwitchCell> delaySyncCell;
    std::unique_ptr<SwitchCell> delayGridFlavorCell;
    std::unique_ptr<SwitchCell> delayFreezeCell;
    std::unique_ptr<SwitchCell> delayKillDryCell;
    std::unique_ptr<SwitchCell> delayPingpongCell;
    std::unique_ptr<SwitchCell> delayFilterTypeCell;
    std::unique_ptr<SwitchCell> delayDuckSourceCell;
    std::unique_ptr<SwitchCell> delayDuckPostCell;

    // Delay group positioned directly in Group 2 panel (no container)

    std::unique_ptr<Segmented3Control> delayGridFlavorSegments;

    void buildCells();

    // [moved] Attachment containers declared after all bound controls (see below)
    
    // Scaling
public:
    float scaleFactor = 1.0f;
    const int baseWidth  = 1500; // 75% of 2000 to try a smaller default
    const int baseHeight = 1000; // increased for Dynamic EQ controls and better layout
private:
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

    // Bottom area toggle and alternate overlay panel
    juce::TextButton bottomAreaToggle; // bottom-center toggle to reveal alternate bottom controls
    struct BottomAltPanel : public juce::Component {
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            // Match main site background gradient exactly (Editor::paint)
            juce::Colour top    = juce::Colour (0xFF2A2C30);
            juce::Colour mid    = juce::Colour (0xFF4A4D55);
            juce::Colour bottom = juce::Colour (0xFF2A2C30);
            juce::ColourGradient bg (top, r.getCentreX(), r.getY(),
                                     bottom, r.getCentreX(), r.getBottom(), false);
            bg.addColour (0.85, mid);
            g.setGradientFill (bg);
            // Fill with rounded corners instead of fillAll()
            g.fillRoundedRectangle (r, 6.0f);
        }
    } bottomAltPanel;          // sliding overlay panel that covers bottom control rows, with gradient BG
    bool bottomAltTargetOn { false };  // target state for slide animation
    float bottomAltSlide01 { 0.0f };   // 0..1 slide progress (0=hidden)
    bool bottomAltAnimating { false }; // animate slide in timer
    // Group 2 overlay layout caching to avoid per-frame reflow
    bool overlayLayoutDirty { true };
    bool overlayContentsBuilt { false };
    juce::Rectangle<int> overlayLocalRect;  // leftContentContainer-local rect for Group 2 overlay
    int overlayActiveBaseline { 0 };
    int overlayHiddenBaseline { 0 };
    int overlayHeightPx { 0 };
    // Fast-path overlay movement during slide (no child reflow)
    void updateGroup2OverlayDuringSlide();
    // New Reverb panel mounted in Group 2
    std::unique_ptr<class ReverbGraphics> reverbPanel;
    int controlRowsHeightPx { 0 };

    // Correlation meter mini component
    class CorrelationMeter : public juce::Component, public juce::Timer
    {
    public:
        ~CorrelationMeter() override { stopTimer(); }
        
        CorrelationMeter (MyPluginAudioProcessor& p, FieldLNF& l) : proc (p), lnf (l) { startTimerHz (25); }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            // Adjust positioning: top moves up 3px (5->2), bottom moves down 2px
            r = r.withY (r.getY() + 2.0f).withHeight (r.getHeight() - 2.0f + 2.0f);
            
            g.setColour (lnf.theme.meters.trackBase);
            g.fillRoundedRectangle (r, 6.0f);
            
            // Standard border treatment: accent border (reduced brightness for meters)
            g.setColour (lnf.theme.accent.withAlpha (0.3f));
            g.drawRoundedRectangle (r, 6.0f, 1.0f);

            const float corr = juce::jlimit (-1.0f, 1.0f, proc.getCorrelation());
            // Thin vertical track centered horizontally
            const float pad = 3.0f;
            const float trackW = juce::jmax (6.0f, r.getWidth() - 2*pad);
            const float cx = r.getX() + r.getWidth() * 0.5f;
            juce::Rectangle<float> track (cx - trackW * 0.5f, r.getY() + pad, trackW, r.getHeight() - 2*pad);
            // Midline
            const float midY = track.getCentreY();
            g.setColour (lnf.theme.hl.withAlpha (0.35f));
            g.fillRoundedRectangle (track, 2.5f);
            g.setColour (lnf.theme.hl.withAlpha (0.6f));
            g.fillRect (juce::Rectangle<float> (track.getX(), midY-0.5f, track.getWidth(), 1.0f));

            // Positive = fill upward; Negative = fill downward
            if (corr >= 0.0f)
            {
                const float h = (track.getHeight() * 0.5f) * corr;
                g.setColour (lnf.theme.meters.positive);
                g.fillRoundedRectangle (juce::Rectangle<float> (track.getX(), midY - h, track.getWidth(), h), 2.0f);
            }
            else
            {
                const float h = (track.getHeight() * 0.5f) * (-corr);
                g.setColour (lnf.theme.meters.negative);
                g.fillRoundedRectangle (juce::Rectangle<float> (track.getX(), midY, track.getWidth(), h), 2.0f);
            }

            // Peak line (thicker bottom border like LR meters)
            g.setColour (lnf.theme.accent.withAlpha (0.6f));
            g.fillRect (juce::Rectangle<float> (track.getX(), track.getBottom() - 1.0f, track.getWidth(), 2.0f));
            
            // Vertical label on the right side: C O R R
            g.setColour (lnf.theme.textMuted);
            g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
            const float labelX = track.getRight() + 2.0f;
            const float step   = 12.0f;
            juce::String chars[] = { "C", "O", "R", "R" };
            float y = r.getY() + pad;
            for (auto& ch : chars)
            {
                g.drawText (ch, juce::Rectangle<int> ((int)labelX, (int)y, (int)(r.getRight()-labelX-1.0f), 12), juce::Justification::centredLeft);
                y += step;
            }
        }
        void timerCallback() override { repaint(); }
        void visibilityChanged() override { if (isVisible()) startTimerHz (15); else stopTimer(); }
    private:
        MyPluginAudioProcessor& proc;
        FieldLNF& lnf;
    };

    CorrelationMeter corrMeter { proc, lnf };

    // Vertical L/R meters (RMS + Peak overlays)
    class VerticalLRMeters : public juce::Component, public juce::Timer
    {
    public:
        ~VerticalLRMeters() override { stopTimer(); }
        
        VerticalLRMeters (MyPluginAudioProcessor& p, FieldLNF& l) : proc(p), lnf(l) { startTimerHz (30); }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto left = r.removeFromLeft (r.getWidth() * 0.5f).reduced (2.0f);
            auto right= r.reduced (2.0f);

            auto drawBar = [&] (juce::Rectangle<float> b, float rms, float peak, const juce::String& label)
            {
                g.setColour (lnf.theme.meters.trackBase);
                g.fillRoundedRectangle (b, 4.0f);
                // Track
                {
                    juce::Colour base = lnf.theme.meters.trackBase;
                    juce::Colour base2 = lnf.theme.meters.trackActive;
                    juce::ColourGradient grad (base, b.getX(), b.getY(), base2, b.getX(), b.getBottom(), false);
                    juce::FillType ft (grad);
                    g.setFillType (ft);
                    g.fillRoundedRectangle (b.reduced (1.0f), 3.5f);
                    g.setFillType (juce::FillType());
                }
                // Standard border treatment: accent border (reduced brightness for meters)
                g.setColour (lnf.theme.accent.withAlpha (0.3f));
                g.drawRoundedRectangle (b, 4.0f, 1.0f);

                // scale 0..1 across height
                auto hRms  = juce::jlimit (0.0f, 1.0f, rms ) * b.getHeight();
                auto hPeak = juce::jlimit (0.0f, 1.0f, peak) * b.getHeight();
                // Zone colours (dBFS thresholds)
                const float rmsDb  = juce::Decibels::gainToDecibels (juce::jlimit (0.000001f, 1.0f, rms),  -60.0f);
                const float peakDb = juce::Decibels::gainToDecibels (juce::jlimit (0.000001f, 1.0f, peak), -60.0f);
                const bool risk    = peakDb >= -1.0f || rmsDb >= -3.0f;
                const bool warn    = !risk && (peakDb >= -6.0f || rmsDb >= -12.0f);
                {
                    juce::Colour safe1 = lnf.theme.accent.withAlpha (0.70f);
                    juce::Colour safe2 = lnf.theme.accent.withAlpha (0.90f);
                    juce::Colour warn1 = juce::Colour (0xFFFFC107).withAlpha (0.75f); // amber
                    juce::Colour warn2 = juce::Colour (0xFFFFA000).withAlpha (0.95f);
                    juce::Colour risk1 = juce::Colour (0xFFFF8A80).withAlpha (0.85f); // soft red
                    juce::Colour risk2 = juce::Colour (0xFFE53935).withAlpha (0.95f);
                    auto c1 = risk ? risk1 : (warn ? warn1 : safe1);
                    auto c2 = risk ? risk2 : (warn ? warn2 : safe2);
                    juce::ColourGradient grad (c1, b.getCentreX(), b.getBottom() - hRms,
                                               c2, b.getCentreX(), b.getBottom(), false);
                    g.setFillType (juce::FillType (grad));
                    g.fillRoundedRectangle (juce::Rectangle<float> (b.getX(), b.getBottom() - hRms, b.getWidth(), hRms), 3.0f);
                    g.setFillType (juce::FillType());
                }
                // Peak line
                g.setColour (risk ? juce::Colour (0xFFE53935) : lnf.theme.accent);
                g.fillRect (juce::Rectangle<float> (b.getX(), b.getBottom() - hPeak, b.getWidth(), 2.0f));
                // Crest factor hint (thin line slightly below peak)
                const float crestH = juce::jmax (0.0f, hPeak - hRms);
                if (crestH > 2.0f)
                {
                    g.setColour (lnf.theme.text.withAlpha (0.20f));
                    g.fillRect (juce::Rectangle<float> (b.getX(), b.getBottom() - hPeak + 2.0f, b.getWidth(), 1.0f));
                }
                // Gloss
                g.setColour (juce::Colours::white.withAlpha (0.06f));
                g.fillRoundedRectangle (juce::Rectangle<float> (b.getX()+1.5f, b.getY()+1.5f, b.getWidth()-3.0f, b.getHeight()*0.25f), 3.0f);

                // Label
                g.setColour (lnf.theme.textMuted);
                g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
                g.drawText (label, b.reduced (4.0f), juce::Justification::centredBottom);

                // dB tick marks (approx) across the bar: -24, -12, -6, -3, -1 dBFS
                auto drawTick = [&] (float db, const char* text)
                {
                    // map dB to linear magnitude (0..1). For UI, assume 0 = -inf, 1 = 0 dBFS
                    const float lin = juce::Decibels::decibelsToGain (db, -60.0f);
                    const float y   = b.getBottom() - juce::jlimit (0.0f, 1.0f, lin) * b.getHeight();
                    g.setColour (lnf.theme.hl.withAlpha (0.6f));
                    g.fillRect (juce::Rectangle<float> (b.getX(), y, b.getWidth(), 1.0f));
                    g.setColour (lnf.theme.textMuted.withAlpha (0.8f));
                    g.drawText (text, juce::Rectangle<int> (b.getX(), (int) y - 8, (int) b.getWidth(), 12), juce::Justification::centredRight);
                };
                drawTick (-24.0f, "-24");
                drawTick (-12.0f, "-12");
                drawTick (-6.0f,  "-6");
                drawTick (-3.0f,  "-3");
                drawTick (-1.0f,  "-1");
            };

            drawBar (left,  proc.getRmsL(), proc.getPeakL(), "L");
            drawBar (right, proc.getRmsR(), proc.getPeakR(), "R");
        }
        void timerCallback() override { if (isShowing()) repaint(); }
        void visibilityChanged() override { if (isVisible()) startTimerHz (20); else stopTimer(); }
    private:
        MyPluginAudioProcessor& proc;
        FieldLNF& lnf;
    };

    VerticalLRMeters lrMeters { proc, lnf };

    // IO Gain Meters (Input/Output RMS)
    class IOGainMeters : public juce::Component, public juce::Timer
    {
    public:
        ~IOGainMeters() override { stopTimer(); }
        
        IOGainMeters (MyPluginAudioProcessor& p, FieldLNF& l) : proc(p), lnf(l) { startTimerHz (30); }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto inB  = r.removeFromLeft (r.getWidth() * 0.5f).reduced (2.0f);
            auto outB = r.reduced (2.0f);
            auto drawOne = [&] (juce::Rectangle<float> b, float rms, const juce::String& label)
            {
                g.setColour (lnf.theme.meters.trackBase); g.fillRoundedRectangle (b, 4.0f);
                // Standard border treatment: accent border (reduced brightness for meters)
                g.setColour (lnf.theme.accent.withAlpha (0.3f));
                g.drawRoundedRectangle (b, 4.0f, 1.0f);
                const float h = juce::jlimit (0.0f, 1.0f, rms) * b.getHeight();
                const float rmsDb = juce::Decibels::gainToDecibels (juce::jlimit (0.000001f, 1.0f, rms), -60.0f);
                const bool risk = rmsDb >= -1.0f;
                const bool warn = !risk && rmsDb >= -6.0f;
                juce::Colour safe1 = lnf.theme.meters.safe.withAlpha (0.55f);
                juce::Colour safe2 = lnf.theme.meters.safe.withAlpha (0.85f);
                juce::Colour warn1 = lnf.theme.meters.warning.withAlpha (0.70f);
                juce::Colour warn2 = lnf.theme.meters.warning.withAlpha (0.90f);
                juce::Colour risk1 = lnf.theme.meters.error.withAlpha (0.80f);
                juce::Colour risk2 = lnf.theme.meters.error.withAlpha (0.95f);
                auto c1 = risk ? risk1 : (warn ? warn1 : safe1);
                auto c2 = risk ? risk2 : (warn ? warn2 : safe2);
                juce::ColourGradient grad (c1, b.getCentreX(), b.getBottom() - h,
                                           c2, b.getCentreX(), b.getBottom(), false);
                g.setFillType (juce::FillType (grad));
                g.fillRoundedRectangle (juce::Rectangle<float> (b.getX(), b.getBottom() - h, b.getWidth(), h), 3.0f);
                g.setFillType (juce::FillType());
                
                // Peak line (thicker bottom border like LR meters)
                g.setColour (risk ? juce::Colour (0xFFE53935) : lnf.theme.accent);
                g.fillRect (juce::Rectangle<float> (b.getX(), b.getBottom() - h, b.getWidth(), 2.0f));
                
                g.setColour (lnf.theme.textMuted);
                g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
                g.drawText (label, b.reduced (4.0f), juce::Justification::centredBottom);
            };
            drawOne (inB,  proc.getInRms(),  "I");
            drawOne (outB, proc.getOutRms(), "O");
        }
        void timerCallback() override { if (isShowing()) repaint(); }
        void visibilityChanged() override { if (isVisible()) startTimerHz (20); else stopTimer(); }
    private:
        MyPluginAudioProcessor& proc; FieldLNF& lnf;
    };

    IOGainMeters ioMeters { proc, lnf };

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

    // Adaptive UI refresh (burst to 60 Hz during user interaction)
    juce::uint32 lastUserInteractionMs { 0 };
    int          uiTimerHzCurrent { 30 };
    void noteUserInteraction() { lastUserInteractionMs = juce::Time::getMillisecondCounter(); }

    struct BurstMouseProxy : public juce::MouseListener
    {
        explicit BurstMouseProxy (MyPluginAudioProcessorEditor& o) : owner (o) {}
        void mouseDown (const juce::MouseEvent&) override               { owner.noteUserInteraction(); }
        void mouseDrag (const juce::MouseEvent&) override               { owner.noteUserInteraction(); }
        void mouseUp   (const juce::MouseEvent&) override               { owner.noteUserInteraction(); }
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override { owner.noteUserInteraction(); }
    private:
        MyPluginAudioProcessorEditor& owner;
    };
    std::unique_ptr<BurstMouseProxy> burstMouseProxy;

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
        ~ShadeOverlay() override { stopTimer(); }
        
        explicit ShadeOverlay (FieldLNF& lnfRef) : lnf(lnfRef)
        {
            setAlwaysOnTop(true);
            setInterceptsMouseClicks(true, true);
            amount.reset(0.0, 0.12);
            amount.setCurrentAndTargetValue(0.0f);
            startTimerHz(30);
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

                drawFieldLogo(g, cover);
            }

            drawHandle(g, getHandle());
        }

        void visibilityChanged() override { if (isVisible()) startTimerHz(30); else stopTimer(); }

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

        float shadeEdgeY () const 
        { 
            // Simplified: just return the actual shade edge position
            auto r = getLocalBounds().toFloat();
            return r.getHeight() * getAmount();
        }

        juce::Rectangle<float> getHandle () const
        {
            auto r = getLocalBounds().toFloat();
            float tabW = juce::jmin(120.0f, r.getWidth() * 0.6f);
            float tabH = 26.0f;  // Slightly reduced from 30.0f for better proportions
            
            // Position handle so its BOTTOM edge aligns with the shade edge
            const float edge = shadeEdgeY();
            float y = juce::jlimit (0.0f, r.getHeight() - tabH, edge - tabH);
            
            // Center the handle horizontally within the extended bounds
            float x = r.getCentreX() - tabW * 0.5f;
            
            return { x, y, tabW, tabH };
        }

        void drawHandle (juce::Graphics& g, juce::Rectangle<float> tab) const
        {
            // Base handle background
            g.setColour (lnf.theme.meters.trackBase.withAlpha (0.85f));
            g.fillRoundedRectangle (tab, 8.0f);
            
            // Hover effects with proper accent colors and outer glow
            if (hoverHandle)
            {
                // Use the theme accent color directly (now darker)
                auto accentColor = lnf.theme.accent;
                
                // True outer glow effect - draw multiple shadow layers for proper outer glow
                juce::DropShadow outerGlow1 (accentColor.withAlpha (0.4f), 20, {0, 0});
                outerGlow1.drawForRectangle (g, tab.getSmallestIntegerContainer());
                
                juce::DropShadow outerGlow2 (accentColor.withAlpha (0.2f), 12, {0, 0});
                outerGlow2.drawForRectangle (g, tab.getSmallestIntegerContainer());
                
                // Accent border using theme accent color
                g.setColour (accentColor.withAlpha (0.9f));
                g.drawRoundedRectangle (tab, 8.0f, 1.5f);
            }
            else
            {
                // Normal border using theme highlight
                g.setColour (lnf.theme.hl.withAlpha (0.6f));
                g.drawRoundedRectangle (tab, 8.0f, 1.0f);
            }

            // Dashed grip bars with hover accent
            const int numBars = 4;
            const float barW = 10.0f, barH = 6.0f, gap = 14.0f;
            const float totalW = numBars * barW + (numBars - 1) * gap;
            float startX = tab.getCentreX() - totalW * 0.5f;
            float y = tab.getCentreY() - barH * 0.5f;

            // Grip bars with theme accent color
            g.setColour (lnf.theme.accent.withAlpha (hoverHandle ? 0.9f : 0.7f));
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

        void drawFieldLogo (juce::Graphics& g, juce::Rectangle<float> area) const
        {
            // Calculate logo size based on covered area - increased to 80%
            const float logoHeight = juce::jmin(area.getHeight() * 0.8f, 200.0f);
            const float logoWidth = logoHeight * 2.5f; // FIELD is wider than tall
            
            // Center the logo in the covered area
            const float logoX = area.getCentreX() - logoWidth * 0.5f;
            const float logoY = area.getCentreY() - logoHeight * 0.5f;
            const auto logoRect = juce::Rectangle<float>(logoX, logoY, logoWidth, logoHeight);
            
            // Create large bold font matching the main logo
            juce::Font logoFont(juce::FontOptions(logoHeight * 0.8f).withStyle("Bold"));
            g.setFont(logoFont);
            
            // Enhanced shadow system with stronger effects (matching header)
            const int shadowLayers = 12; // Increased from 8 to 12
            for (int i = shadowLayers; i > 0; --i)
            {
                const float shadowOffset = (float)i * 3.5f; // Increased offset for more dramatic effect
                const float shadowAlpha = (1.0f - (float)i / shadowLayers) * 0.7f; // Increased alpha for stronger effect (matching header)
                
                // Multiple glow shadows with different colors and intensities
                // Outer accent glow (stronger)
                g.setColour(lnf.theme.accent.withAlpha(shadowAlpha * 0.8f));
                g.drawText("FIELD", logoRect.translated(shadowOffset, shadowOffset), 
                          juce::Justification::centred);
                
                // Secondary glow with brighter accent (stronger)
                g.setColour(lnf.theme.accent.brighter(0.4f).withAlpha(shadowAlpha * 0.6f));
                g.drawText("FIELD", logoRect.translated(shadowOffset * 0.8f, shadowOffset * 0.8f), 
                          juce::Justification::centred);
                
                // Dark shadow for depth with increased intensity (stronger)
                g.setColour(juce::Colours::black.withAlpha(shadowAlpha * 0.9f));
                g.drawText("FIELD", logoRect.translated(shadowOffset * 0.5f, shadowOffset * 0.5f), 
                          juce::Justification::centred);
                
                // Additional depth shadow (stronger)
                g.setColour(juce::Colours::darkgrey.withAlpha(shadowAlpha * 0.5f));
                g.drawText("FIELD", logoRect.translated(shadowOffset * 0.6f, shadowOffset * 0.6f), 
                          juce::Justification::centred);
            }
            
            // Enhanced gradient effect with stronger effects (matching header)
            juce::ColourGradient logoGradient(
                lnf.theme.accent.brighter(0.6f), logoRect.getX(), logoRect.getY(),
                lnf.theme.accent.darker(0.3f), logoRect.getX(), logoRect.getBottom(), false);
            logoGradient.addColour(0.25f, lnf.theme.accent.brighter(0.3f));
            logoGradient.addColour(0.5f, lnf.theme.accent);
            logoGradient.addColour(0.75f, lnf.theme.accent.darker(0.1f));
            
            g.setGradientFill(logoGradient);
            g.drawText("FIELD", logoRect, juce::Justification::centred);
            
            // Enhanced highlight system with stronger effects (matching header)
            // Primary highlight (stronger)
            g.setColour(lnf.theme.accent.brighter(0.7f).withAlpha(0.9f));
            g.drawText("FIELD", logoRect, juce::Justification::centred);
            
            // Secondary highlight for extra shine (stronger)
            g.setColour(lnf.theme.accent.brighter(0.9f).withAlpha(0.5f));
            g.drawText("FIELD", logoRect, juce::Justification::centred);
            
            // Final white highlight for maximum shine (stronger)
            g.setColour(juce::Colours::white.withAlpha(0.4f));
            g.drawText("FIELD", logoRect, juce::Justification::centred);
        }
    };

    std::unique_ptr<ShadeOverlay> xyShade;

    // Mini vertical divider near split toggle

    VerticalDivider splitDivider{lnf}, eqDivLpMono{lnf}, eqDivScoopHp{lnf};
    VerticalDivider volDivPanSpace{lnf}, volDivDuckRight{lnf}, delayDivider{lnf}, motionDivider{lnf};
    // Horizontal dividers between rows

    HorizontalDivider rowDivVol{lnf}, rowDivEQ{lnf};

    // Motion controls removed - now handled by MotionControlsPane
    // Attachment containers (declared AFTER all sliders/buttons/combos they bind to)
    // Ensures attachments are destroyed BEFORE controls during teardown
    std::vector<std::unique_ptr<SliderAttachment>>  attachments;
    std::vector<std::unique_ptr<ButtonAttachment>>  buttonAttachments;
    std::vector<std::unique_ptr<ComboAttachment>>   comboAttachments;
    // Motion attachments removed - now handled by MotionControlsPane
    
    // Motion ComboBoxes and Buttons removed - now handled by MotionControlsPane
    
    // Motion SwitchCell wrappers removed - now handled by MotionControlsPane

    // Phase Mode center group
    ControlContainer phaseCenterContainer;
public:
    PhaseModeButton  phaseModeButton;
    QualityButton    qualityButton;
private:
    std::unique_ptr<juce::ParameterAttachment> phaseModeParamAttach;
    std::unique_ptr<juce::ParameterAttachment> osModeParamAttach;
    std::unique_ptr<juce::ParameterAttachment> qualityParamAttach;

    // Motion parameter management removed - now handled by MotionControlsPane
    
    // MotionBinding removed - now handled by MotionControlsPane

    // Delay visuals are managed by PaneManager's Delay tab

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessorEditor)
}; 
