#include "ReverbEQ.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "features/dynEq/FilterFactory.h"
#include "shared/Core/PluginProcessor.h"

ReverbEQ::ReverbEQ(MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
    : proc(p)
{
    setOpaque(true);
    setLookAndFeel(lnf);
    startTimerHz(30);
    
    // Initialize zoom state
    zoomState.prepare(60.0);
    
    addAndMakeVisible(analyzer);
    analyzer.setInterceptsMouseClicks(false, false);
    analyzer.setAutoHeadroomEnabled(true);
    analyzer.setHeadroomTargetFill(0.70f);
    SpectrumAnalyzer::Params prm; 
    prm.fps = 30; 
    analyzer.setParams(prm);
    analyzer.setDrawGridHorizontal(false);
}

ReverbEQ::~ReverbEQ()
{
    stopTimer();
}

void ReverbEQ::timerCallback()
{
    repaint();
}

void ReverbEQ::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto panel = lf ? lf->theme.meters.panelDark : juce::Colour(0xFF2A2C30);
    auto sh = lf ? lf->theme.sh : juce::Colour(0xFF2A2A2A);
    
    // Background with elevation shadow
    const float cr = 8.0f;
    if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.25f));
    else g.setColour(juce::Colour(0x40000000));
    g.fillRoundedRectangle(r.translated(1.5f, 1.5f), cr);
    
    // Main background
    g.setColour(panel);
    g.fillRoundedRectangle(r, cr);
    
    // Border
    g.setColour(sh);
    g.drawRoundedRectangle(r, cr, 1.0f);
    
    // Draw units and grid
    drawUnits(g);
    
    // Draw EQ curves
    auto rA = analyzer.getBounds().toFloat();
    
    // Per-band curves
    for (size_t i = 0; i < bandPaths.size(); ++i)
    {
        juce::Colour base = bandColourFor((int)i);
        g.setColour(base.withAlpha(selected == (int)i ? 1.0f : 0.90f));
        const float width = (selected == (int)i ? 1.8f : 1.2f);
        g.strokePath(bandPaths[i], juce::PathStrokeType(width));
    }
    
    // Combined EQ curve
    g.setColour(juce::Colours::cyan.withAlpha(0.95f));
    g.strokePath(eqPath, juce::PathStrokeType(3.0f));
    
    // Draw band points
    g.setColour(juce::Colours::yellow.withAlpha(0.95f));
    for (const auto& pt : points)
    {
        const float x = mapHzToX(pt.hz);
        const float y = mapDbToY(pt.db);
        g.fillEllipse(x-8, y-8, 16, 16);
    }
    
    // Selection highlight
    if (selected >= 0 && selected < (int)points.size())
    {
        const auto& pt = points[(size_t)selected];
        const float x = mapHzToX(pt.hz);
        const float y = mapDbToY(pt.db);
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawEllipse(x-12, y-12, 24, 24, 1.6f);
    }
    
    // Hover readout
    if (hoverInPane)
    {
        g.setColour(juce::Colours::white.withAlpha(0.60f));
        juce::String hzText;
        if (hoverHz >= 1000.0f && hoverHz < 10000.0f) 
            hzText = juce::String(hoverHz / 1000.0f, 1) + "k";
        else if (hoverHz >= 10000.0f) 
            hzText = juce::String((int)std::round(hoverHz/1000.0f)) + "k";
        else 
            hzText = juce::String((int)hoverHz);
        
        juce::String lbl = hzText + " Hz";
        auto tb = juce::Rectangle<float>((float)hoverPos.x - 32.0f, rA.getBottom() - 20.0f, 64.0f, 14.0f);
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.fillRoundedRectangle(tb, 4.0f);
        g.setColour(juce::Colours::white.withAlpha(0.80f));
        g.drawFittedText(lbl, tb.toNearestInt(), juce::Justification::centred, 1);
    }
}

void ReverbEQ::resized()
{
    auto r = getLocalBounds().reduced(6);
    analyzer.setBounds(r);
    rebuildEqPath();
}

