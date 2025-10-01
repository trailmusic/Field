#include "DuckingFloat.h"
#include "ReverbParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Utilities/ComponentGreyout.h"

DuckingFloat::DuckingFloat(juce::AudioProcessorValueTreeState& apvts)
    : grLabel("grLabel", "GR"),
      grMeter(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox),
      modeSelector("modeSelector"),
      detectorSelector("detectorSelector"),
      depthSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      thresholdSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      ratioSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      kneeSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      attackSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      releaseSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      bandFreqSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      bandQSlider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      depthLabel("depthLabel", "Depth"),
      thresholdLabel("thresholdLabel", "Threshold"),
      ratioLabel("ratioLabel", "Ratio"),
      kneeLabel("kneeLabel", "Knee"),
      attackLabel("attackLabel", "Attack"),
      releaseLabel("releaseLabel", "Release"),
      bandFreqLabel("bandFreqLabel", "Band Freq"),
      bandQLabel("bandQLabel", "Band Q"),
      modeLabel("modeLabel", "Mode"),
      detectorLabel("detectorLabel", "Detector"),
      depthValue("depthValue", "6.0"),
      thresholdValue("thresholdValue", "-12.0"),
      ratioValue("ratioValue", "3.0"),
      kneeValue("kneeValue", "2.0"),
      attackValue("attackValue", "10.0"),
      releaseValue("releaseValue", "100.0"),
      bandFreqValue("bandFreqValue", "1000.0"),
      bandQValue("bandQValue", "1.0")
{
    setupComponents();
    
    // Set initial size - always expanded
    setSize(300, EXPANDED_HEIGHT);
    setExpanded(true);
}

DuckingFloat::~DuckingFloat()
{
    // APVTS attachments will auto-destruct
}

