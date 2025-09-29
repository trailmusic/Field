#include "LayoutManager.h"
#include "../../Core/PluginEditor.h"

LayoutManager::LayoutManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor)
{
    updateControlGridMetrics();
}

void LayoutManager::performLayout()
{
    // Update metrics based on current size
    updateControlGridMetrics();
    
    // Layout all sections
    layoutHeader();
    layoutMainControls();
    layoutCenterGroup();
    layoutPhaseControls();
    layoutDelayControls();
    layoutReverbControls();
    layoutMotionControls();
    layoutImagerControls();
    layoutMachineControls();
    layoutXYPad();
    layoutSpectrumAnalyzer();
}

void LayoutManager::layoutHeader()
{
    // Extract header layout logic from PluginEditor::performLayout()
    // This method handles the wood bar controls (reduced height) section
    
    const float s = juce::jmax(0.5f, editor.scaleFactor);
    const float sv = juce::jlimit(0.5f, 2.0f, (float)editor.getHeight() / (float)editor.baseHeight);
    const int bottomReserve = Layout::dp(6, sv) + Layout::dp(22, sv);
    auto r = editor.getLocalBounds().reduced(Layout::dp(Layout::PAD, s)).withTrimmedBottom(bottomReserve);
    
    // 1) wood bar controls (reduced height)
    auto woodBar = r.removeFromTop(Layout::dp(50, s));
    juce::Grid header;
    header.rowGap = juce::Grid::Px(Layout::dp(4, s));
    header.columnGap = juce::Grid::Px(0);
    header.alignContent = juce::Grid::AlignContent::center;
    header.justifyContent = juce::Grid::JustifyContent::center;
    header.alignItems = juce::Grid::AlignItems::center;
    header.justifyItems = juce::Grid::JustifyItems::center;
    header.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

    // Compute dynamic left header width based on painted logo size
    juce::Font logoFont(juce::FontOptions(26.0f * s).withStyle("Bold"));
    const int logoTextW = (int)logoFont.getStringWidthFloat("FIELD");
    const int leftInset = Layout::dp(20, s);
    const int bypassW = Layout::dp(56, s);
    const int leftPaddingAfterLogo = Layout::dp(120, s); // increased gap between logo and bypass
    const int leftHeaderW = juce::jmax(Layout::dp(240, s), leftInset + logoTextW + leftPaddingAfterLogo + bypassW);

    header.templateColumns = {
        juce::Grid::TrackInfo(juce::Grid::Px(leftHeaderW)),         // left: reserve for painted logo + bypass
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(60, s))),  // spacer between left and center controls
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(360, s))), // center: preset field
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // prev
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // next
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // A
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // B
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // copy
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(16, s))),  // spacer
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // snap (moved left of split)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(16, s))),  // spacer left of split
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(120, s))), // split
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // link (center group)
        juce::Grid::TrackInfo(juce::Grid::Fr(1)),                   // spacer before right utilities
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(176, s))), // transport clock (right group)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // color mode (right)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s))),  // tooltips (right)
        juce::Grid::TrackInfo(juce::Grid::Px(Layout::dp(40, s)))   // fullscreen (right)
    };

    const int h = Layout::dp(24, s);
    auto sizeBtn = [&](juce::Component& c, int w){ c.setSize(w, h); };

    sizeBtn(editor.bypassButton, Layout::dp(56, s));
    // Build left header group (bypass only)
    if (editor.headerLeftGroup.getParentComponent() != &editor) editor.addAndMakeVisible(editor.headerLeftGroup);
    if (editor.bypassButton.getParentComponent() != &editor.headerLeftGroup) editor.headerLeftGroup.addAndMakeVisible(editor.bypassButton);
    editor.headerLeftGroup.setBounds(0, 0, leftHeaderW, h);
    // Place bypass just after the painted logo text + padding
    editor.bypassButton.setTopLeftPosition(leftInset + logoTextW + leftPaddingAfterLogo, 0);
    
    sizeBtn(editor.abButtonA, Layout::dp(40, s));
    sizeBtn(editor.abButtonB, Layout::dp(40, s));
    sizeBtn(editor.copyButton, Layout::dp(40, s));
    sizeBtn(editor.prevPresetButton, Layout::dp(40, s));
    sizeBtn(editor.nextPresetButton, Layout::dp(40, s));
    editor.presetNameLabel.setMinimumHorizontalScale(0.8f);
    editor.presetNameLabel.setText("Presets", juce::dontSendNotification);
    editor.presetNameLabel.setJustificationType(juce::Justification::centredLeft);
    editor.splitToggle.setSize(Layout::dp(120, s), Layout::dp(28, s));
    sizeBtn(editor.linkButton, Layout::dp(40, s));
    sizeBtn(editor.snapButton, Layout::dp(40, s));
    
    // Transport clock label styling and sizing (larger, right-aligned)
    {
        if (editor.transportClockLabel.getParentComponent() != &editor) editor.addAndMakeVisible(editor.transportClockLabel);
        editor.transportClockLabel.setJustificationType(juce::Justification::centredRight);
        editor.transportClockLabel.setInterceptsMouseClicks(false, false);
        editor.transportClockLabel.setText("00:00.000", juce::dontSendNotification);
        if (auto* lf = dynamic_cast<FieldLNF*>(&editor.getLookAndFeel()))
        {
            editor.transportClockLabel.setColour(juce::Label::textColourId, lf->theme.text);
        }
        // Larger font
        editor.transportClockLabel.setFont(juce::Font(juce::FontOptions(18.0f * s).withStyle("Bold")));
        const int clockW = Layout::dp(176, s);
        editor.transportClockLabel.setSize(clockW, h);
    }
    
    sizeBtn(editor.colorModeButton, Layout::dp(40, s));
    sizeBtn(editor.tooltipsButton, Layout::dp(40, s));
    sizeBtn(editor.fullScreenButton, Layout::dp(40, s));
    sizeBtn(editor.optionsButton, Layout::dp(40, s));

    header.items = {
        juce::GridItem(editor.headerLeftGroup),
        juce::GridItem(), // spacer after bypass
        juce::GridItem(editor.presetField),
        juce::GridItem(editor.prevPresetButton),
        juce::GridItem(editor.nextPresetButton),
        juce::GridItem(editor.abButtonA),
        juce::GridItem(editor.abButtonB),
        juce::GridItem(editor.copyButton),
        juce::GridItem(), // spacer left of divider
        juce::GridItem(editor.snapButton),
        juce::GridItem(), // spacer left of split
        juce::GridItem(editor.splitToggle),
        juce::GridItem(editor.linkButton),
        juce::GridItem(), // spacer before right utilities
        juce::GridItem(editor.transportClockLabel),
        juce::GridItem(editor.colorModeButton),
        juce::GridItem(editor.tooltipsButton),
        juce::GridItem(editor.fullScreenButton),
    };

    auto headerArea = woodBar.reduced(Layout::dp(Layout::GAP, s), Layout::dp(6, s))
                             .withTrimmedBottom(Layout::dp(8, s))
                             .withTrimmedTop(Layout::dp(2, s));
    header.performLayout(headerArea);

    // Tooltip bubble menu callback
    editor.tooltipBubble.onMenu = [this](juce::Point<int> where)
    {
        juce::PopupMenu m; m.setLookAndFeel(&editor.lnf);
        m.addSectionHeader("Tooltip Options");
        m.addItem(1, "Open DYN_EQ Tooltips Doc");
        m.addItem(2, "Turn Assistant Off", editor.tooltipAssistantOn_);
        m.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(juce::Rectangle<int>(where.x, where.y, 1, 1)),
            [this](int r){ if (r == 1) { juce::URL::createWithoutParsing("file://docs/notes/DYN_EQ_TOOLTIPS.md").launchInDefaultBrowser(); }
                           if (r == 2) { editor.tooltipAssistantOn_ = false; editor.tooltipsButton.setToggleState(false, juce::dontSendNotification); editor.tooltipBubble.setVisible(false); editor.repaint(); } });
    };

    if (!editor.tooltipBubble.isOnDesktop()) editor.addChildComponent(editor.tooltipBubble);

    // options + phase mode at bottom-left; help to bottom-right; bottom-center panel toggle
    {
        auto bounds = editor.getLocalBounds();
        const int padding = Layout::dp(8, s);
        const int btnW = Layout::dp(40, s);
        const int btnH = h;
        const int leftY = bounds.getBottom() - btnH - padding;
        editor.optionsButton.setBounds(bounds.getX() + padding, leftY, btnW, btnH);
        editor.phaseModeButton.setBounds(editor.optionsButton.getRight() + Layout::dp(8, s), leftY, btnW, btnH);
        editor.qualityButton.setBounds(editor.phaseModeButton.getRight() + Layout::dp(8, s), leftY, btnW, btnH);
        editor.helpButton.setBounds(bounds.getRight() - btnW - padding, leftY, btnW, btnH);
    }
}

