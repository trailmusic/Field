#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ui/Managers/PaneManager.h"
#include "ui/Components/XYPad.h"
#include "reverb/ReverbParamIDs.h"
#include "ui/Design/Layout.h"
#include "dsp/DelayPresetLibrary.h"
#include "reverb/ui/ReverbGraphics.h"
#include "reverb/ui/ReverbDynEQPane.h"
#include "ui/Controls/ControlGridMetrics.h"
#include "ui/Components/TintMenuLNFEx.h"
#include "ui/Components/BottomChevronLNF.h"
#include "ui/Components/MenuUtils.h"
#include "ui/Components/VizEQ.h"

MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor (MyPluginAudioProcessor& p)
: AudioProcessorEditor (&p), proc (p), presetManager (proc.apvts, nullptr), bypassButton (lnf), shadeOverlay (lnf)
{
    initializePresetSystem();
    initializeManagers();
    initializeSizeConstraints();
    initializeUIComponents();
    initializeButtonCallbacks();
    initializeParameterAttachments();
    initializeShadeOverlay();
    finalizeInitialization();
}

void MyPluginAudioProcessorEditor::initializePresetSystem()
    {
        const auto presetsFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                  .getChildFile ("Field/Presets/delay_presets.json");

        if (presetsFile.existsAsFile())
        {
            const auto text = presetsFile.loadFileAsString();
            const auto root = juce::JSON::parse (text);

        auto importOne = [this] (const juce::var& v)
            {
            if (!v.isObject()) return;
                if (auto* obj = v.getDynamicObject())
                {
                    LibraryPreset pr;
                    pr.meta.name        = obj->getProperty ("name").toString();
                    pr.meta.description = obj->getProperty ("desc").toString();
                    pr.meta.hint        = obj->getProperty ("hint").toString();
                    pr.meta.author      = "Factory";
                    pr.meta.category    = "Delay";

                    if (auto tags = obj->getProperty ("tags"); tags.isArray())
                    for (const auto& t : *tags.getArray()) pr.meta.tags.add (t.toString());

                    if (auto params = obj->getProperty ("params"); params.isObject())
                        if (auto* pv = params.getDynamicObject())
                        for (const auto& kv : pv->getProperties())
                                pr.params.set (kv.name.toString(), kv.value);

                if (pr.meta.name.isNotEmpty())
                    presetStore.addFactoryPreset (pr);
                }
            };

            if (root.isArray())
            {
                for (const auto& it : *root.getArray()) importOne (it);
            }
            else if (auto* d = root.getDynamicObject())
            {
            if (auto arr = d->getProperty ("presets"); arr.isArray())
                for (const auto& it : *arr.getArray()) importOne (it);
            }
        }

    presetStore.scan();
    DBG ("[PresetStore] after add+scan: " << presetStore.getAll().size() << " presets");
}

void MyPluginAudioProcessorEditor::initializeManagers()
{
    layoutManager     = std::make_unique<LayoutManager>     (*this);
    eventManager      = std::make_unique<EventManager>      (*this);
    attachmentManager = std::make_unique<AttachmentManager> (*this);
    cleanupManager    = std::make_unique<CleanupManager>    (*this);
    paintManager      = std::make_unique<PaintManager>      (*this);
    stateManager      = std::make_unique<StateManager>      (*this);
    
    // Initialize meter and slider managers
    meterManager = std::make_unique<MeterManager>(*this);
    sliderManager = std::make_unique<SliderManager>(*this);
    
    // Initialize the managers
    meterManager->initializeMeters();
    sliderManager->initializeSliders();
}

void MyPluginAudioProcessorEditor::initializeSizeConstraints()
{
    if (layoutManager) layoutManager->buildCells();

    const float s     = 1.0f;
    const int   lPx   = Layout::dp ((float) Layout::knobPx (Layout::Knob::L),  s);
    const int   xlPx  = Layout::dp ((float) Layout::knobPx (Layout::Knob::XL), s);
    const int   swW   = Layout::dp ((int) (Layout::ALGO_SWITCH_W * Layout::ALGO_SWITCH_W_RATIO), s);
    const int   numItems = 1 + 1 + 1 + 5 + 3;
    const int   gaps  = numItems - 1;
    const int   gapS  = Layout::dp (Layout::GAP_S, s);

    const int cellWDelayMin  = lPx + Layout::dp (8, s);
    const int delayColsMin   = 7;
    const int delayCardWMin  = delayColsMin * cellWDelayMin + gapS * (delayColsMin - 1) + Layout::dp (Layout::GAP, s);
    const int motionColsMin  = 4;
    const int motionCellWMin = lPx + Layout::dp (8, s);
    const int motionDividerWMin = Layout::dp (8, s);
    const int motionAreaWMin = motionDividerWMin + Layout::dp (Layout::GAP, s) + motionColsMin * motionCellWMin;

    const int calculatedMinWidth =
        xlPx + lPx + swW + 5 * lPx + 3 * lPx + gaps * gapS
                                   + Layout::dp (Layout::PAD, s) * 2
                                   + delayCardWMin + Layout::dp (Layout::GAP, s)
                                   + motionAreaWMin;
    
    const int headerH          = Layout::dp (50, s);
    const int xyMinH           = Layout::dp (Layout::XY_MIN_H, s);
    const int metersH          = Layout::dp (84, s);
    const int bottomReserveMin = Layout::dp (6, s) + Layout::dp (22, s);
    const int gapH             = Layout::dp (Layout::GAP, s);
    const auto gridMetrics     = ControlGridMetrics::compute (baseWidth, baseHeight);
    const int controlsHMin     = gridMetrics.controlsH;

    const int calculatedMinHeight =
        headerH + juce::jmax (xyMinH, metersH) + gapH + controlsHMin + Layout::dp (Layout::PAD, s) + bottomReserveMin;

    const int minWidthAllowed = juce::jmax (800, (int) std::round ((float) baseWidth * 0.5f));
    const int minWidthFloor   = Layout::BP_WIDE;
    const int proposedMinW    = juce::jmin (calculatedMinWidth, minWidthAllowed);

    minWidth  = juce::jmax (minWidthFloor, proposedMinW);
    minHeight = calculatedMinHeight;
    maxWidth  = 3000;
    maxHeight = 2000;

    setResizeLimits (minWidth, minHeight, maxWidth, maxHeight);
    setSize (juce::jmax (baseWidth,  minWidth),
             juce::jmax (baseHeight, calculatedMinHeight));
}

