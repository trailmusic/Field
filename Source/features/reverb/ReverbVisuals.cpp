#include "ReverbVisuals.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/PluginProcessor.h"
#include "ReverbParamIDs.h"

ReverbVisuals::ReverbVisuals(MyPluginAudioProcessor& p,
                            juce::AudioProcessorValueTreeState& s,
                            std::function<float()> getEr,
                            std::function<float()> getTail,
                            std::function<float()> getDuckDb,
                            std::function<float()> getWidthNow)
    : proc(p),
      state(s),
      getErRms(getEr),
      getTailRms(getTail),
      getDuckGrDb(getDuckDb),
      getWidthNow(getWidthNow)
{
    setOpaque(true);
}

ReverbVisuals::~ReverbVisuals()
{
}

void ReverbVisuals::resized()
{
    // Simple layout - we'll fill the entire area
    // The parent will position us correctly
}

void ReverbVisuals::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // Fill background
    g.setColour(th.meters.panelDark);
    g.fillRoundedRectangle(r, 8.0f);
    
    // Strong edge shading for depth
    g.setColour(th.sh.withAlpha(0.6f));
    g.drawRoundedRectangle(r.reduced(0.5f), 8.0f - 0.5f, 2.0f);
    
    // Get current levels
    float er = getErRms ? getErRms() : 0.0f;
    float tail = getTailRms ? getTailRms() : 0.0f;
    
    // Get parameter states
    auto* enabledParam = state.getRawParameterValue(ReverbParamIDs::enabled);
    auto* freezeParam = state.getRawParameterValue(ReverbParamIDs::freeze);
    
    if (!enabledParam || !freezeParam)
    {
        DBG("❌ CRITICAL: Reverb parameters not found in APVTS!");
        return;
    }
    
    const bool enabled = *enabledParam > 0.5f;
    const bool frozen = *freezeParam > 0.5f;
    bool hostBypassed = false;
    if (auto* bp = proc.getBypassParameter()) hostBypassed = (bp->getValue() > 0.5f);
    
    auto nowMs = juce::Time::getMillisecondCounterHiRes();
    auto vizResolve = resolveViz(er, tail, enabled, hostBypassed, frozen, allowIdlePreview, nowMs);
    
    // Global alpha for the viz layer (dim on disabled/idle)
    g.setOpacity(vizResolve.alpha);
    
    switch (currentViewMode)
    {
        case ViewMode::Rays:
            paintRaysInBounds(g, r.reduced(10.0f), vizResolve.er, vizResolve.tail);
            break;
        case ViewMode::Waterfall:
            paintWaterfallInBounds(g, r.reduced(10.0f), vizResolve.er, vizResolve.tail);
            break;
        case ViewMode::Spectral:
            paintSpectralInBounds(g, r.reduced(10.0f), vizResolve.er, vizResolve.tail);
            break;
    }
    
    // Optional banner
    if (vizResolve.banner)
    {
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.setFont(14.0f);
        g.drawFittedText(vizResolve.banner, r.toNearestInt(), juce::Justification::topRight, 1);
    }
}

void ReverbVisuals::setViewMode(ViewMode mode)
{
    if (currentViewMode != mode)
    {
        currentViewMode = mode;
        repaint();
    }
}

void ReverbVisuals::lookAndFeelChanged()
{
    repaint();
}

