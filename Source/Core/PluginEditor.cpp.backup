#include "PluginProcessor.h"
#include "PluginEditor.h"
// XYPaneAdapter removed - XYTab now contains XYPad directly
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

//==============================================================

//==============================================================
// ToggleSwitch (compact, slow animation, keeps original visual)

//==============================================================
// VerticalSlider3D implementation
//==============================================================

//==============================================================
// XYPad (visual upgrade preserved; layout/edges match original)





// ------------------------------------------------
/* ===================== Editor ===================== */
MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor (MyPluginAudioProcessor& p)
: AudioProcessorEditor (&p), proc (p), presetManager (proc.apvts, nullptr), bypassButton(lnf)
{
    // Log: Editor constructor started
    // TEMPORARILY DISABLE file logging to test if this is causing the crash
    // juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    // f.appendText("Editor Ctor: STARTED\n", false, false, "\n");
    
    // Initialize components using dedicated methods
    initializePresetSystem();
    initializeManagers();
    initializeSizeConstraints();
    initializeUIComponents();
    initializeButtonCallbacks();
    initializeParameterAttachments();
    finalizeInitialization();

    // Constructor now uses dedicated initialization methods
}

// Initialization Methods
void MyPluginAudioProcessorEditor::initializePresetSystem()
{
    // Populate factory presets directly from app-data JSON (single source of truth)
    {
        const auto presetsFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                  .getChildFile ("Field/Presets/delay_presets.json");
        if (presetsFile.existsAsFile())
        {
            const auto text = presetsFile.loadFileAsString();
            const auto root = juce::JSON::parse (text);
            auto importOne = [this](const juce::var& v)
            {
                if (! v.isObject()) return;
                if (auto* obj = v.getDynamicObject())
                {
                    LibraryPreset pr;
                    pr.meta.name        = obj->getProperty ("name").toString();
                    pr.meta.description = obj->getProperty ("desc").toString();
                    pr.meta.hint        = obj->getProperty ("hint").toString();
                    pr.meta.author      = "Factory";
                    pr.meta.category    = "Delay";
                    if (auto tags = obj->getProperty ("tags"); tags.isArray())
                        for (auto& t : *tags.getArray()) pr.meta.tags.add (t.toString());
                    if (auto params = obj->getProperty ("params"); params.isObject())
                        if (auto* pv = params.getDynamicObject())
                            for (auto& kv : pv->getProperties())
                                pr.params.set (kv.name.toString(), kv.value);
                    if (pr.meta.name.isNotEmpty()) presetStore.addFactoryPreset (pr);
                }
            };
            if (root.isArray())
            {
                for (const auto& it : *root.getArray()) importOne (it);
            }
            else if (auto* d = root.getDynamicObject())
            {
                auto arr = d->getProperty ("presets");
                if (arr.isArray()) for (const auto& it : *arr.getArray()) importOne (it);
            }
        }
    }
    presetStore.scan();
    DBG("[PresetStore] after add+scan: " << presetStore.getAll().size() << " presets");
}

void MyPluginAudioProcessorEditor::initializeManagers()
{
    // Initialize managers
    layoutManager = std::make_unique<LayoutManager>(*this);
    eventManager = std::make_unique<EventManager>(*this);
    attachmentManager = std::make_unique<AttachmentManager>(*this);
    cleanupManager = std::make_unique<CleanupManager>(*this);
    paintManager = std::make_unique<PaintManager>(*this);
    stateManager = std::make_unique<StateManager>(*this);
}

void MyPluginAudioProcessorEditor::initializeSizeConstraints()
{
    // Build knob cells once after all sliders/labels are created
    if (layoutManager) {
        layoutManager->buildCells();
    }
    
    // Calculate minimum size based on layout requirements
    const float s = 1.0f;
    const int lPx  = Layout::dp ((float) Layout::knobPx (Layout::Knob::L),  s);
    const int xlPx = Layout::dp ((float) Layout::knobPx (Layout::Knob::XL), s);
    const int swW  = Layout::dp ((int) (Layout::ALGO_SWITCH_W * Layout::ALGO_SWITCH_W_RATIO), s);
    // Calculate minimum width needed for all controls
    const int numItems = 1 + 1 + 1 + 5 + 3; // pan, space, switch, duck(5), gain/drive/mix(3)
    const int gaps = numItems - 1;
    const int gapS = Layout::dp (Layout::GAP_S, s);
    // Compute minimum delay card width based on 7 columns using same cell width and inner gap
    const int cellW_delay_min = lPx + Layout::dp (8, s);
    const int delayColsMin = 7;
    const int delayCardWMin = delayColsMin * cellW_delay_min + gapS * (delayColsMin - 1) + Layout::dp (Layout::GAP, s);
    // Compute minimum motion grid width (second divider + gap + 4 motion cells)
    const int motionColsMin = 4;
    const int motionCellWMin = lPx + Layout::dp (8, s);
    const int motionDividerWMin = Layout::dp (8, s);
    const int motionAreaWMin = motionDividerWMin + Layout::dp (Layout::GAP, s) + motionColsMin * motionCellWMin;
    const int calculatedMinWidth = xlPx + lPx + swW + 5*lPx + 3*lPx + gaps * gapS
                                   + Layout::dp (Layout::PAD, s) * 2
                                   + delayCardWMin + Layout::dp (Layout::GAP, s)
                                   + motionAreaWMin;
    
    // Calculate minimum height based on current architecture: visuals + 2x16 controls
    const int headerH = Layout::dp (50, s);
    const int xyMinH = Layout::dp (Layout::XY_MIN_H, s);
    const int metersH = Layout::dp (84, s);
    const int bottomReserveMin = Layout::dp (6, s) + Layout::dp (22, s);
    const int gapH = Layout::dp (Layout::GAP, s);
    const auto gridMetrics = ControlGridMetrics::compute (baseWidth, baseHeight);
    const int controlsHMin = gridMetrics.controlsH; // 2 rows of the flat grid
    const int calculatedMinHeight = headerH + juce::jmax (xyMinH, metersH) + gapH + controlsHMin + Layout::dp (Layout::PAD, s) + bottomReserveMin;
    
    // Store resize constraints
    // Allow some narrowing vs content width, but never below a conservative floor
    const int minWidthAllowed = juce::jmax (800, (int) std::round ((float) baseWidth * 0.5f));
    const int minWidthFloor   = Layout::BP_WIDE; // breakpoint for wide layouts (protects 4x16 flats)
    const int proposedMinW    = juce::jmin (calculatedMinWidth, minWidthAllowed);
    this->minWidth = juce::jmax (minWidthFloor, proposedMinW);
    this->minHeight = calculatedMinHeight;
    this->maxWidth = 3000;
    this->maxHeight = 2000;
    
    // Set minimum size constraints
    setResizeLimits (minWidth, minHeight, maxWidth, maxHeight);
    
    // Set initial size (respecting minimums). Prefer baseWidth over full content width.
    const int initialWidth = juce::jmax (baseWidth, minWidth);
    const int initialHeight = juce::jmax (baseHeight, calculatedMinHeight);
    setSize (initialWidth, initialHeight);
}