void MyPluginAudioProcessorEditor::initializeUIComponents()
{
    juce::MessageManager::callAsync ([this] { repaint(); });

    initializeTheme();
    initializeTimer();
    initializeMouseListener();
}

void MyPluginAudioProcessorEditor::initializeButtonCallbacks()
{
    addAndMakeVisible (optionsButton);
    optionsButton.onClick = [this]
    {
        const bool wasOn = optionsButton.getToggleState();
        optionsButton.setToggleState (true, juce::dontSendNotification);
        optionsButton.repaint();

        TintMenuLNFEx menuLnf;
        menuLnf.defaultTint = lnf.theme.accent;
        menuLnf.hideChecks  = true;
        menuLnf.setColour (juce::PopupMenu::textColourId, lnf.theme.text);

        int curIdx = 0, numChoices = 1;
        if (const auto* rp = proc.apvts.getParameter ("os_mode"))
            if (const auto* cp = dynamic_cast<const juce::AudioParameterChoice*> (rp))
            { curIdx = cp->getIndex(); numChoices = (int) cp->choices.size(); }

        showTintedMenu (optionsButton, menuLnf,
            [this, curIdx, numChoices] (juce::PopupMenu& m, TintMenuLNFEx& lnfEx)
            {
                struct Row { int id; const char* text; juce::Colour tint; bool enabled; };
                const Row rows[] = {
                    { 1, "1x (Off)",          lnf.theme.textMuted,                                                             true },
                    { 2, "2x (High Quality)", lnf.theme.eq.bass.withAlpha (0.95f),                                             numChoices > 1 },
                    { 3, "4x (Ultra)",        lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.08f).withSaturation (0.9f), numChoices > 2 },
                    { 4, "8x (Max)",          lnf.theme.accent.withHue (lnf.theme.accent.getHue() - 0.08f).withBrightness (0.95f), numChoices > 3 },
                    { 5, "16x (Extreme)",     lnf.theme.eq.scoop.withAlpha (0.95f),                                            numChoices > 4 },
                };

                lnfEx.itemTints.clear();
                for (int i = 0; i < (int) std::size (rows); ++i)
                {
                    m.addItem (rows[i].id, rows[i].text, rows[i].enabled, i == curIdx);
                    lnfEx.itemTints.add (rows[i].tint);
                }
            },
            [this, wasOn, numChoices] (int r)
            {
                if (const auto* rp = proc.apvts.getParameter ("os_mode"))
                    if (const auto* cp = dynamic_cast<const juce::AudioParameterChoice*> (rp))
                        optionsButton.setToggleState (cp->getIndex() > 0, juce::dontSendNotification);
                    else
                        optionsButton.setToggleState (wasOn, juce::dontSendNotification);

                optionsButton.repaint();

                if (r <= 0 || r > numChoices) return;

                if (auto* p = proc.apvts.getParameter ("os_mode"))
                {
                    const float norm = numChoices > 1 ? (float) (r - 1) / (float) (numChoices - 1) : 0.0f;
                    p->beginChangeGesture();
                    p->setValueNotifyingHost (norm);
                    p->endChangeGesture();
                }
            });
    };

    auto applyOptionsTint = [this]
    {
        int sel = 0;
        if (const auto* rp = proc.apvts.getParameter ("os_mode"))
            if (const auto* cp = dynamic_cast<const juce::AudioParameterChoice*> (rp))
                sel = cp->getIndex();

        juce::Colour tint = lnf.theme.textMuted;
        juce::String label { "1x" };

        switch (sel)
        {
            case 0: tint = lnf.theme.textMuted;                                                                 label = "1x";  break;
            case 1: tint = lnf.theme.eq.bass.withAlpha (0.95f);                                                 label = "2x";  break;
            case 2: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.08f).withSaturation (0.9f); label = "4x";  break;
            case 3: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() - 0.08f).withBrightness (0.95f);label = "8x";  break;
            case 4: tint = lnf.theme.eq.scoop.withAlpha (0.95f);                                                label = "16x"; break;
        }

        optionsButton.getProperties().set ("accentOverrideARGB", (int) tint.getARGB());
        optionsButton.getProperties().set ("iconOverrideARGB",   (int) tint.getARGB());
        optionsButton.getProperties().set ("labelText",          label);
        optionsButton.setToggleState (true, juce::dontSendNotification);
        optionsButton.repaint();
    };

    applyOptionsTint();

    osSelect.onChange = [this, applyOptionsTint] { applyOptionsTint(); };

    if (!osModeParamAttach)
        if (auto* p = proc.apvts.getParameter ("os_mode"))
            osModeParamAttach = std::make_unique<juce::ParameterAttachment> (
                *p, [applyOptionsTint] (float) { applyOptionsTint(); }, nullptr);

    addAndMakeVisible (helpButton);
    helpButton.setLookAndFeel (&lnf);
    helpButton.onClick = [this]
    {
        struct HelpFAQComponent : public juce::Component
        {
            explicit HelpFAQComponent (FieldLNF& l) : lnf (l)
            {
                addAndMakeVisible (text);
                text.setReadOnly (true);
                text.setMultiLine (true);
                text.setScrollbarsShown (true);
                text.setCaretVisible (false);
                text.setFont (juce::Font (juce::FontOptions (14.0f)));
                text.setText (
                    "FIELD — FAQ\n\n"
                    "Q: How do I change color modes?\n"
                    "A: Click the palette button in the header to cycle Ocean → Green → Pink → Yellow → Grey.\n\n"
                    "Q: Where are colors defined?\n"
                    "A: All colors live in FieldLookAndFeel (FieldLNF::theme). Components never hardcode colors.\n\n"
                    "Q: Why don't knobs move when I resize?\n"
                    "A: Sizing happens in resized() only; layout is responsive via Layout::dp().\n\n"
                    "Q: How do I reset a control?\n"
                    "A: Double-click most knobs/sliders to reset to default.\n\n"
                    "Q: Where are presets saved?\n"
                    "A: In your user data folder under the plugin's presets directory.\n\n"
                );
            }

            void resized() override { text.setBounds (getLocalBounds().reduced (12)); }

            void paint (juce::Graphics& g) override
            {
                g.fillAll (lnf.theme.panel);
                g.setColour (lnf.theme.sh);
                g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 6.0f, 1.0f);
            }

            juce::TextEditor text;
            FieldLNF& lnf;
        };

        auto* content = new HelpFAQComponent (lnf);
        content->setSize (600, 400);

        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned (content);
        opts.dialogTitle = "Help / FAQ";
        opts.componentToCentreAround = this;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        (void) opts.launchAsync();
    };

    addAndMakeVisible (bypassButton);
    bypassButton.onClick = [this]
    {
        if (auto* p = proc.apvts.getParameter ("bypass"))
            p->setValueNotifyingHost (bypassButton.getToggleState() ? 1.0f : 0.0f);
    };

    addAndMakeVisible (colorModeButton);
    addAndMakeVisible (tooltipsButton);
    colorModeButton.setTooltip (ThemeManager::getThemeName (lnf.currentVariant));
    colorModeButton.onClick = [this]
    {
        static ThemeVariant order[] { ThemeVariant::Ocean, ThemeVariant::Green, ThemeVariant::Pink, ThemeVariant::Yellow, ThemeVariant::Grey };

        const auto currentAccent = lnf.theme.accent.getARGB();

        int idx = 0;
        if      (currentAccent == juce::Colour (0xFF5AA9E6).getARGB()) idx = 0;
        else if (currentAccent == juce::Colour (0xFF5AA95A).getARGB()) idx = 1;
        else if (currentAccent == juce::Colour (0xFFE91E63).getARGB()) idx = 2;
        else if (currentAccent == juce::Colour (0xFFFFC107).getARGB()) idx = 3;
        else if (currentAccent == juce::Colour (0xFF9EA3AA).getARGB()) idx = 4;

        idx = (idx + 1) % 5;

        lnf.setTheme (order[idx]);
        colorModeButton.setTooltip (ThemeManager::getThemeName (order[idx]));

        const bool greenNow = (order[idx] == ThemeVariant::Green);
        if (auto* xyTab = panes->getXYTab()) xyTab->setGreenMode (greenNow);

        repaint();
    };

    tooltipsButton.setTooltip ("Tooltip Assistant");
    tooltipsButton.setToggleState (tooltipAssistantOn_, juce::dontSendNotification);
    tooltipsButton.onClick = [this] {};

    addAndMakeVisible (fullScreenButton);
    fullScreenButton.onClick = [this]
    {
        const bool on = fullScreenButton.getToggleState();

        if (auto* tlw = getTopLevelComponent())
            if (auto* rw = dynamic_cast<juce::ResizableWindow*> (tlw))
            {
                if (on)
                {
                    savedBounds = rw->getBounds();
                    rw->setFullScreen (true);
                }
                else
                {
                    rw->setFullScreen (false);
                    if (!savedBounds.isEmpty()) rw->setBounds (savedBounds);
                }
                return;
        }

        if (on) fullScreenButton.setToggleState (false, juce::dontSendNotification);
    };

    addAndMakeVisible (linkButton);
    linkButton.onClick = [this]
    {
        linkButton.setToggleState (!linkButton.getToggleState(), juce::dontSendNotification);
        if (auto* xyTab = panes->getXYTab()) xyTab->setLinked (linkButton.getToggleState());
    };

    addAndMakeVisible (snapButton);
    snapButton.setToggleState (false, juce::dontSendNotification);
    snapButton.onClick = [this]
    {
        const bool on = !snapButton.getToggleState();
        snapButton.setToggleState (on, juce::dontSendNotification);
        if (auto* xyTab = panes->getXYTab()) xyTab->setSnapEnabled (on);
    };

    addAndMakeVisible (presetField);
    presetField.setButtonText ("Search presets…");
    presetField.setColour (juce::TextButton::buttonColourId,    juce::Colours::transparentBlack);
    presetField.setColour (juce::TextButton::buttonOnColourId,  juce::Colours::transparentBlack);

    addAndMakeVisible (prevPresetButton);
    addAndMakeVisible (nextPresetButton);
    prevPresetButton.onClick = [this] {};
    nextPresetButton.onClick = [this] {};

    presetField.onClick = [this]
    {
        static PresetRegistry presetRegistry;
        presetRegistry.reloadAll();

        PresetCommandPalette::show(
            presetRegistry,
            presetField,
            [this] (const PresetEntry& e)
            {
                LibraryPreset tmp; tmp.meta.id = e.id; tmp.meta.name = e.name; tmp.params = e.params;
                presetManager.applyPresetAtomic (tmp);
                presetNameLabel.setText (e.name, juce::dontSendNotification);
            },
            [this] (const PresetEntry& e, bool toA)
            {
                LibraryPreset tmp;
                tmp.params = e.params;
                presetManager.loadToSlot (tmp, toA);
            },
            [reg = &presetRegistry] (const PresetEntry& e, bool fav) { reg->setFavorite (e.id, fav); },
            [this] (juce::String name, juce::StringArray tags, juce::String cat)
            {
                auto pr = presetManager.currentAsPreset (name, cat, tags, "User preset", "", "You");
                presetStore.saveUserPreset (pr);
                presetStore.scan();
            },
            presetField.getButtonText()
        );
    };

    addAndMakeVisible (abButtonA);
    addAndMakeVisible (abButtonB);
    addAndMakeVisible (copyButton);

    abButtonA.setToggleState (true,  juce::dontSendNotification);
    abButtonB.setToggleState (false, juce::dontSendNotification);
    abButtonA.onClick = [this] { if (!abButtonA.getToggleState()) toggleABState(); };
    abButtonB.onClick = [this] { if (!abButtonB.getToggleState()) toggleABState(); };

    setAreaMetallicForCell (abButtonA, MetallicKind::Band);
    setAreaMetallicForCell (abButtonB, MetallicKind::Band);

    copyButton.onClick = [this]
    {
        juce::PopupMenu m;
        m.addItem (1, "Copy A to B");
        m.addItem (2, "Copy B to A");
        m.showMenuAsync (juce::PopupMenu::Options(), [this] (int r)
        {
            if (r == 1) { copyState (true);  pasteState (false); }
            if (r == 2) { copyState (false); pasteState (true);  }
        });
    };

    addAndMakeVisible (splitToggle);
    splitToggle.onToggleChange = [this] (bool split)
    {
        if (auto* xyTab = panes->getXYTab()) xyTab->setSplitMode (split);

        linkButton.setVisible (split);

        const bool showSingle = !split;
        panKnob.setVisible (showSingle);
        panValue.setVisible (showSingle);

        panKnobLeft .setVisible (!showSingle);
        panKnobRight.setVisible (!showSingle);
        panValueLeft .setVisible (!showSingle);
        panValueRight.setVisible (!showSingle);

        resized();
    };
    splitToggle.setToggleState (false, juce::dontSendNotification);
    linkButton.setVisible (false);

    panes = std::make_unique<PaneManager> (proc, proc.apvts.state, &lnf);
    addAndMakeVisible (panes.get());
    panes->setSampleRate (proc.getSampleRate());
    panes->setActive (PaneID::XY, true);

    // Meters are now added to metersContainer above

    addAndMakeVisible (mainControlsContainer);  mainControlsContainer.setTitle ("");  mainControlsContainer.setShowBorder (false);
    addAndMakeVisible (panKnobContainer);       panKnobContainer.setTitle ("");       panKnobContainer.setShowBorder (true);
    addAndMakeVisible (volumeContainer);        volumeContainer.setTitle ("");        volumeContainer.setShowBorder (true);
    addAndMakeVisible (MainContentContainer);   MainContentContainer.setTitle ("");   MainContentContainer.setShowBorder (false);
    
    // Add manager containers
    addAndMakeVisible (meterManager->getMetersContainer());
    addAndMakeVisible (sliderManager->getSlidersContainer());

    addChildComponent (widthGroupContainer);
    widthGroupContainer.setTitle ("");
    widthGroupContainer.setShowBorder (true);
    widthGroupContainer.setVisible (false);
    widthGroupContainer.setInterceptsMouseClicks (false, false);
    
    addChildComponent (gainMixGroupContainer);
    gainMixGroupContainer.setTitle ("");
    gainMixGroupContainer.setShowBorder (false);
    gainMixGroupContainer.setVisible (false);
    gainMixGroupContainer.setInterceptsMouseClicks (false, false);

    for (auto* c : { &gainMixSlot1, &gainMixSlot2 })
    {
        addChildComponent (*c);
        c->setInterceptsMouseClicks (false, false);
        c->setVisible (false);
    }

    addChildComponent (duckGroupContainer);
    duckGroupContainer.setTitle ("");
    duckGroupContainer.setShowBorder (false);
    duckGroupContainer.setVisible (false);
    duckGroupContainer.setInterceptsMouseClicks (false, false);

    for (auto* c : { &duckSlot1, &duckSlot2, &duckSlot3, &widthGroupSlot1, &widthGroupSlot2, &widthGroupSlot3 })
    {
        addChildComponent (*c);
        c->setInterceptsMouseClicks (false, false);
        c->setVisible (false);
    }

    addChildComponent (volGroupContainer);
    volGroupContainer.setTitle ("");
    volGroupContainer.setShowBorder (false);
    volGroupContainer.setVisible (false);
    volGroupContainer.setInterceptsMouseClicks (false, false);

    for (auto* c : { &volSlot1, &volSlot2, &volSlot3, &volSlot4, &volSlot5, &volSlot6, &volSlot7 })
    {
        addChildComponent (*c);
        c->setInterceptsMouseClicks (false, false);
        c->setVisible (false);
    }

    addAndMakeVisible (panSplitContainer);
    panSplitContainer.setVisible (false);
    panSplitContainer.setInterceptsMouseClicks (false, false);

    auto styleRotary = [this] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        constexpr float kStart = juce::MathConstants<float>::pi;
        constexpr float kEnd   = juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi;
        s.setRotaryParameters (kStart, kEnd, true);
        s.setLookAndFeel (&lnf);
    };

    for (juce::Slider* slider : {
             &width,&tilt,&monoHz,&hpHz,&lpHz,&satDrive,&satMix,&air,&bass,&scoop,
                              &widthLo,&widthMid,&widthHi,&xoverLoHz,&xoverHiHz,&rotationDeg,&asymmetry,
                              &shelfShapeS,&filterQ })
    {
        addAndMakeVisible (*slider);
        styleRotary (*slider);
        slider->setVelocityBasedMode (true);
        slider->setVelocityModeParameters (0.85, 1, 0.0, true);
        slider->setMouseDragSensitivity (140);
        slider->addListener (this);
    }

    addAndMakeVisible (gain);
    styleRotary (gain);
    gain.addListener (this);

    auto styleLinear = [this] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setMouseDragSensitivity (140);
        s.setVelocityBasedMode (false);
        s.setSliderSnapsToMousePosition (false);
        s.setDoubleClickReturnValue (true, 0.0);
        s.setLookAndFeel (&lnf);
    };

    for (juce::Slider* slider : { &tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider, &hpQSlider, &lpQSlider })
    {
        addAndMakeVisible (*slider);
        styleLinear (*slider);
        slider->addListener (this);
    }

    addAndMakeVisible (tiltLinkSButton);
    addAndMakeVisible (qLinkButton);
    tiltLinkSButton.setLookAndFeel (&lnf);
    qLinkButton.setLookAndFeel (&lnf);
    tiltLinkSButton.setButtonText ("Tilt uses S");
    qLinkButton.setButtonText ("");

    addAndMakeVisible (panKnob);      styleRotary (panKnob);      panKnob.setRange (-1.0, 1.0, 0.01); panKnob.setOverlayEnabled (false); panKnob.addListener (this);
    addAndMakeVisible (panKnobLeft);  styleRotary (panKnobLeft);  panKnobLeft.setRange (-1.0, 1.0, 0.01);  panKnobLeft.setOverlayEnabled  (true); panKnobLeft.setLabel  ("L"); panKnobLeft.addListener  (this);
    addAndMakeVisible (panKnobRight); styleRotary (panKnobRight); panKnobRight.setRange(-1.0, 1.0, 0.01);  panKnobRight.setOverlayEnabled (true); panKnobRight.setLabel ("R"); panKnobRight.addListener (this);

    panKnob.setVisible (true);
    panKnobLeft.setVisible (false);
    panKnobRight.setVisible (false);

    for (juce::Label* l : {
             &gainValue,&widthValue,&tiltValue,&monoValue,&hpValue,&lpValue,&satDriveValue,&satMixValue,&airValue,&bassValue,&scoopValue,&shelfShapeValue,&filterQValue,
             &panValue,&panValueLeft,&panValueRight,
                             &tiltFreqValue,&scoopFreqValue,&bassFreqValue,&airFreqValue,
                             &widthLoValue,&widthMidValue,&widthHiValue,&xoverLoValue,&xoverHiValue,
             &rotationValue,&asymValue })
    {
        addAndMakeVisible (*l);
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (15.0f * scaleFactor).withStyle ("Bold")));
        l->setColour (juce::Label::textColourId,       lnf.theme.text);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId,    juce::Colours::transparentBlack);
    }

    auto addLiveRepaint = [this] (juce::Slider& s)
    {
        s.onValueChange = [this]
        {
            if (auto* xyTab = panes->getXYTab())
            {
                xyTab->setTiltValue ((float) tilt.getValue());
                xyTab->setHPValue   ((float) hpHz.getValue());
                xyTab->setLPValue   ((float) lpHz.getValue());
                xyTab->setAirValue  ((float) air.getValue());
                xyTab->setBassValue ((float) bass.getValue());
                xyTab->setScoopValue((float) scoop.getValue());
                xyTab->setTiltFreqValue  ((float) tiltFreqSlider.getValue());
                xyTab->setScoopFreqValue ((float) scoopFreqSlider.getValue());
                xyTab->setBassFreqValue  ((float) bassFreqSlider.getValue());
                xyTab->setAirFreqValue   ((float) airFreqSlider.getValue());
                xyTab->setMonoValue      ((float) monoHz.getValue());
                xyTab->repaint();
            }
        };
    };

    for (juce::Slider* slider : { &tilt,&hpHz,&lpHz,&air,&bass,&scoop,&tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider,&monoHz,&shelfShapeS,&filterQ,&hpQSlider,&lpQSlider })
    {
        addLiveRepaint (*slider);
        slider->onDragStart = [this] { proc.setIsEditing (true);  };
        slider->onDragEnd   = [this] { proc.setIsEditing (false); };
    }

    gain.setName ("GAIN"); width.setName ("WIDTH"); tilt.setName ("TILT"); monoHz.setName ("MONO");
    hpHz.setName ("HP Hz"); lpHz.setName ("LP Hz"); satDrive.setName ("DRIVE"); satMix.setName ("MIX");
    air.setName ("AIR"); bass.setName ("BASS"); scoop.setName ("SCOOP");
    shelfShapeS.setName ("Shape"); filterQ.setName ("Q");
    panKnob.setName ("PAN"); panKnobLeft.setName ("PAN L"); panKnobRight.setName ("PAN R");
    tiltFreqSlider.setName ("TILT F"); scoopFreqSlider.setName ("SCOOP F"); bassFreqSlider.setName ("BASS F"); airFreqSlider.setName ("AIR F");
    widthLo.setName ("W LO"); widthMid.setName ("W MID"); widthHi.setName ("W HI");
    xoverLoHz.setName ("XO LO"); xoverHiHz.setName ("XO HI");
    rotationDeg.setName ("ROT"); asymmetry.setName ("ASYM");

    hpHz.setNumDecimalPlacesToDisplay (0);
    lpHz.setNumDecimalPlacesToDisplay (0);

    for (auto* l : { &widthLoName,&widthMidName,&widthHiName,&xoverLoName,&xoverHiName,&rotationName,&asymName,&shufLoName,&shufHiName,&shufXName })
    {
        l->setVisible (false);
        l->setInterceptsMouseClicks (false, false);
    }
    widthLoName.setText ("", juce::dontSendNotification);
    widthMidName.setText ("", juce::dontSendNotification);
    widthHiName.setText ("", juce::dontSendNotification);
    xoverLoName.setText  ("", juce::dontSendNotification);
    xoverHiName.setText  ("", juce::dontSendNotification);
    rotationName.setText ("", juce::dontSendNotification);
    asymName.setText     ("", juce::dontSendNotification);
    shufLoName.setText   ("", juce::dontSendNotification);
    shufHiName.setText   ("", juce::dontSendNotification);
    shufXName.setText    ("", juce::dontSendNotification);

    // Update slider value labels
    sliderValueChanged(&width);
    sliderValueChanged(&tilt);
    sliderValueChanged(&monoHz);
    sliderValueChanged(&hpHz);
    sliderValueChanged(&lpHz);
    sliderValueChanged(&satDrive);
    sliderValueChanged(&satMix);
    sliderValueChanged(&air);
    sliderValueChanged(&bass);
    sliderValueChanged(&scoop);
    sliderValueChanged(&panKnob);
    sliderValueChanged(&panKnobLeft);
    sliderValueChanged(&panKnobRight);
    sliderValueChanged(&tiltFreqSlider);
    sliderValueChanged(&scoopFreqSlider);
    sliderValueChanged(&bassFreqSlider);
    sliderValueChanged(&airFreqSlider);
    sliderValueChanged(&gain);

    updateMutedKnobVisuals();

    addChildComponent (monoSlopeChoice);
    monoSlopeChoice.addItem ("6",  1);
    monoSlopeChoice.addItem ("12", 2);
    monoSlopeChoice.addItem ("24", 3);

    if (!monoSlopeSwitch) monoSlopeSwitch = std::make_unique<MonoSlopeSwitch>();
    addAndMakeVisible (*monoSlopeSwitch);
    monoSlopeSwitch->onChange = [this] (int idx)
    {
        monoSlopeChoice.setSelectedItemIndex (juce::jlimit (0, 2, idx), juce::NotificationType::sendNotification);
    };

    addAndMakeVisible (monoAuditionButton);
    monoAuditionButton.setButtonText ("AUD");

    if (attachmentManager) attachmentManager->attachAllParameters();

    layoutReady = true;

    addAndMakeVisible (splitDivider);
    splitDivider.setVisible (true);

    auto refreshXYOverlays = [this]
    {
        if (auto* xyTab = panes->getXYTab())
        {
            xyTab->setMixValue     (proc.apvts.getRawParameterValue ("sat_mix")->load());
            xyTab->setDriveValue   (proc.apvts.getRawParameterValue ("sat_drive_db")->load());
            xyTab->setTiltValue    (proc.apvts.getRawParameterValue ("tilt")->load());
            xyTab->setHPValue      (proc.apvts.getRawParameterValue ("hp_hz")->load());
            xyTab->setLPValue      (proc.apvts.getRawParameterValue ("lp_hz")->load());
            xyTab->setAirValue     (proc.apvts.getRawParameterValue ("air_db")->load());
            xyTab->setBassValue    (proc.apvts.getRawParameterValue ("bass_db")->load());
            xyTab->setScoopValue   (proc.apvts.getRawParameterValue ("scoop")->load());
            xyTab->setTiltFreqValue  (proc.apvts.getRawParameterValue ("tilt_freq")->load());
            xyTab->setScoopFreqValue (proc.apvts.getRawParameterValue ("scoop_freq")->load());
            xyTab->setBassFreqValue  (proc.apvts.getRawParameterValue ("bass_freq")->load());
            xyTab->setAirFreqValue   (proc.apvts.getRawParameterValue ("air_freq")->load());
            xyTab->setWidthValue   (proc.apvts.getRawParameterValue ("width")->load());
            xyTab->setPanValue     (proc.apvts.getRawParameterValue ("pan")->load());
            xyTab->setGainValue    (proc.apvts.getRawParameterValue ("gain_db")->load());
        }
    };

    if (auto* xyTab = panes->getXYTab())
    {
        xyTab->onChange = [this, refreshXYOverlays] (float x01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (0.0f); split->endChangeGesture(); }
        if (auto* pan   = proc.apvts.getParameter ("pan"))        { pan  ->beginChangeGesture(); pan  ->setValueNotifyingHost (x01);  pan  ->endChangeGesture(); }
        if (auto* dep   = proc.apvts.getParameter ("depth"))      { dep  ->beginChangeGesture(); dep  ->setValueNotifyingHost (y01);  dep  ->endChangeGesture(); }
        refreshXYOverlays();
    };

        xyTab->onSplitChange = [this, refreshXYOverlays] (float l01, float r01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (1.0f); split->endChangeGesture(); }
            if (auto* pL    = proc.apvts.getParameter ("pan_l"))      { pL   ->beginChangeGesture(); pL   ->setValueNotifyingHost (l01);  pL   ->endChangeGesture(); }
            if (auto* pR    = proc.apvts.getParameter ("pan_r"))      { pR   ->beginChangeGesture(); pR   ->setValueNotifyingHost (r01);  pR   ->endChangeGesture(); }
            if (auto* dep   = proc.apvts.getParameter ("depth"))      { dep  ->beginChangeGesture(); dep  ->setValueNotifyingHost (y01);  dep  ->endChangeGesture(); }
        refreshXYOverlays();
    };
    }

    panKnobLeft.addListener (this);
    panKnobRight.addListener (this);

    for (const char* pid :
         { "split_mode","pan","depth","mono_slope_db_oct","eq_shelf_shape","eq_q_link","eq_filter_q","hp_q","lp_q",
           "tilt_link_s","xover_lo_hz","xover_hi_hz","rotation_deg","asymmetry" })
        proc.apvts.addParameterListener (pid, this);

    proc.onAudioSample   = [this] (float L, float R) { if (panes) panes->onAudioSample (L, R); };
    proc.onAudioBlock    = [this] (const float* L, const float* R, int n) { if (panes) panes->onAudioBlock (L, R, n); };
    proc.onAudioBlockPre = [this] (const float* L, const float* R, int n) { if (panes) panes->onAudioBlockPre (L, R, n); };

    struct LocalKeyListener : public juce::KeyListener
    {
        explicit LocalKeyListener (PaneManager* m) : mgr (m) {}
        bool keyPressed (const juce::KeyPress& k, juce::Component*) override
        {
            if (!mgr) return false;
            if (k.getTextCharacter() == '1') { mgr->setActive (PaneID::XY,    true); return true; }
            if (k.getTextCharacter() == '2') { mgr->setActive (PaneID::DynEQ, true); return true; }
            if (k.getTextCharacter() == '3') { mgr->setActive (PaneID::Imager,true); return true; }
            return false;
        }
        PaneManager* mgr;
    };

    keyListener.reset (new LocalKeyListener (panes.get()));
    addKeyListener (keyListener.get());

    addAndMakeVisible (splitDivider);
    syncXYPadWithParameters();
    applyGlobalCursorPolicy();
    startTimerHz (20);

    addChildComponent (imgGroupContainer);
    imgGroupContainer.setTitle ("");
    imgGroupContainer.setShowBorder (false);
    imgGroupContainer.setVisible (false);
    imgGroupContainer.setInterceptsMouseClicks (false, false);

    addChildComponent (volGroupContainer2);
    volGroupContainer2.setTitle ("");
    volGroupContainer2.setShowBorder (false);
    volGroupContainer2.setVisible (false);
    volGroupContainer2.setInterceptsMouseClicks (false, false);

    addAndMakeVisible (monoGroupContainer);
    monoGroupContainer.setTitle ("");
    monoGroupContainer.setShowBorder (true);
    
    resized();
}

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    juce::File f = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Field_CrashLog.txt");
    f.appendText ("Editor Destructor: STARTED\n", false, false, "\n");

    if (cleanupManager) cleanupManager->performCleanup();

    f.appendText ("Editor Destructor: COMPLETE\n", false, false, "\n");
}

