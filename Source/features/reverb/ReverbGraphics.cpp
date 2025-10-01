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
      spectralButton("Spectral"),
      toneEqIndicator(4),
      decayRateEqIndicator(3)
{
    // Create ducking float
    duckingFloat = std::make_unique<DuckingFloat>(state);
    addAndMakeVisible(*duckingFloat);
    
    // Initially show ducking float but greyed out (will be active when DUCK toggle is on)
    duckingFloat->setVisible(true);
    duckingFloat->setActive(false);
    duckingFloat->setGreyedOut(true);
    
    // Add band indicators to the component
    addAndMakeVisible(toneEqIndicator);
    addAndMakeVisible(decayRateEqIndicator);
    
    // Discover parameter IDs for band detection
    // Tone EQ: tb_active_0, tb_active_1, tb_active_2, tb_active_3
    // Decay-Rate EQ: db_active_0, db_active_1, db_active_2
    toneEnabledIds = BandIdFinder::findEnabledIds(state, "tb_active_", "");
    decayEnabledIds = BandIdFinder::findEnabledIds(state, "db_active_", "");
    
    // Debug: Log discovered parameters
    DBG("--- Discovered Tone EQ Parameters: " << toneEnabledIds.size() << " ---");
    for (auto& id : toneEnabledIds)
        DBG("Tone: " << id);
    
    DBG("--- Discovered Decay-Rate EQ Parameters: " << decayEnabledIds.size() << " ---");
    for (auto& id : decayEnabledIds)
        DBG("Decay: " << id);
    
    // Set up band counters for reliable detection
    if (!toneEnabledIds.isEmpty())
    {
        toneCounter.reset(new BandCounter(state, toneEnabledIds, 
            [this](int n) { 
                toneEqIndicator.setActiveBands(n); 
                repaint(); 
            }));
    }
    else
    {
        DBG("No Tone EQ parameters found - using fallback approach");
        // Fallback: Set initial values to 0
        toneEqIndicator.setActiveBands(0);
    }
    
    // Initialize indicators with actual band counts
    updateBandIndicatorsManually();
    
    if (!decayEnabledIds.isEmpty())
    {
        decayCounter.reset(new BandCounter(state, decayEnabledIds, 
            [this](int n) { 
                decayRateEqIndicator.setActiveBands(n); 
                repaint(); 
            }));
    }
    else
    {
        DBG("No Decay-Rate EQ parameters found - using fallback approach");
        // Fallback: Set initial values to 0
        decayRateEqIndicator.setActiveBands(0);
    }
    
        // Create EQ panels
        reverbEQ = std::make_unique<ReverbToneEQ>(proc);
        addAndMakeVisible(*reverbEQ);
        
        decayRateEQ = std::make_unique<DecayRateEQ>(proc);
        addAndMakeVisible(*decayRateEQ);
    
    // Setup visualization control panel
    setupVisualizationControlPanel();
    
    // Setup EQ labels
    setupEQLabels();
    
    // Set initial label colors
    updateLabelColors();
    
    // Start animation timer
    startTimerHz(30);
}

void ReverbGraphics::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // Anti-aliasing fix: Fill entire area first, then rounded rectangle
    const float cr = 8.0f;
    
    // Fill entire rectangular area to prevent white corners
    g.setColour(th.meters.panelDark);
    g.fillRect(r);
    
    // Then draw rounded rectangle on top
    g.fillRoundedRectangle(r, cr);
    
    // Strong edge shading for depth
    g.setColour(th.sh.withAlpha(0.6f));
    g.drawRoundedRectangle(r.reduced(0.5f), cr - 0.5f, 2.0f);
    
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
    
    // Create a temporary graphics context with the correct bounds
    auto tempBounds = juce::Rectangle<int>(0, 0, visualizationArea.getWidth(), visualizationArea.getHeight());
    
    switch (currentViewMode)
    {
        case ViewMode::Rays:
            paintRaysInBounds(g, tempBounds.toFloat());
            break;
        case ViewMode::Waterfall:
            paintWaterfallInBounds(g, tempBounds.toFloat());
            break;
        case ViewMode::Spectral:
            paintSpectralInBounds(g, tempBounds.toFloat());
            break;
    }
    
    g.restoreState();
    
    // Paint GR overlay
    paintGrOverlay(g);
}