void ReverbEQ::rebuildEqPath()
{
    eqPath.clear();
    bandPaths.clear();
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;

    const int N = juce::jmax(128, (int)r.getWidth());
    
    auto bandDbAt = [this](const BandPoint& b, double hz)
    {
        const double logHz = std::log10(juce::jlimit(20.0, 20000.0, hz));
        const double logC = std::log10(juce::jlimit(20.0f, 20000.0f, b.hz));
        const double q = juce::jlimit(0.1, 36.0, (double)b.q);
        const double width = juce::jlimit(0.02, 0.50, 0.22 / q);
        const double d = (logHz - logC) / width;
        
        switch (b.type)
        {
            case 0: { // Bell
                const float w = (float)std::exp(-0.5 * d * d);
                return b.db * w;
            }
            case 1: { // LowShelf
                const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25);
                const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC)));
                return (float)(b.db * s);
            }
            case 2: { // HighShelf
                const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25);
                const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC)));
                return (float)(b.db * (1.0 - s));
            }
            case 3: { // HP
                const double n = 2.0;
                const double fc = std::pow(10.0, logC);
                const double ratio = juce::jlimit(1e-6, 1e6, fc / juce::jlimit(20.0, 20000.0, hz));
                const double att = -std::abs((double)b.db <= 0.01 ? 24.0 : (double)b.db);
                const double mag = 1.0 / std::sqrt(1.0 + std::pow(ratio, 2.0 * n));
                return (float)(att * (1.0 - mag));
            }
            case 4: { // LP
                const double n = 2.0;
                const double fc = std::pow(10.0, logC);
                const double ratio = juce::jlimit(1e-6, 1e6, juce::jlimit(20.0, 20000.0, hz) / fc);
                const double att = -std::abs((double)b.db <= 0.01 ? 24.0 : (double)b.db);
                const double mag = 1.0 / std::sqrt(1.0 + std::pow(ratio, 2.0 * n));
                return (float)(att * (1.0 - mag));
            }
            default: return 0.0f;
        }
    };
    
    auto totalDbAt = [&](double hz)
    {
        float s = 0.0f;
        for (const auto& b : points) s += bandDbAt(b, hz);
        return s;
    };

    auto mapX = [&](int i)
    {
        const double minHz = 20.0, maxHz = 20000.0;
        const double t = (double)i / (double)(N - 1);
        const double a = std::log10(minHz), b = std::log10(maxHz);
        const double logF = juce::jmap(t, 0.0, 1.0, a, b);
        const double hz = std::pow(10.0, logF);
        return std::pair<float,float>((float)hz, mapDbToY(totalDbAt(hz)));
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
        auto mapBand = [&](int i)
        {
            const double minHz = 20.0, maxHz = 20000.0;
            const double t = (double)i / (double)(N - 1);
            const double a = std::log10(minHz), b = std::log10(maxHz);
            const double logF = juce::jmap(t, 0.0, 1.0, a, b);
            const double hz = std::pow(10.0, logF);
            return std::pair<float,float>((float)hz, mapDbToY(bandDbAt(points[bi], hz)));
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

void ReverbEQ::drawUnits(juce::Graphics& g)
{
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;
    
    g.setFont(12.0f);
    auto gridCol = juce::Colours::white.withAlpha(0.10f);
    auto textCol = juce::Colours::white.withAlpha(0.45f);
    g.setColour(gridCol);

    // dB ticks
    const float halfRange = zoomState.getCurrent();
    const float dbVals[] = {18, 12, 6, 0, -6, -12, -18, -24, -30, -36};
    for (float dbv : dbVals)
    {
        if (dbv > halfRange || dbv < -halfRange) continue;
        const float y = mapDbToY(dbv);
        g.setColour(gridCol);
        g.drawLine(r.getX(), y, r.getRight(), y, dbv == 0 ? 1.2f : 0.6f);
        g.setColour(textCol);
        juce::String lbl = juce::String((int)dbv) + " dB";
        g.drawFittedText(lbl, juce::Rectangle<int>((int)r.getX()+4, (int)y-8, 44, 16), juce::Justification::centredLeft, 1);
    }

    // Hz ticks
    const double hzTicks[] = {20, 50, 100, 200, 500, 1000, 1500, 2000, 3000, 4000, 5000, 7000, 8000, 10000, 20000};
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

// Mouse interaction methods
void ReverbEQ::mouseDown(const juce::MouseEvent& e)
{
    selected = hitTestPoint(e.getPosition());
    
    if (selected < 0 && !e.mods.isPopupMenu())
    {
        // Create new band
        BandPoint bp;
        bp.hz = juce::jlimit(20.f, 20000.f, mapXToHz(e.getPosition().x));
        bp.db = juce::jlimit(-24.f, 24.f, mapYToDb(e.getPosition().y));
        
        if (bp.hz <= 50.0f) { bp.type = 3; bp.db = -12.0f; }
        else if (bp.hz >= 10000.0f) { bp.type = 4; bp.db = -12.0f; }
        else { bp.type = 0; }
        
        const int slot = allocateBandSlot();
        if (slot >= 0)
        {
            bp.bandIdx = slot;
            setBandParam(slot, ReverbEQ::ToneBand::active, 1.0f);
            setBandParam(slot, ReverbEQ::ToneBand::freqHz, bp.hz);
            setBandParam(slot, ReverbEQ::ToneBand::gainDb, bp.db);
            setBandParam(slot, ReverbEQ::ToneBand::q, bp.q);
            setBandParam(slot, ReverbEQ::ToneBand::type, (float)bp.type);
            setBandParam(slot, ReverbEQ::ToneBand::phase, (float)bp.phase);
        }
        points.push_back(bp);
        selected = (int)points.size() - 1;
        rebuildEqPath();
        repaint();
    }
    
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu m;
        m.addItem(1, "Delete band", selected >= 0);
        m.addItem(2, "Reset Q", selected >= 0);
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [this](int r)
            {
                if (r == 1 && selected >= 0 && selected < (int)points.size()) 
                { 
                    points.erase(points.begin() + selected); 
                    selected = -1; 
                    rebuildEqPath(); 
                    repaint(); 
                }
                if (r == 2 && selected >= 0 && selected < (int)points.size()) 
                { 
                    points[(size_t)selected].q = 0.707f; 
                    rebuildEqPath(); 
                    repaint(); 
                }
            });
        return;
    }
}

void ReverbEQ::mouseDrag(const juce::MouseEvent& e)
{
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        pt.hz = juce::jlimit(20.f, 20000.f, mapXToHz(e.getPosition().x));
        pt.db = juce::jlimit(-24.f, 24.f, mapYToDb(e.getPosition().y));
        
        if (pt.bandIdx >= 0)
        {
            setBandParam(pt.bandIdx, ReverbEQ::ToneBand::freqHz, pt.hz);
            setBandParam(pt.bandIdx, ReverbEQ::ToneBand::gainDb, pt.db);
        }
        rebuildEqPath();
        repaint();
    }
}

