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
#include "ui/Components/VerticalSlider3D.h"
#include "ui/Components/ToggleSwitch.h"
#include "ui/Components/CorrelationMeter.h"
#include "ui/Components/MonoSlopeSwitch.h"
#include "ui/Components/ShadeOverlay.h"
#include "ui/Components/VerticalLRMeters.h"
#include "ui/Components/IOGainMeters.h"
#include "ui/Components/SwitchCell.h"
#include "ui/Components/Segmented3Control.h"
#include "ui/Components/GainSlider.h"
#include "ui/Components/PanSlider.h"
#include "ui/Components/ControlContainer.h"
#include "ui/Components/UIHelpers.h"
#include "ui/Layout/LayoutManager.h"
#include "ui/Events/EventManager.h"
#include "ui/Managers/AttachmentManager.h"
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
    


    
    // Lightweight container cell for non-knob components (e.g., switches)
    
    
    // Resize handle functionality
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

    // Layout management
    void performLayout();
    void layoutMeters(juce::Rectangle<int> metersArea, float s, float sv);

private:
    // Forward declaration for nested divider type used earlier in members
public:
    MyPluginAudioProcessor& proc;
    FieldLNF lnf;
    // XYPad moved into XYTab - no longer direct member
    // Multi-pane visual dock (XY, Spectrum, Imager)
    std::unique_ptr<class PaneManager> panes;
private:
    std::unique_ptr<juce::KeyListener> keyListener;
    
    // Layout and Event Management
    std::unique_ptr<LayoutManager> layoutManager;
    std::unique_ptr<EventManager> eventManager;
    std::unique_ptr<AttachmentManager> attachmentManager;
    
    // Resize constraints
public:
    int minWidth = 0;
    int minHeight = 0;
    int maxWidth = 3000;
    int maxHeight = 2000;
    int baseWidth = 1500; // 75% of 2000 to try a smaller default
    int baseHeight = 1000; // increased for Dynamic EQ controls and better layout
    float scaleFactor = 1.0f;
    bool tooltipAssistantOn_ = false;
    juce::Component* lastTooltipTarget = nullptr;
    juce::uint32 lastUserInteractionMs = 0;
    int uiTimerHzCurrent = 30;
private:
    
    // UI Components
public:
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
public:
    juce::Slider tiltFreqSlider, scoopFreqSlider, bassFreqSlider, airFreqSlider;
    // EQ shape/Q controls
    juce::Slider shelfShapeS, filterQ;
    juce::ToggleButton tiltLinkSButton, qLinkButton;
    juce::Slider hpQSlider, lpQSlider;
private:
    // Q-cluster dummy hosts (no visible knob in cluster)
    juce::Slider qClusterDummySlider;
    juce::Label  qClusterDummyValue;
    
    // Delay controls
public:
    juce::Slider delayTime, delayFeedback, delayWet, delaySpread, delayWidth, delayModRate, delayModDepth, delayWowflutter, delayJitter, delayPreDelay;
    juce::Slider delayHp, delayLp, delayTilt, delaySat, delayDiffusion, delayDiffuseSize;
    juce::Slider delayDuckDepth, delayDuckAttack, delayDuckRelease, delayDuckThreshold, delayDuckRatio, delayDuckLookahead;
    juce::ComboBox delayMode, delayTimeDiv, delayDuckSource, delayGridFlavor, delayFilterType;
    juce::ToggleButton delayEnabled, delaySync, delayKillDry, delayFreeze, delayPingpong, delayDuckPost, delayDuckLinkGlobal;
private:
    
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
private:

    // Lightweight tooltip bubble shown when tooltip assistant is ON
    public:
    TooltipBubble tooltipBubble;
private:

    // Global Wet Only (Kill Dry) UI toggle (no param binding per instructions)
    juce::ToggleButton wetOnlyToggle;

    public:
public:
    // Placeholder for mono slope switch definition (defined after SpaceAlgorithmSwitch)
    std::unique_ptr<::MonoSlopeSwitch> monoSlopeSwitch;
    
    // OLD REVERB SYSTEM REMOVED - Now using new reverb system in Source/reverb/

    std::unique_ptr<SwitchCell> wetOnlyCell;

    // Dedicated Mono Slope Switch (6/12/24) with independent drawing but same visual language
    // MonoSlopeSwitch class extracted to Source/ui/Components/MonoSlopeSwitch.h
    
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
public:
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
    ControlContainer panKnobContainer;
    
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
public:
    juce::Label leftIndicator, rightIndicator;
    juce::Label gainValue, widthValue, tiltValue, monoValue, hpValue, lpValue, satDriveValue, satMixValue, airValue, bassValue, scoopValue;
    juce::Label shelfShapeValue, filterQValue;
    juce::Label monoSlopeName, monoAudName;
    juce::Label panValue, panValueLeft, panValueRight;
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
    juce::Label panL;

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
public:
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

    CorrelationMeter corrMeter { proc, lnf };

    // Vertical L/R meters (RMS + Peak overlays)

    VerticalLRMeters lrMeters { proc, lnf };

    // IO Gain Meters (Input/Output RMS)

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
public:
    void toggleABState();
    void copyState(bool copyFromA);
    void pasteState(bool pasteToA);
    void updatePresetDisplay();
    
    // Header hover
    bool headerHovered = false;
    bool headerHoverActive = false;
    const int headerHoverOffDelayMs = 160;

    // Adaptive UI refresh (burst to 60 Hz during user interaction)
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

    std::unique_ptr<ShadeOverlay> xyShade;






      

    // Mini vertical divider near split toggle

    VerticalDivider splitDivider{lnf}, eqDivLpMono{lnf}, eqDivScoopHp{lnf};
    VerticalDivider volDivPanSpace{lnf}, volDivDuckRight{lnf}, delayDivider{lnf}, motionDivider{lnf};
    // Horizontal dividers between rows

    HorizontalDivider rowDivVol{lnf}, rowDivEQ{lnf};

    // Motion controls removed - now handled by MotionControlsPane
    // Parameter attachments now handled by AttachmentManager
    
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
