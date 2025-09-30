#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"

namespace ui
{
    // DEV NOTE: One-line helper so all rotaries render identically through FieldLNF.
    inline void paintRotaryWithLNF(juce::Graphics& g, juce::Slider& s, juce::Rectangle<float> bounds)
    {
        // Guard: avoid degenerate drawing when bounds are tiny/invalid
        if (bounds.getWidth() <= 2.0f || bounds.getHeight() <= 2.0f)
            return;
        if (auto* lf = dynamic_cast<FieldLNF*>(&s.getLookAndFeel()))
        {
            const double minV = s.getMinimum();
            const double maxV = s.getMaximum();
            const float pos01 = (maxV > minV) ? (float)((s.getValue() - minV) / (maxV - minV)) : 0.0f;

            lf->drawRotarySlider(g, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                 pos01,
                                 juce::MathConstants<float>::pi,
                                 juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                                 s);
        }
    }
}
