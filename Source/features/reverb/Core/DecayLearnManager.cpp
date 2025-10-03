#include "DecayLearnManager.h"

template<typename Sample>
void DecayLearnManager<Sample>::prepare(double sampleRate, int maxFrames)
{
    sr = sampleRate;
    framesSize = maxFrames;
    frames.assign((size_t)framesSize, std::array<float, kBands>{});
    framesWrite = 0; 
    decimCtr = 0; 
    hasLearned.store(false);
    state.store(State::Idle);
    
    // Reset EMA accumulators
    for (auto& band : ema)
    {
        band.env = 0.0f;
    }
}

template<typename Sample>
void DecayLearnManager<Sample>::resetAll()
{
    state.store(State::Idle);
    hasLearned.store(false);
    framesWrite = 0;
    decimCtr = 0;
    
    // Reset learned values
    learnedLo.store(1.0f);
    learnedMid.store(0.0f);
    learnedHi.store(1.0f);
    
    // Reset EMA accumulators
    for (auto& band : ema)
    {
        band.env = 0.0f;
    }
}

template<typename Sample>
void DecayLearnManager<Sample>::startCapture(double windowSeconds)
{
    windowSec = juce::jlimit(2.0, 8.0, windowSeconds);
    framesWrite = 0; 
    decimCtr = 0; 
    hasLearned.store(false);
    
    // Reset EMA accumulators
    for (auto& band : ema)
    {
        band.env = 0.0f;
    }
    
    state.store(State::Capturing);
}

template<typename Sample>
void DecayLearnManager<Sample>::abort()
{
    state.store(State::Idle);
}

template<typename Sample>
void DecayLearnManager<Sample>::onBlock(const juce::AudioBuffer<Sample>& x)
{
    if (state.load(std::memory_order_relaxed) != State::Capturing) 
        return;

    // 1) Decimate at block level
    if (++decimCtr < kDecim) 
        return;
    decimCtr = 0;

    // 2) Compute band RMS with simple log-spaced IIR banks (very cheap)
    for (int b = 0; b < kBands; ++b) 
    {
        float rms = estimateBandRMSCheap(x, b);
        updateEMA(b, rms);
        frames[(size_t)framesWrite][(size_t)b] = ema[(size_t)b].env;
    }

    if (++framesWrite >= framesSize) 
    {
        state.store(State::Solving);
    }
}

template<typename Sample>
void DecayLearnManager<Sample>::requestSolve(juce::ThreadPool& bg)
{
    if (state.load() != State::Solving) 
        return;
    
    bg.addJob(new DecayLearnSolveJob<Sample>(this), true); // Takes ownership
}

template<typename Sample>
bool DecayLearnManager<Sample>::fetchLearned(float& lo, float& midDb, float& hi) const
{
    if (!hasLearned.load()) 
        return false;
    
    lo = learnedLo.load();
    midDb = learnedMid.load();
    hi = learnedHi.load();
    return true;
}

template<typename Sample>
float DecayLearnManager<Sample>::estimateBandRMSCheap(const juce::AudioBuffer<Sample>& x, int band)
{
    // Phase A: Simple placeholder - just return RMS of entire signal
    // Phase B: Implement proper band analysis here
    
    float sum = 0.0f;
    int numSamples = x.getNumSamples() * x.getNumChannels();
    
    for (int ch = 0; ch < x.getNumChannels(); ++ch)
    {
        const Sample* data = x.getReadPointer(ch);
        for (int i = 0; i < x.getNumSamples(); ++i)
        {
            sum += data[i] * data[i];
        }
    }
    
    return std::sqrt(sum / numSamples);
}

template<typename Sample>
void DecayLearnManager<Sample>::updateEMA(int band, float rms)
{
    // EMA with tau ~ 80ms
    const float alpha = 1.0f - std::exp(-(float)decimCtr / (float)sr / 0.08f);
    ema[(size_t)band].env += (rms - ema[(size_t)band].env) * alpha;
}

// Explicit template instantiations
template struct DecayLearnManager<float>;
template struct DecayLearnManager<double>;
template struct DecayLearnSolveJob<float>;
template struct DecayLearnSolveJob<double>;
