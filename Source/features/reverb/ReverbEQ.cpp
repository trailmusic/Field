#include "ReverbEQ.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "features/dynEq/FilterFactory.h"
#include "shared/Core/PluginProcessor.h"

ReverbToneEQ::ReverbToneEQ(MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
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
    analyzer.setDrawGridHorizontal(false); // we'll draw our own dB units
    
    // Add per-band modules
    addAndMakeVisible(overlay);
    overlay.setVisible(false);
    addAndMakeVisible(badge);
    badge.setVisible(false);
    
    // Setup overlay callbacks
    overlay.onGainChanged = [this](float g) {
        if (selected >= 0 && selected < (int)points.size()) {
            points[(size_t)selected].db = juce::jlimit(-24.f, 24.f, g);
            if (points[(size_t)selected].bandIdx >= 0)
                setBandParam(points[(size_t)selected].bandIdx, ReverbEQParams::ToneBand::gainDb, g);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onQChanged = [this](float q) {
        if (selected >= 0 && selected < (int)points.size()) {
            points[(size_t)selected].q = juce::jlimit(0.1f, 36.0f, q);
            if (points[(size_t)selected].bandIdx >= 0)
                setBandParam(points[(size_t)selected].bandIdx, ReverbEQParams::ToneBand::q, q);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onFreqChanged = [this](float f) {
        if (selected >= 0 && selected < (int)points.size()) {
            points[(size_t)selected].hz = juce::jlimit(20.f, 20000.f, f);
            if (points[(size_t)selected].bandIdx >= 0)
                setBandParam(points[(size_t)selected].bandIdx, ReverbEQParams::ToneBand::freqHz, f);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onTypeChanged = [this](int t) {
        if (selected >= 0 && selected < (int)points.size()) {
            points[(size_t)selected].type = juce::jlimit(0, 2, t);
            if (points[(size_t)selected].bandIdx >= 0)
                setBandParam(points[(size_t)selected].bandIdx, ReverbEQParams::ToneBand::type, (float)t);
            rebuildEqPath(); repaint();
        }
    };
    
    // Setup badge callbacks
    badge.onDelete = [this] {
        if (selected >= 0 && selected < (int)points.size()) {
            const int bandIdx = points[(size_t)selected].bandIdx;
            if (bandIdx >= 0) setBandParam(bandIdx, ReverbEQParams::ToneBand::active, 0.0f);
            points.erase(points.begin() + selected);
            selected = -1; rebuildEqPath(); repaint(); overlay.setVisible(false); badge.setVisible(false);
        }
    };
    
    badge.onBypass = [this](bool off) {
        if (selected >= 0 && selected < (int)points.size()) {
            const int bandIdx = points[(size_t)selected].bandIdx;
            if (bandIdx >= 0) setBandParam(bandIdx, ReverbEQParams::ToneBand::active, off ? 0.0f : 1.0f);
        }
    };
    
    badge.onSetType = [this](int tp) {
        const int idx = (badgeFor >= 0 ? badgeFor : selected);
        if (idx >= 0 && idx < (int)points.size()) {
            auto& p = points[(size_t)idx];
            p.type = juce::jlimit(0, 2, tp);
            if (p.bandIdx >= 0) setBandParam(p.bandIdx, ReverbEQParams::ToneBand::type, (float)p.type);
            rebuildEqPath(); repaint(); positionBadgeFor(idx);
        }
    };
}

ReverbToneEQ::~ReverbToneEQ()
{
    stopTimer();
}

void ReverbToneEQ::timerCallback()
{
    // Drive delayed ghost repaint and hover HUD updates at 30Hz
    repaint();
}

// Mouse interaction methods
void ReverbToneEQ::mouseDown(const juce::MouseEvent& e)
{
    const int h = hitTestPoint(e.getPosition());
    if (h >= 0)
    {
        selected = h;
        auto& pt = points[(size_t)selected];
        overlay.setValues(pt.db, pt.q, pt.hz, pt.type);
        overlay.setVisible(true);
        positionOverlay();
        positionBadgeFor(selected);
    }
    else if (!e.mods.isPopupMenu())
    {
        // Check band limit (4 bands max for Tone EQ)
        if (points.size() >= 4)
        {
            // Show tooltip or visual feedback that limit is reached
            return;
        }
        
        // Create new band
        BandPoint bp; 
        bp.hz = juce::jlimit(20.f, 20000.f, mapXToHz(e.getPosition().x)); 
        bp.db = juce::jlimit(-24.f, 24.f, mapYToDb(e.getPosition().y));
        if (bp.hz <= 50.0f) { bp.type = 1; bp.db = -12.0f; } // Low Shelf
        else if (bp.hz >= 10000.0f) { bp.type = 2; bp.db = -12.0f; } // High Shelf
        else { bp.type = 0; } // Bell
        
        const int slot = allocateBandSlot();
        if (slot >= 0)
        {
            bp.bandIdx = slot;
            setBandParam(slot, ReverbEQParams::ToneBand::active, 1.0f);
            setBandParam(slot, ReverbEQParams::ToneBand::freqHz, bp.hz);
            setBandParam(slot, ReverbEQParams::ToneBand::gainDb, bp.db);
            setBandParam(slot, ReverbEQParams::ToneBand::q, bp.q);
            setBandParam(slot, ReverbEQParams::ToneBand::type, (float)bp.type);
            setBandParam(slot, ReverbEQParams::ToneBand::phase, (float)bp.phase);
        }
        points.push_back(bp);
        selected = (int)points.size() - 1;
        overlay.setValues(bp.db, bp.q, bp.hz, bp.type);
        overlay.setVisible(true);
        positionOverlay();
        positionBadgeFor(selected);
        rebuildEqPath();
        repaint();
    }
    else
    {
        overlay.setVisible(false);
        badge.setVisible(false);
    }
}

void ReverbToneEQ::mouseDrag(const juce::MouseEvent& e)
{
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        pt.hz = juce::jlimit(20.f, 20000.f, mapXToHz(e.getPosition().x));
        pt.db = juce::jlimit(-24.f, 24.f, mapYToDb(e.getPosition().y));
        if (pt.bandIdx >= 0)
        {
            setBandParam(pt.bandIdx, ReverbEQParams::ToneBand::freqHz, pt.hz);
            setBandParam(pt.bandIdx, ReverbEQParams::ToneBand::gainDb, pt.db);
        }
        rebuildEqPath();
        repaint();
    }
}

void ReverbToneEQ::mouseUp(const juce::MouseEvent&)
{
    // Update overlay values after drag
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        overlay.setValues(pt.db, pt.q, pt.hz, pt.type);
        positionOverlay();
        positionBadgeFor(selected);
    }
}

void ReverbToneEQ::mouseMove(const juce::MouseEvent& e)
{
    const int h = hitTestPoint(e.getPosition());
    if (h != hover)
    {
        hover = h;
        if (selected < 0)
        {
            if (hover >= 0) positionBadgeFor(hover);
            else badge.setVisible(false);
        }
        else
        {
            if (hover >= 0) positionBadgeFor(hover);
            else positionBadgeFor(selected);
        }
    }
    hoverPos = e.getPosition();
    auto r = analyzer.getBounds();
    hoverInPane = r.contains(hoverPos);
    if (hoverInPane)
        hoverHz = juce::jlimit(20.0f, 20000.0f, mapXToHz(hoverPos.x));
    repaint();
}

void ReverbToneEQ::mouseExit(const juce::MouseEvent&)
{
    if (selected < 0) badge.setVisible(false);
    hover = -1;
    hoverInPane = false;
    repaint();
}

void ReverbToneEQ::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        pt.q = juce::jlimit(0.1f, 36.0f, pt.q + wheel.deltaY * 0.1f);
        if (pt.bandIdx >= 0)
            setBandParam(pt.bandIdx, ReverbEQParams::ToneBand::q, pt.q);
        rebuildEqPath();
        overlay.setValues(pt.db, pt.q, pt.hz, pt.type);
        positionOverlay();
        positionBadgeFor(selected);
        repaint();
    }
}

void ReverbToneEQ::paint(juce::Graphics& g)
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
        g.setColour(juce::Colours::yellow.withAlpha(0.95f));
        for (const auto& pt : points)
        {
            const float x = mapHzToX(pt.hz);
            const float y = mapDbToY(pt.db);
            g.fillEllipse(x-8, y-8, 16, 16);
        }
        
        // Selected point highlight
        if (selected >= 0 && selected < (int)points.size())
        {
            const auto& pt = points[(size_t)selected];
            const float x = mapHzToX(pt.hz);
            const float y = mapDbToY(pt.db);
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.drawEllipse(x-12, y-12, 24, 24, 1.6f);
        }
    }
}