void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (paintManager) paintManager->paint (g);
}

void MyPluginAudioProcessorEditor::performLayout()
{
    if (!layoutReady) return;
    if (layoutManager) layoutManager->performLayout();
}

void MyPluginAudioProcessorEditor::mouseDown  (const juce::MouseEvent& e) { if (eventManager) eventManager->handleMouseDown  (e); }
void MyPluginAudioProcessorEditor::mouseDrag  (const juce::MouseEvent& e) { noteUserInteraction(); if (eventManager) eventManager->handleMouseDrag  (e); }
void MyPluginAudioProcessorEditor::mouseUp    (const juce::MouseEvent& e) { if (eventManager) eventManager->handleMouseUp    (e); }
void MyPluginAudioProcessorEditor::mouseMove  (const juce::MouseEvent& e) { if (eventManager) eventManager->handleMouseMove  (e); }
void MyPluginAudioProcessorEditor::timerCallback()                        { if (eventManager) eventManager->handleTimerCallback(); }

void MyPluginAudioProcessorEditor::resized()
{
    if (!layoutReady) return;
    performLayout();
}

void MyPluginAudioProcessorEditor::paintOverChildren (juce::Graphics& g) { juce::ignoreUnused (g); }
void MyPluginAudioProcessorEditor::updateGroup2OverlayDuringSlide()      {}
void MyPluginAudioProcessorEditor::applyGlobalCursorPolicy()             {}
void MyPluginAudioProcessorEditor::updateMutedKnobVisuals()              {}
void MyPluginAudioProcessorEditor::parameterChanged (const juce::String&, float) {}
void MyPluginAudioProcessorEditor::syncXYPadWithParameters()             {}

