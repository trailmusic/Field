#pragma once
#include <JuceHeader.h>
#include "ParamPatch.h"

class ProposalCard : public juce::Component
{
public:
    std::function<void(const ParamPatch&)> onApply, onBypass;
    std::function<void()> onOpenWidthPanel;

    void setPatch (const ParamPatch& p) { patch = p; repaint(); }
    const ParamPatch& getPatch() const { return patch; }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    ParamPatch patch;
    juce::TextButton applyBtn { "Apply" }, bypassBtn { "Bypass" }, widthBtn { "Width…" };
};


