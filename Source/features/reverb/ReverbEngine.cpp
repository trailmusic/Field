#include "ReverbEngine.h"

using namespace juce;

void ReverbEngine::prepare (double sr, int maxBlock, int channels)
{
    sampleRate = sr; maxSamples = maxBlock; chans = jmax (1, channels);
    erBuf.setSize (chans, maxSamples);
    tailBuf.setSize (chans, maxSamples);
    tmpBuf.setSize (chans, maxSamples);
    
    // Prepare Early Reflections system
    earlyReflections.prepare (chans, sampleRate);
}

void ReverbEngine::reset ()
{
}

void ReverbEngine::setParams (const ReverbParams& p)
{
    // Configure Early Reflections
    earlyReflections.setParams(p.erTimeMs, p.erDensity, p.erWidthPct, p.erLevelDb);
    
}

void ReverbEngine::processWet (AudioBuffer<float>& wet, const AudioBuffer<float>& sidechain)
{
    ignoreUnused (sidechain);
    
    // Process Early Reflections
    earlyReflections.process(wet, erBuf);
    
    // For now, copy ER to tail (will be replaced with FDN in Phase 2)
    tailBuf.makeCopyOf(erBuf);
    // Meters
    auto rms = [] (const AudioBuffer<float>& b)
    {
        long double s = 0.0; const int ch = b.getNumChannels(), n = b.getNumSamples();
        for (int c=0;c<ch;++c){ const float* d=b.getReadPointer(c); for (int i=0;i<n;++i) s += (long double) d[i]*d[i]; }
        const double v = std::sqrt ((double) s / jmax (1, ch*n));
        return (float) v;
    };
    erRms .store (rms (erBuf));
    tailRms.store (rms (tailBuf));
    // --- Wet dynamic EQ (multi-band placeholder detector) -----------------------
    const int N = tailBuf.getNumSamples();
    const int C = tailBuf.getNumChannels();
    AudioBuffer<float> work (C, N);
    work.makeCopyOf (tailBuf);


    // Sum ER+Tail into wet
    wet.makeCopyOf (tailBuf);
    duckGrDb.store (0.f);
}

// ================================================================
// Early Reflections Implementation
// ================================================================

void ReverbEngine::EarlyReflections::prepare(int channels, double sr)
{
    sampleRate = sr;
    numTaps = 0;
    delayLines.clear();
    delayIndices.clear();
    tapFilters.clear();
    
    // Initialize delay lines for each channel
    delayLines.resize(channels);
    delayIndices.resize(channels, 0);
    
    // Initialize filters for each tap
    tapFilters.resize(MAX_ER_TAPS);
    
    // Generate initial ER tap configuration
    generateERTaps();
}

void ReverbEngine::EarlyReflections::reset()
{
    // Clear all delay lines
    for (auto& line : delayLines)
        std::fill(line.begin(), line.end(), 0.0f);
    
    // Reset delay indices
    std::fill(delayIndices.begin(), delayIndices.end(), 0);
    
    // Reset filter states
    for (auto& filter : tapFilters)
    {
        filter.z1.assign(delayLines.size(), 0.0f);
        filter.z2.assign(delayLines.size(), 0.0f);
    }
}

void ReverbEngine::EarlyReflections::process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output)
{
    const int numChannels = input.getNumChannels();
    const int numSamples = input.getNumSamples();
    
    // Clear output
    output.clear();
    
    // Process each ER tap
    for (int tap = 0; tap < numTaps; ++tap)
    {
        const auto& tapData = taps[tap];
        const int delaySamples = static_cast<int>(tapData.delayMs * sampleRate / 1000.0f);
        
        // Process each channel
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* inputData = input.getReadPointer(ch);
            float* outputData = output.getWritePointer(ch);
            
            // Apply delay
            for (int i = 0; i < numSamples; ++i)
            {
                // Read from delay line
                float delayedSample = 0.0f;
                if (delaySamples > 0 && delaySamples < static_cast<int>(delayLines[ch].size()))
                {
                    int readIndex = (delayIndices[ch] - delaySamples + static_cast<int>(delayLines[ch].size())) % static_cast<int>(delayLines[ch].size());
                    delayedSample = delayLines[ch][readIndex];
                }
                
                // Apply gain and pan
                float processedSample = delayedSample * tapData.gain;
                
                // Apply panning (simple stereo panning)
                if (numChannels == 2)
                {
                    if (ch == 0) // Left channel
                        processedSample *= (1.0f - tapData.pan) * 0.5f;
                    else // Right channel
                        processedSample *= (1.0f + tapData.pan) * 0.5f;
                }
                
                // Apply filter
                if (tap < static_cast<int>(tapFilters.size()))
                {
                    auto& filter = tapFilters[tap];
                    if (filter.z1.size() > static_cast<size_t>(ch))
                    {
                        // Simple filter processing (placeholder - will be enhanced)
                        processedSample = filter.processInPlace(processedSample, ch);
                    }
                }
                
                // Write to output
                outputData[i] += processedSample;
                
                // Update delay line
                delayLines[ch][delayIndices[ch]] = inputData[i];
                delayIndices[ch] = (delayIndices[ch] + 1) % static_cast<int>(delayLines[ch].size());
            }
        }
    }
}

void ReverbEngine::EarlyReflections::setParams(float erTimeMs, float erDensity, float erWidthPct, float erLevelDb)
{
    // Update ER configuration based on parameters
    generateERTaps();
    
    // Apply level scaling
    float levelScale = juce::Decibels::decibelsToGain(erLevelDb);
    for (int i = 0; i < numTaps; ++i)
    {
        taps[i].gain *= levelScale;
    }
}

void ReverbEngine::EarlyReflections::generateERTaps()
{
    // Generate ER taps based on current parameters
    // This is a simplified implementation - will be enhanced with proper room modeling
    
    numTaps = 16; // Start with 16 taps
    
    // Generate tap delays (exponential distribution)
    for (int i = 0; i < numTaps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(numTaps - 1);
        taps[i].delayMs = 5.0f + t * 50.0f; // 5ms to 55ms range
        taps[i].gain = 0.8f * std::exp(-t * 2.0f); // Exponential decay
        taps[i].pan = (static_cast<float>(i % 2) - 0.5f) * 2.0f; // Alternating pan
        taps[i].filterFreq = 1000.0f + t * 4000.0f; // Frequency sweep
        taps[i].filterQ = 0.707f;
    }
    
    // Resize delay lines to accommodate maximum delay
    int maxDelaySamples = static_cast<int>(55.0f * sampleRate / 1000.0f);
    for (auto& line : delayLines)
    {
        line.resize(maxDelaySamples, 0.0f);
    }
}


