#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Core/PluginProcessor.h"
#include "../../Core/FieldLookAndFeel.h"

class VerticalLRMeters : public juce::Component, public juce::Timer
{
public:
    ~VerticalLRMeters() override { stopTimer(); }
    
    VerticalLRMeters (MyPluginAudioProcessor& p, FieldLNF& l) : proc(p), lnf(l) { startTimerHz (30); }
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        auto left = r.removeFromLeft (r.getWidth() * 0.5f).reduced (2.0f);
        auto right= r.reduced (2.0f);

        auto drawBar = [&] (juce::Rectangle<float> b, float rms, float peak, const juce::String& label)
        {
            g.setColour (lnf.theme.meters.trackBase);
            g.fillRoundedRectangle (b, 4.0f);
            // Track
            {
                juce::Colour base = lnf.theme.meters.trackBase;
                juce::Colour base2 = lnf.theme.meters.trackActive;
                juce::ColourGradient grad (base, b.getX(), b.getY(), base2, b.getX(), b.getBottom(), false);
                juce::FillType ft (grad);
                g.setFillType (ft);
                g.fillRoundedRectangle (b.reduced (1.0f), 3.5f);
                g.setFillType (juce::FillType());
            }
            // Standard border treatment: accent border (reduced brightness for meters)
            g.setColour (lnf.theme.accent.withAlpha (0.3f));
            g.drawRoundedRectangle (b, 4.0f, 1.0f);

            // scale 0..1 across height
            auto hRms  = juce::jlimit (0.0f, 1.0f, rms ) * b.getHeight();
            auto hPeak = juce::jlimit (0.0f, 1.0f, peak) * b.getHeight();
            // Zone colours (dBFS thresholds)
            const float rmsDb  = juce::Decibels::gainToDecibels (juce::jlimit (0.000001f, 1.0f, rms),  -60.0f);
            const float peakDb = juce::Decibels::gainToDecibels (juce::jlimit (0.000001f, 1.0f, peak), -60.0f);
            const bool risk    = peakDb >= -1.0f || rmsDb >= -3.0f;
            const bool warn    = !risk && (peakDb >= -6.0f || rmsDb >= -12.0f);
            {
                juce::Colour safe1 = lnf.theme.accent.withAlpha (0.70f);
                juce::Colour safe2 = lnf.theme.accent.withAlpha (0.90f);
                juce::Colour warn1 = juce::Colour (0xFFFFC107).withAlpha (0.75f); // amber
                juce::Colour warn2 = juce::Colour (0xFFFFA000).withAlpha (0.95f);
                juce::Colour risk1 = juce::Colour (0xFFFF8A80).withAlpha (0.85f); // soft red
                juce::Colour risk2 = juce::Colour (0xFFE53935).withAlpha (0.95f);
                auto c1 = risk ? risk1 : (warn ? warn1 : safe1);
                auto c2 = risk ? risk2 : (warn ? warn2 : safe2);
                juce::ColourGradient grad (c1, b.getCentreX(), b.getBottom() - hRms,
                                           c2, b.getCentreX(), b.getBottom(), false);
                g.setFillType (juce::FillType (grad));
                g.fillRoundedRectangle (juce::Rectangle<float> (b.getX(), b.getBottom() - hRms, b.getWidth(), hRms), 3.0f);
                g.setFillType (juce::FillType());
            }
            // Peak line
            g.setColour (risk ? juce::Colour (0xFFE53935) : lnf.theme.accent);
            g.fillRect (juce::Rectangle<float> (b.getX(), b.getBottom() - hPeak, b.getWidth(), 2.0f));
            // Crest factor hint (thin line slightly below peak)
            const float crestH = juce::jmax (0.0f, hPeak - hRms);
            if (crestH > 2.0f)
            {
                g.setColour (lnf.theme.text.withAlpha (0.20f));
                g.fillRect (juce::Rectangle<float> (b.getX(), b.getBottom() - hPeak + 2.0f, b.getWidth(), 1.0f));
            }
            // Gloss
            g.setColour (juce::Colours::white.withAlpha (0.06f));
            g.fillRoundedRectangle (juce::Rectangle<float> (b.getX()+1.5f, b.getY()+1.5f, b.getWidth()-3.0f, b.getHeight()*0.25f), 3.0f);

            // Label
            g.setColour (lnf.theme.textMuted);
            g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
            g.drawText (label, b.reduced (4.0f), juce::Justification::centredBottom);

            // dB tick marks (approx) across the bar: -24, -12, -6, -3, -1 dBFS
            auto drawTick = [&] (float db, const char* text)
            {
                // map dB to linear magnitude (0..1). For UI, assume 0 = -inf, 1 = 0 dBFS
                const float lin = juce::Decibels::decibelsToGain (db, -60.0f);
                const float y   = b.getBottom() - juce::jlimit (0.0f, 1.0f, lin) * b.getHeight();
                g.setColour (lnf.theme.hl.withAlpha (0.6f));
                g.fillRect (juce::Rectangle<float> (b.getX(), y, b.getWidth(), 1.0f));
                g.setColour (lnf.theme.textMuted.withAlpha (0.8f));
                g.drawText (text, juce::Rectangle<int> (b.getX(), (int) y - 8, (int) b.getWidth(), 12), juce::Justification::centredRight);
            };
            drawTick (-24.0f, "-24");
            drawTick (-12.0f, "-12");
            drawTick (-6.0f,  "-6");
            drawTick (-3.0f,  "-3");
            drawTick (-1.0f,  "-1");
        };

        drawBar (left,  proc.getRmsL(), proc.getPeakL(), "L");
        drawBar (right, proc.getRmsR(), proc.getPeakR(), "R");
    }
    void timerCallback() override { if (isShowing()) repaint(); }
    void visibilityChanged() override { if (isVisible()) startTimerHz (20); else stopTimer(); }
private:
    MyPluginAudioProcessor& proc;
    FieldLNF& lnf;
};