void ReverbEQ::mouseUp(const juce::MouseEvent&)
{
    dragging = false;
}

void ReverbEQ::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        const float delta = (float)(wheel.deltaY * (e.mods.isShiftDown() ? 1.0 : 0.2));
        pt.q = juce::jlimit(0.1f, 36.0f, pt.q * (1.0f + delta));
        if (pt.bandIdx >= 0)
            setBandParam(pt.bandIdx, ReverbEQ::ToneBand::q, pt.q);
        rebuildEqPath();
        repaint();
    }
}

void ReverbEQ::mouseMove(const juce::MouseEvent& e)
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
    repaint();
}

void ReverbEQ::mouseExit(const juce::MouseEvent&)
{
    hover = -1;
    hoverInPane = false;
    repaint();
}

// Mapping helpers
float ReverbEQ::mapHzToX(float hz) const
{
    auto r = analyzer.getBounds().toFloat();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = (float)(std::log10(juce::jlimit(minHz, maxHz, hz) / minHz) / std::log10(maxHz / minHz));
    return r.getX() + t * r.getWidth();
}

float ReverbEQ::mapDbToY(float dB) const
{
    auto r = analyzer.getBounds().toFloat();
    const float top = r.getY()+8.f, bottom = r.getBottom()-8.f;
    const float halfRange = zoomState.getCurrent();
    return juce::jmap(dB, +halfRange, -halfRange, top, bottom);
}

