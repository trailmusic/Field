#pragma once
#include <JuceHeader.h>
#include "StereoFieldEngine.h"
#include "../KnobCellMini.h"

// Minimal Imager pane: goniometer (Lissajous) with optional PRE overlay
class ImagerPane : public juce::Component, private juce::Timer
{
public:
    struct Options
    {
        bool showPre = true;
        bool autoGain = true;
        int  fps = 30; // UI refresh
        bool freeze = false;
        enum class Mode { XY, Polar, Width, Heat } mode { Mode::XY };
    };

    ImagerPane()
    {
        setOpaque (false);
        startTimerHz (opts.fps);

        // Tooling UI (right side)
        addAndMakeVisible (preToggle);
        preToggle.setButtonText ("Pre Overlay");
        preToggle.onClick = [this]{
            opts.showPre = preToggle.getToggleState();
            if (onUiChange) onUiChange ("ui_imager_showPre", opts.showPre);
            repaint();
        };

        addAndMakeVisible (freezeToggle);
        freezeToggle.setButtonText ("Freeze");
        freezeToggle.onClick = [this]{
            opts.freeze = freezeToggle.getToggleState();
            if (onUiChange) onUiChange ("ui_imager_freeze", opts.freeze);
        };

        addAndMakeVisible (qualityBox);
        qualityBox.addItem ("15 fps", 15);
        qualityBox.addItem ("30 fps", 30);
        qualityBox.addItem ("60 fps", 60);
        qualityBox.onChange = [this]{
            const int sel = qualityBox.getSelectedId();
            if (sel > 0) { opts.fps = sel; startTimerHz (opts.fps); if (onUiChange) onUiChange ("ui_imager_quality", opts.fps); }
        };

        // Mode buttons (simple sub-tabs)
        addAndMakeVisible (modeXY);   modeXY  .setButtonText ("XY");
        addAndMakeVisible (modePolar);modePolar.setButtonText ("Polar");
        addAndMakeVisible (modeWidth);modeWidth.setButtonText ("Width");
        addAndMakeVisible (modeHeat); modeHeat .setButtonText ("Heat");
        auto setMode = [this](Options::Mode m, const juce::String& v){ opts.mode = m; if (onUiChange) onUiChange ("ui_imager_mode", v); repaint(); resized(); };
        modeXY  .onClick = [=]{ setMode (Options::Mode::XY,   "xy");   };
        modePolar.onClick = [=]{ setMode (Options::Mode::Polar,"polar");};
        modeWidth.onClick = [=]{ setMode (Options::Mode::Width,"width");};
        modeHeat .onClick = [=]{ setMode (Options::Mode::Heat, "heat"); };

        // Floating translucent bottom-center panel (container for Designer controls)
        initDesignerControls();
        overlay = std::make_unique<DesignerOverlay>(*this);
        addAndMakeVisible (*overlay);
        overlay->setVisible (false);
    }

    void setOptions (const Options& o)
    {
        opts = o; startTimerHz (juce::jlimit (10, 120, opts.fps));
        preToggle.setToggleState (opts.showPre, juce::dontSendNotification);
        freezeToggle.setToggleState (opts.freeze, juce::dontSendNotification);
        qualityBox.setSelectedId (juce::jlimit (10, 120, opts.fps), juce::dontSendNotification);
        // highlight current mode
        auto setOn = [&](juce::TextButton& b, bool on){ b.setToggleState (on, juce::dontSendNotification); };
        setOn (modeXY,   opts.mode == Options::Mode::XY);
        setOn (modePolar,opts.mode == Options::Mode::Polar);
        setOn (modeWidth,opts.mode == Options::Mode::Width);
        setOn (modeHeat, opts.mode == Options::Mode::Heat);
        repaint();
    }

