#include "ReverbGraphics.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/PluginProcessor.h"
#include "ReverbParamIDs.h"

ReverbGraphics::ReverbGraphics (MyPluginAudioProcessor& p,
                          juce::AudioProcessorValueTreeState& s,
                          std::function<float()> getEr,
                          std::function<float()> getTail,
                          std::function<float()> getDuckDb,
                          std::function<float()> getWidthNow)
    : proc(p),
      state (s),
      getErRms(getEr),
      getTailRms(getTail),
      getDuckGrDb(getDuckDb),
      getWidthNow(getWidthNow),
      raysButton("Rays"),
      waterfallButton("Waterfall"),
      spectralButton("Spectral")
{
    // Create ducking float
    duckingFloat = std::make_unique<DuckingFloat>(state);
    addAndMakeVisible(*duckingFloat);
    
    // Initially hide ducking float (will be shown when DUCK toggle is on)
    duckingFloat->setVisible(false);
    
        // Create EQ panels
        reverbEQ = std::make_unique<ReverbToneEQ>(proc, &getLookAndFeel());
        addAndMakeVisible(*reverbEQ);
        
        decayRateEQ = std::make_unique<DecayRateEQ>(proc, &getLookAndFeel());
        addAndMakeVisible(*decayRateEQ);
    
    // Setup visualization control panel
    setupVisualizationControlPanel();
    
    // Setup EQ labels
    setupEQLabels();
    
    // Start animation timer
    startTimerHz(30);
}

void ReverbGraphics::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // Background with elevation shadow
    const float cr = 8.0f;
    
    // Elevation shadow
    if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.25f));
    else g.setColour(juce::Colour(0x40000000));
    g.fillRoundedRectangle(r.translated(1.5f, 1.5f), cr);
    
    // Main background
    g.setColour(th.meters.panelDark);
    g.fillRoundedRectangle(r, cr);
    
    // Border
    g.setColour(th.sh);
    g.drawRoundedRectangle(r, cr, 1.0f);
    
    // Content area
    auto contentR = r.reduced(10.0f);
    
    // Paint visualization in the visualization control panel
    auto panelBounds = visualizationControlPanel.getBounds();
    auto visualizationArea = panelBounds.reduced(15);
    visualizationArea.removeFromTop(60); // Space for title and buttons
    
    // Paint visualization based on current mode in the panel area
    g.saveState();
    g.setOrigin(visualizationArea.getX(), visualizationArea.getY());
    g.reduceClipRegion(0, 0, visualizationArea.getWidth(), visualizationArea.getHeight());
    
    switch (currentViewMode)
    {
        case ViewMode::Rays:
            paintRays(g);
            break;
        case ViewMode::Waterfall:
            paintWaterfall(g);
            break;
        case ViewMode::Spectral:
            paintSpectral(g);
            break;
    }
    
    g.restoreState();
    
    // Paint GR overlay
    paintGrOverlay(g);
}

void ReverbGraphics::resized()
{
    auto bounds = getLocalBounds();
    
    // Position ducking float in top-right corner
    if (duckingFloat)
    {
        auto duckingBounds = bounds.removeFromTop(50).removeFromRight(300);
        duckingFloat->setBounds(duckingBounds);
    }
    
    // Horizontal split: EQ panels on left (60%), visualization controls on right (40%)
    auto leftArea = bounds.removeFromLeft(bounds.getWidth() * 0.6f);
    auto rightArea = bounds;
    
    // Add gap between EQs and visuals to prevent accidental clicks
    leftArea.removeFromRight(15); // 15px gap
    rightArea.removeFromLeft(15);  // 15px gap
    
    // Position visualization control panel on the right
    visualizationControlPanel.setBounds(rightArea);
    
    // Layout buttons in horizontal row centered with title
    auto panelBounds = visualizationControlPanel.getBounds();
    auto titleArea = panelBounds.removeFromTop(30); // Space for title
    auto buttonArea = panelBounds.reduced(15);
    
    auto buttonHeight = 30;
    auto buttonWidth = 80;
    auto buttonSpacing = 8;
    auto totalButtonWidth = (buttonWidth * 3) + (buttonSpacing * 2);
    
    // Center buttons in the available space
    auto buttonRow = buttonArea.removeFromTop(buttonHeight);
    auto buttonStartX = buttonRow.getX() + (buttonRow.getWidth() - totalButtonWidth) / 2;
    
    raysButton.setBounds(buttonStartX, buttonRow.getY(), buttonWidth, buttonHeight);
    waterfallButton.setBounds(buttonStartX + buttonWidth + buttonSpacing, buttonRow.getY(), buttonWidth, buttonHeight);
    spectralButton.setBounds(buttonStartX + (buttonWidth + buttonSpacing) * 2, buttonRow.getY(), buttonWidth, buttonHeight);
    
        // Left side: EQ panels with labels
        if (reverbEQ && decayRateEQ)
        {
            // Tone EQ section
            auto toneLabelArea = leftArea.removeFromTop(25);
            toneEqLabel.setBounds(toneLabelArea);
            
            auto toneArea = leftArea.removeFromTop(leftArea.getHeight() * 0.5f);
            reverbEQ->setBounds(toneArea);
            
            // Decay Rate EQ section
            auto decayLabelArea = leftArea.removeFromTop(25);
            decayRateEqLabel.setBounds(decayLabelArea);
            
            auto decayArea = leftArea;
            decayRateEQ->setBounds(decayArea);
        }
    
    // Right side: Visualization area (for future use)
    // The visualization content is drawn in paint() method
}