void ReverbGraphics::resized()
{
    auto bounds = getLocalBounds();
    
    
    // Horizontal split: EQ panels on left (60%), right side split into ducking (top) and visualization (bottom)
    auto leftArea = bounds.removeFromLeft(bounds.getWidth() * 0.6f);
    auto rightArea = bounds;
    
    // Add gap between EQs and right side
    leftArea.removeFromRight(15); // 15px gap
    rightArea.removeFromLeft(15);  // 15px gap
    
    // Split right side: top half for ducking, bottom half for visualization
    auto duckingArea = rightArea.removeFromTop(rightArea.getHeight() * 0.5f);
    auto visualizationArea = rightArea;
    
    // Position ducking label and module
    auto duckingLabelArea = duckingArea.removeFromTop(25);
    duckingLabel.setBounds(duckingLabelArea);
    
    if (duckingFloat)
    {
        duckingFloat->setBounds(duckingArea);
    }
    
    // Position visualization label and panel in bottom half
    auto visualizationLabelArea = visualizationArea.removeFromTop(25);
    visualizationLabel.setBounds(visualizationLabelArea);
    visualizationControlPanel.setBounds(visualizationArea);
    
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
    
        // Left side: EQ panels with labels (50/50 split like right side)
        if (reverbEQ && decayRateEQ)
        {
            // Split left area into two equal halves
            auto topLeftArea = leftArea.removeFromTop(leftArea.getHeight() * 0.5f);
            auto bottomLeftArea = leftArea;
            
            // Top half: Tone EQ
            auto toneLabelArea = topLeftArea.removeFromTop(25);
            
            // Position band indicator and label on the same row
            auto indicatorArea = toneLabelArea.removeFromLeft(60).translated(12, 10); // 12px left padding, 10px down
            toneEqIndicator.setBounds(indicatorArea);
            toneEqLabel.setBounds(toneLabelArea);
            
            reverbEQ->setBounds(topLeftArea);
            
            // Bottom half: Decay Rate EQ
            auto decayLabelArea = bottomLeftArea.removeFromTop(25);
            
            // Position band indicator and label on the same row
            auto decayIndicatorArea = decayLabelArea.removeFromLeft(45).translated(12, 10); // 12px left padding, 10px down
            decayRateEqIndicator.setBounds(decayIndicatorArea);
            decayRateEqLabel.setBounds(decayLabelArea);
            
            decayRateEQ->setBounds(bottomLeftArea);
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
    addAndMakeVisible(duckingLabel);
    addAndMakeVisible(visualizationLabel);
    
    // Configure tone EQ label
    toneEqLabel.setText("TONE EQ", juce::dontSendNotification);
    toneEqLabel.setJustificationType(juce::Justification::centred);
    toneEqLabel.setFont(juce::FontOptions(12.0f).withStyle("bold"));
    
    // Configure decay rate EQ label
    decayRateEqLabel.setText("DECAY-RATE EQ", juce::dontSendNotification);
    decayRateEqLabel.setJustificationType(juce::Justification::centred);
    decayRateEqLabel.setFont(juce::FontOptions(12.0f).withStyle("bold"));
    
    // Configure ducking label
    duckingLabel.setText("DUCKING", juce::dontSendNotification);
    duckingLabel.setJustificationType(juce::Justification::centred);
    duckingLabel.setFont(juce::FontOptions(12.0f).withStyle("bold"));
    
    // Configure visualization label
    visualizationLabel.setText("VISUALIZATION", juce::dontSendNotification);
    visualizationLabel.setJustificationType(juce::Justification::centred);
    visualizationLabel.setFont(juce::FontOptions(12.0f).withStyle("bold"));
    
    // Remove any hardcoded colors - let LNF handle them
    toneEqLabel.removeColour(juce::Label::textColourId);
    decayRateEqLabel.removeColour(juce::Label::textColourId);
    duckingLabel.removeColour(juce::Label::textColourId);
    visualizationLabel.removeColour(juce::Label::textColourId);
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

void ReverbGraphics::lookAndFeelChanged()
{
    // Update label colors to match current theme
    updateLabelColors();
    
    // Force repaint of EQ components to update their colors
    if (reverbEQ) {
        reverbEQ->lookAndFeelChanged();
        reverbEQ->repaint();
    }
    if (decayRateEQ) {
        decayRateEQ->lookAndFeelChanged();
        decayRateEQ->repaint();
    }
    
    // Force repaint of ducking module to update its colors
    if (duckingFloat) {
        duckingFloat->lookAndFeelChanged();
        duckingFloat->repaint();
    }
    
    repaint();
}

void ReverbGraphics::updateLabelColors()
{
    // Get current theme colors from LookAndFeel
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    if (lf) {
        auto accentColor = lf->findColour(FieldLNF::eqLabelTextColourId);
        
        // Update all label colors
        toneEqLabel.setColour(juce::Label::textColourId, accentColor);
        decayRateEqLabel.setColour(juce::Label::textColourId, accentColor);
        duckingLabel.setColour(juce::Label::textColourId, accentColor);
        visualizationLabel.setColour(juce::Label::textColourId, accentColor);
    }
}

void ReverbGraphics::paintRays(juce::Graphics& g)
{
    paintRaysInBounds(g, getLocalBounds().toFloat());
}

void ReverbGraphics::paintRaysInBounds(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto center = bounds.getCentre();
    
    // Get current parameters for ray properties
    auto erLevel = getErRms ? getErRms() : 0.0f;
    auto tailLevel = getTailRms ? getTailRms() : 0.0f;
    auto width = getWidthNow ? getWidthNow() : 0.5f;
    
    // Use default levels if no audio signal
    if (erLevel == 0.0f && tailLevel == 0.0f)
    {
        erLevel = 0.4f;  // Default ER level for visualization
        tailLevel = 0.3f; // Default tail level for visualization
    }
    
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
    paintWaterfallInBounds(g, getLocalBounds().toFloat());
}

void ReverbGraphics::paintWaterfallInBounds(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Get current levels
    auto erLevel = getErRms ? getErRms() : 0.0f;
    auto tailLevel = getTailRms ? getTailRms() : 0.0f;
    
    // Use default levels if no audio signal
    if (erLevel == 0.0f && tailLevel == 0.0f)
    {
        erLevel = 0.3f;  // Default ER level for visualization
        tailLevel = 0.2f; // Default tail level for visualization
    }
    
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
    paintSpectralInBounds(g, getLocalBounds().toFloat());
}

void ReverbGraphics::paintSpectralInBounds(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Get current levels
    auto erLevel = getErRms ? getErRms() : 0.0f;
    auto tailLevel = getTailRms ? getTailRms() : 0.0f;
    
    // Use default levels if no audio signal
    if (erLevel == 0.0f && tailLevel == 0.0f)
    {
        erLevel = 0.4f;  // Default ER level for visualization
        tailLevel = 0.3f; // Default tail level for visualization
    }
    
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
    
    // Band indicators now update automatically via BandCounter listeners
    // Fallback: Manual update if automatic detection failed
    if (toneEnabledIds.isEmpty() || decayEnabledIds.isEmpty())
    {
        updateBandIndicatorsManually();
    }

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
    
    // Always show the ducking module, but control its active/greyed out state
    duckingFloat->setVisible(true);
    duckingFloat->setActive(duckEnabled);
    duckingFloat->setGreyedOut(!duckEnabled);
}

// Manual band indicator update method
void ReverbGraphics::updateBandIndicatorsManually()
{
    // Manual fallback: count active bands by checking parameters directly
    int activeToneBands = 0;
    int activeDecayBands = 0;
    
    // Check Tone EQ bands (4 bands)
    for (int i = 0; i < 4; ++i)
    {
        auto paramName = "tb_active_" + juce::String(i);
        auto activeParam = state.getRawParameterValue(paramName);
        if (activeParam && activeParam->load() > 0.5f)
        {
            activeToneBands++;
        }
    }
    
    // Check Decay-Rate EQ bands (3 bands)
    for (int i = 0; i < 3; ++i)
    {
        auto paramName = "db_active_" + juce::String(i);
        auto activeParam = state.getRawParameterValue(paramName);
        if (activeParam && activeParam->load() > 0.5f)
        {
            activeDecayBands++;
        }
    }
    
    toneEqIndicator.setActiveBands(activeToneBands);
    decayRateEqIndicator.setActiveBands(activeDecayBands);
    repaint();
}

// BandIndicator implementation
ReverbGraphics::BandIndicator::BandIndicator(int maxBands) : maxBands(maxBands), activeBands(0)
{
    setSize((int)(maxBands * circleSpacing), (int)circleSize);
}

void ReverbGraphics::BandIndicator::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    g.setColour(th.accent);
    
    for (int i = 0; i < maxBands; ++i)
    {
        auto circleBounds = juce::Rectangle<float>(i * circleSpacing, 0, circleSize, circleSize);
        
        if (i < activeBands)
        {
            // Filled circle for active bands
            g.fillEllipse(circleBounds);
        }
        else
        {
            // Empty circle with border for inactive bands
            g.drawEllipse(circleBounds, 1.5f);
        }
    }
    
    // Debug background removed - indicators should now be working
}

void ReverbGraphics::BandIndicator::setActiveBands(int count)
{
    activeBands = juce::jlimit(0, maxBands, count);
    repaint();
}

void ReverbGraphics::BandIndicator::setMaxBands(int max)
{
    maxBands = max;
    setSize((int)(maxBands * circleSpacing), (int)circleSize);
    repaint();
}

// VisualizationControlPanel paint implementation
void ReverbGraphics::VisualizationControlPanel::paint(juce::Graphics& g)
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
    
    // Title removed - not needed
}


