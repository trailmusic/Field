#pragma once
#include <JuceHeader.h>
#include "FieldLookAndFeel.h"

// Composite 2x2 cell: top row HP/LP (two knobs), bottom row Q + Cluster (aux)
class QuadKnobCell : public juce::Component
{
public:
    QuadKnobCell(juce::Slider& hpKnob, juce::Label& hpLabel,
                 juce::Slider& lpKnob, juce::Label& lpLabel,
                 juce::Slider& qKnob,  juce::Label& qLabel,
                 juce::Component& clusterContainer)
    : hp(hpKnob), hpVal(hpLabel), lp(lpKnob), lpVal(lpLabel), q(qKnob), qVal(qLabel), cluster(clusterContainer) {}

    void setMetrics (int knobPx, int valuePx, int gapPx);

    void setShowBorder (bool show) { showBorder = show; repaint(); }

    void paint (juce::Graphics& g) override;

    void resized() override;

private:
    void ensureChildren();

    void layoutKnob (juce::Rectangle<int> area, juce::Slider& knob, juce::Label& label);

    juce::Slider& hp; juce::Label& hpVal;
    juce::Slider& lp; juce::Label& lpVal;
    juce::Slider& q;  juce::Label& qVal;
    juce::Component& cluster;

    int K = 88, V = 14, G = 4; // slightly wider
    bool showBorder = true;
};


