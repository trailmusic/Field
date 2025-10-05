#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "processor/PluginProcessor.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/FieldTheme.h"
#include "shared/Core/FieldMetallic.h"
#include "shared/Core/IconSystem.h"
#include "shared/ui/Components/KnobCell.h"
#include "shared/ui/Components/BypassButton.h"
#include "shared/ui/Components/ThemedIconButton.h"
#include "shared/ui/Components/AuditionButton.h"
#include "shared/ui/Components/ABButton.h"
#include "shared/ui/Components/PhaseModeButton.h"
#include "shared/ui/Components/QualityButton.h"
#include "shared/ui/Components/TooltipsButton.h"
#include "shared/ui/Components/HelpButton.h"
#include "shared/ui/Components/TooltipBubble.h"
#include "shared/ui/Components/VerticalDivider.h"
#include "shared/ui/Components/HorizontalDivider.h"
#include "shared/ui/Components/VerticalSlider3D.h"
#include "shared/ui/Components/ToggleSwitch.h"
#include "shared/ui/Components/CorrelationMeter.h"
#include "shared/ui/Components/MonoSlopeSwitch.h"
#include "shared/ui/Components/ShadeOverlay.h"
#include "shared/ui/Components/VerticalLRMeters.h"
#include "shared/ui/Components/IOGainMeters.h"
#include "shared/ui/Components/TransportClock.h"
#include "shared/ui/Managers/MeterManager.h"
#include "shared/ui/Managers/SliderManager.h"
#include "shared/ui/Components/SwitchCell.h"
#include "shared/ui/Components/Segmented3Control.h"
#include "shared/ui/Components/SimpleIconButtons.h"
#include "shared/ui/Components/ComplexIconButtons.h"
#include "shared/ui/Components/PresetArrowButton.h"
#include "shared/ui/Components/GainSlider.h"
#include "shared/ui/Components/PanSlider.h"
#include "shared/ui/Components/ControlContainer.h"
#include "shared/ui/Components/UIHelpers.h"
#include "shared/ui/Layout/LayoutManager.h"
#include "shared/ui/Events/EventManager.h"
#include "shared/ui/Managers/AttachmentManager.h"
#include "shared/ui/Managers/CleanupManager.h"
#include "shared/ui/Managers/PaintManager.h"
#include "shared/ui/Managers/StateManager.h"
#include "shared/Presets/PresetRegistry.h"
#include "shared/Presets/PresetCommandPalette.h"
#include "shared/Presets/PresetManager.h"
#include "shared/ui/Engines/StereoFieldEngine.h"
#include "shared/ui/Managers/PaneManager.h"

using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

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
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sliderValueChanged (juce::Slider* slider) override;
    void comboBoxChanged (juce::ComboBox* comboBox) override;
    void buttonClicked (juce::Button* button) override;
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    void mouseEnter (const juce::MouseEvent& e) override
    {
        auto headerBounds = getLocalBounds().removeFromTop (static_cast<int> (60 * scaleFactor));
        if (headerBounds.contains (e.position.toInt()))
        {
            headerHovered = true;
            headerHoverActive = true;
            stopTimer();
            repaint();
        }
    }
    void mouseExit (const juce::MouseEvent& e) override
    {
        auto headerBounds = getLocalBounds().removeFromTop (static_cast<int> (60 * scaleFactor));
        if (! headerBounds.contains (e.position.toInt()))
        {
            headerHovered = false;
            startTimer (headerHoverOffDelayMs);
        }
    }
    void mouseMove (const juce::MouseEvent& e) override;

    void setScaleFactor (float newScale) override;

    void syncXYPadWithParameters();
    void setupTooltips();

    bool layoutReady { false };

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

    void performLayout();
    void layoutMeters (juce::Rectangle<int> metersArea, float s, float sv);

    MyPluginAudioProcessor& proc;
    FieldLNF lnf;
    std::unique_ptr<class PaneManager> panes;
    std::unique_ptr<class ButtonManager> buttonManager;

public:
    std::unique_ptr<juce::KeyListener> keyListener;
    std::unique_ptr<LayoutManager> layoutManager;
    std::unique_ptr<EventManager> eventManager;
    std::unique_ptr<AttachmentManager> attachmentManager;
    std::unique_ptr<CleanupManager> cleanupManager;
    std::unique_ptr<PaintManager> paintManager;
    std::unique_ptr<StateManager> stateManager;

