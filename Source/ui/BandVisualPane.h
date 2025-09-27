#pragma once
#include <JuceHeader.h>
#include "StereoFieldEngine.h"
#include "../Core/FieldLookAndFeel.h"

// Band Visual Pane: Width mode visuals for Band tab
class BandVisualPane : public juce::Component, private juce::Timer
{
public:
    BandVisualPane();
    ~BandVisualPane() override;

    void setSampleRate (double sr);
    void setWidths (float lo, float mid, float hi);
    void setCrossovers (float loHz, float hiHz);
    void setShuffler (float loPct, float hiPct, float xHz);
    void pushBlock (const float* L, const float* R, int n, bool isPre);
    void resized() override;
    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override;

    void drawGrid (juce::Graphics& g, juce::Rectangle<float> r)
    {
        auto panel = juce::Colours::white.withAlpha (0.10f);
        g.setColour (panel);
        auto inner = r.reduced (8.0f);
        g.drawRoundedRectangle (inner, 6.0f, 1.0f);
        // Removed horizontal and vertical divider lines that were interfering with Band visuals
    }

    void drawWidthEditor (juce::Graphics& g, juce::Rectangle<float> r)
    {
        auto* lf = dynamic_cast<juce::LookAndFeel_V4*>(&getLookAndFeel());
        auto grid = juce::Colours::white.withAlpha (0.12f);
        auto acc  = juce::Colours::aqua.withAlpha (0.80f);
        g.setColour (grid);
        g.drawRoundedRectangle (r, 6.0f, 1.0f);

        // Log freq mapping
        auto xAtHz = [&](float hz)
        {
            const float minHz = 20.0f, maxHz = 20000.0f;
            const float t = (float) (std::log10 (juce::jlimit (minHz, maxHz, hz) / minHz) / std::log10 (maxHz / minHz));
            return juce::jmap (t, 0.0f, 1.0f, r.getX(), r.getRight());
        };

        const float xLo = xAtHz (xoverLoHz);
        const float xHi = xAtHz (xoverHiHz);

        // Bars per band (drawn first, behind center lines)
        auto drawBand = [&](float x0, float x1, float width, juce::Colour c)
        {
            width = juce::jlimit (0.0f, 2.0f, width);
            const float h = juce::jmap (width, 0.0f, 2.0f, r.getHeight() * 0.08f, r.getHeight());
            juce::Rectangle<float> bar (x0, r.getBottom() - h, juce::jmax (6.0f, x1 - x0), h);
            g.setColour (c.withAlpha (0.30f)); g.fillRoundedRectangle (bar, 4.0f);
            g.setColour (c.withAlpha (0.95f)); g.drawRoundedRectangle (bar, 4.0f, 2.0f);
        };

        const auto cLo  = juce::Colours::aqua;
        const auto cMid = juce::Colours::yellow;
        const auto cHi  = juce::Colours::orange;
        drawBand (r.getX(), xLo, widthLo, cLo);
        drawBand (xLo, xHi, widthMid, cMid);
        drawBand (xHi, r.getRight(), widthHi, cHi);

        // LO MID HI labels at the top of the visual area
        auto loRect  = juce::Rectangle<float> (r.getX(), r.getY() + 4.0f, xLo - r.getX(), 16.0f);
        auto midRect = juce::Rectangle<float> (xLo,      r.getY() + 4.0f, xHi - xLo,      16.0f);
        auto hiRect  = juce::Rectangle<float> (xHi,      r.getY() + 4.0f, r.getRight()-xHi,16.0f);

        auto badge = [&] (juce::Rectangle<float> rect, const juce::String& txt)
        {
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            auto bg = (lf ? lf->theme.accent : juce::Colours::cornflowerblue).withAlpha (0.35f);
            g.setColour (bg);
            g.fillRoundedRectangle (rect.reduced (2.0f), 6.0f);
            g.setColour (bg.darker (0.35f));
            g.drawRoundedRectangle (rect.reduced (2.0f), 6.0f, 1.0f);
            g.setColour (lf ? lf->theme.text : juce::Colours::white);
            g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
            g.drawText (txt, rect, juce::Justification::centred);
        };
        badge (loRect,  "LO");
        badge (midRect, "MID");
        badge (hiRect,  "HI");

    }