ReverbVisuals::VizResolve ReverbVisuals::resolveViz(float erLevel, float tailLevel,
                                                    bool enabledParam,
                                                    bool hostBypassed,
                                                    bool freezeParam,
                                                    bool allowPreview,
                                                    double nowMs)
{
    constexpr double HOLD_MS = 450.0;   // grace window to avoid idle flicker
    constexpr float  IDLE_ER   = 0.40f; // tiny defaults
    constexpr float  IDLE_TAIL = 0.30f;
    
    auto nearZero = [](float v) { return std::abs(v) < 1e-5f; };

    // 0) Host/plugin disabled: no animation
    if (!enabledParam || hostBypassed)
        return { VizState::Disabled, 0.f, 0.f, 0.4f, "Bypassed" };

    // 1) Frozen tail: always show tail, no ER animation
    if (freezeParam)
    {
        float tail = tailLevel;
        if (nearZero(tail)) tail = IDLE_TAIL; // ensure we see something when frozen
        return { VizState::Frozen, 0.f, tail, 0.9f, "Frozen" };
    }

    // 2) Active vs idle
    const bool hasSignal = (!nearZero(erLevel) || !nearZero(tailLevel));

    if (hasSignal) {
        // remember last time we had signal
        return { VizState::ActiveSignal, erLevel, tailLevel, 1.0f, nullptr };
    }

    // No signal: decide between holding previous frame or idle preview
    // (call this resolver every repaint with 'nowMs' from Time::getMillisecondCounterHiRes)
    static double lastActiveMs = 0.0;
    if (!nearZero(erLevel) || !nearZero(tailLevel)) { lastActiveMs = nowMs; }
    const bool withinHold = (nowMs - lastActiveMs) < HOLD_MS;

    if (withinHold)
        return { VizState::ActiveSignal, erLevel, tailLevel, 1.0f, nullptr };

    if (allowPreview)
        return { VizState::IdlePreview, IDLE_ER, IDLE_TAIL, 0.75f, nullptr };

    // Idle preview disabled → draw a very faint frame (no motion)
    return { VizState::Disabled, 0.f, 0.f, 0.35f, nullptr };
}

void ReverbVisuals::paintRaysInBounds(juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail)
{
    auto n = 32;
    auto cx = bounds.getCentreX(), cy = bounds.getCentreY();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.48f;

    // visibility floor so idle preview is visible but subtle
    const float strength = juce::jlimit(0.08f, 0.25f, 0.12f + 0.40f * juce::jmax(er, tail));
    g.setColour(juce::Colours::white.withAlpha(strength));

    for (int i = 0; i < n; ++i)
    {
        float ang = (float)i / n * juce::MathConstants<float>::twoPi;
        g.drawLine(cx, cy, cx + radius * std::cos(ang), cy + radius * std::sin(ang), 1.0f);
    }
}

void ReverbVisuals::paintWaterfallInBounds(juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail)
{
    // Waterfall visualization using theme greys
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // Create gradient from theme greys - intensity based on signal levels
    float intensity = juce::jmax(er, tail);
    juce::ColourGradient gradient;
    gradient.addColour(0.0, th.meters.panelDark);
    gradient.addColour(0.33, th.meters.panelMedium);
    gradient.addColour(0.66, th.meters.panelLight);
    gradient.addColour(1.0, th.meters.panelLight);
    
    gradient.point1 = juce::Point<float>(bounds.getX(), bounds.getY());
    gradient.point2 = juce::Point<float>(bounds.getX(), bounds.getBottom());
    
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Add texture lines - more visible with higher signal
    g.setColour(th.meters.panelLight.withAlpha(0.3f + 0.4f * intensity));
    for (int i = 0; i < 8; ++i)
    {
        float y = bounds.getY() + (bounds.getHeight() * i / 8.0f);
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 0.5f);
    }
}

void ReverbVisuals::paintSpectralInBounds(juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail)
{
    // Spectral visualization
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // Draw spectral bars
    const int numBars = 32;
    const float barWidth = bounds.getWidth() / numBars;
    
    for (int i = 0; i < numBars; ++i)
    {
        float barHeight = bounds.getHeight() * (0.1f + 0.9f * (er + tail) * (0.5f + 0.5f * std::sin(i * 0.2f)));
        float x = bounds.getX() + i * barWidth;
        float y = bounds.getBottom() - barHeight;
        
        g.setColour(th.meters.panelLight.withAlpha(0.7f));
        g.fillRect(x, y, barWidth - 1, barHeight);
    }
}
