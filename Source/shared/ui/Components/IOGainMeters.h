#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../Core/PluginProcessor.h"
#include "../../Core/FieldLookAndFeel.h"

class IOGainMeters : public juce::Component, public juce::Timer
{
public:
    ~IOGainMeters() override { stopTimer(); }
    
    IOGainMeters (MyPluginAudioProcessor& p, FieldLNF& l) : proc(p), lnf(l) { startTimerHz (30); }
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        auto inB  = r.removeFromLeft (r.getWidth() * 0.5f).reduced (2.0f);
        auto outB = r.reduced (2.0f);
        auto drawOne = [&] (juce::Rectangle<float> b, float rms, const juce::String& label)
        {
            g.setColour (lnf.theme.meters.trackBase); g.fillRoundedRectangle (b, 4.0f);
            // Standard border treatment: accent border (reduced brightness for meters)
            g.setColour (lnf.theme.accent.withAlpha (0.3f));
            g.drawRoundedRectangle (b, 4.0f, 1.0f);
            const float h = juce::jlimit (0.0f, 1.0f, rms) * b.getHeight();
            const float rmsDb = juce::Decibels::gainToDecibels (juce::jlimit (0.000001f, 1.0f, rms), -60.0f);
            const bool risk = rmsDb >= -1.0f;
            const bool warn = !risk && rmsDb >= -6.0f;
            juce::Colour safe1 = lnf.theme.meters.safe.withAlpha (0.55f);
            juce::Colour safe2 = lnf.theme.meters.safe.withAlpha (0.85f);
            juce::Colour warn1 = lnf.theme.meters.warning.withAlpha (0.70f);
            juce::Colour warn2 = lnf.theme.meters.warning.withAlpha (0.90f);
            juce::Colour risk1 = lnf.theme.meters.error.withAlpha (0.80f);
            juce::Colour risk2 = lnf.theme.meters.error.withAlpha (0.95f);
            auto c1 = risk ? risk1 : (warn ? warn1 : safe1);
            auto c2 = risk ? risk2 : (warn ? warn2 : safe2);
            juce::ColourGradient grad (c1, b.getCentreX(), b.getBottom() - h,
                                       c2, b.getCentreX(), b.getBottom(), false);
            g.setFillType (juce::FillType (grad));
            g.fillRoundedRectangle (juce::Rectangle<float> (b.getX(), b.getBottom() - h, b.getWidth(), h), 3.0f);
            g.setFillType (juce::FillType());
            
            // Peak line (thicker bottom border like LR meters)
            g.setColour (risk ? juce::Colour (0xFFE53935) : lnf.theme.accent);
            g.fillRect (juce::Rectangle<float> (b.getX(), b.getBottom() - h, b.getWidth(), 2.0f));
            
            g.setColour (lnf.theme.textMuted);
            g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
            g.drawText (label, b.reduced (4.0f), juce::Justification::centredBottom);
        };
        drawOne (inB,  proc.getInRms(),  "I");
        drawOne (outB, proc.getOutRms(), "O");
    }
    void timerCallback() override { if (isShowing()) repaint(); }
    void visibilityChanged() override { if (isVisible()) startTimerHz (20); else stopTimer(); }
private:
    MyPluginAudioProcessor& proc; FieldLNF& lnf;
};
