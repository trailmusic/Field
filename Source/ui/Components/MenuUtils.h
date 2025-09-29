#pragma once

#include <JuceHeader.h>
#include "TintMenuLNFEx.h"

template <typename BuildFn, typename ResultFn>
static void showTintedMenu (juce::Component& anchor, const TintMenuLNFEx& configuredLnf,
                            BuildFn&& build, ResultFn&& onResult)
{
    auto lnfHold = std::make_shared<TintMenuLNFEx>();
    // Copy relevant configuration
    lnfHold->defaultTint = configuredLnf.defaultTint;
    lnfHold->hideChecks  = configuredLnf.hideChecks;
    lnfHold->itemTints   = configuredLnf.itemTints;
    // Copy colours that might have been set on configuredLnf
    lnfHold->setColour (juce::PopupMenu::textColourId,
                        configuredLnf.findColour (juce::PopupMenu::textColourId));
    lnfHold->setColour (juce::PopupMenu::highlightedBackgroundColourId,
                        configuredLnf.findColour (juce::PopupMenu::highlightedBackgroundColourId));
    lnfHold->setColour (juce::PopupMenu::highlightedTextColourId,
                        configuredLnf.findColour (juce::PopupMenu::highlightedTextColourId));

    juce::PopupMenu m; m.setLookAndFeel (lnfHold.get());

    build (m, *lnfHold);

    auto* parent = anchor.getTopLevelComponent();
    juce::PopupMenu::Options opt;
    opt = opt.withTargetComponent (&anchor)
             .withParentComponent (parent)
             .withMinimumWidth (juce::jmax (160, anchor.getWidth()));

    m.showMenuAsync (opt, [lnfHold, onResult] (int r) { onResult (r); });
}
