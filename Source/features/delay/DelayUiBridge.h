#pragma once

#include <JuceHeader.h>

// A lock-free, RT-safe single-producer/single-consumer bridge for Delay visuals.
// Audio thread pushes one frame per audio block; UI thread polls at ~30-60 Hz.
struct DelayMetricsFrame
{
    // Time / Sync
    double tempoBpm = 120.0; bool sync = true; int timeDiv = 4; int gridFlavor = 0; double timeMs = 350.0;
    // Stereo / Time
    double timeSamplesL = 0.0; double timeSamplesR = 0.0; double stereoSpreadPct = 0.0; bool pingpong = false; double crossfeedPct = 0.0; double width = 1.0;
    // Feedback / Wet
    double feedbackPct = 0.0; double wet01 = 0.0; bool killDry = false; bool freeze = false;
    // Modulation
    int mode = 0; double modRateHz = 0.0; double modDepthMs = 0.0; double jitterPct = 0.0;
    // Tone / Diffusion
    double hpHz = 120.0; double lpHz = 12000.0; double tiltDb = 0.0; double sat = 0.0; double diffusion = 0.0; double diffuseSizeMs = 18.0;
    // Ducking
    int duckSource = 0; bool duckPost = true; double duckThrDb = -26.0; double duckRatio = 2.0; double duckDepth = 0.0; double duckAtkMs = 10.0; double duckRelMs = 200.0; double duckLookMs = 5.0;
    // Live meters
    float duckGrDb = 0.0f; float preRmsL = 0.0f; float preRmsR = 0.0f; float postRmsL = 0.0f; float postRmsR = 0.0f; float wetRmsL = 0.0f; float wetRmsR = 0.0f;
};

class DelayUiBridge
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels)
    {
        juce::ignoreUnused (sampleRate, maxBlockSize, numChannels);
        fifo.reset();
        frames.ensureStorageAllocated (capacity);
        frames.clearQuick();
        // Pre-fill storage so we can memcpy into contiguous backing store
        frames.resize (capacity);
        writeIndex = 0; readIndex = -1; // -1 means none available yet
    }

    // Called on audio thread once per block (lock-free)
    inline void pushMetrics (const DelayMetricsFrame& frame) noexcept
    {
        int start1, size1, start2, size2;
        fifo.prepareToWrite (1, start1, size1, start2, size2);
        if (size1 > 0) frames.setUnchecked (start1, frame);
        if (size2 > 0) frames.setUnchecked (start2, frame);
        fifo.finishedWrite (size1 + size2);
    }

    // Called on UI thread; returns true if a frame was read
    inline bool pullLatest (DelayMetricsFrame& out)
    {
        int start1, size1, start2, size2;
        // Drain all but keep only last to avoid UI backlog
        int available = fifo.getNumReady();
        if (available <= 0) return false;
        fifo.prepareToRead (available, start1, size1, start2, size2);
        DelayMetricsFrame last;
        if (size1 > 0) last = frames.getReference (start1 + size1 - 1);
        if (size2 > 0) last = frames.getReference (start2 + size2 - 1);
        fifo.finishedRead (size1 + size2);
        out = last;
        return true;
    }

    void clear() { fifo.reset(); }

private:
    static constexpr int capacity = 256; // frames
    juce::AbstractFifo fifo { capacity };
    juce::Array<DelayMetricsFrame> frames;
    int writeIndex = 0;
    int readIndex = -1;
};