void DuckingFloat::setupComponents()
{
    // No expand button needed - module is always expanded
    
    // Old GR meter components removed - now using custom paintGrMeter
    
    // Mode selector - full names in menu, abbreviations in display
    addAndMakeVisible(modeSelector);
    modeSelector.addItem("General", 1);
    modeSelector.addItem("Vocal", 2);
    modeSelector.addItem("DrumBus", 3);
    modeSelector.addItem("Guitar", 4);
    modeSelector.addItem("Keys", 5);
    modeSelector.setSelectedId(1);
    modeSelector.setTextWhenNothingSelected("");
    modeSelector.setTextWhenNoChoicesAvailable("");
    
    // Detector selector - full names in menu, abbreviations in display
    addAndMakeVisible(detectorSelector);
    detectorSelector.addItem("Dry", 1);
    detectorSelector.addItem("ER", 2);
    detectorSelector.addItem("Tail", 3);
    detectorSelector.addItem("Wet Sum", 4);
    detectorSelector.setSelectedId(1);
    detectorSelector.setTextWhenNothingSelected("");
    detectorSelector.setTextWhenNoChoicesAvailable("");
    
    // Ducking controls
    addAndMakeVisible(depthSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    addAndMakeVisible(kneeSlider);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(bandFreqSlider);
    addAndMakeVisible(bandQSlider);
    
    // Labels (mode and detector labels hidden - chevron only)
    modeLabel.setVisible(false);
    detectorLabel.setVisible(false);
    
    // Hide the knob labels since KnobCell components handle their own labels
    depthLabel.setVisible(false);
    thresholdLabel.setVisible(false);
    ratioLabel.setVisible(false);
    kneeLabel.setVisible(false);
    attackLabel.setVisible(false);
    releaseLabel.setVisible(false);
    bandFreqLabel.setVisible(false);
    bandQLabel.setVisible(false);
    
    // Create KnobCell components to display values inside knobs
    depthKnobCell = std::make_unique<KnobCell>(depthSlider, depthValue, "DEP");
    thresholdKnobCell = std::make_unique<KnobCell>(thresholdSlider, thresholdValue, "THR");
    ratioKnobCell = std::make_unique<KnobCell>(ratioSlider, ratioValue, "RAT");
    kneeKnobCell = std::make_unique<KnobCell>(kneeSlider, kneeValue, "KNE");
    attackKnobCell = std::make_unique<KnobCell>(attackSlider, attackValue, "ATK");
    releaseKnobCell = std::make_unique<KnobCell>(releaseSlider, releaseValue, "REL");
    bandFreqKnobCell = std::make_unique<KnobCell>(bandFreqSlider, bandFreqValue, "FREQ");
    bandQKnobCell = std::make_unique<KnobCell>(bandQSlider, bandQValue, "Q");
    
    // Set value label mode to Managed so KnobCell positions the labels correctly
    depthKnobCell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    thresholdKnobCell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    ratioKnobCell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    kneeKnobCell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    attackKnobCell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    releaseKnobCell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    bandFreqKnobCell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    bandQKnobCell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    
    // Make KnobCells metallic compliant
    depthKnobCell->getProperties().set("reverbMetallic", true);
    thresholdKnobCell->getProperties().set("reverbMetallic", true);
    ratioKnobCell->getProperties().set("reverbMetallic", true);
    kneeKnobCell->getProperties().set("reverbMetallic", true);
    attackKnobCell->getProperties().set("reverbMetallic", true);
    releaseKnobCell->getProperties().set("reverbMetallic", true);
    bandFreqKnobCell->getProperties().set("reverbMetallic", true);
    bandQKnobCell->getProperties().set("reverbMetallic", true);
    
    // Add KnobCell components to the layout
    addAndMakeVisible(*depthKnobCell);
    addAndMakeVisible(*thresholdKnobCell);
    addAndMakeVisible(*ratioKnobCell);
    addAndMakeVisible(*kneeKnobCell);
    addAndMakeVisible(*attackKnobCell);
    addAndMakeVisible(*releaseKnobCell);
    addAndMakeVisible(*bandFreqKnobCell);
    addAndMakeVisible(*bandQKnobCell);
    
    // Configure sliders with Field LookAndFeel
    auto configureSlider = [this](juce::Slider& slider, double min, double max, double step, double val, const juce::String& suffix) {
        slider.setRange(min, max, step);
        slider.setValue(val);
        slider.setTextValueSuffix(suffix);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); // No text box like other knobs
        // Set proper rotary parameters for Field styling
        slider.setRotaryParameters(juce::MathConstants<float>::pi,
                                   juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                                   true);
        // Apply Field LookAndFeel to all sliders
        slider.setLookAndFeel(&getLookAndFeel());
        // Ensure sliders are enabled and visible
        slider.setEnabled(true);
        slider.setVisible(true);
    };
    
    configureSlider(depthSlider, 0.0, 20.0, 0.1, 6.0, " dB");
    configureSlider(thresholdSlider, -40.0, 0.0, 0.1, -12.0, " dB");
    configureSlider(ratioSlider, 1.0, 10.0, 0.1, 3.0, ":1");
    configureSlider(kneeSlider, 0.0, 10.0, 0.1, 2.0, " dB");
    configureSlider(attackSlider, 0.1, 100.0, 0.1, 10.0, " ms");
    configureSlider(releaseSlider, 10.0, 1000.0, 1.0, 100.0, " ms");
    configureSlider(bandFreqSlider, 80.0, 8000.0, 1.0, 1000.0, " Hz");
    configureSlider(bandQSlider, 0.1, 10.0, 0.1, 1.0, " Q");
    
    // Configure labels with Field LookAndFeel
    auto configureLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        // Apply Field LookAndFeel to all labels
        label.setLookAndFeel(&getLookAndFeel());
    };
    
    configureLabel(depthLabel, "Depth");
    configureLabel(thresholdLabel, "Threshold");
    configureLabel(ratioLabel, "Ratio");
    configureLabel(kneeLabel, "Knee");
    configureLabel(attackLabel, "Attack");
    configureLabel(releaseLabel, "Release");
    configureLabel(bandFreqLabel, "Band Freq");
    configureLabel(bandQLabel, "Band Q");
    configureLabel(modeLabel, "Mode");
    configureLabel(detectorLabel, "Detector");
    
    // Apply Field LookAndFeel to combo boxes with chevron styling
    auto configureComboBox = [this](juce::ComboBox& combo) {
        // Apply Field LookAndFeel to all combo boxes
        combo.setLookAndFeel(&getLookAndFeel());
        // Set properties for chevron-only styling with abbreviations
        combo.getProperties().set("chevronOnly", true);
        combo.getProperties().set("themeCompliant", true);
        combo.getProperties().set("hoverEffects", true);
        combo.getProperties().set("abbreviationMode", true);
        // Ensure text is centered and properly sized for abbreviations
        combo.setJustificationType(juce::Justification::centred);
    };
    
    configureComboBox(modeSelector);
    configureComboBox(detectorSelector);
    
    // KnobCell components handle their own value display internally
    
    // Ensure all components are properly enabled after setup
    setActive(true);
    setGreyedOut(false);
}

