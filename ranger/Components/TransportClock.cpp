#include "TransportClock.h"
#include "../../Core/FieldLookAndFeel.h"

TransportClock::TransportClock(MyPluginAudioProcessor& processor)
    : proc(processor)
{
    // Setup clock label
    addAndMakeVisible(clockLabel);
    clockLabel.setJustificationType(juce::Justification::centredRight);
    clockLabel.setInterceptsMouseClicks(false, false);
    clockLabel.setText("00:00.000", juce::dontSendNotification);
    
    // Start timer for updates
    startTimerHz(UPDATE_HZ);
}

TransportClock::~TransportClock()
{
    stopTimer();
}

void TransportClock::paint(juce::Graphics& g)
{
    // Clock component doesn't need custom painting - the label handles display
    juce::ignoreUnused(g);
}

void TransportClock::resized()
{
    clockLabel.setBounds(getLocalBounds());
}

void TransportClock::lookAndFeelChanged()
{
    // Apply theme colors to clock label
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
    {
        clockLabel.setColour(juce::Label::textColourId, lf->theme.text);
        clockLabel.setFont(juce::Font(juce::FontOptions(18.0f).withStyle("Bold")));
    }
}

void TransportClock::timerCallback()
{
    updateClockDisplay();
}

void TransportClock::updateClockDisplay()
{
    // 1) Pull latest snapshot (fast path: atomic pointer, with fallback to FIFO)
    MyPluginAudioProcessor::ClockSnapshot snap = lastClock;
    if (auto* p = proc.lastClockForUI.load(std::memory_order_acquire))
        snap = *p;
    else
    {
        int s1, n1, s2, n2;
        proc.clockFifo.prepareToRead(1, s1, n1, s2, n2);
        if (n1 > 0) snap = proc.clockRing[(size_t)s1];
        proc.clockFifo.finishedRead(n1 + n2);
    }
    lastClock = snap;

    // 2) Latency-compensated display time
    const double visSecTarget = juce::jmax(0.0, (snap.samplePos - snap.latencySamples) / juce::jmax(1.0, snap.sampleRate));

    // 3) Slew when playing, snap when stopped or big discontinuity
    const double nowMs = juce::Time::getMillisecondCounterHiRes();
    const double frameDt = (lastClockUpdateMs == 0) ? (1.0 / UPDATE_HZ) : (nowMs - lastClockUpdateMs) / 1000.0;
    lastClockUpdateMs = (juce::uint32)nowMs;

    const double shown =
        (!snap.playing || std::abs(visSecTarget - clockSmoother.shownSec) > 0.35)
        ? (clockSmoother.snapTo(visSecTarget), visSecTarget)
        : clockSmoother.stepToward(visSecTarget, frameDt, 8.0); // up to 8 sec/sec slew

    // 4) Update label only when text actually changes (reduces repaints)
    const juce::String text = formatClock(shown);
    if (clockLabel.getText() != text)
        clockLabel.setText(text, juce::dontSendNotification);

    // Optional: show ▶ and 🔁 icons subtly
    clockLabel.setTooltip(
        juce::String(snap.playing ? "▶ Playing" : "❚❚ Stopped")
        + (snap.looping ? " • 🔁 Loop" : "")
        + juce::String(" • ") + juce::String(snap.bpm, 2) + " BPM"
    );
}