    // Background waveform for Width view (mono Mid and Side envelopes)
    void drawWidthWaveform (juce::Graphics& g, juce::Rectangle<float> r)
    {
        const int N = (int) fifoPost.size(); if (N <= 8) return;
        auto mapX = [&](int i){ return juce::jmap ((float) i, 0.0f, (float) N-1, r.getX(), r.getRight()); };
        auto mapY = [&](float v, float scale){ return r.getCentreY() - v * scale * (r.getHeight() * 0.40f); };
        const float target = 0.70f;
        const float scale = autoGain ? (rmsPost > 1.0e-6f ? target / rmsPost : 1.0f) : 1.0f;
        juce::Path mPath, sPath;
        for (int i = 0; i < N; ++i)
        {
            const auto pr = fifoPost[(size_t) i];
            const float L = juce::jlimit (-1.5f, 1.5f, pr.first);
            const float R = juce::jlimit (-1.5f, 1.5f, pr.second);
            const float M = 0.5f * (L + R);
            const float S = 0.5f * (L - R);
            const float x = mapX (i);
            const float yM = mapY (M, scale);
            const float yS = mapY (S, scale);
            if (i == 0) { mPath.startNewSubPath (x, yM); sPath.startNewSubPath (x, yS); }
            else        { mPath.lineTo (x, yM);          sPath.lineTo (x, yS); }
        }
        g.setColour (juce::Colours::white.withAlpha (0.10f)); g.strokePath (mPath, juce::PathStrokeType (1.0f));
        g.setColour (juce::Colours::aqua .withAlpha (0.10f)); g.strokePath (sPath, juce::PathStrokeType (1.0f));
    }