void ReverbGraphics::setupVisualizationControlPanel()
{
    // Add the visualization control panel as a child component
    addAndMakeVisible(visualizationControlPanel);
    
    // Add buttons to the main component (not the control panel)
    addAndMakeVisible(raysButton);
    addAndMakeVisible(waterfallButton);
    addAndMakeVisible(spectralButton);
    
    // Set up custom paint for the control panel
    visualizationControlPanel.setOpaque(true);
    
    // Configure button text and styling
    raysButton.setButtonText("Rays");
    waterfallButton.setButtonText("Waterfall");
    spectralButton.setButtonText("Spectral");
    
    // Set up button callbacks
    raysButton.onClick = [this] { setViewMode(ViewMode::Rays); };
    waterfallButton.onClick = [this] { setViewMode(ViewMode::Waterfall); };
    spectralButton.onClick = [this] { setViewMode(ViewMode::Spectral); };
    
    // Set initial button states
    raysButton.setToggleState(true, juce::dontSendNotification);
    waterfallButton.setToggleState(false, juce::dontSendNotification);
    spectralButton.setToggleState(false, juce::dontSendNotification);
    
    // Style the buttons with Field theme
    auto styleButton = [](juce::TextButton& button) {
        button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2D2D2D));
        button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF4A90E2));
        button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFFFFFFF));
        button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFCCCCCC));
    };
    
    styleButton(raysButton);
    styleButton(waterfallButton);
    styleButton(spectralButton);
}

void ReverbGraphics::setupEQLabels()
{
    // Add EQ labels as visible components
    addAndMakeVisible(toneEqLabel);
    addAndMakeVisible(decayRateEqLabel);
    
    // Configure tone EQ label
    toneEqLabel.setText("TONE EQ", juce::dontSendNotification);
    toneEqLabel.setJustificationType(juce::Justification::centred);
    toneEqLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF4A90E2));
    toneEqLabel.setFont(juce::FontOptions(12.0f).withStyle("bold"));
    
    // Configure decay rate EQ label
    decayRateEqLabel.setText("DECAY-RATE EQ", juce::dontSendNotification);
    decayRateEqLabel.setJustificationType(juce::Justification::centred);
    decayRateEqLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF4A90E2));
    decayRateEqLabel.setFont(juce::FontOptions(12.0f).withStyle("bold"));
}

void ReverbGraphics::setViewMode(ViewMode mode)
{
    currentViewMode = mode;
    
    // Update button states
    raysButton.setToggleState(mode == ViewMode::Rays, juce::dontSendNotification);
    waterfallButton.setToggleState(mode == ViewMode::Waterfall, juce::dontSendNotification);
    spectralButton.setToggleState(mode == ViewMode::Spectral, juce::dontSendNotification);
    
    repaint();
}

