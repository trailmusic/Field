#include "CorrelationMeter.h"

CorrelationMeter::CorrelationMeter (MyPluginAudioProcessor& p, FieldLNF& l) 
    : proc (p), lnf (l) 
{ 
    startTimerHz (25); 
}

void CorrelationMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    // Adjust positioning: top moves up 3px (5->2), bottom moves down 2px
    r = r.withY (r.getY() + 2.0f).withHeight (r.getHeight() - 2.0f + 2.0f);
    
    g.setColour (lnf.theme.meters.trackBase);
    // Fill entire area first to prevent white corners
    g.fillRect(r);
    g.fillRoundedRectangle (r, 6.0f);
    
    // Standard border treatment: accent border (reduced brightness for meters)
    g.setColour (lnf.theme.accent.withAlpha (0.3f));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);

    const float corr = juce::jlimit (-1.0f, 1.0f, proc.getCorrelation());
    // Thin vertical track centered horizontally
    const float pad = 3.0f;
    const float trackW = juce::jmax (6.0f, r.getWidth() - 2*pad);
    const float cx = r.getX() + r.getWidth() * 0.5f;
    juce::Rectangle<float> track (cx - trackW * 0.5f, r.getY() + pad, trackW, r.getHeight() - 2*pad);
    // Midline
    const float midY = track.getCentreY();
    g.setColour (lnf.theme.hl.withAlpha (0.35f));
    g.fillRoundedRectangle (track, 2.5f);
    g.setColour (lnf.theme.hl.withAlpha (0.6f));
    g.fillRect (juce::Rectangle<float> (track.getX(), midY-0.5f, track.getWidth(), 1.0f));

    // Positive = fill upward; Negative = fill downward
    if (corr >= 0.0f)
    {
        const float h = (track.getHeight() * 0.5f) * corr;
        g.setColour (lnf.theme.meters.positive);
        g.fillRoundedRectangle (juce::Rectangle<float> (track.getX(), midY - h, track.getWidth(), h), 2.0f);
    }
    else
    {
        const float h = (track.getHeight() * 0.5f) * (-corr);
        g.setColour (lnf.theme.meters.negative);
        g.fillRoundedRectangle (juce::Rectangle<float> (track.getX(), midY, track.getWidth(), h), 2.0f);
    }

    // Peak line (thicker bottom border like LR meters)
    g.setColour (lnf.theme.accent.withAlpha (0.6f));
    g.fillRect (juce::Rectangle<float> (track.getX(), track.getBottom() - 1.0f, track.getWidth(), 2.0f));
    
    // Vertical label on the right side: C O R R
    g.setColour (lnf.theme.textMuted);
    g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
    const float labelX = track.getRight() + 2.0f;
    const float step   = 12.0f;
    juce::String chars[] = { "C", "O", "R", "R" };
    float y = r.getY() + pad;
    for (auto& ch : chars)
    {
        g.drawText (ch, juce::Rectangle<int> ((int)labelX, (int)y, (int)(r.getRight()-labelX-1.0f), 12), juce::Justification::centredLeft);
        y += step;
    }
}

void CorrelationMeter::timerCallback() 
{ 
    repaint(); 
}

void CorrelationMeter::visibilityChanged() 
{ 
    if (isVisible()) 
        startTimerHz (15); 
    else 
        stopTimer(); 
}