void MyPluginAudioProcessorEditor::setScaleFactor (float newScale)
{
    scaleFactor = newScale;
    resized();
    repaint();
}

void MyPluginAudioProcessorEditor::sliderValueChanged (juce::Slider* s)
{
    if (eventManager) eventManager->handleSliderValueChanged (s);
    updateMutedKnobVisuals();
}

void MyPluginAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBox)
{
    if (eventManager) eventManager->handleComboBoxChanged (comboBox);
}

void MyPluginAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (eventManager) eventManager->handleButtonClicked (button);
}

void MyPluginAudioProcessorEditor::updatePresetDisplay()
{
    if (stateManager) stateManager->updatePresetDisplay();
}

void MyPluginAudioProcessorEditor::saveCurrentState()               { if (stateManager) stateManager->saveCurrentState(); }
void MyPluginAudioProcessorEditor::loadState (bool a)               { if (stateManager) stateManager->loadState (a); }
void MyPluginAudioProcessorEditor::toggleABState()                  { if (stateManager) stateManager->toggleABState(); }
void MyPluginAudioProcessorEditor::copyState (bool fromA)           { if (stateManager) stateManager->copyState (fromA); }
void MyPluginAudioProcessorEditor::pasteState (bool pasteToA)       { if (stateManager) stateManager->pasteState (pasteToA); }