void ReverbToneEQ::resized()
{
    auto r = getLocalBounds().reduced(6);
    analyzer.setBounds(r);
    rebuildEqPath();
}

void ReverbToneEQ::rebuildEqPath()
{
    eqPath.clear();
    bandPaths.clear();
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;

    const int N = juce::jmax(128, (int)r.getWidth());
    
    // Build combined EQ curve
    auto totalDbAt = [this](double hz)
    {
        float s = 0.0f;
        for (const auto& b : points) 
        {
            s += bandDbAtForPaint(b, (float)hz);
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
        auto mapBand = [&](int i){
            const double minHz = 20.0, maxHz = 20000.0;
            const double t = (double)i / (double)(N - 1);
            const double a = std::log10(minHz), b = std::log10(maxHz);
            const double logF = juce::jmap(t, 0.0, 1.0, a, b);
            const double hz = std::pow(10.0, logF);
            return std::pair<float,float>((float)hz, mapDbToY(bandDbAtForPaint(points[bi], (float)hz)));
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

void ReverbToneEQ::drawUnits(juce::Graphics& g)
{
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;
    
    g.setFont(12.0f);
    auto gridCol = juce::Colours::white.withAlpha(0.10f);
    auto textCol = juce::Colours::white.withAlpha(0.45f);
    g.setColour(gridCol);

    // dB ticks
    const float halfRange = zoomState.getCurrent();
    const float dbVals[] = { 18, 12, 6, 0, -6, -12, -18, -24, -30, -36 };
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


// Mapping helpers
float ReverbToneEQ::mapHzToX(float hz) const
{
    auto r = analyzer.getBounds().toFloat();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = (float)(std::log10(juce::jlimit(minHz, maxHz, hz) / minHz) / std::log10(maxHz / minHz));
    return r.getX() + t * r.getWidth();
}

float ReverbToneEQ::mapDbToY(float dB) const
{
    auto r = analyzer.getBounds().toFloat();
    const float top = r.getY()+8.f, bottom = r.getBottom()-8.f;
    const float halfRange = zoomState.getCurrent();
    return juce::jmap(dB, +halfRange, -halfRange, top, bottom);
}

float ReverbToneEQ::mapXToHz(int px) const
{
    auto r = analyzer.getBounds();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = juce::jlimit(0.0f, 1.0f, (px - (float)r.getX()) / (float)r.getWidth());
    const float a = std::log10(minHz), b = std::log10(maxHz);
    return std::pow(10.0f, juce::jmap(t, 0.0f, 1.0f, a, b));
}

float ReverbToneEQ::mapYToDb(int py) const
{
    auto r = analyzer.getBounds();
    const float halfRange = zoomState.getCurrent();
    return juce::jmap((float)py, (float)r.getY(), (float)r.getBottom(), +halfRange, -halfRange);
}

// Hit testing
int ReverbToneEQ::hitTestPoint(juce::Point<int> p) const
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

// Band management
int ReverbToneEQ::allocateBandSlot()
{
    for (int i = 0; i < kMaxBands; ++i)
    {
        auto id = bandId(ReverbEQParams::ToneBand::active, i);
        if (auto* v = proc.apvts.getRawParameterValue(id))
        {
            if (v->load() < 0.5f)
                return i;
        }
    }
    return -1;
}

void ReverbToneEQ::setBandParam(int bandIdx, const char* baseId, float value)
{
    auto id = bandId(baseId, bandIdx);
    if (auto* p = proc.apvts.getParameter(id))
    {
        const float norm = p->convertTo0to1(value);
        p->setValueNotifyingHost(norm);
    }
}

float ReverbToneEQ::getBandParamFloat(int bandIdx, const char* baseId, float fallback) const
{
    auto id = bandId(baseId, bandIdx);
    if (auto* v = proc.apvts.getRawParameterValue(id)) return v->load();
    return fallback;
}

// Visual helpers
juce::Colour ReverbToneEQ::bandColourFor(int bandIdx) const
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

float ReverbToneEQ::bandDbAtForPaint(const BandPoint& b, float hz) const
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
            return b.db * w;
        }
        case 1: { // Low Shelf
            const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC)));
            return (float)(b.db * s);
        }
        case 2: { // High Shelf
            const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC)));
            return (float)(b.db * (1.0 - s));
        }
        default: return 0.0f;
    }
}

