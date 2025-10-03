#pragma once
#include <juce_dsp/juce_dsp.h>

// Phase processing banks for different phase modes
struct PhaseBanks
{
    // State
    double sampleRate = 48000.0;
    int blockSize = 512;
    int numChannels = 2;
    int currentPhaseMode = 0;
    
    // Latency bookkeeping (derive from actual kernels if available)
    int naturalLatencySamples   = 0;      // typically 0 for minimum-phase/IIR
    int hybridKernelLen         = 1025;   // placeholder until wired to real FIR
    int hybridLatencySamples    = (1025 - 1) / 2;
    int fullLinearKernelLen     = 4097;   // placeholder until wired to real FIR
    int fullLinearLatencySamples= (4097 - 1) / 2;
    
    // Reusable temp buffer for double path conversion (avoid per-block alloc)
    juce::AudioBuffer<float> tempFloat;
    
    // FIR/IIR processing objects (to be implemented based on your existing phase system)
    // These would contain your existing FIR/IIR filter banks
    
    void prepare(double sr, int block, int channels, int phaseMode)
    {
        sampleRate = sr;
        blockSize = block;
        numChannels = channels;
        currentPhaseMode = phaseMode;
        
        // Ensure reusable buffer sized (no RT allocations in process())
        tempFloat.setSize (juce::jmax (1, channels), juce::jmax (1, block), false, false, true);
        
        // Initialize phase processing based on mode
        switch (phaseMode)
        {
            case 0: // Zero phase - IIR only
                prepareZeroPhase();
                break;
            case 1: // Natural - minimum-phase / low-latency path
                prepareNatural();
                break;
            case 2: // Hybrid - FIR for HP/LP, IIR for tone
                prepareHybridLinear();
                break;
            case 3: // Full Linear - all FIR
                prepareFullLinear();
                break;
            default:
                prepareZeroPhase();
                break;
        }
    }
    
    void reset()
    {
        // TODO: reset/clear any internal filter or convolver states when added
        // Keeping placeholder for now to maintain API
    }
    
    int latencyFor(int phaseMode) const
    {
        switch (phaseMode)
        {
            case 0: return 0;                                       // Zero
            case 1: return naturalLatencySamples;                    // Natural
            case 2: return hybridLatencySamples;                     // Hybrid (FIR HP/LP)
            case 3: return fullLinearLatencySamples;                 // Full Linear FIR
            default: return 0;
        }
    }
    
    void process(juce::dsp::AudioBlock<float>& block)
    {
        // Process based on current phase mode
        switch (currentPhaseMode)
        {
            case 0: processZeroPhase(block);   break;
            case 1: processNatural(block);     break;
            case 2: processHybridLinear(block);break;
            case 3: processFullLinear(block);  break;
            default: /* no-op */               break;
        }
    }
    
    void process(juce::dsp::AudioBlock<double>& block)
    {
        // Double path: reuse preallocated float buffer (tempFloat) to avoid RT alloc
        const int C = (int) block.getNumChannels();
        const int N = (int) block.getNumSamples();
        if (tempFloat.getNumChannels() < C || tempFloat.getNumSamples() < N)
            tempFloat.setSize (juce::jmax (tempFloat.getNumChannels(), C), juce::jmax (tempFloat.getNumSamples(), N), false, false, true);

        // Convert double -> float
        for (int ch = 0; ch < C; ++ch)
        {
            const double* src = block.getChannelPointer(ch);
            float*        dst = tempFloat.getWritePointer(ch);
            for (int i = 0; i < N; ++i) dst[i] = (float) src[i];
        }

        // Process as float
        juce::dsp::AudioBlock<float> floatBlock (tempFloat);
        process(floatBlock);

        // Convert back float -> double
        for (int ch = 0; ch < C; ++ch)
        {
            const float*  src = tempFloat.getReadPointer(ch);
            double*       dst = block.getChannelPointer(ch);
            for (int i = 0; i < N; ++i) dst[i] = (double) src[i];
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
        // TODO: set hybridKernelLen from actual FIR; derive latency
        hybridLatencySamples = (juce::jmax (1, hybridKernelLen) - 1) / 2;
    }
    
    void prepareFullLinear()
    {
        // Initialize full FIR setup
        // All processing through FIR filters
        // TODO: set fullLinearKernelLen from actual FIR; derive latency
        fullLinearLatencySamples = (juce::jmax (1, fullLinearKernelLen) - 1) / 2;
    }

    void prepareNatural()
    {
        // Initialize minimum-phase / low-latency chain (usually IIR or min-phase FIR)
        naturalLatencySamples = 0; // update if any lookahead is used
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
        juce::ignoreUnused(block);
    }
    
    void processFullLinear(juce::dsp::AudioBlock<float>& block)
    {
        // Full linear processing - all FIR
        // This would delegate to your existing FIR processing
        juce::ignoreUnused(block);
    }

    void processNatural(juce::dsp::AudioBlock<float>& block)
    {
        // Natural processing (minimum phase / low-latency)
        // Delegate to your existing IIR/min-phase stage
        juce::ignoreUnused(block);
    }
};
