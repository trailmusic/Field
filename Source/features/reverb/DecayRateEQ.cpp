#include "DecayRateEQ.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "features/dynEq/FilterFactory.h"
#include "shared/Core/PluginProcessor.h"

DecayRateEQ::DecayRateEQ(MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
    : proc(p)
{
    setOpaque(true);
    setLookAndFeel(lnf);
    startTimerHz(30);
    
    // Initialize zoom state for decay multiplier range
    zoomState.prepare(1.5f); // 0.5x to 2.0x range
    
    addAndMakeVisible(analyzer);
    analyzer.setInterceptsMouseClicks(false, false);
    analyzer.setAutoHeadroomEnabled(true);
    analyzer.setHeadroomTargetFill(0.70f);
    SpectrumAnalyzer::Params prm; 
    prm.fps = 30; 
    analyzer.setParams(prm);
    analyzer.setDrawGridHorizontal(false); // we'll draw our own multiplier units
}

DecayRateEQ::~DecayRateEQ()
{
    stopTimer();
}

void DecayRateEQ::timerCallback()
{
    // Drive delayed ghost repaint and hover HUD updates at 30Hz
    repaint();
}

void DecayRateEQ::paint(juce::Graphics& g)
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
    
    // Draw units and grid
    drawUnits(g);
    
    // Draw EQ curves
    auto rA = analyzer.getBounds().toFloat();
    if (!rA.isEmpty())
    {
        // Combined EQ curve (macro) slightly more prominent
        g.setColour(th.accent.withAlpha(0.95f));
        g.strokePath(eqPath, juce::PathStrokeType(3.0f));
        
        // Per-band curves
        for (size_t i = 0; i < bandPaths.size(); ++i)
        {
            juce::Colour base = bandColourFor((int)i);
            g.setColour(base.withAlpha(0.90f));
            g.strokePath(bandPaths[i], juce::PathStrokeType(1.2f));
        }
        
        // Draw band points
        g.setColour(juce::Colours::orange.withAlpha(0.95f));
        for (const auto& pt : points)
        {
            const float x = mapHzToX(pt.hz);
            const float y = mapMultToY(pt.mult);
            g.fillEllipse(x-8, y-8, 16, 16);
        }
        
        // Selected point highlight
        if (selected >= 0 && selected < (int)points.size())
        {
            const auto& pt = points[(size_t)selected];
            const float x = mapHzToX(pt.hz);
            const float y = mapMultToY(pt.mult);
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.drawEllipse(x-12, y-12, 24, 24, 1.6f);
        }
    }
}

void DecayRateEQ::resized()
{
    auto r = getLocalBounds().reduced(6);
    analyzer.setBounds(r);
    rebuildEqPath();
}

void DecayRateEQ::rebuildEqPath()
{
    eqPath.clear();
    bandPaths.clear();
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;

    const int N = juce::jmax(128, (int)r.getWidth());
    
    // Build combined EQ curve
    auto totalMultAt = [this](double hz)
    {
        float s = 1.0f; // Start with 1.0x (no change)
        for (const auto& b : points) 
        {
            s *= bandMultAtForPaint(b, (float)hz);
        }
        return s;
    };
    
    auto mapX = [&](int i)
    {
        const double minHz = 20.0, maxHz = 20000.0;
        const double t = (double)i / (double)(N - 1);
        const double a = std::log10(minHz), b = std::log10(maxHz);
        const double logF = juce::jmap(t, 0.0, 1.0, a, b);
        const double hz = std::pow(10.0, logF);
        return std::pair<float,float>((float)hz, mapMultToY(totalMultAt(hz)));
    };

    auto p0 = mapX(0); 
    eqPath.startNewSubPath(r.getX(), p0.second);
    for (int i = 1; i < N; ++i)
    {
        auto p = mapX(i);
        const float x = r.getX() + (float)i / (float)(N - 1) * r.getWidth();
        eqPath.lineTo(x, p.second);
    }
    
    // Per-band paths
    bandPaths.resize(points.size());
    for (size_t bi = 0; bi < points.size(); ++bi)
    {
        auto& bp = bandPaths[bi];
        auto mapBand = [&](int i){
            const double minHz = 20.0, maxHz = 20000.0;
            const double t = (double)i / (double)(N - 1);
            const double a = std::log10(minHz), b = std::log10(maxHz);
            const double logF = juce::jmap(t, 0.0, 1.0, a, b);
            const double hz = std::pow(10.0, logF);
            return std::pair<float,float>((float)hz, mapMultToY(bandMultAtForPaint(points[bi], (float)hz)));
        };
        auto q0 = mapBand(0); 
        bp.startNewSubPath(r.getX(), q0.second);
        for (int i = 1; i < N; ++i)
        {
            auto q = mapBand(i);
            const float x = r.getX() + (float)i / (float)(N - 1) * r.getWidth();
            bp.lineTo(x, q.second);
        }
    }
}