// Per-band module implementations
void ReverbToneEQ::positionOverlay()
{
    if (selected < 0 || selected >= (int)points.size()) return;
    
    auto r = getLocalBounds();
    const int w = 200, h = 120;
    
    // Get the selected band point position
    const auto& pt = points[(size_t)selected];
    const float bandX = mapHzToX(pt.hz);
    const float bandY = mapDbToY(pt.db);
    
    // Start with band point position
    int ox = (int)bandX - w/2;
    int oy = (int)bandY - h/2;
    
    // Smart positioning to avoid overlap with band point
    const int bandRadius = 12; // Band point click radius
    const int margin = 20; // Additional margin from band point
    
    // Check if overlay would overlap with band point
    bool overlapsBand = (ox <= bandX + bandRadius + margin && 
                        ox + w >= bandX - bandRadius - margin &&
                        oy <= bandY + bandRadius + margin && 
                        oy + h >= bandY - bandRadius - margin);
    
    if (overlapsBand)
    {
        // Position overlay to the right of band point
        ox = (int)bandX + bandRadius + margin;
        oy = (int)bandY - h/2;
        
        // If that goes off screen, try to the left
        if (ox + w > r.getRight())
        {
            ox = (int)bandX - w - bandRadius - margin;
        }
        
        // If still off screen, try above
        if (ox < r.getX() || ox + w > r.getRight())
        {
            ox = (int)bandX - w/2;
            oy = (int)bandY - h - bandRadius - margin;
        }
        
        // If still off screen, try below
        if (oy < r.getY())
        {
            oy = (int)bandY + bandRadius + margin;
        }
    }
    
    // Final bounds checking
    if (ox < r.getX()) ox = r.getX() + 10;
    if (ox + w > r.getRight()) ox = r.getRight() - w - 10;
    if (oy < r.getY()) oy = r.getY() + 10;
    if (oy + h > r.getBottom()) oy = r.getBottom() - h - 10;
    
    overlay.setBounds(ox, oy, w, h);
    overlay.setVisible(true);
}