void MyPluginAudioProcessorEditor::initializeUIComponents()
{
    // Initial layout deferred until layoutReady is true
    // Defer one tick to ensure LookAndFeel and attachments settle, then repaint
    juce::MessageManager::callAsync ([this]
    {
        // OLD REVERB SYSTEM REMOVED
        // OLD REVERB SYSTEM REMOVED
        repaint();
    });
    lnf.theme.accent = juce::Colour (0xFF5AA9E6); // ocean default
    lnf.setupColours();
    setLookAndFeel (&lnf);

    // Drive editor heartbeat at 30 Hz (UI updates throttled within timer)
    startTimerHz (30);
    uiTimerHzCurrent = 30;
    addMouseListener (this, true); // receive events from all children
}

void MyPluginAudioProcessorEditor::initializeButtonCallbacks()
{
    // Options menu (oversampling) with per-mode tint
    addAndMakeVisible (optionsButton);
    optionsButton.onClick = [this]
    {
        const bool wasOn = optionsButton.getToggleState();
        optionsButton.setToggleState (true, juce::dontSendNotification);
        optionsButton.repaint();

        TintMenuLNFEx menuLnf; menuLnf.defaultTint = lnf.theme.accent; menuLnf.hideChecks = true;
        menuLnf.setColour (juce::PopupMenu::textColourId, lnf.theme.text);

        int curIdx = 0, numChoices = 1;
        if (auto* rp = proc.apvts.getParameter ("os_mode"))
            if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp)) { curIdx = cp->getIndex(); numChoices = cp->choices.size(); }

        showTintedMenu (optionsButton, menuLnf,
            // BUILD
            [this, curIdx, numChoices] (juce::PopupMenu& m, TintMenuLNFEx& lnfEx)
            {
                m.addSectionHeader ("Oversampling");
                struct Row { int id; const char* text; juce::Colour tint; bool enabled; };
                juce::Array<Row> rows;
                rows.add ({ 1, "1x (Off)",          lnf.theme.textMuted, true });
                rows.add ({ 2, "2x (High Quality)", lnf.theme.eq.bass.withAlpha (0.95f),   numChoices > 1 });   // vibrant green
                rows.add ({ 3, "4x (Ultra)",        lnf.theme.accent.withHue   (lnf.theme.accent.getHue() + 0.08f).withSaturation (0.9f), numChoices > 2 });
                rows.add ({ 4, "8x (Max)",          lnf.theme.accent.withHue   (lnf.theme.accent.getHue() - 0.08f).withBrightness (0.95f), numChoices > 3 });
                rows.add ({ 5, "16x (Extreme)",     lnf.theme.eq.scoop.withAlpha (0.95f), numChoices > 4 });    // bold magenta/plum

                lnfEx.itemTints.clear();
                for (int i = 0; i < rows.size(); ++i)
                {
                    m.addItem (rows[i].id, rows[i].text, rows[i].enabled, i == curIdx);
                    lnfEx.itemTints.add (rows[i].tint);
                }
            },
            // RESULT
            [this, wasOn, numChoices] (int r)
            {
                // restore button active state to real OS>1×
                if (auto* rp = proc.apvts.getParameter ("os_mode"))
                    if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp))
                        optionsButton.setToggleState (cp->getIndex() > 0, juce::dontSendNotification);
                    else optionsButton.setToggleState (wasOn, juce::dontSendNotification);
                optionsButton.repaint();

                if (r == 0) return;
                if (r < 1 || r > numChoices) return;
                if (auto* p = proc.apvts.getParameter ("os_mode"))
                {
                    const float norm = numChoices > 1 ? (float) (r - 1) / (float) (numChoices - 1) : 0.0f;
                    p->beginChangeGesture(); p->setValueNotifyingHost (norm); p->endChangeGesture();
                }
            });
    };

    // Tint Options button based on oversampling choice
    auto applyOptionsTint = [this]
    {
        int sel = 0;
        if (auto* rp = proc.apvts.getParameter ("os_mode"))
            if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp)) sel = cp->getIndex();
        // Map index to label and tint
        juce::Colour tint = lnf.theme.textMuted;
        juce::String label = "1x";
        switch (sel)
        {
            case 0: tint = lnf.theme.textMuted; label = "1x"; break;
            case 1: tint = lnf.theme.eq.bass.withAlpha (0.95f); label = "2x"; break;
            case 2: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.08f).withSaturation (0.9f); label = "4x"; break;
            case 3: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() - 0.08f).withBrightness (0.95f); label = "8x"; break;
            case 4: tint = lnf.theme.eq.scoop.withAlpha (0.95f); label = "16x"; break;
        }
        optionsButton.getProperties().set ("accentOverrideARGB", (int) tint.getARGB());
        optionsButton.getProperties().set ("iconOverrideARGB", (int) tint.getARGB());
        optionsButton.getProperties().set ("labelText", label);
        optionsButton.setToggleState (true, juce::dontSendNotification);
        optionsButton.repaint();
    };
    applyOptionsTint();
    osSelect.onChange = [this, applyOptionsTint]
    {
        applyOptionsTint();
    };
    // Also listen to APVTS os_mode directly to avoid drift
    if (!osModeParamAttach)
    {
        if (auto* p = proc.apvts.getParameter ("os_mode"))
        {
            osModeParamAttach = std::make_unique<juce::ParameterAttachment>(*p, [applyOptionsTint](float){ applyOptionsTint(); }, nullptr);
        }
    }

    // Help button → FAQ dialog
    addAndMakeVisible (helpButton);
    helpButton.onClick = [this]
    {
        struct HelpFAQComponent : public juce::Component
        {
            HelpFAQComponent (FieldLNF& l) : lnf(l)
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
            void resized() override
            {
                text.setBounds (getLocalBounds().reduced (12));
            }
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

    // Bypass (attach to param if present)
    addAndMakeVisible (bypassButton);
    bypassButton.onClick = [this]
    {
        if (auto* p = proc.apvts.getParameter ("bypass"))
            p->setValueNotifyingHost (bypassButton.getToggleState() ? 1.0f : 0.0f);
    };

    // History panel removed

    // Header Undo/Redo removed

    // Color mode cycle (Ocean → Green → Pink → Yellow → Grey)
    addAndMakeVisible (colorModeButton);
    addAndMakeVisible (tooltipsButton);
    colorModeButton.setTooltip (ThemeManager::getThemeName (lnf.currentVariant));
    colorModeButton.onClick = [this]
    {
        // Determine current by accent; rotate deterministically through variants
        using TV = ThemeVariant;
        static ThemeVariant order[] = { ThemeVariant::Ocean, ThemeVariant::Green, ThemeVariant::Pink, ThemeVariant::Yellow, ThemeVariant::Grey };
        auto currentAccent = lnf.theme.accent.getARGB();
        int idx = 0;
        if (currentAccent == juce::Colour (0xFF5AA9E6).getARGB()) idx = 0; // Ocean
        else if (currentAccent == juce::Colour (0xFF5AA95A).getARGB()) idx = 1; // Green
        else if (currentAccent == juce::Colour (0xFFE91E63).getARGB()) idx = 2; // Pink
        else if (currentAccent == juce::Colour (0xFFFFC107).getARGB()) idx = 3; // Yellow
        else if (currentAccent == juce::Colour (0xFF9EA3AA).getARGB()) idx = 4; // Grey
        idx = (idx + 1) % 5;
        lnf.setTheme (order[idx]);
        colorModeButton.setTooltip (ThemeManager::getThemeName (order[idx]));
        // Propagate to components that cache green flag
        const bool greenNow = (order[idx] == ThemeVariant::Green);
        // OLD REVERB SYSTEM REMOVED
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setGreenMode(greenNow);
        }
        repaint();
    };

    tooltipsButton.setTooltip ("Tooltip Assistant");
    tooltipsButton.setToggleState (tooltipAssistantOn_, juce::dontSendNotification);
    // Tooltip button logic delegated to EventManager
    tooltipsButton.onClick = [this] { /* Delegated to EventManager */ };

    // Full screen (top-level window kiosk toggle; restore original bounds)
    addAndMakeVisible (fullScreenButton);
    fullScreenButton.onClick = [this]
    {
        const bool on = fullScreenButton.getToggleState();

        if (auto* tlw = getTopLevelComponent())
        {
            if (auto* rw = dynamic_cast<juce::ResizableWindow*>(tlw))
            {
                if (on)
                {
                    // Save current window bounds to restore later
                    savedBounds = rw->getBounds();
                    rw->setFullScreen (true);
                }
                else
                {
                    rw->setFullScreen (false);
                    if (!savedBounds.isEmpty())
                        rw->setBounds (savedBounds);
                }
                return;
            }
        }

        // Fallback: if no top-level resizable window is accessible, do nothing to avoid bad states
        // Reset the toggle to off if we couldn't enter fullscreen safely
        if (on)
            fullScreenButton.setToggleState (false, juce::dontSendNotification);
    };

    // Link + Snap
    addAndMakeVisible (linkButton);
    linkButton.onClick = [this]
    {
        linkButton.setToggleState (!linkButton.getToggleState(), juce::dontSendNotification);
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setLinked(linkButton.getToggleState());
        }
    };
    addAndMakeVisible (snapButton);
    snapButton.setToggleState (false, juce::dontSendNotification); // default OFF per your note
    snapButton.onClick = [this]
    {
        const bool on = !snapButton.getToggleState();
        snapButton.setToggleState (on, juce::dontSendNotification);
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setSnapEnabled(on);
        }
    };

    // Presets UI
    // Preset field + arrows
    addAndMakeVisible (presetField);
    presetField.setButtonText ("Search presets…");
    presetField.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetField.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (prevPresetButton);
    addAndMakeVisible (nextPresetButton);

    prevPresetButton.onClick = [this]
    {
        // no-op: legacy preset combo removed
    };
    nextPresetButton.onClick = [this]
    {
        // no-op: legacy preset combo removed
    };
    // Open command-palette when clicking the preset field; arrows remain for prev/next only
    presetField.onClick = [this]
    {
        static PresetRegistry presetRegistry; // lifetime across openings
        presetRegistry.reloadAll();
        PresetCommandPalette::show(
            presetRegistry, presetField,
            // Apply
            [this](const PresetEntry& e){
                // Convert NamedValueSet to APVTS via PresetManager
                LibraryPreset tmp; tmp.meta.id = e.id; tmp.meta.name = e.name; tmp.params = e.params;
                presetManager.applyPresetAtomic (tmp);
                presetNameLabel.setText (e.name, juce::dontSendNotification);
            },
            // Load to slot
            [this](const PresetEntry& e, bool toA){ LibraryPreset tmp; tmp.params = e.params; presetManager.loadToSlot (tmp, toA); },
            // Star (persist favorite)
            [reg=&presetRegistry](const PresetEntry& e, bool fav){ reg->setFavorite (e.id, fav); },
            // Save As
            [this](juce::String name, juce::StringArray tags, juce::String cat){ auto pr = presetManager.currentAsPreset (name, cat, tags, "User preset", "", "You"); presetStore.saveUserPreset (pr); presetStore.scan(); },
            presetField.getButtonText()
        );
    };

    // Removed savePresetButton handler
    // A/B + copy
    addAndMakeVisible (abButtonA);
    addAndMakeVisible (abButtonB);
    addAndMakeVisible (copyButton);
    abButtonA.setToggleState (true, juce::dontSendNotification);
    abButtonB.setToggleState (false, juce::dontSendNotification);
    abButtonA.onClick = [this]{ if (!abButtonA.getToggleState()) toggleABState(); };
    abButtonB.onClick = [this]{ if (!abButtonB.getToggleState()) toggleABState(); };
    
    // Set metallic properties for A/B buttons to enable Button Switch styling
    setAreaMetallicForCell (abButtonA, MetallicKind::Band);
    setAreaMetallicForCell (abButtonB, MetallicKind::Band);
    copyButton.onClick = [this]
    {
        juce::PopupMenu m; m.addItem (1, "Copy A to B"); m.addItem (2, "Copy B to A");
        m.showMenuAsync (juce::PopupMenu::Options(), [this](int r)
        {
            if (r == 1) { copyState (true);  pasteState (false); }
            if (r == 2) { copyState (false); pasteState (true);  }
        });
    };

    // Split toggle
    addAndMakeVisible (splitToggle);
    splitToggle.onToggleChange = [this] (bool split)
    {
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setSplitMode(split);
        }
        linkButton.setVisible (split);
        panKnob.setVisible (!split);
        panValue.setVisible (!split);
        panKnobLeft .setVisible (split);
        panKnobRight.setVisible (split);
        panValueLeft .setVisible (split);
        panValueRight.setVisible (split);
        resized();
    };
    splitToggle.setToggleState (false, juce::dontSendNotification);
    linkButton.setVisible (false);
    // Multi-pane dock (XY, Dynamic EQ, Imager) + shade overlay
    panes = std::make_unique<PaneManager> (proc, proc.apvts.state, &lnf);
    addAndMakeVisible (panes.get());
    panes->setSampleRate (proc.getSampleRate());
    // keep-warm removed; no pane warm-up needed
    // Default to XY view on startup
    panes->setActive (PaneID::XY, true);
    // Spectrum removed; Dynamic EQ pane owns its analyzer styling

    // xyShade functionality removed - handled by PaneManager

    // Add meter components
    addAndMakeVisible (corrMeter);
    addAndMakeVisible (lrMeters);
    addAndMakeVisible (ioMeters);

    // Containers
    addAndMakeVisible (mainControlsContainer); mainControlsContainer.setTitle (""); mainControlsContainer.setShowBorder (false);
    addAndMakeVisible (panKnobContainer);      panKnobContainer.setTitle ("");     panKnobContainer.setShowBorder (true);
    addAndMakeVisible (volumeContainer);       volumeContainer.setTitle ("");   volumeContainer.setShowBorder (true);
    // Delay container will be removed; lay out directly on right side
    // addAndMakeVisible (delayContainer);        delayContainer.setTitle ("");     delayContainer.setShowBorder (true);
    // Row containers for EQ/Image are no longer used
    addAndMakeVisible (MainContentContainer);  MainContentContainer.setTitle ("");    MainContentContainer.setShowBorder (false);
    addAndMakeVisible (rightSlidersContainer);   rightSlidersContainer.setTitle ("");     rightSlidersContainer.setShowBorder (false);
    addAndMakeVisible (metersContainer);       metersContainer.setTitle ("");         metersContainer.setShowBorder (false);
    
    
    // Add 3D vertical sliders to rightSlidersContainer
    rightSlidersContainer.addAndMakeVisible (inputSlider);
    rightSlidersContainer.addAndMakeVisible (outputSlider);
    rightSlidersContainer.addAndMakeVisible (mixSlider);
    
    
    
    // Configure sliders (ranges and values already set in VerticalSlider3D constructor)
    inputSlider.setTextValueSuffix (" dB");
    inputSlider.setLookAndFeel (&lnf);
    
    outputSlider.setTextValueSuffix (" dB");
    outputSlider.setLookAndFeel (&lnf);
    
    mixSlider.setRange (0.0, 100.0, 0.1);  // Mix slider has different range
    mixSlider.setValue (100.0);
    mixSlider.setTextValueSuffix (" %");
    mixSlider.setLookAndFeel (&lnf);
    
    // Add small units display to sliders
    inputSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    outputSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    
    // Set component names for handle labels
    inputSlider.setName ("inputSlider");
    outputSlider.setName ("outputSlider");
    mixSlider.setName ("mixSlider");
    
    // Style labels with knobcell color background
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto sh = lf->theme.meters.panelDark;
    
    auto labelStyle = [&](juce::Label& label) {
        label.setColour (juce::Label::backgroundColourId, sh.withAlpha (0.15f));
        label.setColour (juce::Label::textColourId, juce::Colours::white);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (10.0f, juce::Font::bold));
    };
    

    // Width group (image row, bottom-right): invisible container + placeholder slots for spanning grid
    addChildComponent (widthGroupContainer);
    widthGroupContainer.setTitle ("");
    widthGroupContainer.setShowBorder (true);
    widthGroupContainer.setVisible (false);
    widthGroupContainer.setInterceptsMouseClicks (false, false);
    
    // Gain+Drive+Mix group (volume row): invisible container to horizontally arrange GAIN/DRIVE/MIX (all XL)
    addChildComponent (gainMixGroupContainer);
    gainMixGroupContainer.setTitle ("");
    gainMixGroupContainer.setShowBorder (false);
    gainMixGroupContainer.setVisible (false);
    gainMixGroupContainer.setInterceptsMouseClicks (false, false);
    addChildComponent (gainMixSlot1); gainMixSlot1.setInterceptsMouseClicks (false, false); gainMixSlot1.setVisible (false);
    addChildComponent (gainMixSlot2); gainMixSlot2.setInterceptsMouseClicks (false, false); gainMixSlot2.setVisible (false);

    // Ducking group (Depth, Attack, Release, Threshold) – invisible container
    addChildComponent (duckGroupContainer);
    duckGroupContainer.setTitle ("");
    duckGroupContainer.setShowBorder (false);
    duckGroupContainer.setVisible (false);
    duckGroupContainer.setInterceptsMouseClicks (false, false);
    addChildComponent (duckSlot1); duckSlot1.setInterceptsMouseClicks (false, false); duckSlot1.setVisible (false);
    addChildComponent (duckSlot2); duckSlot2.setInterceptsMouseClicks (false, false); duckSlot2.setVisible (false);
    addChildComponent (duckSlot3); duckSlot3.setInterceptsMouseClicks (false, false); duckSlot3.setVisible (false);
    addChildComponent (widthGroupSlot1); widthGroupSlot1.setInterceptsMouseClicks (false, false); widthGroupSlot1.setVisible (false);
    addChildComponent (widthGroupSlot2); widthGroupSlot2.setInterceptsMouseClicks (false, false); widthGroupSlot2.setVisible (false);
    addChildComponent (widthGroupSlot3); widthGroupSlot3.setInterceptsMouseClicks (false, false); widthGroupSlot3.setVisible (false);

    // Volume row unified group (pan, reverb, algo switch, duck group, gain/drive/mix)
    addChildComponent (volGroupContainer);
    volGroupContainer.setTitle("");
    volGroupContainer.setShowBorder(false);
    volGroupContainer.setVisible(false);
    volGroupContainer.setInterceptsMouseClicks(false, false);
    addChildComponent (volSlot1); volSlot1.setInterceptsMouseClicks(false,false); volSlot1.setVisible(false);
    addChildComponent (volSlot2); volSlot2.setInterceptsMouseClicks(false,false); volSlot2.setVisible(false);
    addChildComponent (volSlot3); volSlot3.setInterceptsMouseClicks(false,false); volSlot3.setVisible(false);
    addChildComponent (volSlot4); volSlot4.setInterceptsMouseClicks(false,false); volSlot4.setVisible(false);
    addChildComponent (volSlot5); volSlot5.setInterceptsMouseClicks(false,false); volSlot5.setVisible(false);
    addChildComponent (volSlot6); volSlot6.setInterceptsMouseClicks(false,false); volSlot6.setVisible(false);
    addChildComponent (volSlot7); volSlot7.setInterceptsMouseClicks(false,false); volSlot7.setVisible(false);

    // Split-pan overlay container
    addAndMakeVisible (panSplitContainer);
    panSplitContainer.setVisible (false);
    panSplitContainer.setInterceptsMouseClicks (false, false);

    // Sliders/knobs
    // NOTE: These names/IDs match your processor (original)
    auto style = [this](juce::Slider& s, bool main = false)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        // Align with FieldLNF ticks: π → π + 2π
        constexpr float kStart = juce::MathConstants<float>::pi;
        constexpr float kEnd   = juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi;
        s.setRotaryParameters (kStart, kEnd, true);
        s.setLookAndFeel (&lnf);
    };

    // main rotary
    for (juce::Slider* slider : { &width,&tilt,&monoHz,&hpHz,&lpHz,&satDrive,&satMix,&air,&bass,&scoop,
                              &widthLo,&widthMid,&widthHi,&xoverLoHz,&xoverHiHz,&rotationDeg,&asymmetry,
                              &shelfShapeS,&filterQ })
    {
        addAndMakeVisible (*slider);
        style (*slider);
        slider->setVelocityBasedMode (true);
        slider->setVelocityModeParameters (0.85, 1, 0.0, true);
        slider->setMouseDragSensitivity (140);
        slider->addListener (this);
    }
    addAndMakeVisible (gain); style (gain); gain.addListener (this);

    // micro sliders (freq) + HP/LP Q minis
    for (juce::Slider* slider : { &tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider, &hpQSlider, &lpQSlider })
    {
        addAndMakeVisible (*slider);
        slider->setSliderStyle (juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider->setMouseDragSensitivity (140);
        slider->setVelocityBasedMode (false);
        slider->setSliderSnapsToMousePosition (false);
        slider->setDoubleClickReturnValue (true, 0.0);
        slider->setLookAndFeel (&lnf);
        slider->addListener (this);
    }

    // link buttons (will be placed inside a cell later)
    addAndMakeVisible (tiltLinkSButton);
    addAndMakeVisible (qLinkButton);
    tiltLinkSButton.setLookAndFeel (&lnf);
    qLinkButton.setLookAndFeel (&lnf);
    tiltLinkSButton.setButtonText ("Tilt uses S");
    qLinkButton.setButtonText (""); // visual icon-only; state-driven paint
    // pan + reverb + ducking
    addAndMakeVisible (panKnob);      style (panKnob, true);     panKnob.setRange (-1.0, 1.0, 0.01); panKnob.setOverlayEnabled (false); panKnob.addListener (this);
    addAndMakeVisible (panKnobLeft);  style (panKnobLeft, true); panKnobLeft.setRange (-1.0, 1.0, 0.01); panKnobLeft.setOverlayEnabled (true);  panKnobLeft.setLabel ("L"); panKnobLeft.addListener (this);
    addAndMakeVisible (panKnobRight); style (panKnobRight, true);panKnobRight.setRange(-1.0, 1.0, 0.01); panKnobRight.setOverlayEnabled (true); panKnobRight.setLabel ("R"); panKnobRight.addListener (this);

    panKnob.setVisible (true);
    panKnobLeft.setVisible (false);
    panKnobRight.setVisible (false);

    // Legacy Group 1 reverb depth knob is no longer used; keep hidden

    // values
    for (juce::Label* l : { &gainValue,&widthValue,&tiltValue,&monoValue,&hpValue,&lpValue,&satDriveValue,&satMixValue,&airValue,&bassValue,&scoopValue,&shelfShapeValue,&filterQValue,
                             &panValue,&panValueLeft,&panValueRight,
                             &tiltFreqValue,&scoopFreqValue,&bassFreqValue,&airFreqValue,
                             &widthLoValue,&widthMidValue,&widthHiValue,&xoverLoValue,&xoverHiValue,
                             &rotationValue,&asymValue,
                             &delayTimeValue,&delayFeedbackValue,&delayWetValue,&delaySpreadValue,&delayWidthValue,&delayModRateValue,&delayModDepthValue,&delayWowflutterValue,&delayJitterValue,
                             &delayHpValue,&delayLpValue,&delayTiltValue,&delaySatValue,&delayDiffusionValue,&delayDiffuseSizeValue,
                             &delayDuckDepthValue,&delayDuckAttackValue,&delayDuckReleaseValue,&delayDuckThresholdValue,&delayDuckRatioValue,&delayDuckLookaheadValue })
    {
        addAndMakeVisible (*l);
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (15.0f * scaleFactor).withStyle ("Bold")));
        l->setColour (juce::Label::textColourId, lnf.theme.text);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    // Live EQ curve updates while turning knobs
    auto addLiveRepaint = [this](juce::Slider& s)
    {
        s.onValueChange = [this]
        {
            if (auto* xyTab = panes->getXYTab()) {
                xyTab->setTiltValue((float) tilt.getValue());
                xyTab->setHPValue((float) hpHz.getValue());
                xyTab->setLPValue((float) lpHz.getValue());
                xyTab->setAirValue((float) air.getValue());
            }
            if (auto* xyTab = panes->getXYTab()) {
                xyTab->setBassValue((float) bass.getValue());
                xyTab->setScoopValue((float) scoop.getValue());
                xyTab->setTiltFreqValue((float) tiltFreqSlider.getValue());
                xyTab->setScoopFreqValue((float) scoopFreqSlider.getValue());
                xyTab->setBassFreqValue((float) bassFreqSlider.getValue());
            }
            if (auto* xyTab = panes->getXYTab()) {
                xyTab->setAirFreqValue((float) airFreqSlider.getValue());
                xyTab->setMonoValue((float) monoHz.getValue());
                xyTab->repaint();
            }
        };
    };
    for (juce::Slider* slider : { &tilt,&hpHz,&lpHz,&air,&bass,&scoop,&tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider,&monoHz,&shelfShapeS,&filterQ,&hpQSlider,&lpQSlider })
    {
        addLiveRepaint (*slider);
        slider->onDragStart = [this]{ proc.setIsEditing (true); };
        slider->onDragEnd   = [this]{ proc.setIsEditing (false); };
    }

    // slider names (for LNF knob labels)
    gain.setName ("GAIN"); width.setName ("WIDTH"); tilt.setName ("TILT"); monoHz.setName ("MONO");
    hpHz.setName ("HP Hz"); lpHz.setName ("LP Hz"); satDrive.setName ("DRIVE"); satMix.setName ("MIX");
    air.setName ("AIR"); bass.setName ("BASS"); scoop.setName ("SCOOP");
    shelfShapeS.setName ("Shape"); filterQ.setName ("Q");
    panKnob.setName ("PAN"); panKnobLeft.setName ("PAN L"); panKnobRight.setName ("PAN R");
    tiltFreqSlider.setName ("TILT F"); scoopFreqSlider.setName ("SCOOP F"); bassFreqSlider.setName ("BASS F"); airFreqSlider.setName ("AIR F");
    // Imaging knob labels
    widthLo.setName ("W LO"); widthMid.setName ("W MID"); widthHi.setName ("W HI");
    xoverLoHz.setName ("XO LO"); xoverHiHz.setName ("XO HI");
    rotationDeg.setName ("ROT"); asymmetry.setName ("ASYM");

    // HP/LP value label precision: integer Hz
    hpHz.setNumDecimalPlacesToDisplay (0);
    lpHz.setNumDecimalPlacesToDisplay (0);

    // Imaging static text labels under knobs (hidden to avoid duplication with value labels)
    for (auto* l : { &widthLoName,&widthMidName,&widthHiName,&xoverLoName,&xoverHiName,&rotationName,&asymName,&shufLoName,&shufHiName,&shufXName })
    {
        l->setVisible (false);
        l->setInterceptsMouseClicks (false, false);
    }
    widthLoName.setText ("", juce::dontSendNotification);
    widthMidName.setText("", juce::dontSendNotification);
    widthHiName.setText ("", juce::dontSendNotification);
    xoverLoName.setText ("", juce::dontSendNotification);
    xoverHiName.setText ("", juce::dontSendNotification);
    rotationName.setText("", juce::dontSendNotification);
    asymName.setText   ("", juce::dontSendNotification);
    shufLoName.setText ("", juce::dontSendNotification);
    shufHiName.setText ("", juce::dontSendNotification);
    shufXName.setText  ("", juce::dontSendNotification);
    // Delay controls initialization
    for (juce::Slider* slider : { &delayTime, &delayFeedback, &delayWet, &delaySpread, &delayWidth, &delayModRate, &delayModDepth, &delayWowflutter, &delayJitter, &delayPreDelay,
                                  &delayHp, &delayLp, &delayTilt, &delaySat, &delayDiffusion, &delayDiffuseSize,
                                  &delayDuckDepth, &delayDuckAttack, &delayDuckRelease, &delayDuckThreshold, &delayDuckRatio, &delayDuckLookahead })
    {
        addAndMakeVisible (*slider);
        style (*slider);
        slider->addListener (this);
    }
    
    // Delay combo boxes
    for (juce::ComboBox* combo : { &delayMode, &delayTimeDiv, &delayDuckSource, &delayGridFlavor, &delayFilterType })
    {
        addAndMakeVisible (*combo);
        combo->setLookAndFeel (&lnf);
        combo->addListener (this);
        // Metallic styling now handled in DelayControlsPane
    }
    
    // Delay toggle buttons
    for (juce::ToggleButton* button : { &delayEnabled, &delaySync, &delayKillDry, &delayFreeze, &delayPingpong, &delayDuckPost, &delayDuckLinkGlobal })
    {
        addAndMakeVisible (*button);
        button->setLookAndFeel (&lnf);
        button->addListener (this);
        // Metallic styling now handled in DelayControlsPane
    }

    // Delay row-1 explicit captions (ensure text visible in SwitchCell)
    delayEnabled.setButtonText (""); delayEnabled.setToggleState (delayEnabled.getToggleState(), juce::dontSendNotification);
    delaySync.setButtonText ("");      delaySync.setToggleState (delaySync.getToggleState(), juce::dontSendNotification);
    delayPingpong.setButtonText (""); delayPingpong.setToggleState (delayPingpong.getToggleState(), juce::dontSendNotification);
    delayFreeze.setButtonText ("");  delayFreeze.setToggleState (delayFreeze.getToggleState(), juce::dontSendNotification);
    delayKillDry.setButtonText (""); delayKillDry.setToggleState (delayKillDry.getToggleState(), juce::dontSendNotification);
    delayMode.setTextWhenNothingSelected ("Mode");
    // Populate Delay Mode items from APVTS choice parameter so popup shows options
    if (auto* delayModeParam = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("delay_mode")))
    {
        delayMode.clear();
        for (int i = 0; i < delayModeParam->choices.size(); ++i)
            delayMode.addItem (delayModeParam->choices[i], i + 1);
        // Reflect current value without firing callbacks; attachment will keep in sync
        delayMode.setSelectedId (delayModeParam->getIndex() + 1, juce::dontSendNotification);
        // Per-item tints (Digital, Analog, Tape)
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            juce::Array<juce::Colour> tints;
            tints.add (lnf.theme.accent.withAlpha (0.85f));                                   // Digital: accent
            tints.add (lnf.theme.eq.tilt.withAlpha (0.90f));                                   // Analog: orange tilt
            tints.add (lnf.theme.eq.bass.withHue (lnf.theme.eq.bass.getHue() - 0.10f));        // Tape: warmer green
            lf->setPopupItemTints (tints);
        }
        delayMode.getProperties().set ("tintedSelected", true);
    }

    // Populate Delay Duck Source items from APVTS choice parameter so popup shows options
    if (auto* duckSrcParam = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("delay_duck_source")))
    {
        delayDuckSource.clear();
        for (int i = 0; i < duckSrcParam->choices.size(); ++i)
            delayDuckSource.addItem (duckSrcParam->choices[i], i + 1);
        delayDuckSource.setSelectedId (duckSrcParam->getIndex() + 1, juce::dontSendNotification);
        // Per-item tints (Pre, Post, External)
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            juce::Array<juce::Colour> tints;
            tints.add (lnf.theme.eq.hp.withAlpha (0.95f));                                     // Pre: HP blue
            tints.add (lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.12f));          // Post: cyan shift
            tints.add (lnf.theme.eq.scoop.withAlpha (0.95f));                                  // External: plum
            lf->setPopupItemTints (tints);
        }
        delayDuckSource.getProperties().set ("tintedSelected", true);
    }
    
    // Delay value labels
    for (juce::Label* l : { &delayTimeValue, &delayFeedbackValue, &delayWetValue, &delaySpreadValue, &delayWidthValue, &delayModRateValue, &delayModDepthValue, &delayWowflutterValue, &delayJitterValue, &delayPreDelayValue,
                            &delayHpValue, &delayLpValue, &delayTiltValue, &delaySatValue, &delayDiffusionValue, &delayDiffuseSizeValue,
                            &delayDuckDepthValue, &delayDuckAttackValue, &delayDuckReleaseValue, &delayDuckThresholdValue, &delayDuckRatioValue, &delayDuckLookaheadValue })
    {
        addAndMakeVisible (*l);
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (13.0f * scaleFactor).withStyle ("Bold")));
        l->setColour (juce::Label::textColourId, lnf.theme.text);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    
    // Motion value labels
    // Motion value labels removed - now handled by MotionControlsPane
    
    // Delay name labels
    for (auto* l : { &delayTimeName, &delayFeedbackName, &delayWetName, &delaySpreadName, &delayWidthName, &delayModRateName, &delayModDepthName, &delayWowflutterName, &delayJitterName, &delayPreDelayName,
                     &delayHpName, &delayLpName, &delayTiltName, &delaySatName, &delayDiffusionName, &delayDiffuseSizeName,
                     &delayDuckDepthName, &delayDuckAttackName, &delayDuckReleaseName, &delayDuckThresholdName, &delayDuckRatioName, &delayDuckLookaheadName })
    {
        l->setVisible (false);
        l->setInterceptsMouseClicks (false, false);
        l->setText ("", juce::dontSendNotification);
    }
    
    // Set delay control names
    delayTime.setName ("TIME"); delayFeedback.setName ("FB"); delayWet.setName ("WET"); delaySpread.setName ("SPREAD");
    delayWidth.setName ("WIDTH"); delayModRate.setName ("MOD RATE"); delayModDepth.setName ("MOD DEPTH");
    delayWowflutter.setName ("WOW"); delayJitter.setName ("JITTER");
    delayHp.setName ("HP"); delayLp.setName ("LP"); delayTilt.setName ("TILT"); delaySat.setName ("SAT");
    delayDiffusion.setName ("DIFF"); delayDiffuseSize.setName ("SIZE");
    delayDuckDepth.setName ("DEPTH"); delayDuckAttack.setName ("ATT"); delayDuckRelease.setName ("REL");
    delayDuckThreshold.setName ("THR"); delayDuckRatio.setName ("DUCK RAT"); delayDuckLookahead.setName ("LA");
    delayPreDelay.setName ("PRE");
    
    // Motion control names removed - now handled by MotionControlsPane

    // seed value labels with current values
    sliderValueChanged (&width);
    sliderValueChanged (&tilt);
    sliderValueChanged (&monoHz);
    sliderValueChanged (&hpHz);
    sliderValueChanged (&lpHz);
    sliderValueChanged (&satDrive);
    sliderValueChanged (&satMix);
    sliderValueChanged (&air);
    sliderValueChanged (&bass);
    sliderValueChanged (&scoop);
    sliderValueChanged (&panKnob);
    sliderValueChanged (&panKnobLeft);
    sliderValueChanged (&panKnobRight);
    sliderValueChanged (&tiltFreqSlider);
    sliderValueChanged (&scoopFreqSlider);
    sliderValueChanged (&bassFreqSlider);
    sliderValueChanged (&airFreqSlider);
    sliderValueChanged (&gain);
    updateMutedKnobVisuals();

    // Mono Maker slope & audition controls
    // Keep legacy ComboBox hidden for APVTS attachment; drive it from switch
    addChildComponent (monoSlopeChoice);
    monoSlopeChoice.addItem ("6",  1);
    monoSlopeChoice.addItem ("12", 2);
    monoSlopeChoice.addItem ("24", 3);

    // Center group controls moved to XYControlsPane (complete implementation there)
    if (!monoSlopeSwitch)
        monoSlopeSwitch = std::make_unique<MonoSlopeSwitch>();
    addAndMakeVisible (*monoSlopeSwitch);
    monoSlopeSwitch->onChange = [this](int idx)
    {
        monoSlopeChoice.setSelectedItemIndex (juce::jlimit (0, 2, idx), juce::NotificationType::sendNotification);
    };
    addAndMakeVisible (monoAuditionButton);
    monoAuditionButton.setButtonText ("AUD");

    // Parameter attachments delegated to AttachmentManager
    if (attachmentManager) {
        attachmentManager->attachAllParameters();
    }

    // All children created; allow layout from now on
    layoutReady = true;

    // OLD REVERB SYSTEM REMOVED - Now using new reverb system in Source/reverb/

    // header divider
    addAndMakeVisible (splitDivider);
    splitDivider.setVisible (true);

    // XY callbacks -> AVTS
    auto refreshXYOverlays = [this]
    {
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setMixValue   (proc.apvts.getRawParameterValue ("sat_mix")->load());
            xyTab->setDriveValue (proc.apvts.getRawParameterValue ("sat_drive_db")->load());
            xyTab->setTiltValue  (proc.apvts.getRawParameterValue ("tilt")->load());
            xyTab->setHPValue    (proc.apvts.getRawParameterValue ("hp_hz")->load());
            xyTab->setLPValue    (proc.apvts.getRawParameterValue ("lp_hz")->load());
            xyTab->setAirValue   (proc.apvts.getRawParameterValue ("air_db")->load());
            xyTab->setBassValue  (proc.apvts.getRawParameterValue ("bass_db")->load());
            xyTab->setScoopValue (proc.apvts.getRawParameterValue ("scoop")->load());
            xyTab->setTiltFreqValue  (proc.apvts.getRawParameterValue ("tilt_freq")->load());
            xyTab->setScoopFreqValue (proc.apvts.getRawParameterValue ("scoop_freq")->load());
            xyTab->setBassFreqValue  (proc.apvts.getRawParameterValue ("bass_freq")->load());
            xyTab->setAirFreqValue   (proc.apvts.getRawParameterValue ("air_freq")->load());
            xyTab->setWidthValue (proc.apvts.getRawParameterValue ("width")->load());
            xyTab->setPanValue   (proc.apvts.getRawParameterValue ("pan")->load());
            xyTab->setGainValue  (proc.apvts.getRawParameterValue ("gain_db")->load());
        }
    };

    if (auto* xyTab = panes->getXYTab()) {
        xyTab->onChange = [this, refreshXYOverlays](float x01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (0.0f); split->endChangeGesture(); }
        if (auto* pan   = proc.apvts.getParameter ("pan"))        { pan  ->beginChangeGesture(); pan  ->setValueNotifyingHost (x01);  pan  ->endChangeGesture(); }
        if (auto* dep   = proc.apvts.getParameter ("depth"))      { dep  ->beginChangeGesture(); dep  ->setValueNotifyingHost (y01);  dep  ->endChangeGesture(); }
        refreshXYOverlays();
    };
        xyTab->onSplitChange = [this, refreshXYOverlays](float l01, float r01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (1.0f); split->endChangeGesture(); }
        if (auto* pL = proc.apvts.getParameter ("pan_l")) { pL->beginChangeGesture(); pL->setValueNotifyingHost (l01); pL->endChangeGesture(); }
        if (auto* pR = proc.apvts.getParameter ("pan_r")) { pR->beginChangeGesture(); pR->setValueNotifyingHost (r01); pR->endChangeGesture(); }
        if (auto* dep= proc.apvts.getParameter ("depth")) { dep->beginChangeGesture(); dep->setValueNotifyingHost (y01); dep->endChangeGesture(); }
        refreshXYOverlays();
    };
    }
    // listeners for split overlay % etc.
    panKnobLeft.addListener (this);
    panKnobRight.addListener (this);
    // Parameter attachments now handled by AttachmentManager

    // Center group attachments
    // Center group parameter attachments moved to XYControlsPane (complete implementation there)

    // EQ and bypass parameter attachments now handled by AttachmentManager
    
    // Delay parameter attachments now handled by AttachmentManager

            // Motion includes removed - now handled by MotionControlsPane
            
            // Motion ComboBoxes, Buttons, and Sliders removed - now handled by MotionControlsPane
            
            // Motion ComboBox tinting removed - now handled by MotionControlsPane
            
            // Motion controls are now handled by MotionControlsPane - no longer created here
            
            // Motion parameter attachments removed - now handled by MotionControlsPane

    // parameter listeners (host→UI)
    // OLD REVERB SYSTEM REMOVED
    proc.apvts.addParameterListener ("split_mode", this);
    proc.apvts.addParameterListener ("pan",        this);
    proc.apvts.addParameterListener ("depth",      this);
    proc.apvts.addParameterListener ("mono_slope_db_oct", this);
    // EQ shape/Q visual linkage (live updates)
    proc.apvts.addParameterListener ("eq_shelf_shape", this);
    proc.apvts.addParameterListener ("eq_q_link",      this);
    proc.apvts.addParameterListener ("eq_filter_q",    this);
    proc.apvts.addParameterListener ("hp_q",           this);
    // Motion panner selection removed - now handled by MotionControlsPane
    proc.apvts.addParameterListener ("lp_q",           this);
    proc.apvts.addParameterListener ("tilt_link_s",    this);
    // Imaging overlays
    proc.apvts.addParameterListener ("xover_lo_hz",    this);
    proc.apvts.addParameterListener ("xover_hi_hz",    this);
    proc.apvts.addParameterListener ("rotation_deg",   this);
    proc.apvts.addParameterListener ("asymmetry",      this);

    // audio callbacks -> panes
    proc.onAudioSample   = [this](float L, float R) { if (panes) panes->onAudioSample (L, R); };
    proc.onAudioBlock    = [this](const float* L, const float* R, int n) { if (panes) panes->onAudioBlock (L, R, n); };
    proc.onAudioBlockPre = [this](const float* L, const float* R, int n) { if (panes) panes->onAudioBlockPre (L, R, n); };
    // XYPad sample rate now handled by XYTab

    // Keybindings for panes and keep-warm
    struct LocalKeyListener : public juce::KeyListener {
        PaneManager* mgr;
        explicit LocalKeyListener (PaneManager* m) : mgr (m) {}
        bool keyPressed (const juce::KeyPress& k, juce::Component*) override
        {
            if (!mgr) return false;
            if (k.getTextCharacter()=='1') { mgr->setActive (PaneID::XY, true);       return true; }
            if (k.getTextCharacter()=='2') { mgr->setActive (PaneID::DynEQ, true); return true; }
            if (k.getTextCharacter()=='3') { mgr->setActive (PaneID::Imager, true);   return true; }
            // keep-warm toggle removed (K/k)
            return false;
        }
    };
    keyListener.reset (new LocalKeyListener (panes.get()));
    addKeyListener (keyListener.get());

    // divider line component
    addAndMakeVisible (splitDivider);

    // sync XY with current values
    syncXYPadWithParameters();

    // pointer cursors on interactive children
    applyGlobalCursorPolicy();

    startTimerHz (20);

    // Image row group
    addChildComponent (imgGroupContainer);
    imgGroupContainer.setTitle("");
    imgGroupContainer.setShowBorder(false);
    imgGroupContainer.setVisible(false);
    imgGroupContainer.setInterceptsMouseClicks(false, false);
    // Row 2 group (Reverb, switch, Ducking group, Delay group)
    addChildComponent (volGroupContainer2);
    volGroupContainer2.setTitle("");
    volGroupContainer2.setShowBorder(false);
    volGroupContainer2.setVisible(false);
    volGroupContainer2.setInterceptsMouseClicks(false, false);
    // Mono group container
    addAndMakeVisible (monoGroupContainer);
    monoGroupContainer.setTitle ("");
    monoGroupContainer.setShowBorder (true);
    // Reverb group container
    // Reverb row container no longer used; layout directly
    
    // Re-enable resized() call since crash is happening after constructor
    resized();
}

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    // Editor destructor - restore crash logging for debugging
    juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    f.appendText("Editor Destructor: STARTED\n", false, false, "\n");
    
    // Delegate cleanup to CleanupManager
    if (cleanupManager) {
        cleanupManager->performCleanup();
    }
    
    f.appendText("Editor Destructor: COMPLETE\n", false, false, "\n");
}
void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Delegate painting to PaintManager
    if (paintManager) {
        paintManager->paint(g);
    }
}
void MyPluginAudioProcessorEditor::performLayout()
{
    if (!layoutReady) return;

    // Delegate layout to LayoutManager
    if (layoutManager) {
        layoutManager->performLayout();
        return;
    }
}

void MyPluginAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (eventManager)
    {
        eventManager->handleMouseDown(e);
    }
}

void MyPluginAudioProcessorEditor::mouseDrag (const juce::MouseEvent& e)
{
    noteUserInteraction();
    if (eventManager)
    {
        eventManager->handleMouseDrag(e);
    }
}

void MyPluginAudioProcessorEditor::mouseUp (const juce::MouseEvent& e)
{
    if (eventManager)
    {
        eventManager->handleMouseUp(e);
    }
}

void MyPluginAudioProcessorEditor::resized()
{
    if (!layoutReady) return;
    performLayout();
}

void MyPluginAudioProcessorEditor::mouseMove (const juce::MouseEvent& e)
{
    if (eventManager)
    {
        eventManager->handleMouseMove(e);
    }
}

// Repaint waveform from UI thread at ~30 Hz
void MyPluginAudioProcessorEditor::timerCallback()
{
    if (eventManager)
    {
        eventManager->handleTimerCallback();
    }
    
    // Additional timer logic that needs to stay in PluginEditor
    // This will be implemented when we extract the remaining timer logic
}

void MyPluginAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    juce::ignoreUnused (g);
    // No extra overlay on top of children for now
}

// Move-only animation of Group 2 overlay; avoid reflow during slide
void MyPluginAudioProcessorEditor::updateGroup2OverlayDuringSlide()
{
    // Implementation moved to LayoutManager
}