void ReverbGraphics::paintRays(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto center = bounds.getCentre();
    
    // Get current parameters for ray properties
    auto erLevel = getErRms ? getErRms() : 0.0f;
    auto tailLevel = getTailRms ? getTailRms() : 0.0f;
    auto width = getWidthNow ? getWidthNow() : 0.5f;
    
    // Number of rays based on density
    int numRays = 20 + (int)(erLevel * 30);
    numRays = juce::jlimit(10, 50, numRays);
    
    // Ray properties
    float rayLength = bounds.getWidth() * 0.3f;
    float rayThickness = 2.0f;
    
    // Color based on tail level
    auto rayColor = juce::Colour::fromHSV(0.6f, 0.8f, 0.3f + tailLevel * 0.7f, 0.8f);
    g.setColour(rayColor);
    
    // Draw rays with random jitter
    juce::Random random;
    for (int i = 0; i < numRays; ++i)
    {
        // Random angle with some clustering
        float angle = (float)i / numRays * juce::MathConstants<float>::twoPi;
        angle += random.nextFloat() * 0.2f - 0.1f; // Jitter
        
        // Ray start and end points
        auto start = center;
        auto end = center + juce::Point<float>(rayLength * cosf(angle), rayLength * sinf(angle));
        
        // Vary thickness based on density
        float currentThickness = rayThickness * (0.5f + erLevel * 0.5f);
        
        g.drawLine(start.x, start.y, end.x, end.y, currentThickness);
    }
}

void ReverbGraphics::paintWaterfall(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Get current levels
    auto erLevel = getErRms ? getErRms() : 0.0f;
    auto tailLevel = getTailRms ? getTailRms() : 0.0f;
    
    // Create gradient bands
    juce::ColourGradient gradient;
    gradient.point1 = bounds.getTopLeft();
    gradient.point2 = bounds.getBottomLeft();
    
    // Color stops based on levels
    auto baseColor = juce::Colour::fromHSV(0.3f, 0.6f, 0.2f, 0.8f);
    auto highlightColor = juce::Colour::fromHSV(0.3f, 0.8f, 0.6f, 0.9f);
    
    gradient.addColour(0.0f, baseColor);
    gradient.addColour(erLevel, highlightColor);
    gradient.addColour(1.0f, baseColor);
    
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    // Add texture overlay
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    for (int i = 0; i < 20; ++i)
    {
        float y = bounds.getY() + (float)i / 20.0f * bounds.getHeight();
        g.drawHorizontalLine((int)y, bounds.getX(), bounds.getRight());
    }
}

void ReverbGraphics::paintSpectral(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Get current levels
    auto erLevel = getErRms ? getErRms() : 0.0f;
    auto tailLevel = getTailRms ? getTailRms() : 0.0f;
    
    // Draw frequency response curves
    juce::Path erPath, tailPath;
    
    // ER curve (higher frequencies)
    erPath.startNewSubPath(bounds.getX(), bounds.getBottom());
    for (int i = 0; i < bounds.getWidth(); i += 2)
    {
        float x = bounds.getX() + i;
        float freq = juce::jmap((float)i, 0.0f, bounds.getWidth(), 20.0f, 20000.0f);
        float response = erLevel * (1.0f - (freq - 1000.0f) / 19000.0f);
        float y = bounds.getBottom() - response * bounds.getHeight() * 0.5f;
        erPath.lineTo(x, y);
    }
    erPath.lineTo(bounds.getRight(), bounds.getBottom());
    erPath.closeSubPath();
    
    // Tail curve (lower frequencies)
    tailPath.startNewSubPath(bounds.getX(), bounds.getBottom());
    for (int i = 0; i < bounds.getWidth(); i += 2)
    {
        float x = bounds.getX() + i;
        float freq = juce::jmap((float)i, 0.0f, bounds.getWidth(), 20.0f, 20000.0f);
        float response = tailLevel * (freq / 1000.0f);
        float y = bounds.getBottom() - response * bounds.getHeight() * 0.3f;
        tailPath.lineTo(x, y);
    }
    tailPath.lineTo(bounds.getRight(), bounds.getBottom());
    tailPath.closeSubPath();
    
    // Fill paths
    g.setColour(juce::Colour::fromHSV(0.6f, 0.7f, 0.4f, 0.6f));
    g.fillPath(erPath);
    
    g.setColour(juce::Colour::fromHSV(0.1f, 0.7f, 0.4f, 0.6f));
    g.fillPath(tailPath);
    
    // Draw outlines
    g.setColour(juce::Colour::fromHSV(0.6f, 0.8f, 0.8f, 0.9f));
    g.strokePath(erPath, juce::PathStrokeType(2.0f));
    
    g.setColour(juce::Colour::fromHSV(0.1f, 0.8f, 0.8f, 0.9f));
    g.strokePath(tailPath, juce::PathStrokeType(2.0f));
}

