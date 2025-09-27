#include "BandVisualPane.h"

BandVisualPane::BandVisualPane()
{
    setOpaque (false);
    startTimerHz (30); // Fixed 30fps for Band visuals
    
    // Initialize audio data buffers
    fifoPre.resize (1024);
    fifoPost.resize (1024);
    writePre = 0;
    writePost = 0;
    havePre = false;
    havePost = false;
    rmsPost = 0.0f;
    rmsPre = 0.0f;
    autoGain = true;
    
    // Initialize engine
    StereoFieldEngine::Settings s;
    s.fftOrder = 11;
    s.bandsPerDecade = 12;
    s.historyWidthPx = 64;
    engine.prepare (sampleRate, s);
    enginePrepared = true;
}

BandVisualPane::~BandVisualPane()
{
    stopTimer();
}

void BandVisualPane::setSampleRate (double sr)
{
    sampleRate = (sr > 0 ? sr : 48000.0);
    if (enginePrepared)
    {
        StereoFieldEngine::Settings s;
        s.fftOrder = 11;
        s.bandsPerDecade = 12;
        s.historyWidthPx = 64;
        engine.prepare (sampleRate, s);
    }
}

void BandVisualPane::setWidths (float lo, float mid, float hi)
{
    widthLo = juce::jlimit (0.0f, 2.0f, lo);
    widthMid= juce::jlimit (0.0f, 2.0f, mid);
    widthHi = juce::jlimit (0.0f, 2.0f, hi);
    repaint();
}

void BandVisualPane::setCrossovers (float loHz, float hiHz)
{
    xoverLoHz = juce::jlimit (40.0f, 400.0f, loHz);
    xoverHiHz = juce::jlimit (800.0f, 6000.0f, hiHz);
    if (xoverHiHz <= xoverLoHz) xoverHiHz = juce::jlimit (xoverLoHz + 10.0f, 6000.0f, xoverHiHz);
    repaint();
}

void BandVisualPane::setShuffler (float loPct, float hiPct, float xHz)
{
    shufLoPct = loPct; shufHiPct = hiPct; shufXHz = juce::jlimit (150.0f, 2000.0f, xHz);
    repaint();
}

void BandVisualPane::pushBlock (const float* L, const float* R, int n, bool isPre)
{
    if (n <= 0) return;
    const juce::SpinLock::ScopedLockType sl (dataLock);
    if (isPre)
    {
        for (int i = 0; i < n; ++i)
        {
            fifoPre[writePre] = { L[i], R[i] };
            writePre = (writePre + 1) % fifoPre.size();
        }
        havePre = true;
    }
    else
    {
        for (int i = 0; i < n; ++i)
        {
            fifoPost[writePost] = { L[i], R[i] };
            writePost = (writePost + 1) % fifoPost.size();
        }
        havePost = true;
        
        // Update RMS for auto-gain
        if (autoGain)
        {
            float sum = 0.0f;
            for (int i = 0; i < n; ++i)
            {
                const float Lv = L[i], Rv = R[i];
                sum += Lv * Lv + Rv * Rv;
            }
            rmsPost = std::sqrt (sum / (2.0f * n));
        }
    }
}

void BandVisualPane::resized()
{
    // Band visuals take full area - no tooling UI
}

void BandVisualPane::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (juce::Colours::black.withAlpha (1.0f)); // Fully opaque background
    g.fillRoundedRectangle (b.reduced (2.0f), 8.0f);

    const juce::SpinLock::ScopedTryLockType tl (dataLock);
    if (!tl.isLocked()) { drawGrid (g, b); return; }
    drawGrid (g, b);

    // Band-specific: Width mode only
    drawWidthWaveform (g, b.reduced (8.0f));
    drawWidthEditor (g, b.reduced (8.0f));
    drawWidthOverlay (g, b.reduced (8.0f));
    drawWidthHints (g, b.reduced (8.0f));
    
    // Shuffler visual strip
    drawShufflerStrip (g, b.reduced (8.0f));
}

void BandVisualPane::timerCallback()
{
    repaint();
}