void DecayRateEQ::drawUnits(juce::Graphics& g)
{
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;
    
    g.setFont(12.0f);
    auto gridCol = juce::Colours::white.withAlpha(0.10f);
    auto textCol = juce::Colours::white.withAlpha(0.45f);
    g.setColour(gridCol);

    // Decay multiplier ticks
    const float multVals[] = { 2.0f, 1.5f, 1.0f, 0.75f, 0.5f };
    for (float mult : multVals)
    {
        const float y = mapMultToY(mult);
        g.setColour(gridCol);
        g.drawLine(r.getX(), y, r.getRight(), y, mult == 1.0f ? 1.2f : 0.6f);
        g.setColour(textCol);
        juce::String lbl = juce::String(mult, 1) + "×";
        g.drawFittedText(lbl, juce::Rectangle<int>((int)r.getX()+4, (int)y-8, 44, 16), juce::Justification::centredLeft, 1);
    }

    // Hz ticks
    const double hzTicks[] = { 20, 50, 100, 200, 500, 1000, 1500, 2000, 3000, 4000, 5000, 7000, 8000, 10000, 20000 };
    for (double hz : hzTicks)
    {
        const float x = mapHzToX((float)hz);
        g.setColour(gridCol);
        g.drawLine(x, r.getBottom()-16.0f, x, r.getBottom(), 0.8f);
        g.setColour(textCol);
        juce::String lbl;
        if (hz >= 1000.0 && hz < 10000.0) lbl = juce::String(hz/1000.0, 1) + "k";
        else if (hz >= 10000.0) lbl = juce::String((int)std::round(hz/1000.0)) + "k";
        else lbl = juce::String((int)hz);
        g.drawFittedText(lbl, juce::Rectangle<int>((int)x-18, (int)r.getBottom()-30, 36, 14), juce::Justification::centred, 1);
    }
}

// Mouse interaction
void DecayRateEQ::mouseDown(const juce::MouseEvent& e)
{
    selected = hitTestPoint(e.getPosition());
    if (selected < 0 && !e.mods.isPopupMenu())
    {
        // Create new band
        DecayBandPoint bp; 
        bp.hz = juce::jlimit(20.f, 20000.f, mapXToHz(e.getPosition().x)); 
        bp.mult = juce::jlimit(0.5f, 2.0f, mapYToMult(e.getPosition().y));
        if (bp.hz <= 50.0f) { bp.type = 1; } // TiltLo
        else if (bp.hz >= 10000.0f) { bp.type = 2; } // TiltHi
        else { bp.type = 0; } // Bell
        
        const int slot = allocateBandSlot();
        if (slot >= 0)
        {
            bp.bandIdx = slot;
            setBandParam(slot, ReverbEQParams::DecayBand::active, 1.0f);
            setBandParam(slot, ReverbEQParams::DecayBand::freqHz, bp.hz);
            setBandParam(slot, ReverbEQParams::DecayBand::decayMult, bp.mult);
            setBandParam(slot, ReverbEQParams::DecayBand::q, bp.q);
        }
        points.push_back(bp);
        selected = (int)points.size() - 1;
        rebuildEqPath(); 
        repaint();
    }
}

void DecayRateEQ::mouseDrag(const juce::MouseEvent& e)
{
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        pt.hz = juce::jlimit(20.f, 20000.f, mapXToHz(e.getPosition().x));
        pt.mult = juce::jlimit(0.5f, 2.0f, mapYToMult(e.getPosition().y));
        if (pt.bandIdx >= 0)
        {
            setBandParam(pt.bandIdx, ReverbEQParams::DecayBand::freqHz, pt.hz);
            setBandParam(pt.bandIdx, ReverbEQParams::DecayBand::decayMult, pt.mult);
        }
        rebuildEqPath();
        repaint();
    }
}

void DecayRateEQ::mouseUp(const juce::MouseEvent&)
{
    dragging = false;
}

void DecayRateEQ::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        const float delta = (float)(wheel.deltaY * (e.mods.isShiftDown() ? 1.0 : 0.2));
        pt.q = juce::jlimit(0.1f, 36.0f, pt.q * (1.0f + delta));
        if (pt.bandIdx >= 0)
            setBandParam(pt.bandIdx, ReverbEQParams::DecayBand::q, pt.q);
        rebuildEqPath();
        repaint();
    }
}

void DecayRateEQ::mouseMove(const juce::MouseEvent& e)
{
    const int h = hitTestPoint(e.getPosition());
    if (h != hover)
    {
        hover = h;
        repaint();
    }
    hoverPos = e.getPosition();
    auto r = analyzer.getBounds();
    hoverInPane = r.contains(hoverPos);
    if (hoverInPane)
        hoverHz = juce::jlimit(20.0f, 20000.0f, mapXToHz(hoverPos.x));
}

