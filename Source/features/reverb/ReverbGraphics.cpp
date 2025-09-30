#include "ReverbGraphics.h"
#include "DecayCurveComponent.h"
#include "ReverbScopeComponent.h"
#include "ReverbParamIDs.h"
#include "shared/ui/Components/KnobCell.h"
#include "shared/Core/FieldLookAndFeel.h"

using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

ReverbGraphics::ReverbGraphics (juce::AudioProcessorValueTreeState& s,
                          std::function<float()> getEr,
                          std::function<float()> getTail,
                          std::function<float()> getDuckDb,
                          std::function<float()> getWidthNow)
    : state (s)
{
    // Visuals-only: no top switches/algorithm here; those live in Group 2 controls

    canvas.reset (new ReverbCanvasComponent (state,
                                             std::move(getEr), std::move(getTail), std::move(getDuckDb), std::move(getWidthNow)));
    addAndMakeVisible (*canvas);

    // DynEQ pane now handled by ReverbTab

    // No 5x4 grid attachments here (those live in Group 2 UI)

    // Ducking strip
    addAndMakeVisible (duckMode);
    if (auto* ch2 = dynamic_cast<juce::AudioParameterChoice*>(state.getParameter (ReverbIDs::duckMode)))
    {
        duckMode.clear(); for (int i=0;i<ch2->choices.size(); ++i) duckMode.addItem (ch2->choices[i], i+1);
        duckMode.setSelectedId (ch2->getIndex()+1, juce::dontSendNotification);
    }
    duckModeA = std::make_unique<CA> (state, ReverbIDs::duckMode, duckMode);
    auto addDuck = [&](const char* id, juce::Slider& s){ addAndMakeVisible (s); duckAtts.push_back (std::make_unique<SA> (state, id, s)); };
    addDuck (ReverbIDs::duckDepthDb, duckDepth);
    addDuck (ReverbIDs::duckThrDb,   duckThr);
    addDuck (ReverbIDs::duckKneeDb,  duckKnee);
    addDuck (ReverbIDs::duckRatio,   duckRatio);
    addDuck (ReverbIDs::duckAtkMs,   duckAtk);
    addDuck (ReverbIDs::duckRelMs,   duckRel);
    addDuck (ReverbIDs::duckLaMs,    duckLa);
    addDuck (ReverbIDs::duckRmsMs,   duckRms);
    addDuck (ReverbIDs::duckBandHz,  duckBandHz);
    addDuck (ReverbIDs::duckBandQ,   duckBandQ);
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
    auto r = getLocalBounds().reduced (6);
    // No header controls

    auto top = r.removeFromTop (juce::roundToInt (r.getHeight() * 0.62f));
    canvas->setBounds (top);
    // DynEQ pane now handled by ReverbTab
}


