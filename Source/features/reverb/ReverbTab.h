#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ReverbTab — Composite tab: Visualization canvas + 2×16 control grid
// ----------------------------------------------------------------------------
// DEV NOTES
// - Owns two children:
//     1) ReverbGraphics  (visualization + EQ shells, right-side ducking strip)
//     2) ReverbControlsPane (2×16 grid of knobs/switches)
// - Layout is responsive via ControlGridMetrics::compute(...):
//     * Top area: full-width ReverbGraphics
//     * Bottom area: controls grid (can be hidden)
// - Theme changes: rely on JUCE propagation; no need to manually call child
//   lookAndFeelChanged(), but we keep a forwarder for robustness.
// - Keep this header light; if you split later, forward-declare and include
//   heavy headers in the .cpp.
//
// TODO (optional):
// [ ] Add a segmented control here to switch ReverbVisuals view modes.
// [ ] Persist "controls visible" in state.
// [ ] Expose a method to set a fixed controls height for compact modes.
// ─────────────────────────────────────────────────────────────────────────────

#include <JuceHeader.h>
#include "ReverbGraphics.h"
#include "ReverbControlsPane.h"
#include "shared/ui/Controls/ControlGridMetrics.h"
#include "shared/Core/PluginProcessor.h"

class ReverbTab final : public juce::Component
{
public:
    explicit ReverbTab (MyPluginAudioProcessor& p)
        : proc (p)
    {
       #if JUCE_DEBUG
        DBG ("[ReverbTab] ctor");
       #endif

        // Visualization pane (uses processor-provided meters)
        reverbPanel = std::make_unique<ReverbGraphics> (
            p, p.apvts,
            [&p]{ return p.getReverbErRms();      },
            [&p]{ return p.getReverbTailRms();    },
            [&p]{ return p.getReverbDuckGrDb();   },
            [&p]{ return p.getReverbWidthNow();   }
        );
        addAndMakeVisible (*reverbPanel);

        // Controls grid (visible by default)
        controls = std::make_unique<ReverbControlsPane> (p.apvts);
        controls->setVisible (true);
        addAndMakeVisible (*controls);
    }

    ~ReverbTab () override = default;

    // Show/hide the bottom controls grid
    void setControlsVisible (bool on)
    {
        if (controls) controls->setVisible (on);
        resized (); // relayout
    }

    [[nodiscard]] ReverbGraphics* getReverbCanvas () const noexcept { return reverbPanel.get (); }

    // ─────────────────────────────────────────────────────────────────────────
    // juce::Component
    // ─────────────────────────────────────────────────────────────────────────
    void resized () override
    {
        auto r = getLocalBounds ();

        // Compute responsive grid metrics from current size
        const auto m = ControlGridMetrics::compute (r.getWidth (), r.getHeight ());
        if (controls)
        {
            controls->setCellMetrics (m.knobPx, m.valuePx, m.labelGapPx, m.colW);
            controls->setRowHeightPx (m.rowH);
        }

        // Bottom reserve for controls (only if visible)
        auto controlsArea = r.removeFromBottom (controls && controls->isVisible () ? m.controlsH : 0);

        if (reverbPanel) reverbPanel->setBounds (r);
        if (controls   && controls->isVisible ()) controls->setBounds (controlsArea);

        // No z-order juggling needed: non-overlapping layout.
    }

    void lookAndFeelChanged () override
    {
        // JUCE already propagates LNF changes to children; explicit forwarder
        // added in case children need to recalc theme-dependent caches.
        if (reverbPanel) reverbPanel->lookAndFeelChanged ();
        if (controls)    controls->lookAndFeelChanged ();
        juce::Component::lookAndFeelChanged ();
    }

private:
    MyPluginAudioProcessor&                  proc;
    std::unique_ptr<ReverbGraphics>          reverbPanel;
    std::unique_ptr<ReverbControlsPane>      controls;

    JUCE_LEAK_DETECTOR (ReverbTab)
};