void MyPluginAudioProcessorEditor::layoutMeters (juce::Rectangle<int> metersArea, float s, float sv)
{
    // Delegate meter layout to MeterManager
    if (meterManager) {
        meterManager->layoutMeters(metersArea, s, sv);
    }
}

void MyPluginAudioProcessorEditor::initializeParameterAttachments()
{
    if (attachmentManager) attachmentManager->attachAllParameters();
}

void MyPluginAudioProcessorEditor::finalizeInitialization()
{
    resized();
}


// ============================================================================
// Helper Methods for Initialization
// ============================================================================

void MyPluginAudioProcessorEditor::initializeTheme()
{
    lnf.theme.accent = juce::Colour (0xFF5AA9E6);
    lnf.setupColours();
    setLookAndFeel (&lnf);
}

void MyPluginAudioProcessorEditor::initializeTimer()
{
    startTimerHz (30);
    uiTimerHzCurrent = 30;
}

void MyPluginAudioProcessorEditor::initializeMouseListener()
{
    addMouseListener (this, true);
}

// ============================================================================
// DRY Helper Methods
// ============================================================================

void MyPluginAudioProcessorEditor::addShowHide(juce::Component& component, bool visible)
{
    component.setVisible(visible);
    component.setInterceptsMouseClicks(visible, visible);
}

