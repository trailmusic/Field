#pragma once
#include <JuceHeader.h>
#include "ParamPatch.h"

class MachineEngine
{
public:
    enum class Quality { Fast=0, Standard=1, Deep=2 };
    enum class Target  { Streaming=0, Club=1, Podcast=2, Reference=3 };

    void setSampleRate (double sr) { sampleRate = sr; }
    void push (const float* L, const float* R, int n);
    void analyzeAsync (Target t, Quality q);
    void cancel();
    bool hasResults() const;
    std::vector<ParamPatch> takeResults();

    void setUsePre (bool b) { usePre.store (b); }
    void setFreeze (bool b) { freeze.store (b); }

private:
    double sampleRate { 48000.0 };
    juce::AbstractFifo fifo { 1<<18 };
    juce::AudioBuffer<float> buf { 2, 1<<18 };
    std::atomic<bool> usePre { false }, freeze { false };
    std::atomic<bool> running { false }, cancelFlag { false };
    mutable juce::CriticalSection resLock;
    std::vector<ParamPatch> lastResults;
};