void DuckingFloat::setExpanded(bool shouldExpand)
{
    expanded = shouldExpand;
    updateLayout();
    repaint();
}

void DuckingFloat::updateGrMeter(float grDb)
{
    currentGrDb = juce::jlimit(-20.0f, 0.0f, grDb);
    // Custom GR meter painted in paintGrMeter method
    repaint();
}

void DuckingFloat::setActive(bool activeState)
{
    active = activeState;
    updateLayout();
    repaint();
}

void DuckingFloat::setGreyedOut(bool greyedOutState)
{
    greyedOut = greyedOutState;
    updateLayout();
    repaint();
}

void DuckingFloat::setVisible(bool shouldBeVisible)
{
    juce::Component::setVisible(shouldBeVisible);
    if (shouldBeVisible)
    {
        updateLayout();
    }
}

void DuckingFloat::lookAndFeelChanged()
{
    // Force repaint to update colors when theme changes
    repaint();
}

void DuckingFloat::setLookAndFeel(juce::LookAndFeel* newLookAndFeel)
{
    juce::Component::setLookAndFeel(newLookAndFeel);
    
    // Apply LookAndFeel to all child components
    modeSelector.setLookAndFeel(newLookAndFeel);
    detectorSelector.setLookAndFeel(newLookAndFeel);
    depthSlider.setLookAndFeel(newLookAndFeel);
    thresholdSlider.setLookAndFeel(newLookAndFeel);
    ratioSlider.setLookAndFeel(newLookAndFeel);
    kneeSlider.setLookAndFeel(newLookAndFeel);
    attackSlider.setLookAndFeel(newLookAndFeel);
    releaseSlider.setLookAndFeel(newLookAndFeel);
    bandFreqSlider.setLookAndFeel(newLookAndFeel);
    bandQSlider.setLookAndFeel(newLookAndFeel);
    
    // Apply to labels
    depthLabel.setLookAndFeel(newLookAndFeel);
    thresholdLabel.setLookAndFeel(newLookAndFeel);
    ratioLabel.setLookAndFeel(newLookAndFeel);
    kneeLabel.setLookAndFeel(newLookAndFeel);
    attackLabel.setLookAndFeel(newLookAndFeel);
    releaseLabel.setLookAndFeel(newLookAndFeel);
    bandFreqLabel.setLookAndFeel(newLookAndFeel);
    bandQLabel.setLookAndFeel(newLookAndFeel);
    modeLabel.setLookAndFeel(newLookAndFeel);
    detectorLabel.setLookAndFeel(newLookAndFeel);
    
    // Apply to KnobCell components
    if (depthKnobCell) depthKnobCell->setLookAndFeel(newLookAndFeel);
    if (thresholdKnobCell) thresholdKnobCell->setLookAndFeel(newLookAndFeel);
    if (ratioKnobCell) ratioKnobCell->setLookAndFeel(newLookAndFeel);
    if (kneeKnobCell) kneeKnobCell->setLookAndFeel(newLookAndFeel);
    if (attackKnobCell) attackKnobCell->setLookAndFeel(newLookAndFeel);
    if (releaseKnobCell) releaseKnobCell->setLookAndFeel(newLookAndFeel);
    if (bandFreqKnobCell) bandFreqKnobCell->setLookAndFeel(newLookAndFeel);
    if (bandQKnobCell) bandQKnobCell->setLookAndFeel(newLookAndFeel);
}

void DuckingFloat::resized()
{
    updateLayout();
}