    void setSampleRate (double sr) { sampleRate = (sr > 0 ? sr : 48000.0); }
    void setWidths (float lo, float mid, float hi)
    {
        widthLo = juce::jlimit (0.0f, 2.0f, lo);
        widthMid= juce::jlimit (0.0f, 2.0f, mid);
        widthHi = juce::jlimit (0.0f, 2.0f, hi);
        if (opts.mode == Options::Mode::Width) repaint();
    }
    void setCrossovers (float loHz, float hiHz)
    {
        xoverLoHz = juce::jlimit (40.0f, 400.0f, loHz);
        xoverHiHz = juce::jlimit (800.0f, 6000.0f, hiHz);
        if (xoverHiHz <= xoverLoHz) xoverHiHz = juce::jlimit (xoverLoHz + 10.0f, 6000.0f, xoverHiHz);
        if (opts.mode == Options::Mode::Width) repaint();
    }

    // Public accessors for PaneManager polling
    float getWidthLo() const { return widthLo; }
    float getWidthMid() const { return widthMid; }
    float getWidthHi() const { return widthHi; }

    // UI-thread pulls feed in blocks
    void pushBlock (const float* L, const float* R, int n, bool isPre)
    {
        if (n <= 0 || L == nullptr) return;
        const juce::SpinLock::ScopedLockType sl (dataLock);
        auto& buf = isPre ? fifoPre : fifoPost;
        const int cap = (int) buf.size();
        if (cap <= 0) return;
        // Decimate to cap by striding
        const int stride = juce::jmax (1, n / cap);
        int w = (isPre ? writePre : writePost);
        for (int i = 0; i < n; i += stride)
        {
            const float l = L[i];
            const float r = (R ? R[i] : L[i]);
            buf[(size_t) w] = { l, r };
            w = (w + 1) % cap;
        }
        if (isPre) writePre = w; else writePost = w;
        if (isPre) havePre = true; else havePost = true;

        // Update RMS for auto-gain
        const int M = juce::jmin (cap, 256);
        double sum2 = 0.0; int k = 0; int idx = w;
        for (int i = 0; i < M; ++i)
        {
            idx = (idx - 1 + cap) % cap;
            const auto p = buf[(size_t) idx];
            sum2 += (double) p.first * (double) p.first + (double) p.second * (double) p.second;
            ++k;
        }
        const float rms = (k > 0) ? (float) std::sqrt (sum2 / (2.0 * (double) k)) : 0.0f;
        if (!isPre) rmsPost = 0.9f * rmsPost + 0.1f * rms; else rmsPre = 0.9f * rmsPre + 0.1f * rms;

        // Feed heatmap engine
        engine.pushBlock (L, (R ? R : L), n, isPre);
    }

