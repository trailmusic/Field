#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Phase processing banks for different phase modes
struct PhaseBanks
{
    // State
    double sampleRate = 48000.0;
    int blockSize = 512;
    int numChannels = 2;
    int currentPhaseMode = 0;
    
    // FIR/IIR processing objects (to be implemented based on your existing phase system)
    // These would contain your existing FIR/IIR filter banks
    
    void prepare(double sr, int block, int channels, int phaseMode)
    {
        sampleRate = sr;
        blockSize = block;
        numChannels = channels;
        currentPhaseMode = phaseMode;
        
        // Initialize phase processing based on mode
        switch (phaseMode)
        {
            case 0: // Zero phase - IIR only
                prepareZeroPhase();
                break;
            case 2: // Hybrid - FIR for HP/LP, IIR for tone
                prepareHybridLinear();
                break;
            case 3: // Full Linear - all FIR
                prepareFullLinear();
                break;
        }
    }
    
    int latencyFor(int phaseMode) const
    {
        switch (phaseMode) 
        { 
            case 0: return 0;      // Zero phase - no latency
            case 2: return 64;     // Hybrid - moderate latency
            case 3: return 256;     // Full Linear - higher latency
            default: return 0; 
        }
    }
    
    void process(juce::dsp::AudioBlock<float>& block)
    {
        // Process based on current phase mode
        switch (currentPhaseMode)
        {
            case 0: processZeroPhase(block); break;
            case 2: processHybridLinear(block); break;
            case 3: processFullLinear(block); break;
        }
    }
    
    void process(juce::dsp::AudioBlock<double>& block)
    {
        // For double precision, convert to float, process, convert back
        // This is a placeholder - you'd implement the actual double precision processing
        juce::AudioBuffer<float> floatBuffer(block.getNumChannels(), (int)block.getNumSamples());
        
        // Convert double to float
        for (int ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* src = block.getChannelPointer(ch);
            auto* dst = floatBuffer.getWritePointer(ch);
            for (int i = 0; i < (int)block.getNumSamples(); ++i)
                dst[i] = static_cast<float>(src[i]);
        }
        
        // Process as float
        juce::dsp::AudioBlock<float> floatBlock(floatBuffer);
        process(floatBlock);
        
        // Convert back to double
        for (int ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* src = floatBuffer.getReadPointer(ch);
            auto* dst = block.getChannelPointer(ch);
            for (int i = 0; i < (int)block.getNumSamples(); ++i)
                dst[i] = static_cast<double>(src[i]);
        }
    }
    
private:
    void prepareZeroPhase()
    {
        // Initialize IIR filters for zero phase mode
        // This would use your existing IIR filter setup
    }
    
    void prepareHybridLinear()
    {
        // Initialize hybrid FIR/IIR setup
        // FIR for HP/LP, IIR for tone processing
    }
    
    void prepareFullLinear()
    {
        // Initialize full FIR setup
        // All processing through FIR filters
    }
    
    void processZeroPhase(juce::dsp::AudioBlock<float>& block)
    {
        // Zero phase processing - IIR filters only
        // This would delegate to your existing IIR processing
    }
    
    void processHybridLinear(juce::dsp::AudioBlock<float>& block)
    {
        // Hybrid processing - FIR for HP/LP, IIR for tone
        // This would delegate to your existing hybrid processing
    }
    
    void processFullLinear(juce::dsp::AudioBlock<float>& block)
    {
        // Full linear processing - all FIR
        // This would delegate to your existing FIR processing
    }
};
