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
    
    // GR meter
    addAndMakeVisible(grMeter);
    grMeter.setRange(-20.0, 0.0, 0.1);
    grMeter.setValue(0.0);
    grMeter.setSliderStyle(juce::Slider::LinearHorizontal);
    grMeter.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    grMeter.setColour(juce::Slider::trackColourId, juce::Colour(0xFF4A4A4A));
    grMeter.setColour(juce::Slider::thumbColourId, juce::Colour(0xFF00FF00));
    
    // GR label
    addAndMakeVisible(grLabel);
    grLabel.setText("GR", juce::dontSendNotification);
    grLabel.setJustificationType(juce::Justification::centred);
    grLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFFFFFFF));
    
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
    
    // Configure sliders with enhanced styling
    auto configureSlider = [](juce::Slider& slider, double min, double max, double step, double val, const juce::String& suffix) {
        slider.setRange(min, max, step);
        slider.setValue(val);
        slider.setTextValueSuffix(suffix);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF4A90E2));
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF333333));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xFFFFFFFF));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFCCCCCC));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF2D2D2D));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF555555));
    };
    
    configureSlider(depthSlider, 0.0, 20.0, 0.1, 6.0, " dB");
    configureSlider(thresholdSlider, -40.0, 0.0, 0.1, -12.0, " dB");
    configureSlider(ratioSlider, 1.0, 10.0, 0.1, 3.0, ":1");
    configureSlider(kneeSlider, 0.0, 10.0, 0.1, 2.0, " dB");
    configureSlider(attackSlider, 0.1, 100.0, 0.1, 10.0, " ms");
    configureSlider(releaseSlider, 10.0, 1000.0, 1.0, 100.0, " ms");
    configureSlider(bandFreqSlider, 80.0, 8000.0, 1.0, 1000.0, " Hz");
    configureSlider(bandQSlider, 0.1, 10.0, 0.1, 1.0, " Q");
    
    // Configure labels with enhanced styling
    auto configureLabel = [](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colour(0xFFE0E0E0));
        label.setFont(juce::FontOptions(11.0f).withStyle("bold"));
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
    
    // Enhanced combo box styling
    auto configureComboBox = [](juce::ComboBox& combo) {
        combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF2D2D2D));
        combo.setColour(juce::ComboBox::textColourId, juce::Colour(0xFFE0E0E0));
        combo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xFF4A90E2));
        combo.setColour(juce::ComboBox::buttonColourId, juce::Colour(0xFF4A4A4A));
        combo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF555555));
    };
    
    configureComboBox(modeSelector);
    configureComboBox(detectorSelector);
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
    grMeter.setValue(currentGrDb);
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

void DuckingFloat::resized()
{
    updateLayout();
}

void DuckingFloat::updateLayout()
{
    auto bounds = getLocalBounds().toFloat();
    
    // Set component enabled state based on active and greyed out state
    bool componentsEnabled = active && !greyedOut;
    
    grMeter.setEnabled(componentsEnabled);
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
    
    if (expanded)
    {
        // Expanded layout
        auto headerArea = bounds.removeFromTop(40.0f);
        
        // Header with GR meter
        auto grArea = headerArea.removeFromLeft(80);
        grLabel.setBounds(grArea.removeFromTop(15).toNearestInt());
        grMeter.setBounds(grArea.toNearestInt());
        
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
        // Collapsed layout - just header
        auto grArea = bounds.removeFromLeft(80);
        grLabel.setBounds(grArea.removeFromTop(15).toNearestInt());
        grMeter.setBounds(grArea.toNearestInt());
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
        g.setColour(juce::Colour(0x80000000)); // Semi-transparent black overlay
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // Add "INACTIVE" text
        g.setColour(juce::Colour(0xFF666666));
        g.setFont(juce::FontOptions(12.0f).withStyle("bold"));
        g.drawText("INACTIVE", bounds, juce::Justification::centred);
    }
}

void DuckingFloat::paintCollapsed(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // Pill-style background with elevation shadow
    const float cr = PILL_CORNER_RADIUS;
    
    // Elevation shadow
    if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.3f));
    else g.setColour(juce::Colour(0x40000000));
    g.fillRoundedRectangle(bounds.translated(1.0f, 1.0f), cr);
    
    // Main background
    g.setColour(th.meters.panelDark);
    g.fillRoundedRectangle(bounds, cr);
    
    // Border with subtle highlight
    g.setColour(th.sh.withAlpha(0.6f));
    g.drawRoundedRectangle(bounds, cr, 1.0f);
    
    // GR meter with enhanced styling
    paintGrMeter(g, bounds.removeFromRight(80).reduced(5));
}

void DuckingFloat::paintExpanded(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    const float cr = 8.0f;
    
    // Elevation shadow
    if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.25f));
    else g.setColour(juce::Colour(0x40000000));
    g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), cr);
    
    // Main background
    g.setColour(th.meters.panelDark);
    g.fillRoundedRectangle(bounds, cr);
    
    // Border
    g.setColour(th.sh);
    g.drawRoundedRectangle(bounds, cr, 1.0f);
    
    // Header background with subtle gradient
    auto headerArea = bounds.removeFromTop(40.0f);
    juce::ColourGradient headerGradient(th.meters.panelDark, 0, 0, 
                                       th.meters.panelDark.darker(0.1f), 0, headerArea.getHeight(), false);
    g.setGradientFill(headerGradient);
    g.fillRoundedRectangle(headerArea, cr);
    
    // Header border
    g.setColour(th.sh.withAlpha(0.3f));
    g.drawRoundedRectangle(headerArea, cr, 0.5f);
    
    // GR meter in header with enhanced styling
    auto grArea = headerArea.removeFromRight(80).reduced(5);
    paintGrMeter(g, grArea);
}

void DuckingFloat::paintGrMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    const float cr = 4.0f;
    
    // Background with subtle gradient
    juce::ColourGradient bgGradient(th.meters.panelDark.darker(0.2f), 0, 0,
                                   th.meters.panelDark.darker(0.4f), 0, bounds.getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds, cr);
    
    // GR level bar with smooth gradient
    auto grLevel = juce::jmap(currentGrDb, -20.0f, 0.0f, 0.0f, 1.0f);
    auto grBar = bounds.removeFromLeft(bounds.getWidth() * grLevel);
    
    // Enhanced color scheme based on GR level
    juce::Colour startColor, endColor;
    if (currentGrDb > -3.0f) {
        // Red for heavy GR
        startColor = juce::Colour(0xFFFF4444);
        endColor = juce::Colour(0xFFCC0000);
    } else if (currentGrDb > -6.0f) {
        // Orange for moderate GR
        startColor = juce::Colour(0xFFFFAA44);
        endColor = juce::Colour(0xFFCC6600);
    } else {
        // Green for light GR
        startColor = juce::Colour(0xFF44FF44);
        endColor = juce::Colour(0xFF00CC00);
    }
    
    juce::ColourGradient grGradient(startColor, 0, 0, endColor, 0, grBar.getHeight(), false);
    g.setGradientFill(grGradient);
    g.fillRoundedRectangle(grBar, cr);
    
    // Subtle inner highlight
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(grBar.reduced(0.5f), cr - 0.5f, 0.5f);
    
    // Border with theme integration
    g.setColour(th.sh.withAlpha(0.8f));
    g.drawRoundedRectangle(bounds, cr, 1.0f);
    
    // GR value text overlay
    if (grBar.getWidth() > 20) {
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.setFont(10.0f);
        g.drawText(juce::String(currentGrDb, 1) + " dB", grBar, juce::Justification::centred);
    }
}