public:
    int minWidth = 0, minHeight = 0, maxWidth = 3000, maxHeight = 2000;
    int baseWidth = 1500;
    int baseHeight = 1000;
    float scaleFactor = 1.0f;

    bool tooltipAssistantOn_ = false;
    juce::Component* lastTooltipTarget = nullptr;
    juce::uint32 lastUserInteractionMs = 0;
    int uiTimerHzCurrent = 30;

    GainSlider   gain;
    juce::Slider width, tilt, monoHz, hpHz, lpHz, satDrive, satMix, air, bass, scoop;
    juce::ComboBox monoSlopeChoice;
    AuditionButton monoAuditionButton;
    juce::Slider widthLo, widthMid, widthHi;
    juce::Slider xoverLoHz, xoverHiHz;
    juce::Slider rotationDeg, asymmetry;
    PanSlider    panKnob;
    PanSlider    panKnobLeft, panKnobRight;
    juce::ComboBox osSelect;

    BypassButton  bypassButton;
    ToggleSwitch  splitToggle;
    ShadeOverlay  shadeOverlay;

    juce::Viewport  controlsViewport;
    juce::Component controlsContent;
    juce::Component group1Container;
    juce::Component group2Container;

    juce::Slider tiltFreqSlider, scoopFreqSlider, bassFreqSlider, airFreqSlider;

    juce::Slider shelfShapeS, filterQ;
    juce::ToggleButton tiltLinkSButton, qLinkButton;
    juce::Slider hpQSlider, lpQSlider;

private:
    juce::Slider qClusterDummySlider;
    juce::Label  qClusterDummyValue;

public:
    OptionsButton    optionsButton;
    LinkButton       linkButton;
    SnapButton       snapButton;
    FullScreenButton fullScreenButton;
    ColorModeButton  colorModeButton;
    TooltipsButton   tooltipsButton;
    HelpButton       helpButton;
    CopyButton       copyButton;
    LockButton       lockButton;

    TooltipBubble tooltipBubble;

    juce::ToggleButton wetOnlyToggle;

    std::unique_ptr<::MonoSlopeSwitch> monoSlopeSwitch;
    std::unique_ptr<SwitchCell> wetOnlyCell;

public:
    ABButton abButtonA { true }, abButtonB { false };
    PresetArrowButton prevPresetButton { true }, nextPresetButton { false };
    juce::TextButton presetField;
    juce::Label presetNameLabel;
    juce::Component headerLeftGroup;

    std::unique_ptr<TransportClock> transportClock;

    juce::Component panSplitContainer;

public:
    PresetStore       presetStore;
    NewPresetManager  presetManager;

    ControlContainer mainControlsContainer, volumeContainer;

    ControlContainer MainContentContainer;
    ControlContainer panKnobContainer;
    ControlContainer widthGroupContainer;
    juce::Component  widthGroupSlot1, widthGroupSlot2, widthGroupSlot3;
    ControlContainer gainMixGroupContainer;
    juce::Component  gainMixSlot1, gainMixSlot2;
    ControlContainer duckGroupContainer;
    juce::Component  duckSlot1, duckSlot2, duckSlot3;
    ControlContainer volGroupContainer, eqGroupContainer, imgGroupContainer, volGroupContainer2, monoGroupContainer;
    juce::Component  volSlot1, volSlot2, volSlot3, volSlot4, volSlot5, volSlot6, volSlot7;

public:
    juce::Label leftIndicator, rightIndicator;
    juce::Label gainValue, widthValue, tiltValue, monoValue, hpValue, lpValue, satDriveValue, satMixValue, airValue, bassValue, scoopValue;
    juce::Label shelfShapeValue, filterQValue;
    juce::Label monoSlopeName, monoAudName;
    juce::Label panValue, panValueLeft, panValueRight;
    juce::Label tiltFreqValue, scoopFreqValue, bassFreqValue, airFreqValue;
    juce::Label widthLoValue, widthMidValue, widthHiValue, xoverLoValue, xoverHiValue, rotationValue, asymValue;

    juce::Label widthLoName, widthMidName, widthHiName;
    juce::Label xoverLoName, xoverHiName;
    juce::Label rotationName, asymName;
    juce::Label shufLoName, shufHiName, shufXName;
    juce::Label gainL, widthL, tiltL, monoL, hpL, lpL, satDriveL, satMixL;
    juce::Label panL;

    std::unique_ptr<KnobCell> panCell;
    std::unique_ptr<KnobCell> widthCell, widthLoCell, widthMidCell, widthHiCell;
    std::unique_ptr<KnobCell> gainCell, satDriveCell, satMixCell, monoCell;
    std::unique_ptr<KnobCell> bassCell, airCell, tiltCell, scoopCell, hpCell, lpCell;
    std::unique_ptr<KnobCell> xoverLoCell, xoverHiCell, rotationCell, asymCell;
    std::unique_ptr<KnobCell> shelfShapeCell, filterQCell, qClusterCell;

    const int standardKnobSize = 80;

    void initializePresetSystem();
    void initializeManagers();
    void initializeSizeConstraints();
    void initializeUIComponents();
    void initializeButtonCallbacks();
    void initializeParameterAttachments();
    void finalizeInitialization();

    void styleSlider (juce::Slider& s);
    void styleMainSlider (juce::Slider& s);
    void updateParameterLocks();
    void drawRecessedLabel (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text, bool isActive = true);
    void drawKnobWithIntegratedValue (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& knobName, const juce::String& value, bool isActive = true);

    juce::Point<int> resizeStart;
    juce::Rectangle<int> originalBounds;

