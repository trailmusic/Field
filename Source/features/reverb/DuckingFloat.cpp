#include "DuckingFloat.h"
#include "ReverbParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"

DuckingFloat::DuckingFloat(juce::AudioProcessorValueTreeState& apvts)
    : expandButton("DUCKING", "Expand/Collapse ducking controls"),
      grLabel("grLabel", "GR"),
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
    
    // Set initial size
    setSize(300, COLLAPSED_HEIGHT);
    setExpanded(false);
}

DuckingFloat::~DuckingFloat()
{
    // APVTS attachments will auto-destruct
}

void DuckingFloat::setupComponents()
{
    // Expand button
    addAndMakeVisible(expandButton);
    expandButton.setButtonText("DUCKING");
    expandButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2D2D2D));
    expandButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFFFFFFF));
    expandButton.onClick = [this] { setExpanded(!expanded); };
    
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
    
    // Configure sliders
    depthSlider.setRange(0.0, 20.0, 0.1);
    depthSlider.setValue(6.0);
    depthSlider.setTextValueSuffix(" dB");
    
    thresholdSlider.setRange(-40.0, 0.0, 0.1);
    thresholdSlider.setValue(-12.0);
    thresholdSlider.setTextValueSuffix(" dB");
    
    ratioSlider.setRange(1.0, 10.0, 0.1);
    ratioSlider.setValue(3.0);
    ratioSlider.setTextValueSuffix(":1");
    
    kneeSlider.setRange(0.0, 10.0, 0.1);
    kneeSlider.setValue(2.0);
    kneeSlider.setTextValueSuffix(" dB");
    
    attackSlider.setRange(0.1, 100.0, 0.1);
    attackSlider.setValue(10.0);
    attackSlider.setTextValueSuffix(" ms");
    
    releaseSlider.setRange(10.0, 1000.0, 1.0);
    releaseSlider.setValue(100.0);
    releaseSlider.setTextValueSuffix(" ms");
    
    bandFreqSlider.setRange(80.0, 8000.0, 1.0);
    bandFreqSlider.setValue(1000.0);
    bandFreqSlider.setTextValueSuffix(" Hz");
    
    bandQSlider.setRange(0.1, 10.0, 0.1);
    bandQSlider.setValue(1.0);
    bandQSlider.setTextValueSuffix(" Q");
    
    // Configure labels
    depthLabel.setJustificationType(juce::Justification::centred);
    thresholdLabel.setJustificationType(juce::Justification::centred);
    ratioLabel.setJustificationType(juce::Justification::centred);
    kneeLabel.setJustificationType(juce::Justification::centred);
    attackLabel.setJustificationType(juce::Justification::centred);
    releaseLabel.setJustificationType(juce::Justification::centred);
    bandFreqLabel.setJustificationType(juce::Justification::centred);
    bandQLabel.setJustificationType(juce::Justification::centred);
    modeLabel.setJustificationType(juce::Justification::centred);
    detectorLabel.setJustificationType(juce::Justification::centred);
    
    // Set label colors
    auto labelColor = juce::Colour(0xFFCCCCCC);
    depthLabel.setColour(juce::Label::textColourId, labelColor);
    thresholdLabel.setColour(juce::Label::textColourId, labelColor);
    ratioLabel.setColour(juce::Label::textColourId, labelColor);
    kneeLabel.setColour(juce::Label::textColourId, labelColor);
    attackLabel.setColour(juce::Label::textColourId, labelColor);
    releaseLabel.setColour(juce::Label::textColourId, labelColor);
    bandFreqLabel.setColour(juce::Label::textColourId, labelColor);
    bandQLabel.setColour(juce::Label::textColourId, labelColor);
    modeLabel.setColour(juce::Label::textColourId, labelColor);
    detectorLabel.setColour(juce::Label::textColourId, labelColor);
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
    
    if (expanded)
    {
        // Expanded layout
        auto headerArea = bounds.removeFromTop(40.0f);
        
        // Header with expand button and GR meter
        expandButton.setBounds(headerArea.removeFromLeft(80).toNearestInt());
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
        expandButton.setBounds(bounds.removeFromLeft(80).toNearestInt());
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
}

void DuckingFloat::paintCollapsed(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Pill-style background
    g.setColour(juce::Colour(0xFF2D2D2D));
    g.fillRoundedRectangle(bounds, PILL_CORNER_RADIUS);
    
    // Border
    g.setColour(juce::Colour(0xFF555555));
    g.drawRoundedRectangle(bounds, PILL_CORNER_RADIUS, 1.0f);
    
    // GR meter
    paintGrMeter(g, bounds.removeFromRight(80).reduced(5));
}

void DuckingFloat::paintExpanded(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Main background
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.fillRoundedRectangle(bounds, 8.0f);
    
    // Border
    g.setColour(juce::Colour(0xFF444444));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
    
    // Header background
    auto headerArea = bounds.removeFromTop(40.0f);
    g.setColour(juce::Colour(0xFF2D2D2D));
    g.fillRoundedRectangle(headerArea, 8.0f);
    
    // GR meter in header
    auto grArea = headerArea.removeFromRight(80).reduced(5);
    paintGrMeter(g, grArea);
}

void DuckingFloat::paintGrMeter(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Background
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // GR level bar
    auto grLevel = juce::jmap(currentGrDb, -20.0f, 0.0f, 0.0f, 1.0f);
    auto grBar = bounds.removeFromLeft(bounds.getWidth() * grLevel);
    
    // Color based on GR level
    if (currentGrDb > -3.0f)
        g.setColour(juce::Colour(0xFFFF0000)); // Red for heavy GR
    else if (currentGrDb > -6.0f)
        g.setColour(juce::Colour(0xFFFF8800)); // Orange for moderate GR
    else
        g.setColour(juce::Colour(0xFF00FF00)); // Green for light GR
    
    g.fillRoundedRectangle(grBar, 4.0f);
    
    // Border
    g.setColour(juce::Colour(0xFF666666));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}