void DecayRateEQ::mouseExit(const juce::MouseEvent&)
{
    hover = -1;
    hoverInPane = false;
    repaint();
}

// Mapping helpers
float DecayRateEQ::mapHzToX(float hz) const
{
    auto r = analyzer.getBounds().toFloat();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = (float)(std::log10(juce::jlimit(minHz, maxHz, hz) / minHz) / std::log10(maxHz / minHz));
    return r.getX() + t * r.getWidth();
}

float DecayRateEQ::mapMultToY(float mult) const
{
    auto r = analyzer.getBounds().toFloat();
    const float top = r.getY()+8.f, bottom = r.getBottom()-8.f;
    const float halfRange = zoomState.getCurrent();
    return juce::jmap(mult, 2.0f, 0.5f, top, bottom);
}

float DecayRateEQ::mapXToHz(int px) const
{
    auto r = analyzer.getBounds();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = juce::jlimit(0.0f, 1.0f, (px - (float)r.getX()) / (float)r.getWidth());
    const float a = std::log10(minHz), b = std::log10(maxHz);
    return std::pow(10.0f, juce::jmap(t, 0.0f, 1.0f, a, b));
}

float DecayRateEQ::mapYToMult(int py) const
{
    auto r = analyzer.getBounds();
    const float halfRange = zoomState.getCurrent();
    return juce::jmap((float)py, (float)r.getY(), (float)r.getBottom(), 2.0f, 0.5f);
}

// Hit testing
int DecayRateEQ::hitTestPoint(juce::Point<int> p) const
{
    const float radius = 12.0f;
    for (int i = (int)points.size()-1; i >= 0; --i)
    {
        const float x = mapHzToX(points[(size_t)i].hz);
        const float y = mapMultToY(points[(size_t)i].mult);
        if (juce::Point<float>(x, y).getDistanceFrom(p.toFloat()) <= radius)
            return i;
        }
    return -1;
}

// Band management
int DecayRateEQ::allocateBandSlot()
{
    for (int i = 0; i < kMaxBands; ++i)
    {
        auto id = bandId(ReverbEQParams::DecayBand::active, i);
        if (auto* v = proc.apvts.getRawParameterValue(id))
        {
            if (v->load() < 0.5f)
                return i;
        }
    }
    return -1;
}

void DecayRateEQ::setBandParam(int bandIdx, const char* baseId, float value)
{
    auto id = bandId(baseId, bandIdx);
    if (auto* p = proc.apvts.getParameter(id))
    {
        const float norm = p->convertTo0to1(value);
        p->setValueNotifyingHost(norm);
    }
}

float DecayRateEQ::getBandParamFloat(int bandIdx, const char* baseId, float fallback) const
{
    auto id = bandId(baseId, bandIdx);
    if (auto* v = proc.apvts.getRawParameterValue(id)) return v->load();
    return fallback;
}

// Visual helpers
juce::Colour DecayRateEQ::bandColourFor(int bandIdx) const
{
    juce::Colour accent = juce::Colours::orange;
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        accent = lf->theme.accent;
    const float baseHue = accent.getHue();
    const float baseSat = juce::jlimit(0.25f, 0.95f, accent.getSaturation());
    const float baseBrt = juce::jlimit(0.35f, 0.95f, accent.getBrightness());
    const float golden = 0.61803398875f;
    float hue = std::fmod(baseHue + golden * (float)(bandIdx + 1), 1.0f);
    hue = juce::jlimit(0.0f, 1.0f, 0.65f * hue + 0.35f * baseHue);
    float sat = juce::jlimit(0.30f, 0.95f, baseSat * 0.9f + 0.1f);
    float brt = juce::jlimit(0.40f, 0.95f, baseBrt * 0.9f + 0.1f);
    return juce::Colour::fromHSV(hue, sat, brt, 1.0f);
}

float DecayRateEQ::bandMultAtForPaint(const DecayBandPoint& b, float hz) const
{
    const double logHz = std::log10(juce::jlimit(20.0f, 20000.0f, hz));
    const double logC = std::log10(juce::jlimit(20.0f, 20000.0f, b.hz));
    const double q = juce::jlimit(0.1, 36.0, (double)b.q);
    const double width = juce::jlimit(0.02, 0.50, 0.22 / q);
    const double d = (logHz - logC) / width;
    
    switch (b.type)
    {
        case 0: { // Bell
            const float w = (float)std::exp(-0.5 * d * d);
            return juce::jmap(w, 0.0f, 1.0f, 1.0f, b.mult);
        }
        case 1: { // TiltLo
            const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC)));
            return juce::jmap((float)s, 0.0f, 1.0f, 1.0f, b.mult);
        }
        case 2: { // TiltHi
            const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC)));
            return juce::jmap((float)(1.0 - s), 0.0f, 1.0f, 1.0f, b.mult);
        }
        default: return 1.0f;
    }
}