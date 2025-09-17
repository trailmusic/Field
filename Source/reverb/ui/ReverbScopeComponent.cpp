#include "ReverbScopeComponent.h"

using namespace juce;

void ReverbScopeComponent::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (6);
    g.setColour (Colours::black.withAlpha (0.4f)); g.fillRoundedRectangle (r, 6.f);
    g.setColour (Colours::white.withAlpha (0.2f)); g.drawRoundedRectangle (r, 6.f, 1.2f);

    auto mapY = [r](float v){ return jmap (Decibels::gainToDecibels (v + 1e-6f), -60.f, 0.f, r.getBottom()-8, r.getY()+8); };

    const float er  = getEr ? getEr()   : 0.f;
    const float tail= getTail ? getTail(): 0.f;
    const float w   = getWidth ? getWidth(): 100.f;
    const float gr  = getDuck ? getDuck() : 0.f;

    float xMid = r.getX() + r.getWidth() * 0.25f;
    g.setColour (Colours::grey.withAlpha (0.85f)); g.fillRect ((float) (xMid-30.0f), (float) mapY (er),   20.0f, (float) (r.getBottom()-8.0f - mapY (er)));
    g.setColour (Colours::aqua.withAlpha (0.95f)); g.fillRect ((float) (xMid+10.0f), (float) mapY (tail), 20.0f, (float) (r.getBottom()-8.0f - mapY (tail)));

    float wx = r.getX() + r.getWidth() * 0.62f;
    g.setColour (Colours::white.withAlpha (0.6f));
    g.drawText ("Width", Rectangle<int> ((int) wx, (int) r.getY()+6, 60, 14), Justification::left, false);
    g.setColour (Colours::orange);
    g.fillRoundedRectangle (wx, r.getBottom()-14, jlimit (0.f, r.getWidth()*0.35f, w*0.01f * r.getWidth()*0.35f), 8, 4);

    g.setColour (Colours::red.withAlpha (0.9f));
    g.drawText (String (jmax (0.f, gr), 1) + " dB GR", Rectangle<int> ((int) wx, (int) r.getCentreY()-8, 90, 16), Justification::left, false);
}