void MyPluginAudioProcessorEditor::addChildHidden(juce::Component& component)
{
    addChildComponent(component);
    component.setInterceptsMouseClicks(false, false);
    component.setVisible(false);
}

void MyPluginAudioProcessorEditor::styleRotary(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, lnf.theme.accent);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, lnf.theme.text.withAlpha(0.3f));
    slider.setColour(juce::Slider::thumbColourId, lnf.theme.accent);
}

void MyPluginAudioProcessorEditor::styleLinear(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    slider.setColour(juce::Slider::trackColourId, lnf.theme.accent);
    slider.setColour(juce::Slider::thumbColourId, lnf.theme.accent);
}

void MyPluginAudioProcessorEditor::seedLabels(std::initializer_list<juce::Label*> labels, const juce::Font& font, juce::Colour colour)
{
    for (auto* label : labels)
    {
        if (label)
        {
            label->setFont(font);
            label->setColour(juce::Label::textColourId, colour);
            label->setJustificationType(juce::Justification::centred);
        }
    }
}

void MyPluginAudioProcessorEditor::setParam(juce::RangedAudioParameter& param, float value)
{
    param.beginChangeGesture();
    param.setValueNotifyingHost(value);
    param.endChangeGesture();
}

// ============================================================================
// Parameter Listener Management
// ============================================================================

