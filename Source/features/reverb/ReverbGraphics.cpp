#include "ReverbGraphics.h"
#include "shared/Core/FieldLookAndFeel.h"

ReverbGraphics::ReverbGraphics (juce::AudioProcessorValueTreeState& s,
                          std::function<float()> getEr,
                          std::function<float()> getTail,
                          std::function<float()> getDuckDb,
                          std::function<float()> getWidthNow)
    : state (s)
{
    // Clean slate - only background, no visuals
}

void ReverbGraphics::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // AB Button styling: Solid panel background with elevation shadow
    const float cr = 8.0f; // Match KnobCell corner radius
    
    // Elevation shadow first (AB button style)
    if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.25f));
    else g.setColour(juce::Colour(0x40000000));
    g.fillRoundedRectangle(r.translated(1.5f, 1.5f), cr);
    
    // Solid panel background (no aliasing)
    g.setColour(th.meters.panelDark);
    g.fillRoundedRectangle(r, cr);
    
    // Border (AB button style)
    g.setColour(th.sh);
    g.drawRoundedRectangle(r, cr, 1.0f);
    
    // Add 10px top and bottom padding for content
    auto contentR = r.reduced(0, 10.0f);
}

void ReverbGraphics::resized()
{
    // Clean slate - no visual components to resize
}


