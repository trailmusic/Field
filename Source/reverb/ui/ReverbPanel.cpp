#include "ReverbPanel.h"
#include "DecayCurveComponent.h"
#include "ReverbEQComponent.h"
#include "ReverbScopeComponent.h"
#include "../ReverbParamIDs.h"
#include "../../KnobCell.h"

using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

ReverbPanel::ReverbPanel (juce::AudioProcessorValueTreeState& s,
                          std::function<float()> getEr,
                          std::function<float()> getTail,
                          std::function<float()> getDuckDb,
                          std::function<float()> getWidthNow)
    : state (s)
{
    // Visuals-only: no top switches/algorithm here; those live in Group 2 controls

    canvas.reset (new ReverbCanvasComponent (state,
                                             std::move(getEr), std::move(getTail), std::move(getDuckDb), std::move(getWidthNow)));
    if (dyneqGrProvider) canvas->setDynEqGrProvider (dyneqGrProvider);
    addAndMakeVisible (*canvas);

    dyneqPane = std::make_unique<ReverbDynEQPane> (state);
    addAndMakeVisible (*dyneqPane);

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

void ReverbPanel::resized()
{
    auto r = getLocalBounds().reduced (6);
    // No header controls

    auto top = r.removeFromTop (juce::roundToInt (r.getHeight() * 0.62f));
    canvas->setBounds (top);
    dyneqPane->setBounds (r);
}