public:
    juce::Rectangle<int> savedBounds;

    struct BottomAltPanel : public juce::Component
    {
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            juce::Colour top = juce::Colour (0xFF2A2C30);
            juce::Colour mid = juce::Colour (0xFF4A4D55);
            juce::Colour bottom = juce::Colour (0xFF2A2C30);
            juce::ColourGradient bg (top, r.getCentreX(), r.getY(), bottom, r.getCentreX(), r.getBottom(), false);
            bg.addColour (0.85, mid);
            g.setGradientFill (bg);
            g.fillRoundedRectangle (r, 6.0f);
        }
    } bottomAltPanel;

    bool  bottomAltTargetOn   { false };
    float bottomAltSlide01    { 0.0f };
    bool  bottomAltAnimating  { false };
    bool  overlayLayoutDirty  { true };
    bool  overlayContentsBuilt{ false };
    juce::Rectangle<int> overlayLocalRect;
    int overlayActiveBaseline { 0 };
    int overlayHiddenBaseline { 0 };
    int overlayHeightPx       { 0 };
    void updateGroup2OverlayDuringSlide();

    std::unique_ptr<class ReverbGraphics> reverbPanel;
    int controlRowsHeightPx { 0 };

    std::unique_ptr<MeterManager> meterManager;
    std::unique_ptr<SliderManager> sliderManager;

    bool isGreenMode = false;
    int  currentAlgorithm = 0;

    void saveCurrentState();
    void loadState (bool loadStateA);
    void toggleABState();
    void copyState (bool copyFromA);
    void pasteState (bool pasteToA);
    void updatePresetDisplay();
    
    void saveColorMode();
    void loadColorMode();
    juce::PropertiesFile::Options getColorModePropsOptions();
    
    void initializeShadeOverlay();

    bool headerHovered = false;
    bool headerHoverActive = false;
    const int headerHoverOffDelayMs = 160;

    void noteUserInteraction() { lastUserInteractionMs = juce::Time::getMillisecondCounter(); }
    struct BurstMouseProxy : public juce::MouseListener
    {
        explicit BurstMouseProxy (MyPluginAudioProcessorEditor& o) : owner (o) {}
        void mouseDown (const juce::MouseEvent&) override { owner.noteUserInteraction(); }
        void mouseDrag (const juce::MouseEvent&) override { owner.noteUserInteraction(); }
        void mouseUp   (const juce::MouseEvent&) override { owner.noteUserInteraction(); }
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override { owner.noteUserInteraction(); }
    private:
        MyPluginAudioProcessorEditor& owner;
    };
    std::unique_ptr<BurstMouseProxy> burstMouseProxy;

    juce::Rectangle<int> dividerVolBounds;

    void applyGlobalCursorPolicy();
    void updateMutedKnobVisuals();
    void childrenChanged() override { juce::Component::childrenChanged(); applyGlobalCursorPolicy(); }

    VerticalDivider splitDivider { lnf }, eqDivLpMono { lnf }, eqDivScoopHp { lnf };
    VerticalDivider volDivPanSpace { lnf }, volDivDuckRight { lnf }, delayDivider { lnf }, motionDivider { lnf };
    HorizontalDivider rowDivVol { lnf }, rowDivEQ { lnf };

    ControlContainer phaseCenterContainer;

private:
    void initializeTheme();
    void initializeTimer();
    void initializeMouseListener();
    void initializeMeters();

    void addShowHide(juce::Component& component, bool visible);
    void addChildHidden(juce::Component& component);
    void styleRotary(juce::Slider& slider);
    void styleLinear(juce::Slider& slider);
    void seedLabels(std::initializer_list<juce::Label*> labels, const juce::Font& font, juce::Colour colour);
    void setParam(juce::RangedAudioParameter& param, float value);
    
    void registerParameterListeners();
    void removeParameterListeners();
    
    struct ThemeConfig { juce::Colour tint; juce::String label; };
    ThemeConfig applyOptionsTint(int choiceIndex);
    
    std::unique_ptr<juce::ParameterAttachment> phaseModeParamAttach;
    std::unique_ptr<juce::ParameterAttachment> osModeParamAttach;
    std::unique_ptr<juce::ParameterAttachment> qualityParamAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPluginAudioProcessorEditor)
};