void LayoutManager::layoutMainControls()
{
    // Extract main controls layout logic from PluginEditor::performLayout()
    // This method handles the main XY area + vertical meters on right side
    
    const float s = juce::jmax(0.5f, editor.scaleFactor);
    const float sv = juce::jlimit(0.5f, 2.0f, (float)editor.getHeight() / (float)editor.baseHeight);
    const int bottomReserve = Layout::dp(6, sv) + Layout::dp(22, sv);
    auto r = editor.getLocalBounds().reduced(Layout::dp(Layout::PAD, s)).withTrimmedBottom(bottomReserve);
    
    // Remove the header area that was already processed
    r.removeFromTop(Layout::dp(50, s));
    
    // Clean layout: meters (left) → center content → sliders (right)
    
    // 1) Calculate meters width
    const int lPx_rs = Layout::dp((float)Layout::knobPx(Layout::Knob::L), s);
    const int cellW_rs = lPx_rs + Layout::dp(8, s);
    const int colW_m = juce::jlimit(Layout::dp(24, s), Layout::dp(56, s), juce::roundToInt(cellW_rs * 0.75f));
    const int corrW_m = juce::jmax(Layout::dp(10, s), juce::roundToInt(colW_m * 0.5f));
    const int inter_m = juce::jmax(1, Layout::dp(Layout::GAP_S, s) / 2);
    const int outerPadM_X = juce::jmax(1, Layout::dp(Layout::GAP_S, s));
    const int targetStripW = colW_m * 2 + corrW_m + inter_m * 2 + outerPadM_X * 2;
    const int metersWidth = juce::jlimit(Layout::dp(96, s), Layout::dp(240, s), targetStripW);
    
    // 2) Calculate sliders width
    const int slidersWidth = juce::jlimit(Layout::dp(80, s), Layout::dp(120, s), 
                                         juce::roundToInt(r.getWidth() * 0.15f));
    
    // 3) Layout meters on the left
    auto metersArea = r.removeFromLeft(metersWidth);
    editor.layoutMeters(metersArea, s, sv);
    
    // 4) Layout sliders on the right
    auto slidersArea = r.removeFromRight(slidersWidth);
    editor.rightSlidersContainer.setBounds(slidersArea);
    
    // Layout the individual sliders horizontally within the container
    const int sliderWidth = slidersArea.getWidth() / 3;
    editor.inputSlider.setBounds(0, 0, sliderWidth, slidersArea.getHeight());
    editor.outputSlider.setBounds(sliderWidth, 0, sliderWidth, slidersArea.getHeight());
    editor.mixSlider.setBounds(sliderWidth * 2, 0, sliderWidth, slidersArea.getHeight());
    
    // 5) Layout main content in the center (remaining area)
    if (editor.panes) {
        editor.panes->setBounds(r);
        editor.panes->resized();
    }
}