void ReverbToneEQ::positionBadgeFor(int idx)
{
    if (idx < 0 || idx >= (int)points.size()) return;
    const auto& pt = points[(size_t)idx];
    const float x = mapHzToX(pt.hz);
    const float y = mapDbToY(pt.db);
    
    const int w = 120, h = 60;
    auto pane = analyzer.getBounds();
    
    // Smart positioning to avoid overlap with band point
    const int bandRadius = 12;
    const int margin = 15;
    
    // Try positioning to the right first
    int ox = (int)x + bandRadius + margin;
    int oy = (int)y - h/2;
    
    // If that goes off screen, try to the left
    if (ox + w > pane.getRight())
    {
        ox = (int)x - w - bandRadius - margin;
    }
    
    // If still off screen, try above
    if (ox < pane.getX())
    {
        ox = (int)x - w/2;
        oy = (int)y - h - bandRadius - margin;
    }
    
    // If still off screen, try below
    if (oy < pane.getY())
    {
        oy = (int)y + bandRadius + margin;
    }
    
    // Final bounds checking
    if (ox < pane.getX()) ox = pane.getX() + 5;
    if (ox + w > pane.getRight()) ox = pane.getRight() - w - 5;
    if (oy < pane.getY()) oy = pane.getY() + 5;
    if (oy + h > pane.getBottom()) oy = pane.getBottom() - h - 5;
    
    badge.setBounds(ox, oy, w, h);
    badge.setVisible(true);
    badge.setValues(0.0f, pt.hz, pt.type, false);
    badge.setDetails(pt.q, pt.db, false, false, 0.0f, false, "St", 0, "Pre");
    
    // Per-band accent color
    juce::Colour accent = bandColourFor(idx);
    badge.setAccentColour(accent);
    overlay.setAccentColour(accent);
    badge.toFront(true);
}

