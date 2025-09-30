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
    
    // Setup view mode buttons
    setupViewModeButtons();
    
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
    
    // Paint visualization based on current mode
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
    
    // Position view mode buttons in top-left
    auto buttonArea = bounds.removeFromTop(30).removeFromLeft(200);
    raysButton.setBounds(buttonArea.removeFromLeft(60));
    waterfallButton.setBounds(buttonArea.removeFromLeft(70));
    spectralButton.setBounds(buttonArea.removeFromLeft(70));
    
    // Vertical split: EQ panels on left (60%), visualizations on right (40%)
    auto leftArea = bounds.removeFromLeft(bounds.getWidth() * 0.6f);
    auto rightArea = bounds;
    
        // Left side: EQ panels
        if (reverbEQ && decayRateEQ)
        {
            auto toneArea = leftArea.removeFromTop(leftArea.getHeight() * 0.5f);
            auto decayArea = leftArea;
            
            reverbEQ->setBounds(toneArea);
            decayRateEQ->setBounds(decayArea);
        }
    
    // Right side: Visualization area (for future use)
    // The visualization content is drawn in paint() method
}

void ReverbGraphics::setupViewModeButtons()
{
    addAndMakeVisible(raysButton);
    addAndMakeVisible(waterfallButton);
    addAndMakeVisible(spectralButton);
    
    raysButton.setButtonText("Rays");
    waterfallButton.setButtonText("Waterfall");
    spectralButton.setButtonText("Spectral");
    
    raysButton.onClick = [this] { setViewMode(ViewMode::Rays); };
    waterfallButton.onClick = [this] { setViewMode(ViewMode::Waterfall); };
    spectralButton.onClick = [this] { setViewMode(ViewMode::Spectral); };
    
    // Set initial button states
    raysButton.setToggleState(true, juce::dontSendNotification);
    waterfallButton.setToggleState(false, juce::dontSendNotification);
    spectralButton.setToggleState(false, juce::dontSendNotification);
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
    auto bounds = getLocalBounds().toFloat().reduced(20.0f);
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
    auto bounds = getLocalBounds().toFloat().reduced(20.0f);
    
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
    auto bounds = getLocalBounds().toFloat().reduced(20.0f);
    
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