void LayoutManager::layoutCenterGroup()
{
    // Center group controls moved to XYControlsPane (complete implementation there)
    // No layout needed in PluginEditor as center group is handled by XY tab
}

void LayoutManager::layoutPhaseControls()
{
    // PLACEHOLDER: Phase controls layout logic
    // This will be implemented by extracting from PluginEditor::performLayout()
    // without changing any visual behavior
}

void LayoutManager::layoutDelayControls()
{
    // PLACEHOLDER: Delay controls layout logic
    // This will be implemented by extracting from PluginEditor::performLayout()
    // without changing any visual behavior
}

void LayoutManager::layoutReverbControls()
{
    // PLACEHOLDER: Reverb controls layout logic
    // This will be implemented by extracting from PluginEditor::performLayout()
    // without changing any visual behavior
}

void LayoutManager::layoutMotionControls()
{
    // PLACEHOLDER: Motion controls layout logic
    // This will be implemented by extracting from PluginEditor::performLayout()
    // without changing any visual behavior
}

void LayoutManager::layoutImagerControls()
{
    // PLACEHOLDER: Imager controls layout logic
    // This will be implemented by extracting from PluginEditor::performLayout()
    // without changing any visual behavior
}

void LayoutManager::layoutMachineControls()
{
    // PLACEHOLDER: Machine controls layout logic
    // This will be implemented by extracting from PluginEditor::performLayout()
    // without changing any visual behavior
}

void LayoutManager::layoutXYPad()
{
    // PLACEHOLDER: XY Pad layout logic
    // This will be implemented by extracting from PluginEditor::performLayout()
    // without changing any visual behavior
}

void LayoutManager::layoutSpectrumAnalyzer()
{
    // PLACEHOLDER: Spectrum analyzer layout logic
    // This will be implemented by extracting from PluginEditor::performLayout()
    // without changing any visual behavior
}

