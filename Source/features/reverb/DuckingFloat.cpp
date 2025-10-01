#include "DuckingFloat.h"
#include "ReverbParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"

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
      detectorLabel("detectorLabel", "Detector")
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
    
    // Mode selector
    addAndMakeVisible(modeSelector);
    modeSelector.addItem("General", 1);
    modeSelector.addItem("Vocal", 2);
    modeSelector.addItem("DrumBus", 3);
    modeSelector.addItem("Guitar", 4);
    modeSelector.addItem("Keys", 5);
    modeSelector.setSelectedId(1);
    
    // Detector selector
    addAndMakeVisible(detectorSelector);
    detectorSelector.addItem("Dry", 1);
    detectorSelector.addItem("ER", 2);
    detectorSelector.addItem("Tail", 3);
    detectorSelector.addItem("Wet Sum", 4);
    detectorSelector.setSelectedId(1);
    
    // Ducking controls
    addAndMakeVisible(depthSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    addAndMakeVisible(kneeSlider);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(bandFreqSlider);
    addAndMakeVisible(bandQSlider);
    
    // Labels
    addAndMakeVisible(depthLabel);
    addAndMakeVisible(thresholdLabel);
    addAndMakeVisible(ratioLabel);
    addAndMakeVisible(kneeLabel);
    addAndMakeVisible(attackLabel);
    addAndMakeVisible(releaseLabel);
    addAndMakeVisible(bandFreqLabel);
    addAndMakeVisible(bandQLabel);
    addAndMakeVisible(modeLabel);
    addAndMakeVisible(detectorLabel);
    
    // Configure sliders with Field LookAndFeel
    auto configureSlider = [this](juce::Slider& slider, double min, double max, double step, double val, const juce::String& suffix) {
        slider.setRange(min, max, step);
        slider.setValue(val);
        slider.setTextValueSuffix(suffix);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
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
    
    // Apply Field LookAndFeel to combo boxes
    auto configureComboBox = [this](juce::ComboBox& combo) {
        // Apply Field LookAndFeel to all combo boxes
        combo.setLookAndFeel(&getLookAndFeel());
    };
    
    configureComboBox(modeSelector);
    configureComboBox(detectorSelector);
    
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

void DuckingFloat::resized()
{
    updateLayout();
}

void DuckingFloat::updateLayout()
{
    auto bounds = getLocalBounds().toFloat();
    
    // Set component enabled state based on active and greyed out state
    bool componentsEnabled = active && !greyedOut;
    
    // Ensure sliders are properly configured for Field LookAndFeel
    for (auto* slider : {&depthSlider, &thresholdSlider, &ratioSlider, &kneeSlider, 
                        &attackSlider, &releaseSlider, &bandFreqSlider, &bandQSlider}) {
        // Always reapply LookAndFeel to ensure proper styling
        slider->setLookAndFeel(&getLookAndFeel());
    }
    
    // Old GR meter removed - using custom paintGrMeter
    modeSelector.setEnabled(componentsEnabled);
    detectorSelector.setEnabled(componentsEnabled);
    depthSlider.setEnabled(componentsEnabled);
    thresholdSlider.setEnabled(componentsEnabled);
    ratioSlider.setEnabled(componentsEnabled);
    kneeSlider.setEnabled(componentsEnabled);
    attackSlider.setEnabled(componentsEnabled);
    releaseSlider.setEnabled(componentsEnabled);
    bandFreqSlider.setEnabled(componentsEnabled);
    bandQSlider.setEnabled(componentsEnabled);
    
    // Disable labels when greyed out
    depthLabel.setEnabled(componentsEnabled);
    thresholdLabel.setEnabled(componentsEnabled);
    ratioLabel.setEnabled(componentsEnabled);
    kneeLabel.setEnabled(componentsEnabled);
    attackLabel.setEnabled(componentsEnabled);
    releaseLabel.setEnabled(componentsEnabled);
    bandFreqLabel.setEnabled(componentsEnabled);
    bandQLabel.setEnabled(componentsEnabled);
    modeLabel.setEnabled(componentsEnabled);
    detectorLabel.setEnabled(componentsEnabled);
    
    if (expanded)
    {
        // Expanded layout
        auto headerArea = bounds.removeFromTop(40.0f);
        
        // Header area for custom GR meter (painted in paintExpanded)
        
        // Mode and detector selectors
        auto selectorArea = bounds.removeFromTop(30);
        modeLabel.setBounds(selectorArea.removeFromLeft(60).toNearestInt());
        modeSelector.setBounds(selectorArea.removeFromLeft(80).toNearestInt());
        detectorLabel.setBounds(selectorArea.removeFromLeft(60).toNearestInt());
        detectorSelector.setBounds(selectorArea.removeFromLeft(80).toNearestInt());
        
        // Ducking controls in 2 rows
        auto controlArea = bounds;
        auto row1 = controlArea.removeFromTop(controlArea.getHeight() * 0.5f);
        auto row2 = controlArea;
        
        // Row 1: Depth, Threshold, Ratio, Knee
        auto cellWidth = row1.getWidth() / 4.0f;
        depthLabel.setBounds(row1.removeFromLeft(cellWidth).removeFromTop(15).toNearestInt());
        depthSlider.setBounds(row1.removeFromLeft(cellWidth).toNearestInt());
        thresholdLabel.setBounds(row1.removeFromLeft(cellWidth).removeFromTop(15).toNearestInt());
        thresholdSlider.setBounds(row1.removeFromLeft(cellWidth).toNearestInt());
        ratioLabel.setBounds(row1.removeFromLeft(cellWidth).removeFromTop(15).toNearestInt());
        ratioSlider.setBounds(row1.removeFromLeft(cellWidth).toNearestInt());
        kneeLabel.setBounds(row1.removeFromLeft(cellWidth).removeFromTop(15).toNearestInt());
        kneeSlider.setBounds(row1.removeFromLeft(cellWidth).toNearestInt());
        
        // Row 2: Attack, Release, Band Freq, Band Q
        cellWidth = row2.getWidth() / 4.0f;
        attackLabel.setBounds(row2.removeFromLeft(cellWidth).removeFromTop(15).toNearestInt());
        attackSlider.setBounds(row2.removeFromLeft(cellWidth).toNearestInt());
        releaseLabel.setBounds(row2.removeFromLeft(cellWidth).removeFromTop(15).toNearestInt());
        releaseSlider.setBounds(row2.removeFromLeft(cellWidth).toNearestInt());
        bandFreqLabel.setBounds(row2.removeFromLeft(cellWidth).removeFromTop(15).toNearestInt());
        bandFreqSlider.setBounds(row2.removeFromLeft(cellWidth).toNearestInt());
        bandQLabel.setBounds(row2.removeFromLeft(cellWidth).removeFromTop(15).toNearestInt());
        bandQSlider.setBounds(row2.removeFromLeft(cellWidth).toNearestInt());
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
        g.setColour(juce::Colour(0x40000000)); // Lighter semi-transparent black overlay
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // Add "INACTIVE" text
        g.setColour(juce::Colour(0xFF999999)); // Lighter grey text
        g.setFont(juce::FontOptions(12.0f).withStyle("bold"));
        g.drawText("INACTIVE", bounds, juce::Justification::centred);
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
    
    // Header area for GR meter
    auto headerArea = bounds.removeFromTop(50.0f);
    
    // GR meter spanning full header width
    auto grArea = headerArea.reduced(10, 5);
    paintGrMeter(g, grArea);
}

void DuckingFloat::paintGrMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    const float cr = 4.0f;
    const float meterHeight = 20.0f;
    
    // Create meter area with proper height
    auto meterArea = bounds.removeFromBottom(meterHeight);
    
    // Track background (like other Field meters)
    g.setColour(th.meters.trackBase);
    g.fillRoundedRectangle(meterArea, cr);
    
    // Track with gradient (like other Field meters)
    {
        juce::Colour base = th.meters.trackBase;
        juce::Colour base2 = th.meters.trackActive;
        juce::ColourGradient grad(base, meterArea.getX(), meterArea.getY(), base2, meterArea.getX(), meterArea.getBottom(), false);
        juce::FillType ft(grad);
        g.setFillType(ft);
        g.fillRoundedRectangle(meterArea.reduced(1.0f), cr - 1.0f);
        g.setFillType(juce::FillType());
    }
    
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
        
        // Peak line (thicker bottom border like LR meters)
        juce::Colour peakColor;
        if (currentGrDb > -3.0f) {
            peakColor = th.meters.error;
        } else if (currentGrDb > -12.0f) {
            peakColor = lf->theme.accent;
        } else {
            peakColor = th.meters.trackBase.withAlpha(0.6f);
        }
        g.setColour(peakColor);
        g.fillRect(juce::Rectangle<float>(grBar.getX(), grBar.getBottom() - 2.0f, grBar.getWidth(), 2.0f));
        
        // GR value text overlay
        if (grBar.getWidth() > 20) {
            g.setColour(juce::Colours::white.withAlpha(0.9f));
            g.setFont(juce::FontOptions(10.0f).withStyle("bold"));
            g.drawText(juce::String(currentGrDb, 1) + " dB", grBar, juce::Justification::centred);
        }
    }
    
    // Meter label and units
    auto labelArea = bounds.removeFromTop(15.0f);
    g.setColour(th.text.withAlpha(0.8f));
    g.setFont(juce::FontOptions(11.0f).withStyle("bold"));
    g.drawText("GR", labelArea.removeFromLeft(20), juce::Justification::centred);
    
    // Units and scale
    g.setColour(th.text.withAlpha(0.6f));
    g.setFont(juce::FontOptions(9.0f));
    g.drawText("dB", labelArea.removeFromRight(15), juce::Justification::centred);
    
    // Scale markers
    auto scaleArea = labelArea.reduced(5, 0);
    g.setColour(th.text.withAlpha(0.4f));
    g.setFont(juce::FontOptions(8.0f));
    
    // Draw scale markers: -20, -15, -10, -5, 0
    for (int i = 0; i <= 4; ++i) {
        float db = -20.0f + (i * 5.0f);
        float x = scaleArea.getX() + (i * scaleArea.getWidth() / 4.0f);
        g.drawText(juce::String(db, 0), juce::Rectangle<float>(x - 10, scaleArea.getY(), 20, scaleArea.getHeight()), juce::Justification::centred);
    }
}