void DuckingFloat::updateLayout()
{
    auto bounds = getLocalBounds().toFloat();
    
    // Set component enabled state based on active and greyed out state
    bool componentsEnabled = active && !greyedOut;
    
    // Apply greyout to all components using the utility
    ComponentGreyout::setGreyedOut(*this, !componentsEnabled, 0.4f);
    
    // Ensure sliders are properly configured for Field LookAndFeel
    for (auto* slider : {&depthSlider, &thresholdSlider, &ratioSlider, &kneeSlider, 
                        &attackSlider, &releaseSlider, &bandFreqSlider, &bandQSlider}) {
        // Always reapply LookAndFeel to ensure proper styling
        slider->setLookAndFeel(&getLookAndFeel());
    }
    
    if (expanded)
    {
        // Expanded layout - GR meter at the very top
        auto grMeterArea = bounds.removeFromTop(35.0f); // Increased height for units
        grMeterBounds = grMeterArea.toFloat();
        
        // Mode and detector selectors below GR meter
        auto selectorArea = bounds.removeFromTop(30);
        modeSelector.setBounds(selectorArea.removeFromLeft(selectorArea.getWidth() * 0.5f).reduced(2).toNearestInt());
        detectorSelector.setBounds(selectorArea.reduced(2).toNearestInt());
        
        // Knobs area below selectors
        auto knobArea = bounds;
        
        // Position 8 knobs vertically in 2 columns
        auto leftColumn = knobArea.removeFromLeft(knobArea.getWidth() * 0.5f);
        auto rightColumn = knobArea;
        
        // Left column: Depth, Threshold, Ratio, Knee
        auto cellHeight = leftColumn.getHeight() / 4.0f;
        depthKnobCell->setBounds(leftColumn.removeFromTop(cellHeight).toNearestInt());
        thresholdKnobCell->setBounds(leftColumn.removeFromTop(cellHeight).toNearestInt());
        ratioKnobCell->setBounds(leftColumn.removeFromTop(cellHeight).toNearestInt());
        kneeKnobCell->setBounds(leftColumn.removeFromTop(cellHeight).toNearestInt());
        
        // Right column: Attack, Release, Band Freq, Band Q
        cellHeight = rightColumn.getHeight() / 4.0f;
        attackKnobCell->setBounds(rightColumn.removeFromTop(cellHeight).toNearestInt());
        releaseKnobCell->setBounds(rightColumn.removeFromTop(cellHeight).toNearestInt());
        bandFreqKnobCell->setBounds(rightColumn.removeFromTop(cellHeight).toNearestInt());
        bandQKnobCell->setBounds(rightColumn.removeFromTop(cellHeight).toNearestInt());
    }
    else
    {
        // Collapsed layout - custom GR meter painted in paintCollapsed
    }
}

void DuckingFloat::paint(juce::Graphics& g)
{
    if (expanded)
        paintExpanded(g);
    else
        paintCollapsed(g);
    
    // Apply greyed out overlay if inactive
    if (greyedOut || !active)
    {
        auto bounds = getLocalBounds().toFloat();
        ComponentGreyout::paintGreyoutOverlay(g, bounds, 0.4f, 8.0f);
    }
}

void DuckingFloat::paintCollapsed(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // Anti-aliasing fix: Fill entire area first, then rounded rectangle
    const float cr = PILL_CORNER_RADIUS;
    
    // Fill entire rectangular area to prevent white corners
    g.setColour(th.meters.panelDark);
    g.fillRect(bounds);
    
    // Then draw rounded rectangle on top
    g.fillRoundedRectangle(bounds, cr);
    
    // Strong edge shading for depth
    g.setColour(th.sh.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), cr - 0.5f, 2.0f);
    
    // Border for definition
    g.setColour(th.accent.withAlpha(0.9f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), cr - 1.0f, 1.5f);
    
    // GR meter removed - now handled in paintExpanded only
}

void DuckingFloat::paintExpanded(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    const float cr = 8.0f;
    
    // Anti-aliasing fix: Fill entire area first, then rounded rectangle
    g.setColour(th.meters.panelDark);
    g.fillRect(bounds);
    
    // Then draw rounded rectangle on top
    g.fillRoundedRectangle(bounds, cr);
    
    // Strong edge shading for depth
    g.setColour(th.sh.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), cr - 0.5f, 2.0f);
    
    // Border for definition
    g.setColour(th.accent.withAlpha(0.9f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), cr - 1.0f, 1.5f);
    
    // Additional thin theme border for better visibility
    g.setColour(th.text.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), cr - 0.5f, 0.5f);
    
    // Draw GR meter in the reserved area (spans full vertical height)
    if (grMeterBounds.getWidth() > 0 && grMeterBounds.getHeight() > 0)
    {
        paintGrMeter(g, grMeterBounds);
    }
}