    void resized() override
    {
        const int W = juce::jmax (32, getWidth());
        const int H = juce::jmax (32, getHeight());
        const int cap = juce::jlimit (256, 4096, (W + H));
        const juce::SpinLock::ScopedLockType sl (dataLock);
        fifoPost.assign ((size_t) cap, {0.0f, 0.0f}); writePost = 0; havePost = false;
        fifoPre .assign ((size_t) cap, {0.0f, 0.0f}); writePre  = 0; havePre  = false;

        // Layout tooling area on right
        auto r = getLocalBounds();
        const int sideW = juce::jlimit (120, 220, W / 4);
        auto side = r.removeFromRight (sideW).reduced (8);
        auto row  = [&](int h){ auto a = side.removeFromTop (h); side.removeFromTop (6); return a; };
        preToggle   .setBounds (row (24));
        freezeToggle.setBounds (row (24));
        qualityBox  .setBounds (row (24));
        auto modes = row (28);
        const int mW = (modes.getWidth() - 9) / 4;
        modeXY  .setClickingTogglesState (true);
        modePolar.setClickingTogglesState (true);
        modeWidth.setClickingTogglesState (true);
        modeHeat .setClickingTogglesState (true);
        modeXY  .setBounds (modes.removeFromLeft (mW)); modes.removeFromLeft (3);
        modePolar.setBounds (modes.removeFromLeft (mW)); modes.removeFromLeft (3);
        modeWidth.setBounds (modes.removeFromLeft (mW)); modes.removeFromLeft (3);
        modeHeat .setBounds (modes.removeFromLeft (mW));

        // Resize heatmap history width
        engine.setHistoryWidth (juce::jmax (64, W - sideW - 16));

        // Floating Designer overlay: bottom-centered within main area (outside right panel)
        auto mainArea = r.toFloat();
        const int ow = juce::jlimit (260, 520, (int) std::round (mainArea.getWidth() * 0.70f));
        const int oh = juce::jlimit (110, 180, (int) std::round (mainArea.getHeight() * 0.22f));
        const int ox = (int) mainArea.getX() + ((int) mainArea.getWidth() - ow) / 2;
        const int oy = (int) mainArea.getBottom() - oh - 10;
        if (! overlayBoundsSet)
            overlayBounds = juce::Rectangle<int> (ox, oy, ow, oh);
        // Clamp to main area
        overlayBounds.setPosition ({ juce::jlimit ((int) mainArea.getX()+4, (int) (mainArea.getRight()-overlayBounds.getWidth()-4), overlayBounds.getX()),
                                     juce::jlimit ((int) mainArea.getY()+4, (int) (mainArea.getBottom()-overlayBounds.getHeight()-4), overlayBounds.getY()) });
        if (overlay) overlay->setBounds (overlayBounds);
        if (overlay) overlay->setVisible (opts.mode == Options::Mode::Width);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        auto* lf = dynamic_cast<juce::LookAndFeel_V4*>(&getLookAndFeel());
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (b.reduced (2.0f), 8.0f);

        const juce::SpinLock::ScopedTryLockType tl (dataLock);
        if (!tl.isLocked()) { drawGrid (g, b); return; }
        drawGrid (g, b);

        if (opts.mode == Options::Mode::XY)
        {
            const float target = 0.70f;
            const float scalePost = opts.autoGain ? (rmsPost > 1.0e-6f ? target / rmsPost : 1.0f) : 1.0f;
            const float scalePre  = opts.autoGain ? (rmsPre  > 1.0e-6f ? target / rmsPre  : 1.0f) : 1.0f;
            if (opts.showPre && havePre)
                drawGoniometer (g, b.reduced (8.0f), fifoPre,  writePre,  juce::Colours::white.withAlpha (0.35f), scalePre);
            if (havePost && !opts.freeze)
                drawGoniometer (g, b.reduced (8.0f), fifoPost, writePost, juce::Colours::aqua.withAlpha (0.70f), scalePost);
            drawCorrelationBar (g, b);
        }
        else if (opts.mode == Options::Mode::Polar)
        {
            const float target = 0.70f;
            const float scale = opts.autoGain ? (rmsPost > 1.0e-6f ? target / rmsPost : 1.0f) : 1.0f;
            drawPolarEnergy (g, b.reduced (8.0f), fifoPost, writePost, scale);
        }
        else if (opts.mode == Options::Mode::Width)
        {
            drawWidthWaveform (g, b.reduced (8.0f));
            drawWidthEditor (g, b.reduced (8.0f));
            drawWidthOverlay (g, b.reduced (8.0f));
            drawWidthHints (g, b.reduced (8.0f));
        }
        else if (opts.mode == Options::Mode::Heat)
        {
            drawHeatmap (g, b.reduced (8.0f));
        }
    }

private:
    void timerCallback() override { repaint(); }

    void drawGrid (juce::Graphics& g, juce::Rectangle<float> r)
    {
        auto panel = juce::Colours::white.withAlpha (0.10f);
        g.setColour (panel);
        auto inner = r.reduced (8.0f);
        g.drawRoundedRectangle (inner, 6.0f, 1.0f);
        // mono line
        g.drawLine (inner.getX(), inner.getCentreY(), inner.getRight(), inner.getCentreY(), 0.8f);
        g.drawLine (inner.getCentreX(), inner.getY(), inner.getCentreX(), inner.getBottom(), 0.8f);
    }