void LayoutManager::setResizeConstraints()
{
    // Set minimum and maximum size constraints
    editor.setResizable(true, true);
    editor.setResizeLimits(800, 600, 3000, 2000);
}

void LayoutManager::updateControlGridMetrics()
{
    // Update grid metrics based on current editor size
    auto bounds = editor.getLocalBounds();
    currentWidth = bounds.getWidth();
    currentHeight = bounds.getHeight();
    
    calculateGridMetrics();
}

juce::Rectangle<int> LayoutManager::getControlBounds(int row, int col, int width, int height) const
{
    return calculateControlBounds(row, col, width, height);
}

juce::Rectangle<int> LayoutManager::getHeaderBounds() const
{
    return juce::Rectangle<int>(0, 0, currentWidth, 60);
}

juce::Rectangle<int> LayoutManager::getMainAreaBounds() const
{
    return juce::Rectangle<int>(0, 60, currentWidth, currentHeight - 60);
}

juce::Rectangle<int> LayoutManager::getTabAreaBounds() const
{
    return juce::Rectangle<int>(0, currentHeight - 40, currentWidth, 40);
}

void LayoutManager::calculateGridMetrics()
{
    // Calculate grid metrics based on current size
    // This will be implemented based on the specific grid system
}

juce::Rectangle<int> LayoutManager::calculateControlBounds(int row, int col, int width, int height) const
{
    // Calculate bounds for a control at the specified grid position
    // This will be implemented based on the specific grid system
    return juce::Rectangle<int>(col * 100, row * 100, width * 100, height * 100);
}

void LayoutManager::positionKnobCell(juce::Component& component, int row, int col)
{
    auto bounds = getControlBounds(row, col);
    component.setBounds(bounds);
}

void LayoutManager::positionButton(juce::Component& component, int row, int col)
{
    auto bounds = getControlBounds(row, col);
    component.setBounds(bounds);
}

void LayoutManager::positionComboBox(juce::Component& component, int row, int col)
{
    auto bounds = getControlBounds(row, col);
    component.setBounds(bounds);
}

void LayoutManager::positionSlider(juce::Component& component, int row, int col)
{
    auto bounds = getControlBounds(row, col);
    component.setBounds(bounds);
}

void LayoutManager::positionLabel(juce::Component& component, int row, int col)
{
    auto bounds = getControlBounds(row, col);
    component.setBounds(bounds);
}