void DuckingFloat::paintGrMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    const float cr = 4.0f;
    const float meterHeight = 20.0f;
    const float unitsHeight = 12.0f;
    
    // Reserve space for units at the top
    auto unitsArea = bounds.removeFromTop(unitsHeight);
    auto meterArea = bounds.removeFromBottom(meterHeight);
    
    // Reduce meter width by 5px on each side to avoid border conflict
    meterArea = meterArea.reduced(5.0f, 0.0f);
    
    // No background - transparent track
    
    // Standard border treatment: accent border (reduced brightness for meters)
    g.setColour(lf->theme.accent.withAlpha(0.3f));
    g.drawRoundedRectangle(meterArea, cr, 1.0f);
    
    // Draw inactive state indicator when ducking is disabled
    if (!active || greyedOut) {
        // Inactive state - more visible line at the bottom
        auto inactiveArea = juce::Rectangle<float>(meterArea.getX(), meterArea.getBottom() - 2.0f, meterArea.getWidth(), 2.0f);
        g.setColour(th.accent.withAlpha(0.3f));
        g.fillRoundedRectangle(inactiveArea, 1.0f);
    }
    // Draw ready state indicator when ducking is enabled but no GR yet
    else if (active && !greyedOut && currentGrDb >= -0.1f) {
        // Subtle ready state - thin line at the bottom
        auto readyArea = juce::Rectangle<float>(meterArea.getX(), meterArea.getBottom() - 2.0f, meterArea.getWidth(), 2.0f);
        g.setColour(th.accent.withAlpha(0.4f));
        g.fillRoundedRectangle(readyArea, 1.0f);
    }
    
    // Only draw GR bar if there's actual gain reduction (more than -0.1 dB)
    if (currentGrDb < -0.1f) {
        // GR level bar with smooth gradient
        auto grLevel = juce::jmap(currentGrDb, -20.0f, 0.0f, 0.0f, 1.0f);
        auto grBar = juce::Rectangle<float>(meterArea.getX(), meterArea.getY(), meterArea.getWidth() * grLevel, meterArea.getHeight());
        
        // Enhanced color scheme based on GR level (using Field theme colors)
        juce::Colour startColor, endColor;
        if (currentGrDb > -3.0f) {
            // Error zone - softer red
            startColor = th.meters.error.withAlpha(0.60f);
            endColor = th.meters.error.withAlpha(0.75f);
        } else if (currentGrDb > -6.0f) {
            // Warning zone - amber
            startColor = th.meters.warning.withAlpha(0.70f);
            endColor = th.meters.warning.withAlpha(0.90f);
        } else if (currentGrDb > -12.0f) {
            // Safe zone - blue
            startColor = th.meters.safe.withAlpha(0.55f);
            endColor = th.meters.safe.withAlpha(0.85f);
        } else {
            // Calm default state - neutral gray when no GR (including 0.0 dB)
            startColor = th.meters.trackBase.withAlpha(0.3f);
            endColor = th.meters.trackBase.withAlpha(0.5f);
        }
        
        juce::ColourGradient grGradient(startColor, grBar.getX(), grBar.getY(), endColor, grBar.getX(), grBar.getBottom(), false);
        g.setFillType(juce::FillType(grGradient));
        g.fillRoundedRectangle(grBar, cr - 1.0f);
        g.setFillType(juce::FillType());
        
        // No peak line or text overlay - clean meter with just colors
    }
    
    // Draw scale markers inside the meter (0, -5, -10, -15, -20 dB)
    g.setColour(th.text.withAlpha(0.4f));
    g.setFont(juce::FontOptions(8.0f));
    
    // Add padding to prevent labels from being cut off at the edges
    auto paddedMeterArea = meterArea.reduced(6.0f, 0.0f); // 6px padding on each side
    
    for (int i = 0; i <= 4; ++i) {
        float db = i * -5.0f; // 0, -5, -10, -15, -20
        float x = paddedMeterArea.getX() + (i * paddedMeterArea.getWidth() / 4.0f);
        g.drawText(juce::String(db, 0), juce::Rectangle<float>(x - 8, meterArea.getY() + 2, 16, meterArea.getHeight() - 4), juce::Justification::centred);
    }
    
    // Draw units at the top
    g.setColour(th.text.withAlpha(0.8f)); // Use brighter text color with slight transparency
    g.setFont(juce::FontOptions(9.0f));
    g.drawText("GR dB", unitsArea, juce::Justification::centred);
}
