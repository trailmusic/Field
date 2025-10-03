#pragma once

#include <JuceHeader.h>

/*
====================================================================================================
 DecayLearnManager
 ---------------------------------------------------------------------------------------------------
 Purpose
    Manages the Sidechain Learn feature for decay profile learning.
    Captures spectral energy from external signals and learns appropriate decay profiles.

 Architecture
    - State machine: Idle → Capturing → Solving → Ready → Error
    - Lock-free capture ring for audio thread safety
    - Background solver thread for analysis
    - No allocations in audio thread

 Thread Safety
    - Audio thread: onBlock() only (O(1) operations)
    - Background thread: solveAsync() for analysis
    - State changes are atomic

 Integration
    - Lives inside FieldChain<Sample>
    - Uses existing detector signal (Dry/ER/Tail/Wet)
    - Integrates with existing DecayProfileSmoother

 Notes
    - Phase A: Scaffolding only (no audio changes)
    - Phase B: Full solver implementation
====================================================================================================
*/

template<typename Sample>
struct DecayLearnManager
{
    // ================================================================
    // 🎯 SIDECHAIN LEARN SYSTEM (JANUARY 2025)
    // ================================================================
    // CRITICAL: Auto-learn decay profiles from external signals
    // These parameters enable intelligent decay curve learning
    // ================================================================
    
    // Configuration
    static constexpr int kBands = 24;
    static constexpr int kDecim = 4;   // Decimate blocks to lighten UI/solve load
    
    // State machine
    enum class State { Idle, Capturing, Solving, Ready, Error };
    std::atomic<State> state { State::Idle };
    
    // Capture ring (lock-free single writer: audio thread; single reader: solver)
    struct BandRMS {
        float env = 0.0f;  // Recursive RMS per band (EMA)
    };
    std::array<BandRMS, kBands> ema{};      // Real-time accumulators
    std::vector<std::array<float, kBands>> frames; // Preallocated in prepare()
    int framesWrite = 0, framesSize = 0, decimCtr = 0;
    
    // Learned profile (lowMult, midGainDb, highMult)
    std::atomic<bool> hasLearned { false };
    std::atomic<float> learnedLo  { 1.0f };
    std::atomic<float> learnedMid { 0.0f }; // dB around ~1.5 kHz
    std::atomic<float> learnedHi  { 1.0f };
    
    // Control parameters
    double sr = 48000.0;
    double windowSec = 4.0;
    float  strength = 0.5f;
    
    // API
    void prepare(double sampleRate, int maxFrames);
    void resetAll();
    void startCapture(double windowSeconds);
    void abort();
    void onBlock(const juce::AudioBuffer<Sample>& detector);
    void requestSolve(juce::ThreadPool& bg);
    bool fetchLearned(float& lo, float& midDb, float& hi) const;
    
    // State queries
    State getState() const { return state.load(std::memory_order_relaxed); }
    bool isCapturing() const { return getState() == State::Capturing; }
    bool isReady() const { return getState() == State::Ready; }
    bool hasLearnedProfile() const { return hasLearned.load(std::memory_order_relaxed); }
    
    // Control
    void setStrength(float s) { strength = juce::jlimit(0.0f, 1.0f, s); }
    void setWindow(double seconds) { windowSec = juce::jlimit(2.0, 8.0, seconds); }
    
private:
    // Internal helpers
    float estimateBandRMSCheap(const juce::AudioBuffer<Sample>& x, int band);
    void updateEMA(int band, float rms);
};

// Background solver job
template<typename Sample>
struct DecayLearnSolveJob : public juce::ThreadPoolJob
{
    DecayLearnManager<Sample>* manager;
    
    DecayLearnSolveJob(DecayLearnManager<Sample>* m) 
        : ThreadPoolJob("DecayLearnSolve"), manager(m) {}
    
    JobStatus runJob() override
    {
        // Phase A: Placeholder - no actual solving yet
        // Phase B: Implement 24-band solver here
        if (manager)
        {
            manager->hasLearned.store(true);
            manager->state.store(DecayLearnManager<Sample>::State::Ready);
        }
        return jobHasFinished;
    }
};