void MyPluginAudioProcessorEditor::setScaleFactor (float newScale)
{
    scaleFactor = newScale;
    resized();
    repaint();
}

void MyPluginAudioProcessorEditor::sliderValueChanged (juce::Slider* s)
{
    if (eventManager)
    {
        eventManager->handleSliderValueChanged(s);
    }
    
    // Refresh muted visuals when any control changes
    updateMutedKnobVisuals();
}

void MyPluginAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (eventManager)
    {
        eventManager->handleComboBoxChanged(comboBox);
    }
}

void MyPluginAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (eventManager)
    {
        eventManager->handleButtonClicked(button);
    }
}

void MyPluginAudioProcessorEditor::applyGlobalCursorPolicy()
{
    // Implementation moved to LayoutManager
}

void MyPluginAudioProcessorEditor::updateMutedKnobVisuals()
{
    // Implementation moved to LayoutManager
}

void MyPluginAudioProcessorEditor::parameterChanged (const juce::String& id, float nv)
{
    // Implementation moved to EventManager
}

void MyPluginAudioProcessorEditor::updatePresetDisplay() 
{ 
    if (stateManager) stateManager->updatePresetDisplay(); 
}

void MyPluginAudioProcessorEditor::syncXYPadWithParameters()
{
    // Implementation moved to LayoutManager
}