    void drawGoniometer (juce::Graphics& g, juce::Rectangle<float> r,
                         const std::vector<std::pair<float,float>>& buf, int w,
                         juce::Colour col, float scale)
    {
        const int N = (int) buf.size();
        if (N <= 4) return;
        juce::Path p;
        const float cx = r.getCentreX(), cy = r.getCentreY();
        const float rad = juce::jmin (r.getWidth(), r.getHeight()) * 0.48f;
        int idx = w;
        bool started = false;
        const int points = juce::jmin (N, 3000);
        for (int i = 0; i < points; ++i)
        {
            idx = (idx - 1 + N) % N; // oldest->newest direction
            const auto pr = buf[(size_t) idx];
            const float x = cx + juce::jlimit (-1.5f, 1.5f, pr.first  * scale) * rad;
            const float y = cy - juce::jlimit (-1.5f, 1.5f, pr.second * scale) * rad;
            if (!started) { p.startNewSubPath (x, y); started = true; }
            else          { p.lineTo (x, y); }
        }
        g.setColour (col.withAlpha (0.18f)); g.strokePath (p, juce::PathStrokeType (6.0f));
        g.setColour (col.withAlpha (0.55f)); g.strokePath (p, juce::PathStrokeType (1.4f));
    }

    void drawCorrelationBar (juce::Graphics& g, juce::Rectangle<float> r)
    {
        // Simple instantaneous estimator from last few samples
        const int N = (int) fifoPost.size(); if (N <= 16) return;
        float sumL=0, sumR=0, sumLL=0, sumRR=0, sumLR=0; int K=0; int idx = writePost;
        for (int i = 0; i < juce::jmin (N, 512); ++i)
        {
            idx = (idx - 1 + N) % N; const auto pr = fifoPost[(size_t) idx];
            const float L = pr.first, R = pr.second;
            sumL += L; sumR += R; sumLL += L*L; sumRR += R*R; sumLR += L*R; ++K;
        }
        float rCorr = 0.0f;
        if (K > 0)
        {
            const float mL = sumL / (float) K, mR = sumR / (float) K;
            const float num = sumLR - (float) K * mL * mR;
            const float den = std::sqrt (juce::jmax (1.0e-12f, (sumLL - (float) K * mL * mL) * (sumRR - (float) K * mR * mR)));
            rCorr = (den > 0 ? juce::jlimit (-1.0f, 1.0f, num / den) : 0.0f);
        }
        auto bar = r.removeFromRight (juce::jmin (80.0f, r.getWidth() * 0.12f)).reduced (10.0f);
        g.setColour (juce::Colours::white.withAlpha (0.15f));
        g.fillRoundedRectangle (bar, 5.0f);
        const float midY = bar.getCentreY();
        const float x0 = bar.getX() + 8.0f, x1 = bar.getRight() - 8.0f;
        g.setColour (juce::Colours::white.withAlpha (0.25f));
        g.drawLine (x0, midY, x1, midY, 1.0f);
        const float t = 0.5f * (rCorr + 1.0f);
        const float x = juce::jmap (t, 0.0f, 1.0f, x0, x1);
        g.setColour (rCorr >= 0 ? juce::Colours::aqua : juce::Colours::orange);
        g.fillEllipse (x - 4.0f, midY - 4.0f, 8.0f, 8.0f);
    }

    // --- Polar energy scope ---
    void drawPolarEnergy (juce::Graphics& g, juce::Rectangle<float> r,
                          const std::vector<std::pair<float,float>>& buf, int w, float scale)
    {
        const int N = (int) buf.size(); if (N <= 8) return;
        const float cx = r.getCentreX(), cy = r.getCentreY();
        const float rad = juce::jmin (r.getWidth(), r.getHeight()) * 0.48f;
        int idx = w;
        juce::Path path; bool started=false;
        const int points = juce::jmin (N, 3000);
        for (int i = 0; i < points; ++i)
        {
            idx = (idx - 1 + N) % N; const auto pr = buf[(size_t) idx];
            const float L = juce::jlimit (-1.5f, 1.5f, pr.first  * scale);
            const float R = juce::jlimit (-1.5f, 1.5f, pr.second * scale);
            const float M = 0.5f * (L + R);
            const float S = 0.5f * (L - R);
            const float theta = 0.5f * std::atan2 (S, M);
            const float energy = std::sqrt (M*M + S*S);
            const float x = cx + std::cos (theta) * rad * energy;
            const float y = cy - std::sin (theta) * rad * energy;
            if (!started) { path.startNewSubPath (x, y); started = true; }
            else          { path.lineTo (x, y); }
        }
        g.setColour (juce::Colours::aqua.withAlpha (0.18f)); g.strokePath (path, juce::PathStrokeType (6.0f));
        g.setColour (juce::Colours::aqua.withAlpha (0.75f)); g.strokePath (path, juce::PathStrokeType (1.6f));
    }

