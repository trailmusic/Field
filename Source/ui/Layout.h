
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "Design.h"
namespace UI {
inline void placeRow(juce::Rectangle<int> row,
                     std::initializer_list<juce::Component*> comps,
                     int gapPx = gap)
{
    int n = (int) comps.size();
    if (n <= 0) return;
    int w = (row.getWidth() - (n - 1) * gapPx) / n;
    for (auto* c : comps) {
        if (!c) continue;
        c->setBounds(row.removeFromLeft(w).reduced(gapPx / 2));
        row.removeFromLeft(gapPx);
    }
}
}