void MyPluginAudioProcessorEditor::saveCurrentState()
{
    if (stateManager) stateManager->saveCurrentState();
}
void MyPluginAudioProcessorEditor::loadState (bool loadStateA)
{
    if (stateManager) stateManager->loadState(loadStateA);
}

void MyPluginAudioProcessorEditor::toggleABState()
{
    if (stateManager) stateManager->toggleABState();
}

void MyPluginAudioProcessorEditor::copyState (bool fromA) 
{ 
    if (stateManager) stateManager->copyState(fromA); 
}

void MyPluginAudioProcessorEditor::pasteState (bool pasteToA)
{
    if (stateManager) stateManager->pasteState(pasteToA);
}

void MyPluginAudioProcessorEditor::layoutMeters(juce::Rectangle<int> metersArea, float s, float sv)
{
    // Layout the three meter components in the right strip
    if (metersArea.getWidth() <= 0 || metersArea.getHeight() <= 0) return;
    
    // Calculate meter dimensions
    const int meterWidth = juce::jlimit(Layout::dp(24, s), Layout::dp(56, s), 
                                       juce::roundToInt(metersArea.getWidth() * 0.75f));
    const int corrWidth = juce::jmax(Layout::dp(10, s), juce::roundToInt(meterWidth * 0.5f));
    const int interGap = juce::jmax(1, Layout::dp(Layout::GAP_S, s) / 2);
    const int outerPadX = juce::jmax(1, Layout::dp(Layout::GAP_S, s));
    const int outerPadY = Layout::dp(Layout::GAP, sv);
    
    // Calculate total width needed
    const int totalWidth = meterWidth * 2 + corrWidth + interGap * 2 + outerPadX * 2;
    const int actualWidth = juce::jlimit(Layout::dp(96, s), Layout::dp(240, s), totalWidth);
    
    // Center the meters in the available area
    auto centeredArea = metersArea.withWidth(actualWidth).withX(metersArea.getX() + (metersArea.getWidth() - actualWidth) / 2);
    
    // Position the three meters
    auto ioArea = centeredArea.removeFromLeft(meterWidth).reduced(outerPadX, outerPadY);
    auto lrArea = centeredArea.removeFromLeft(meterWidth).reduced(outerPadX, outerPadY);
    auto corrArea = centeredArea.removeFromLeft(corrWidth).reduced(outerPadX, outerPadY);
    
    // Set bounds for each meter
    ioMeters.setBounds(ioArea);
    lrMeters.setBounds(lrArea);
    corrMeter.setBounds(corrArea);
}

void MyPluginAudioProcessorEditor::initializeParameterAttachments()
{
    if (attachmentManager)
    {
        attachmentManager->attachAllParameters();
    }
}

void MyPluginAudioProcessorEditor::finalizeInitialization()
{
    // History removed
    // History group dividers removed
    
    // Re-enable resized() call since crash is happening after constructor
    resized();
}

// end