// BandOverlay implementation
ReverbToneEQ::BandOverlay::BandOverlay()
{
    setInterceptsMouseClicks(true, true);
    
    // Setup sliders
    gain.setSliderStyle(juce::Slider::LinearHorizontal);
    gain.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    gain.setRange(-24.0, 24.0, 0.1);
    gain.onValueChange = [this] { if (!updating && onGainChanged) onGainChanged((float)gain.getValue()); };
    gain.onDragStart = [this] { if (onDragAny) onDragAny(true); };
    gain.onDragEnd = [this] { if (onDragAny) onDragAny(false); };
    addAndMakeVisible(gain);
    
    q.setSliderStyle(juce::Slider::LinearHorizontal);
    q.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    q.setRange(0.1, 36.0, 0.01);
    q.onValueChange = [this] { if (!updating && onQChanged) onQChanged((float)q.getValue()); };
    q.onDragStart = [this] { if (onDragAny) onDragAny(true); };
    q.onDragEnd = [this] { if (onDragAny) onDragAny(false); };
    addAndMakeVisible(q);
    
    freq.setSliderStyle(juce::Slider::LinearHorizontal);
    freq.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 18);
    freq.setRange(20.0, 20000.0, 0.01);
    freq.setSkewFactorFromMidPoint(1000.0);
    freq.onValueChange = [this] { if (!updating && onFreqChanged) onFreqChanged((float)freq.getValue()); };
    freq.onDragStart = [this] { if (onDragAny) onDragAny(true); };
    freq.onDragEnd = [this] { if (onDragAny) onDragAny(false); };
    addAndMakeVisible(freq);
    
    // Setup labels
    gainLabel.setText("GAIN", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(gainLabel);
    
    qLabel.setText("Q", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(qLabel);
    
    freqLabel.setText("FREQ", juce::dontSendNotification);
    freqLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(freqLabel);
    
    // Setup type combo
    typeCb.addItemList(juce::StringArray{"Bell", "LowShelf", "HighShelf"}, 1);
    typeCb.onChange = [this] { if (!updating && onTypeChanged) onTypeChanged(typeCb.getSelectedItemIndex()); };
    addAndMakeVisible(typeCb);
    
    typeLabel.setText("TYPE", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(typeLabel);
}

ReverbToneEQ::BandOverlay::~BandOverlay() {}

void ReverbToneEQ::BandOverlay::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    
    // Background with accent
    juce::Colour bg = juce::Colours::darkgrey.darker(0.20f);
    g.setColour(bg.withAlpha(0.96f));
    g.fillRoundedRectangle(r, 8.0f);
    
    // Accent border
    g.setColour(accentColour.withAlpha(0.8f));
    g.drawRoundedRectangle(r, 8.0f, 2.0f);
    
    // Accent strip
    juce::Rectangle<float> strip = r.removeFromLeft(3.0f).reduced(0.5f, 2.0f);
    g.setColour(accentColour);
    g.fillRoundedRectangle(strip, 1.5f);
}

void ReverbToneEQ::BandOverlay::resized()
{
    auto r = getLocalBounds().reduced(8);
    const int labelW = 40, sliderH = 20, gap = 4;
    
    // Gain row
    gainLabel.setBounds(r.removeFromTop(sliderH).removeFromLeft(labelW));
    gain.setBounds(r.removeFromTop(sliderH));
    r.removeFromTop(gap);
    
    // Q row
    qLabel.setBounds(r.removeFromTop(sliderH).removeFromLeft(labelW));
    q.setBounds(r.removeFromTop(sliderH));
    r.removeFromTop(gap);
    
    // Freq row
    freqLabel.setBounds(r.removeFromTop(sliderH).removeFromLeft(labelW));
    freq.setBounds(r.removeFromTop(sliderH));
    r.removeFromTop(gap);
    
    // Type row
    typeLabel.setBounds(r.removeFromTop(sliderH).removeFromLeft(labelW));
    typeCb.setBounds(r.removeFromTop(sliderH));
}

void ReverbToneEQ::BandOverlay::setValues(float gainVal, float qVal, float freqVal, int type)
{
    updating = true;
    gain.setValue(gainVal, juce::dontSendNotification);
    q.setValue(qVal, juce::dontSendNotification);
    freq.setValue(freqVal, juce::dontSendNotification);
    typeCb.setSelectedItemIndex(type, juce::dontSendNotification);
    updating = false;
}

void ReverbToneEQ::BandOverlay::setAccentColour(juce::Colour c)
{
    accentColour = c;
    repaint();
}

// BandBadge implementation
ReverbToneEQ::BandBadge::BandBadge()
{
    setInterceptsMouseClicks(true, true);
    
    // Setup buttons
    deleteBtn.setButtonText("×");
    deleteBtn.onClick = [this] { if (onDelete) onDelete(); };
    addAndMakeVisible(deleteBtn);
    
    bypassBtn.setButtonText("BYP");
    bypassBtn.onClick = [this] { if (onBypass) onBypass(bypassBtn.getToggleState()); };
    addAndMakeVisible(bypassBtn);
    
    typeBtn.setButtonText("Bell");
    typeBtn.onClick = [this] { if (onSetType) onSetType(0); };
    addAndMakeVisible(typeBtn);
    
    // Setup labels
    freqLabel.setText("1.0k", juce::dontSendNotification);
    freqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(freqLabel);
    
    gainLabel.setText("0.0", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);
    
    qLabel.setText("0.7", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(qLabel);
}

ReverbToneEQ::BandBadge::~BandBadge() {}

void ReverbToneEQ::BandBadge::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    
    // Background
    juce::Colour bg = juce::Colours::darkgrey.darker(0.30f);
    g.setColour(bg.withAlpha(0.95f));
    g.fillRoundedRectangle(r, 6.0f);
    
    // Accent border
    g.setColour(accentColour.withAlpha(0.9f));
    g.drawRoundedRectangle(r, 6.0f, 1.5f);
    
    // Accent strip
    juce::Rectangle<float> strip = r.removeFromLeft(2.0f).reduced(0.5f, 1.0f);
    g.setColour(accentColour);
    g.fillRoundedRectangle(strip, 1.0f);
}

void ReverbToneEQ::BandBadge::resized()
{
    auto r = getLocalBounds().reduced(4);
    const int btnW = 24, btnH = 16;
    
    // Top row: buttons
    deleteBtn.setBounds(r.removeFromTop(btnH).removeFromLeft(btnW));
    bypassBtn.setBounds(r.removeFromTop(btnH).removeFromLeft(btnW));
    typeBtn.setBounds(r.removeFromTop(btnH).removeFromLeft(btnW));
    
    // Bottom row: labels
    freqLabel.setBounds(r.removeFromTop(btnH));
    gainLabel.setBounds(r.removeFromTop(btnH));
    qLabel.setBounds(r.removeFromTop(btnH));
}

void ReverbToneEQ::BandBadge::setValues(float gr, float freq, int type, bool bypass)
{
    currentGr = gr;
    currentFreq = freq;
    currentType = type;
    currentBypass = bypass;
    
    freqLabel.setText(juce::String(freq, freq >= 1000.0f ? 1 : 0) + (freq >= 1000.0f ? "k" : ""), juce::dontSendNotification);
    bypassBtn.setToggleState(bypass, juce::dontSendNotification);
    
    const char* typeNames[] = {"Bell", "LS", "HS"};
    typeBtn.setButtonText(typeNames[juce::jlimit(0, 2, type)]);
}

void ReverbToneEQ::BandBadge::setDetails(float q, float gain, bool dynOn, bool dynUp, float dynRange, bool specOn, const juce::String& channel, int slopeDb, const juce::String& tap)
{
    currentQ = q;
    currentGain = gain;
    
    gainLabel.setText(juce::String(gain, 1), juce::dontSendNotification);
    qLabel.setText(juce::String(q, 2), juce::dontSendNotification);
}

void ReverbToneEQ::BandBadge::setAccentColour(juce::Colour c)
{
    accentColour = c;
    repaint();
}