void LayoutManager::buildCells()
{
    // Row 1
    if (!editor.widthCell)   editor.widthCell   = std::make_unique<KnobCell>(editor.width,    editor.widthValue,    "WIDTH");
    if (!editor.widthLoCell) editor.widthLoCell = std::make_unique<KnobCell>(editor.widthLo,  editor.widthLoValue,  "W LO");
    if (!editor.widthMidCell)editor.widthMidCell= std::make_unique<KnobCell>(editor.widthMid, editor.widthMidValue, "W MID");
    if (!editor.widthHiCell) editor.widthHiCell = std::make_unique<KnobCell>(editor.widthHi,  editor.widthHiValue,  "W HI");
    if (!editor.gainCell)    editor.gainCell    = std::make_unique<KnobCell>(editor.gain,     editor.gainValue,     "GAIN");
    if (!editor.satDriveCell)editor.satDriveCell= std::make_unique<KnobCell>(editor.satDrive, editor.satDriveValue, "DRIVE");
    if (!editor.satMixCell)  editor.satMixCell  = std::make_unique<KnobCell>(editor.satMix,   editor.satMixValue,   "MIX");
    if (!editor.monoCell)    editor.monoCell    = std::make_unique<KnobCell>(editor.monoHz,   editor.monoValue,     "MONO");
    // Legacy spaceCell (REVERB) removed from Group 1 row; Reverb amount lives in Group 2 as WET

    if (!editor.bassCell)     editor.bassCell     = std::make_unique<KnobCell>(editor.bass,  editor.bassValue,  "BASS");
    if (!editor.airCell)      editor.airCell      = std::make_unique<KnobCell>(editor.air,   editor.airValue,   "AIR");
    if (!editor.tiltCell)     editor.tiltCell     = std::make_unique<KnobCell>(editor.tilt,  editor.tiltValue,  "TILT");
    if (!editor.scoopCell)    editor.scoopCell    = std::make_unique<KnobCell>(editor.scoop, editor.scoopValue, "SCOOP");
    if (!editor.hpCell)       editor.hpCell       = std::make_unique<KnobCell>(editor.hpHz,  editor.hpValue,    "HP Hz");
    if (!editor.lpCell)       editor.lpCell       = std::make_unique<KnobCell>(editor.lpHz,  editor.lpValue,    "LP Hz");

    if (!editor.xoverLoCell)  editor.xoverLoCell  = std::make_unique<KnobCell>(editor.xoverLoHz, editor.xoverLoValue, "XO LO");
    if (!editor.xoverHiCell)  editor.xoverHiCell  = std::make_unique<KnobCell>(editor.xoverHiHz, editor.xoverHiValue, "XO HI");
    if (!editor.rotationCell) editor.rotationCell = std::make_unique<KnobCell>(editor.rotationDeg, editor.rotationValue, "ROT");
    if (!editor.asymCell)     editor.asymCell     = std::make_unique<KnobCell>(editor.asymmetry,   editor.asymValue,     "ASYM");
    // SHUF cells moved to Band tab

    if (!editor.delayTimeCell)      editor.delayTimeCell       = std::make_unique<KnobCell>(editor.delayTime,      editor.delayTimeValue,      "TIME");
    if (!editor.delayFeedbackCell)  editor.delayFeedbackCell   = std::make_unique<KnobCell>(editor.delayFeedback,  editor.delayFeedbackValue,  "FB");
    if (!editor.delayWetCell)       editor.delayWetCell        = std::make_unique<KnobCell>(editor.delayWet,       editor.delayWetValue,       "WET");
    if (!editor.delaySpreadCell)    editor.delaySpreadCell     = std::make_unique<KnobCell>(editor.delaySpread,    editor.delaySpreadValue,    "SPREAD");
    if (!editor.delayWidthCell)     editor.delayWidthCell      = std::make_unique<KnobCell>(editor.delayWidth,     editor.delayWidthValue,     "WIDTH");
    if (!editor.delayModRateCell)   editor.delayModRateCell    = std::make_unique<KnobCell>(editor.delayModRate,   editor.delayModRateValue,   "RATE");
    if (!editor.delayModDepthCell)  editor.delayModDepthCell   = std::make_unique<KnobCell>(editor.delayModDepth,  editor.delayModDepthValue,  "DEPTH");
    if (!editor.delayWowflutterCell)editor.delayWowflutterCell = std::make_unique<KnobCell>(editor.delayWowflutter,editor.delayWowflutterValue,"WOW");
    if (!editor.delayJitterCell)    editor.delayJitterCell     = std::make_unique<KnobCell>(editor.delayJitter,    editor.delayJitterValue,    "JITTER");
    if (!editor.delayHpCell)        editor.delayHpCell         = std::make_unique<KnobCell>(editor.delayHp,        editor.delayHpValue,        "HP");
    if (!editor.delayLpCell)        editor.delayLpCell         = std::make_unique<KnobCell>(editor.delayLp,        editor.delayLpValue,        "LP");
    if (!editor.delayTiltCell)      editor.delayTiltCell       = std::make_unique<KnobCell>(editor.delayTilt,      editor.delayTiltValue,      "TILT");
    if (!editor.delaySatCell)       editor.delaySatCell        = std::make_unique<KnobCell>(editor.delaySat,       editor.delaySatValue,       "SAT");
    if (!editor.delayDiffusionCell) editor.delayDiffusionCell  = std::make_unique<KnobCell>(editor.delayDiffusion, editor.delayDiffusionValue, "DIFF");
    if (!editor.delayDiffuseSizeCell)editor.delayDiffuseSizeCell= std::make_unique<KnobCell>(editor.delayDiffuseSize, editor.delayDiffuseSizeValue, "SIZE");
    if (!editor.delayDuckDepthCell) editor.delayDuckDepthCell  = std::make_unique<KnobCell>(editor.delayDuckDepth, editor.delayDuckDepthValue, "DEPTH");
    if (!editor.delayDuckAttackCell)editor.delayDuckAttackCell = std::make_unique<KnobCell>(editor.delayDuckAttack,editor.delayDuckAttackValue,"ATT");
    if (!editor.delayDuckReleaseCell)editor.delayDuckReleaseCell=std::make_unique<KnobCell>(editor.delayDuckRelease,editor.delayDuckReleaseValue,"REL");
    if (!editor.delayJitterCell) editor.delayJitterCell = std::make_unique<KnobCell>(editor.delayJitter, editor.delayJitterValue, "JITTER");
    if (!editor.delayDuckRatioCell) editor.delayDuckRatioCell = std::make_unique<KnobCell>(editor.delayDuckRatio, editor.delayDuckRatioValue, "RAT");
}