    // --- Width editor ---
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

        // Draw crossovers (draggable)
        auto drawX = [&](float x){ juce::Path p; p.startNewSubPath (x, r.getY()); p.lineTo (x, r.getBottom());
                                   const float dashes[] = { 5.0f, 4.0f }; juce::Path dashed; juce::PathStrokeType (1.2f).createDashedStroke (dashed, p, dashes, 2);
                                   g.setColour (grid.withAlpha (0.8f)); g.strokePath (dashed, juce::PathStrokeType (1.6f)); };
        drawX (xLo); drawX (xHi);

        // Bars per band
        auto drawBand = [&](float x0, float x1, float width, juce::Colour c)
        {
            width = juce::jlimit (0.0f, 2.0f, width);
            const float h = juce::jmap (width, 0.0f, 2.0f, r.getHeight() * 0.08f, r.getHeight());
            juce::Rectangle<float> bar (x0, r.getBottom() - h, juce::jmax (6.0f, x1 - x0), h);
            g.setColour (c.withAlpha (0.30f)); g.fillRoundedRectangle (bar, 4.0f);
            g.setColour (c.withAlpha (0.95f)); g.drawRoundedRectangle (bar, 4.0f, 2.0f);
            // Top region marker like XY Pad
            auto top = juce::Rectangle<float> (bar.getX(), r.getY() + 2.0f, bar.getWidth(), juce::jlimit (4.0f, 8.0f, r.getHeight()*0.05f));
            g.setColour (c.withAlpha (0.55f)); g.fillRoundedRectangle (top, 2.0f);
        };