void ReverbGraphics::paintGrOverlay(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Get current GR level
    auto grDb = getDuckGrDb ? getDuckGrDb() : 0.0f;
    
    if (grDb < -0.1f) // Only show if there's actual gain reduction
    {
        // Semi-transparent overlay
        g.setColour(juce::Colours::red.withAlpha(0.3f));
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // GR text
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText(juce::String(grDb, 1) + " dB GR", bounds, juce::Justification::centred);
    }
}

void ReverbGraphics::timerCallback()
{
    // Update animation time
    animationTime += ANIMATION_SPEED;
    if (animationTime > juce::MathConstants<float>::twoPi)
        animationTime -= juce::MathConstants<float>::twoPi;
    
    // Update ducking module visibility based on DUCK toggle
    updateDuckingModuleVisibility();
    
    // Update ducking float GR meter
    if (duckingFloat && getDuckGrDb)
    {
        auto grDb = getDuckGrDb();
        duckingFloat->updateGrMeter(grDb);
    }
    
    // Repaint for animation
    repaint();
}

void ReverbGraphics::setSampleRate(double sr)
{
    if (reverbEQ) reverbEQ->setSampleRate(sr);
    if (decayRateEQ) decayRateEQ->setSampleRate(sr);
}

void ReverbGraphics::pause()
{
    if (reverbEQ) reverbEQ->pause();
    if (decayRateEQ) decayRateEQ->pause();
}

void ReverbGraphics::resume()
{
    if (reverbEQ) reverbEQ->resume();
    if (decayRateEQ) decayRateEQ->resume();
}

void ReverbGraphics::pushBlock(const float* L, const float* R, int n)
{
    if (reverbEQ) reverbEQ->pushBlock(L, R, n);
    if (decayRateEQ) decayRateEQ->pushBlock(L, R, n);
}

void ReverbGraphics::pushBlockPre(const float* L, const float* R, int n)
{
    if (reverbEQ) reverbEQ->pushBlockPre(L, R, n);
    if (decayRateEQ) decayRateEQ->pushBlockPre(L, R, n);
}

void ReverbGraphics::updateDuckingModuleVisibility()
{
    if (!duckingFloat) return;
    
    // Check the DUCK toggle state from the APVTS
    auto duckOnParam = state.getRawParameterValue(ReverbParamIDs::duckOn);
    bool duckEnabled = duckOnParam ? (*duckOnParam > 0.5f) : false;
    
    // Show/hide ducking module based on DUCK toggle
    bool shouldBeVisible = duckEnabled;
    if (duckingFloat->isVisible() != shouldBeVisible)
    {
        duckingFloat->setVisible(shouldBeVisible);
        
        // If showing, bring to front
        if (shouldBeVisible)
        {
            duckingFloat->toFront(false);
        }
    }
}

// VisualizationControlPanel paint implementation
void ReverbGraphics::VisualizationControlPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    const float cr = 8.0f;
    
    // Recessed effect - inner shadow
    if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.4f));
    else g.setColour(juce::Colour(0x60000000));
    g.fillRoundedRectangle(bounds, cr);
    
    // Main background with recessed gradient (darker on top, lighter on bottom)
    juce::ColourGradient bgGradient(th.meters.panelDark.darker(0.3f), 0, 0,
                                   th.meters.panelDark.darker(0.1f), 0, bounds.getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillRoundedRectangle(bounds, cr);
    
    // Inner border for recessed effect
    g.setColour(th.sh.withAlpha(0.8f));
    g.drawRoundedRectangle(bounds, cr, 1.0f);
    
    // Inner highlight for recessed effect
    g.setColour(th.sh.withAlpha(0.2f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), cr - 1.0f, 1.0f);
    
    // Thin border around visualization container for better visibility
    g.setColour(th.text.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds, cr, 1.0f);
    
    // Title
    g.setColour(th.text);
    g.setFont(juce::FontOptions(14.0f).withStyle("bold"));
    g.drawText("Visualization", bounds.removeFromTop(25).reduced(10, 0), juce::Justification::centredLeft);
}