float ReverbEQ::mapXToHz(int px) const
{
    auto r = analyzer.getBounds();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = juce::jlimit(0.0f, 1.0f, (px - (float)r.getX()) / (float)r.getWidth());
    const float a = std::log10(minHz), b = std::log10(maxHz);
    return std::pow(10.0f, juce::jmap(t, 0.0f, 1.0f, a, b));
}

float ReverbEQ::mapYToDb(int py) const
{
    auto r = analyzer.getBounds();
    const float halfRange = zoomState.getCurrent();
    return juce::jmap((float)py, (float)r.getY(), (float)r.getBottom(), +halfRange, -halfRange);
}

int ReverbEQ::hitTestPoint(juce::Point<int> p) const
{
    const float radius = 12.0f;
    for (int i = (int)points.size()-1; i >= 0; --i)
    {
        const float x = mapHzToX(points[(size_t)i].hz);
        const float y = mapDbToY(points[(size_t)i].db);
        if (juce::Point<float>(x, y).getDistanceFrom(p.toFloat()) <= radius)
            return i;
    }
    return -1;
}

int ReverbEQ::allocateBandSlot()
{
    for (int i = 0; i < kMaxBands; ++i)
    {
        auto id = bandId(ReverbEQ::ToneBand::active, i);
        if (auto* v = proc.apvts.getRawParameterValue(id))
        {
            if (v->load() < 0.5f)
                return i;
        }
    }
    return -1;
}

void ReverbEQ::setBandParam(int bandIdx, const char* baseId, float value)
{
    auto id = bandId(baseId, bandIdx);
    if (auto* p = proc.apvts.getParameter(id))
    {
        const float norm = p->convertTo0to1(value);
        p->setValueNotifyingHost(norm);
    }
}

float ReverbEQ::getBandParamFloat(int bandIdx, const char* baseId, float fallback) const
{
    auto id = bandId(baseId, bandIdx);
    if (auto* v = proc.apvts.getRawParameterValue(id)) return v->load();
    return fallback;
}

juce::Colour ReverbEQ::bandColourFor(int bandIdx) const
{
    juce::Colour accent = juce::Colours::deepskyblue;
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

float ReverbEQ::bandDbAtForPaint(const BandPoint& b, float hz) const
{
    const double logHz = std::log10(juce::jlimit(20.0f, 20000.0f, hz));
    const double logC = std::log10(juce::jlimit(20.0f, 20000.0f, b.hz));
    const double q = juce::jlimit(0.1, 36.0, (double)b.q);
    const double width = juce::jlimit(0.02, 0.50, 0.22 / q);
    const double d = (logHz - logC) / width;
    
    switch (b.type)
    {
        case 0: { const float w = (float)std::exp(-0.5 * d * d); return b.db * w; }
        case 1: { const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25); const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC))); return (float)(b.db * s); }
        case 2: { const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25); const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC))); return (float)(b.db * (1.0 - s)); }
        case 3: { const double n = 2.0; const double fc = std::pow(10.0, logC); const double ratio = juce::jlimit(1e-6, 1e6, fc / juce::jlimit(20.0, 20000.0, (double)hz)); const double att = -std::abs((double)b.db <= 0.01 ? 24.0 : (double)b.db); const double mag = 1.0 / std::sqrt(1.0 + std::pow(ratio, 2.0 * n)); return (float)(att * (1.0 - mag)); }
        case 4: { const double n = 2.0; const double fc = std::pow(10.0, logC); const double ratio = juce::jlimit(1e-6, 1e6, juce::jlimit(20.0, 20000.0, (double)hz) / fc); const double att = -std::abs((double)b.db <= 0.01 ? 24.0 : (double)b.db); const double mag = 1.0 / std::sqrt(1.0 + std::pow(ratio, 2.0 * n)); return (float)(att * (1.0 - mag)); }
        default: return 0.0f;
    }
}
