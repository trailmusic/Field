#pragma once
#include <JuceHeader.h>
class FieldLNF;

class DoubleKnobCell : public juce::Component
{
public:
    DoubleKnobCell(juce::Slider& leftKnob, juce::Label& leftLabel,
                   juce::Slider& rightKnob, juce::Label& rightLabel)
    : lKnob(leftKnob), lLabel(leftLabel), rKnob(rightKnob), rLabel(rightLabel) {}

    void setMetrics (int knobPx, int valuePx, int gapPx)
    {
        K = juce::jmax (16, knobPx);
        V = juce::jmax (0,  valuePx);
        G = juce::jmax (0,  gapPx);
        resized(); repaint();
    }

    void setShowBorder (bool show) { showBorder = show; repaint(); }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void ensureChildren();
    void layoutOne (juce::Rectangle<int> area, juce::Slider& knob, juce::Label& label);
    void drawRecessedBadge (juce::Graphics& g, juce::Label& label);

    juce::Slider& lKnob;  juce::Label& lLabel;
    juce::Slider& rKnob;  juce::Label& rLabel;

    int K = 88, V = 14, G = 4; // slightly wider
    bool showBorder = true;
};