    // Live measured width overlay using engine (|S|/(|M|+eps) per band)
    void drawWidthOverlay (juce::Graphics& g, juce::Rectangle<float> r)
    {
        const int B = engine.getBandCount(); if (B <= 0) return;
        const auto& wp = engine.getWidthPerBandPost(); if ((int) wp.size() < B) return;
        // polyline across bands mapped to X by band center and Y by width
        auto xAtHz = [&](double hz){ const double minHz=20.0, maxHz=20000.0; double t=(std::log10 (juce::jlimit(minHz,maxHz,hz)/minHz)/std::log10(maxHz/minHz)); return juce::jmap ((float)t, 0.0f,1.0f, r.getX(), r.getRight()); };
        auto yAtW  = [&](float w){ const float h = juce::jmap (juce::jlimit (0.0f,2.0f,w), 0.0f, 2.0f, r.getHeight()*0.08f, r.getHeight()); return r.getBottom() - h; };
        juce::Path p; bool started=false;
        for (int bi = 0; bi < B; ++bi)
        {
            const double fC = engine.getBandCenterHz (bi);
            const float  wv = wp[(size_t) bi];
            const float  x  = (float) xAtHz (fC);
            const float  y  = yAtW (wv);
            if (!started) { p.startNewSubPath (x, y); started=true; } else { p.lineTo (x, y); }
        }
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Hints: draggable arrows on XO lines and subtle arrows on bars
    void drawWidthHints (juce::Graphics& g, juce::Rectangle<float> r)
    {
        auto xAtHz = [&](float hz){ const float minHz=20.0f, maxHz=20000.0f; const float t=(float)(std::log10 (juce::jlimit(minHz,maxHz,hz)/minHz)/std::log10(maxHz/minHz)); return juce::jmap (t, 0.0f,1.0f, r.getX(), r.getRight()); };
        const float xLo = xAtHz (xoverLoHz);
        const float xHi = xAtHz (xoverHiHz);
        auto drawArrow = [&](float x)
        {
            juce::Path a; const float yC = r.getY() + r.getHeight() * 0.30f; const float sz=6.0f; // lift arrows higher
            a.startNewSubPath (x - 10, yC); a.lineTo (x - 2, yC);
            a.addTriangle (x - 2, yC - 4, x - 2, yC + 4, x - 10, yC);
            a.startNewSubPath (x + 2, yC); a.lineTo (x + 10, yC);
            a.addTriangle (x + 2, yC - 4, x + 2, yC + 4, x + 10, yC);
            g.setColour (juce::Colours::white.withAlpha (0.35f)); g.strokePath (a, juce::PathStrokeType (1.2f));
        };
        drawArrow (xLo); drawArrow (xHi);

        // Vertical drag hints for band value lines (up/down arrows at band centers)
        auto yAtW  = [&](float w){ const float h = juce::jmap (juce::jlimit (0.0f,2.0f,w), 0.0f, 2.0f, r.getHeight()*0.08f, r.getHeight()); return r.getBottom() - h; };
        const float cxLo  = (r.getX() + xLo) * 0.5f;
        const float cxMid = (xLo + xHi) * 0.5f;
        const float cxHi  = (xHi + r.getRight()) * 0.5f;
        auto drawVert = [&](float cx, float w)
        {
            const float y = yAtW (w);
            juce::Path p;
            // Up arrow
            p.startNewSubPath (cx, y - 12.0f); p.lineTo (cx, y - 4.0f);
            p.addTriangle (cx - 4.0f, y - 8.0f, cx + 4.0f, y - 8.0f, cx, y - 14.0f);
            // Down arrow
            p.startNewSubPath (cx, y + 4.0f); p.lineTo (cx, y + 12.0f);
            p.addTriangle (cx - 4.0f, y + 8.0f, cx + 4.0f, y + 8.0f, cx, y + 14.0f);
            g.setColour (juce::Colours::white.withAlpha (0.35f));
            g.strokePath (p, juce::PathStrokeType (1.2f));
        };
        drawVert (cxLo,  widthLo);
        drawVert (cxMid, widthMid);
        drawVert (cxHi,  widthHi);
    }

    void drawShufflerStrip (juce::Graphics& g, juce::Rectangle<float> r)
    {
        // Shuffler width strip (bottom, 6x taller - doubled from 36px to 72px)
        auto band = r.removeFromBottom (72.0f);
        auto xAtHz = [&](float hz){ const float minHz=20.0f, maxHz=20000.0f; const float t=(float)(std::log10(juce::jlimit(minHz,maxHz,hz)/minHz)/std::log10(maxHz/minHz)); return juce::jmap(t,0.0f,1.0f,band.getX(),band.getRight()); };
        const float xX = xAtHz (juce::jlimit (150.0f, 2000.0f, shufXHz));

        auto widthH = [&] (float pct)
        {
            const float h = juce::jmap (juce::jlimit (0.0f, 200.0f, pct), 0.0f, 200.0f, 8.0f, band.getHeight() - 8.0f);
            return juce::jmax (8.0f, h);
        };

        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto gridCol = lf ? lf->theme.text : juce::Colours::white;
        
        // More sophisticated colors using theme palette
        auto loColor = lf ? lf->theme.eq.bass : juce::Colour (0xFF7FB069); // Moss green
        auto hiColor = lf ? lf->theme.eq.air : juce::Colour (0xFF8B6FA1);  // Plum purple
        auto accentColor = lf ? lf->theme.accent : juce::Colour (0xFF5AA9E6);

        // Background grid
        g.setColour (gridCol.withAlpha (0.15f));
        g.fillRect (band);

        // Left segment (Lo%) with sophisticated color
        g.setColour (loColor.withAlpha (0.4f));
        g.fillRect (juce::Rectangle<float> (band.getX(), band.getBottom() - widthH (shufLoPct), xX - band.getX(), widthH (shufLoPct)));
        
        // Right segment (Hi%) with sophisticated color  
        g.setColour (hiColor.withAlpha (0.4f));
        g.fillRect (juce::Rectangle<float> (xX, band.getBottom() - widthH (shufHiPct), band.getRight() - xX, widthH (shufHiPct)));

        // Border around segments
        g.setColour (loColor.withAlpha (0.7f));
        g.drawRect (juce::Rectangle<float> (band.getX(), band.getBottom() - widthH (shufLoPct), xX - band.getX(), widthH (shufLoPct)), 1.0f);
        
        g.setColour (hiColor.withAlpha (0.7f));
        g.drawRect (juce::Rectangle<float> (xX, band.getBottom() - widthH (shufHiPct), band.getRight() - xX, widthH (shufHiPct)), 1.0f);

        // Center lines drawn on top of segments (always visible)
        // Center vertical line (always visible)
        g.setColour (gridCol.withAlpha (0.6f));
        g.drawVerticalLine (juce::roundToInt (xX), band.getY(), band.getBottom());
        
        // Center horizontal line (always visible)
        const float centerY = band.getY() + band.getHeight() * 0.5f;
        g.setColour (gridCol.withAlpha (0.4f));
        g.drawHorizontalLine (juce::roundToInt (centerY), band.getX(), band.getRight());
        
        // Additional horizontal grid lines
        g.setColour (gridCol.withAlpha (0.2f));
        for (int i = 1; i <= 2; ++i)
        {
            const float y = band.getY() + i * (band.getHeight() / 3.0f);
            g.drawHorizontalLine (juce::roundToInt (y), band.getX(), band.getRight());
        }

        // Units and labels
        g.setColour (gridCol.withAlpha (0.8f));
        g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")));
        
        // LO label
        g.drawText ("LO", juce::Rectangle<float> (band.getX() + 4, band.getY() + 2, 20, 12), juce::Justification::centredLeft);
        
        // HI label  
        g.drawText ("HI", juce::Rectangle<float> (band.getRight() - 24, band.getY() + 2, 20, 12), juce::Justification::centredRight);
        
        // Percentage values
        g.setFont (juce::Font (juce::FontOptions (9.0f)));
        g.setColour (gridCol.withAlpha (0.7f));
        
        // LO percentage
        g.drawText (juce::String (shufLoPct, 0) + "%", 
                   juce::Rectangle<float> (band.getX() + 4, centerY - 6, 30, 12), 
                   juce::Justification::centredLeft);
        
        // HI percentage
        g.drawText (juce::String (shufHiPct, 0) + "%", 
                   juce::Rectangle<float> (band.getRight() - 34, centerY - 6, 30, 12), 
                   juce::Justification::centredRight);
        
        // Crossover frequency
        g.drawText (juce::String (shufXHz, 0) + " Hz", 
                   juce::Rectangle<float> (xX - 25, band.getY() + 2, 50, 12), 
                   juce::Justification::centred);
    }
    
    // Draw center lines on top of everything
    void drawCenterLines (juce::Graphics& g, juce::Rectangle<float> r)
    {
        auto xAtHz = [&](float hz)
        {
            const float minHz = 20.0f, maxHz = 20000.0f;
            const float t = (float) (std::log10 (juce::jlimit (minHz, maxHz, hz) / minHz) / std::log10 (maxHz / minHz));
            return juce::jmap (t, 0.0f, 1.0f, r.getX(), r.getRight());
        };
        
        const float xLo = xAtHz (xoverLoHz);
        const float xHi = xAtHz (xoverHiHz);
        
        auto grid = juce::Colours::white.withAlpha (0.12f);
        
        // Draw crossovers (draggable) - drawn on top of everything
        auto drawX = [&](float x){ juce::Path p; p.startNewSubPath (x, r.getY()); p.lineTo (x, r.getBottom());
                                   const float dashes[] = { 5.0f, 4.0f }; juce::Path dashed; juce::PathStrokeType (1.2f).createDashedStroke (dashed, p, dashes, 2);
                                   g.setColour (grid.withAlpha (0.8f)); g.strokePath (dashed, juce::PathStrokeType (1.6f)); };
        drawX (xLo); drawX (xHi);
    }

    // Data members
    double sampleRate = 48000.0;
    float widthLo = 1.0f, widthMid = 1.0f, widthHi = 1.0f;
    float xoverLoHz = 150.0f, xoverHiHz = 2000.0f;
    // Shuffle parameters
    float shufLoPct = 100.0f, shufHiPct = 100.0f, shufXHz = 700.0f;
    
    // Audio data
    std::vector<std::pair<float,float>> fifoPre, fifoPost;
    int writePre = 0, writePost = 0;
    bool havePre = false, havePost = false;
    float rmsPost = 0.0f, rmsPre = 0.0f;
    bool autoGain = true;
    juce::SpinLock dataLock;
    
    // Engine integration
    StereoFieldEngine engine;
    bool enginePrepared = false;
    
    // Interaction state for width editor
    enum class DragKind { None, XLo, XHi, BandLo, BandMid, BandHi };
    DragKind drag { DragKind::None };
    float dragStartX = 0.0f;
    float dragStartVal = 0.0f;
    
    // Mouse interaction for draggable crossovers and width values
    void mouseDown (const juce::MouseEvent& e) override
    {
        auto r = getLocalBounds().toFloat().reduced (8.0f);
        auto xAtHz = [&](float hz){ const float minHz=20.0f, maxHz=20000.0f; const float t=(float)(std::log10(juce::jlimit(minHz,maxHz,hz)/minHz)/std::log10(maxHz/minHz)); return juce::jmap(t,0.0f,1.0f,r.getX(),r.getRight()); };
        const float xLo = xAtHz (xoverLoHz);
        const float xHi = xAtHz (xoverHiHz);
        const float px = e.position.x;
        const float hit = 8.0f;
        if (std::abs (px - xLo) < hit) { drag = DragKind::XLo; dragStartX = px; dragStartVal = xoverLoHz; return; }
        if (std::abs (px - xHi) < hit) { drag = DragKind::XHi; dragStartX = px; dragStartVal = xoverHiHz; return; }
        // Band region selection: prefer vertical drag cursor with higher hit radius near current value line
        if (px < xLo) drag = DragKind::BandLo;
        else if (px < xHi) drag = DragKind::BandMid;
        else drag = DragKind::BandHi;
        dragStartX = e.position.y; // use Y for width edit
        if (drag == DragKind::BandLo)  dragStartVal = widthLo;
        if (drag == DragKind::BandMid) dragStartVal = widthMid;
        if (drag == DragKind::BandHi)  dragStartVal = widthHi;
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
    }
    
    void mouseMove (const juce::MouseEvent& e) override
    {
        auto r = getLocalBounds().toFloat().reduced (8.0f);
        auto xAtHz = [&](float hz){ const float minHz=20.0f, maxHz=20000.0f; const float t=(float)(std::log10(juce::jlimit(minHz,maxHz,hz)/minHz)/std::log10(maxHz/minHz)); return juce::jmap(t,0.0f,1.0f,r.getX(),r.getRight()); };
        auto yAtW  = [&](float w){ const float h = juce::jmap (juce::jlimit (0.0f,2.0f,w), 0.0f, 2.0f, r.getHeight()*0.08f, r.getHeight()); return r.getBottom() - h; };
        const float xLo = xAtHz (xoverLoHz);
        const float xHi = xAtHz (xoverHiHz);
        const float px = e.position.x;
        const float py = e.position.y;
        const float hitX = 8.0f;
        const bool nearX = (std::abs (px - xLo) < hitX) || (std::abs (px - xHi) < hitX);
        // Treat entire band region as draggable area
        bool overBand = (px >= r.getX() && px < xLo) || (px >= xLo && px < xHi) || (px >= xHi && px <= r.getRight());
        setMouseCursor ((nearX || overBand) ? juce::MouseCursor::PointingHandCursor
                                            : juce::MouseCursor::NormalCursor);
    }
    
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (drag == DragKind::None) return;
        auto r = getLocalBounds().toFloat().reduced (8.0f);
        auto hzAtX = [&](float x){ const float minHz=20.0f, maxHz=20000.0f; const float t=juce::jlimit(0.0f,1.0f, juce::jmap(x, r.getX(), r.getRight(), 0.0f, 1.0f)); return minHz * std::pow (maxHz/minHz, t); };
        if (drag == DragKind::XLo)
        {
            const float newHz = juce::jlimit (40.0f, juce::jmin (xoverHiHz - 10.0f, 400.0f), hzAtX (e.position.x));
            xoverLoHz = newHz; if (onParamEdit) onParamEdit ("xover_lo_hz", xoverLoHz); repaint(); return;
        }
        if (drag == DragKind::XHi)
        {
            const float newHz = juce::jlimit (juce::jmax (xoverLoHz + 10.0f, 800.0f), 6000.0f, hzAtX (e.position.x));
            xoverHiHz = newHz; if (onParamEdit) onParamEdit ("xover_hi_hz", xoverHiHz); repaint(); return;
        }
        // Bands: map vertical drag to width 0..2
        const float dy = (dragStartX - e.position.y) / juce::jmax (1.0f, r.getHeight());
        const float delta = dy * 2.0f; // 1.0 height => +/-2.0 width
        float v = juce::jlimit (0.0f, 2.0f, dragStartVal + delta);
        if (drag == DragKind::BandLo)  { widthLo = v;  if (onParamEdit) onParamEdit ("width_lo",  widthLo); }
        if (drag == DragKind::BandMid) { widthMid = v; if (onParamEdit) onParamEdit ("width_mid", widthMid); }
        if (drag == DragKind::BandHi)  { widthHi = v;  if (onParamEdit) onParamEdit ("width_hi",  widthHi); }
        repaint();
    }
    
    void mouseUp (const juce::MouseEvent&) override { drag = DragKind::None; setMouseCursor (juce::MouseCursor::NormalCursor); }
    
public:
    // Callbacks
    std::function<void(const juce::String&, const juce::var&)> onUiChange;
    std::function<void(const juce::String&, float)> onParamEdit;
};