        const auto cLo  = juce::Colours::aqua;
        const auto cMid = juce::Colours::yellow;
        const auto cHi  = juce::Colours::orange;
        drawBand (r.getX(), xLo, widthLo, cLo);
        drawBand (xLo, xHi, widthMid, cMid);
        drawBand (xHi, r.getRight(), widthHi, cHi);
    }

    // Background waveform for Width view (mono Mid and Side envelopes)
    void drawWidthWaveform (juce::Graphics& g, juce::Rectangle<float> r)
    {
        const int N = (int) fifoPost.size(); if (N <= 8) return;
        auto mapX = [&](int i){ return juce::jmap ((float) i, 0.0f, (float) N-1, r.getX(), r.getRight()); };
        auto mapY = [&](float v, float scale){ return r.getCentreY() - v * scale * (r.getHeight() * 0.40f); };
        const float target = 0.70f;
        const float scale = opts.autoGain ? (rmsPost > 1.0e-6f ? target / rmsPost : 1.0f) : 1.0f;
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

    // --- Heatmap drawing ---
    void drawHeatmap (juce::Graphics& g, juce::Rectangle<float> r)
    {
        // Prepare engine on first use
        if (!enginePrepared)
        {
            StereoFieldEngine::Settings s;
            s.fftOrder = 11; s.bandsPerDecade = 12; s.historyWidthPx = juce::jmax (64, (int) r.getWidth());
            engine.prepare (sampleRate, s);
            engine.setEnablePre (opts.showPre);
            enginePrepared = true;
        }
        // Process more frequently in Heat mode to reduce latency perception
        engine.process();
        if (opts.mode == Options::Mode::Heat) engine.process();

        const auto& img = engine.getImagePost();
        const int W = img.getWidth();
        const int H = img.getHeight();
        const int x = engine.getWriteX();
        auto drawSlice = [&](const juce::Image& im, int sx, int sw, float dx, float alpha)
        {
            if (sw <= 0) return;
            g.setOpacity (alpha);
            const int dw = (int) std::round (r.getWidth() * (float) sw / (float) W);
            const int dh = (int) std::round (r.getHeight());
            const int dxInt = (int) std::round (dx);
            const int dyInt = (int) std::round (r.getY());
            g.drawImage (im, dxInt, dyInt, dw, dh, sx, 0, sw, H);
            g.setOpacity (1.0f);
        };
        const int rightW = W - (x + 1);
        drawSlice (img, x + 1, rightW, r.getX(), 1.0f);
        drawSlice (img, 0, x + 1, r.getX() + r.getWidth() * (float) rightW / (float) W, 1.0f);
        if (opts.showPre)
        {
            const auto& pre = engine.getImagePre();
            drawSlice (pre, x + 1, rightW, r.getX(), 0.35f);
            drawSlice (pre, 0, x + 1, r.getX() + r.getWidth() * (float) rightW / (float) W, 0.35f);
        }
    }

    Options opts;
    double sampleRate = 48000.0;

    juce::SpinLock dataLock;
    std::vector<std::pair<float,float>> fifoPost; int writePost = 0; bool havePost = false;
    std::vector<std::pair<float,float>> fifoPre;  int writePre  = 0; bool havePre  = false;
    float rmsPost = 0.0f, rmsPre = 0.0f;

    // Tooling UI
    juce::ToggleButton preToggle, freezeToggle;
    juce::ComboBox qualityBox;
    juce::TextButton modeXY, modePolar, modeWidth, modeHeat;
    // Designer controls (owned by pane, hosted in overlay)
    juce::Slider sideTiltDbOct, pivotHz, autoDepth, autoThrDb, autoAtkMs, autoRelMs, maxWidth;
    juce::Label  valSideTilt, valPivot, valAutoDepth, valAutoThr, valAtk, valRel, valMax;
    std::unique_ptr<MiniKnobCell> kcSideTilt, kcPivot, kcAutoDepth, kcAutoThr, kcAtk, kcRel, kcMax;
    juce::Label  lblSideTilt, lblPivot, lblAutoDepth, lblAutoThr, lblAtk, lblRel, lblMax;

public:
    std::function<void(const juce::String&, const juce::var&)> onUiChange;
    std::function<void(const juce::String& paramID, float value)> onParamEdit;

private:
    // State mirrored from APVTS for width editor
    float widthLo = 1.0f, widthMid = 1.0f, widthHi = 1.0f;
    float xoverLoHz = 150.0f, xoverHiHz = 2000.0f;

    // Accessors for PaneManager reflection
    float getLoWidth() const { return widthLo; }
    float getMidWidth() const { return widthMid; }
    float getHiWidth() const { return widthHi; }

    // Interaction state for width editor
    enum class DragKind { None, XLo, XHi, BandLo, BandMid, BandHi };
    DragKind drag { DragKind::None };
    float dragStartX = 0.0f;
    float dragStartVal = 0.0f;

public:
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (opts.mode != Options::Mode::Width) return;
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
        if (opts.mode != Options::Mode::Width) { setMouseCursor (juce::MouseCursor::NormalCursor); return; }
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
        if (opts.mode != Options::Mode::Width || drag == DragKind::None) return;
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

    // Heatmap engine integration
    StereoFieldEngine engine;
    bool enginePrepared = false;

    // --- Floating Designer overlay component ---
    struct DesignerOverlay : public juce::Component
    {
        ImagerPane& owner;
        juce::Point<int> dragStartPos {0,0};
        juce::Rectangle<int> dragStartBounds;
        DesignerOverlay (ImagerPane& p) : owner(p)
        {
            setInterceptsMouseClicks (true, true);
            // Re-parent controls into this overlay
            addAndMakeVisible (owner.sideTiltDbOct);
            addAndMakeVisible (owner.pivotHz);
            addAndMakeVisible (owner.autoDepth);
            addAndMakeVisible (owner.autoThrDb);
            addAndMakeVisible (owner.autoAtkMs);
            addAndMakeVisible (owner.autoRelMs);
            addAndMakeVisible (owner.maxWidth);
        }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            juce::Colour cTop = juce::Colour (0xCC1F242B);
            juce::Colour cBot = juce::Colour (0x661F242B);
            juce::Colour stroke = juce::Colours::white.withAlpha (0.10f);
            juce::Colour inner  = juce::Colours::white.withAlpha (0.06f);
            juce::Path rr; rr.addRoundedRectangle (r.reduced (1.0f), 8.0f);
            juce::ColourGradient grad (cTop, r.getX(), r.getY(), cBot, r.getX(), r.getBottom(), false);
            g.setGradientFill (grad); g.fillPath (rr);
            g.setColour (inner); g.fillPath (rr);
            g.setColour (stroke); g.strokePath (rr, juce::PathStrokeType (1.2f));
        }
        void resized() override
        {
            auto r = getLocalBounds().reduced (10);
            const int gap = 8;
            auto placeKnobs = [&](juce::Rectangle<int> area, std::initializer_list<std::tuple<const char*, juce::Slider*, juce::Label*>> items)
            {
                const int n = (int) items.size(); if (n <= 0) return;
                const int w = (area.getWidth() - gap * (n - 1)) / n;
                int x = area.getX();
                for (auto& it : items)
                {
                    const char* caption; juce::Slider* slider; juce::Label* val;
                    std::tie (caption, slider, val) = it;
                    juce::Rectangle<int> cell (x, area.getY(), w, area.getHeight());
                    const int kSize = juce::jmin (cell.getWidth(), cell.getHeight() - 28);
                    juce::Rectangle<int> kBox (kSize, kSize);
                    kBox = kBox.withCentre ({ cell.getCentreX(), cell.getY() + kSize / 2 });
                    slider->setBounds (kBox);
                    auto cap = juce::Rectangle<int> (cell.getX(), kBox.getBottom() + 2, cell.getWidth(), 14);
                    auto valb= juce::Rectangle<int> (cell.getX(), cap.getBottom(), cell.getWidth(), 14);
                    juce::Label* c = nullptr;
                    if (caption != nullptr)
                    {
                        c = new juce::Label();
                        c->setText (caption, juce::dontSendNotification);
                        c->setJustificationType (juce::Justification::centred);
                        c->setInterceptsMouseClicks (false, false);
                        ownedCaptions.add (c);
                        addAndMakeVisible (c);
                        c->setBounds (cap);
                    }
                    if (val) { val->setJustificationType (juce::Justification::centred); val->setBounds (valb); }
                    x += w + gap;
                }
            };
            ownedCaptions.clear (true);
            // Single-row layout with all 7 knobs
            placeKnobs (r, { { "Side Tilt", &owner.sideTiltDbOct, &owner.valSideTilt },
                             { "Pivot Hz", &owner.pivotHz, &owner.valPivot },
                             { "Auto Depth", &owner.autoDepth, &owner.valAutoDepth },
                             { "Auto Thr",  &owner.autoThrDb, &owner.valAutoThr },
                             { "ATT",       &owner.autoAtkMs, &owner.valAtk },
                             { "REL",       &owner.autoRelMs, &owner.valRel },
                             { "Max",       &owner.maxWidth,  &owner.valMax } });
        }
        void paintOverChildren (juce::Graphics&) override {}
        juce::OwnedArray<juce::Label> ownedCaptions;
        void mouseDown (const juce::MouseEvent&) override {}
        void mouseDrag (const juce::MouseEvent&) override {}
        void mouseUp   (const juce::MouseEvent&) override {}
    };

    std::unique_ptr<DesignerOverlay> overlay;
    juce::Rectangle<int> overlayBounds { 0,0,0,0 };
    bool overlayBoundsSet { false };

    juce::Rectangle<int> getMainAreaBounds() const
    {
        auto r = getLocalBounds();
        const int W = juce::jmax (32, getWidth());
        const int sideW = juce::jlimit (120, 220, W / 4);
        r.removeFromRight (sideW);
        return r;
    }

    void initDesignerControls()
    {
        // Always-on Designer controls; labels
        lblSideTilt.setText ("Side Tilt dB/Oct", juce::dontSendNotification);
        lblPivot.setText    ("Pivot Hz",         juce::dontSendNotification);
        lblAutoDepth.setText("Auto Depth",       juce::dontSendNotification);
        lblAutoThr.setText  ("Auto Thr dB",      juce::dontSendNotification);
        lblAtk.setText      ("Attack ms",        juce::dontSendNotification);
        lblRel.setText      ("Release ms",       juce::dontSendNotification);
        lblMax.setText      ("Max Width",        juce::dontSendNotification);

        auto prepSlider = [&](juce::Slider& s, juce::Label& valueLbl, float min, float max, float step, const juce::String& pid)
        {
            s.setRange (min, max, step);
            s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            s.setRotaryParameters (-juce::MathConstants<float>::halfPi, juce::MathConstants<float>::pi * 1.5f, true);
            valueLbl.setInterceptsMouseClicks (false, false);
            s.onValueChange = [this, &s, &valueLbl, pid]
            {
                if (onParamEdit) onParamEdit (pid, (float) s.getValue());
                valueLbl.setText (juce::String (s.getValue(), 2), juce::dontSendNotification);
            };
        };

        prepSlider (sideTiltDbOct, valSideTilt, -6.0f, 6.0f, 0.01f,  "width_side_tilt_db_oct");
        prepSlider (pivotHz,       valPivot,    150.0f, 2000.0f, 1.0f, "width_tilt_pivot_hz");
        prepSlider (autoDepth,     valAutoDepth,0.0f, 1.0f, 0.001f,   "width_auto_depth");
        prepSlider (autoThrDb,     valAutoThr, -24.0f, 12.0f, 0.01f,  "width_auto_thr_db");
        prepSlider (autoAtkMs,     valAtk,       1.0f, 200.0f, 0.1f,  "width_auto_atk_ms");
        prepSlider (autoRelMs,     valRel,      20.0f, 1200.0f, 0.1f, "width_auto_rel_ms");
        prepSlider (maxWidth,      valMax,       0.5f, 2.5f, 0.001f,  "width_max");

        // Defaults matching processor layout
        sideTiltDbOct.setValue (0.0, juce::dontSendNotification);  valSideTilt.setText ("0.00", juce::dontSendNotification);
        pivotHz.setValue (650.0, juce::dontSendNotification);      valPivot.setText    ("650", juce::dontSendNotification);
        autoDepth.setValue (0.0, juce::dontSendNotification);      valAutoDepth.setText("0.00", juce::dontSendNotification);
        autoThrDb.setValue (-3.0, juce::dontSendNotification);     valAutoThr.setText  ("-3.0", juce::dontSendNotification);
        autoAtkMs.setValue (25.0, juce::dontSendNotification);     valAtk.setText      ("25.0", juce::dontSendNotification);
        autoRelMs.setValue (250.0, juce::dontSendNotification);    valRel.setText      ("250", juce::dontSendNotification);
        maxWidth.setValue (2.0, juce::dontSendNotification);       valMax.setText      ("2.00", juce::dontSendNotification);

        // Restore overlay bounds from persisted UI state if available
        if (onUiChange) onUiChange ("ui_imager_overlay_bounds_query", true);
    }

    // Utility: serialize/deserialize rectangle to var
    juce::var serializeRect (const juce::Rectangle<int>& r) const
    {
        juce::DynamicObject* o = new juce::DynamicObject();
        o->setProperty ("x", r.getX()); o->setProperty ("y", r.getY()); o->setProperty ("w", r.getWidth()); o->setProperty ("h", r.getHeight());
        return juce::var (o);
    }
    static juce::Rectangle<int> deserializeRect (const juce::var& v)
    {
        if (auto* o = v.getDynamicObject())
        {
            auto x = (int) o->getProperty ("x"); auto y = (int) o->getProperty ("y");
            auto w = (int) o->getProperty ("w"); auto h = (int) o->getProperty ("h");
            return { x, y, w, h };
        }
        return {};
    }
};