void MyPluginAudioProcessorEditor::registerParameterListeners()
{
    // Parameter listeners are handled by AttachmentManager
    // This method is kept for future use if needed
}

void MyPluginAudioProcessorEditor::removeParameterListeners()
{
    // Parameter listeners are handled by AttachmentManager
    // This method is kept for future use if needed
}

// ============================================================================
// Theming Helpers
// ============================================================================

MyPluginAudioProcessorEditor::ThemeConfig MyPluginAudioProcessorEditor::applyOptionsTint(int choiceIndex)
{
    static const std::array<ThemeConfig, 5> themeConfigs = {{
        {juce::Colour(0xFF5AA9E6), "Blue"},
        {juce::Colour(0xFFE65A5A), "Red"}, 
        {juce::Colour(0xFF5AE65A), "Green"},
        {juce::Colour(0xFFE6E65A), "Yellow"},
        {juce::Colour(0xFFE65AE6), "Purple"}
    }};
    
    if (choiceIndex >= 0 && choiceIndex < static_cast<int>(themeConfigs.size()))
    {
        return themeConfigs[choiceIndex];
    }
    
    return themeConfigs[0]; // Default to blue
}

// ============================================================================
// Performance & Safety Optimizations
// ============================================================================


void MyPluginAudioProcessorEditor::initializeMeters()
{
    // Meter initialization delegated to MeterManager
    if (meterManager) {
        meterManager->getCorrelationMeter().setLookAndFeel(&lnf);
        meterManager->getLRMeters().setLookAndFeel(&lnf);
        meterManager->getIOGainMeters().setLookAndFeel(&lnf);
    }
}

void MyPluginAudioProcessorEditor::initializeShadeOverlay()
{
    // Initialize ShadeOverlay directly
    addAndMakeVisible(shadeOverlay);
    
    // Initialize ShadeOverlay with PaneManager integration
    shadeOverlay.onAmountChanged = [this](float amount) {
        if (panes) {
            panes->setActiveShade(amount);
        }
    };
    
    // Set initial shade amount from PaneManager
    if (panes) {
        shadeOverlay.setAmount(panes->getActiveShade(), false);
        // Set PaneManager reference for getting active graphics container bounds
        shadeOverlay.setPaneManager(panes.get());
    }
}
