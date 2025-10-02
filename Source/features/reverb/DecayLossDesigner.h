#pragma once
/*
====================================================================================================
 DecayLossDesigner — Converts Decay-Rate EQ UI to FDN feedback loss coefficients
----------------------------------------------------------------------------------------------------
 Purpose
    - Maps Decay-Rate EQ multipliers to per-band T60 targets
    - Converts T60(f) to feedback loss coefficients a(f) for FDN lines
    - Provides smooth coefficient updates to prevent clicks

 Design
    - Takes DecayRateProfile from UI (Bell/TiltLo/TiltHi bands)
    - Computes target T60(f) curve across frequency spectrum
    - Converts to per-line loss coefficients with smoothing
    - Handles coefficient interpolation for smooth parameter changes

 Usage
    - Call setDecayProfile() when Decay-Rate EQ changes
    - Call getLossCoeffs() to get current coefficients for FDN lines
    - Coefficients are smoothed to prevent audio artifacts
====================================================================================================
*/

#include <JuceHeader.h>
#include "ReverbTypes.h"

namespace fieldverb
{
class DecayLossDesigner
{
public:
    void prepare (double sampleRate, int numLines)
    {
        fs = sampleRate;
        numFDNLines = numLines;
        
        // Initialize coefficient smoothing
        smoothingTimeMs = 50.0; // 50ms smoothing
        tauSec = smoothingTimeMs * 0.001;
        
        // Initialize coefficient arrays
        targetCoeffs.assign(numLines, 1.0f);
        currentCoeffs.assign(numLines, 1.0f);
        
        // Default: uniform decay across all lines
        setBaseT60(1.8f);
    }
    
    void setDecayProfile (const DecayRateProfile& profile)
    {
        if (profile.bands.empty())
        {
            // No decay-rate shaping - use base T60 for all lines
            setBaseT60(baseT60Sec);
            return;
        }
        
        // Compute target T60(f) curve from Decay-Rate EQ
        computeTargetT60Curve(profile);
        
        // Map T60 curve to per-line loss coefficients
        mapT60ToLossCoeffs();
    }
    
    void setBaseT60 (float seconds)
    {
        baseT60Sec = juce::jlimit(0.1f, 60.0f, seconds);
        
        // Set all lines to base T60 (will be converted to feedback gains later)
        for (int i = 0; i < numFDNLines; ++i)
        {
            targetCoeffs[i] = baseT60Sec;
        }
    }
    
    void updateCoeffs (int blockSize = 1)
    {
        // Correct smoothing coefficient calculation
        const double aBlock = std::exp(-(double)blockSize / (fs * tauSec));
        
        // Smooth coefficient updates to prevent clicks
        for (int i = 0; i < numFDNLines; ++i)
        {
            currentCoeffs[i] = (float)(targetCoeffs[i] + aBlock * (currentCoeffs[i] - targetCoeffs[i]));
        }
    }
    
    const std::vector<float>& getLossCoeffs() const { return currentCoeffs; }
    
    // Convert T60 curve to per-line feedback gains using line delays
    void designLossPerLine (const std::vector<double>& lineDelaySec, std::vector<float>& out_g)
    {
        out_g.resize(numFDNLines);
        
        for (int i = 0; i < numFDNLines; ++i)
        {
            // Get T60 for this line's representative frequency
            const float lineFreq = lineFreqHzFromDelay(lineDelaySec[i]);
            const float t60ForLine = logInterp(t60Freqs, t60Curve, lineFreq);
            
            // Convert to per-cycle feedback gain: g = 10^(-3 * T_rt / T60)
            const double Trt = lineDelaySec[i]; // round-trip time in seconds
            const double g = std::pow(10.0, -3.0 * Trt / t60ForLine);
            out_g[i] = (float)juce::jlimit(0.0, 0.999999, g);
        }
    }
    
private:
    void computeTargetT60Curve (const DecayRateProfile& profile)
    {
        // Frequency analysis points (log-spaced from 20Hz to 20kHz)
        const int numFreqPoints = 64;
        std::vector<float> freqHz(numFreqPoints);
        std::vector<float> t60Target(numFreqPoints);
        
        // Generate log-spaced frequency points
        for (int i = 0; i < numFreqPoints; ++i)
        {
            const float logFreq = std::log10(20.0f) + (std::log10(20000.0f) - std::log10(20.0f)) * i / (numFreqPoints - 1);
            freqHz[i] = std::pow(10.0f, logFreq);
            t60Target[i] = baseT60Sec; // Start with base T60
        }
        
        // Apply Decay-Rate EQ bands to T60 curve
        for (const auto& band : profile.bands)
        {
            applyDecayRateBand(freqHz, t60Target, band);
        }
        
        // Store the computed T60 curve for mapping to FDN lines
        t60Curve = t60Target;
        t60Freqs = freqHz;
    }
    
    void applyDecayRateBand (const std::vector<float>& freqs, std::vector<float>& t60, const DecayRateBand& band)
    {
        const float centerFreq = band.freqHz;
        const float mult = band.mult; // Decay multiplier (0.5x = faster decay, 2.0x = slower)
        const float q = band.q;
        
        for (size_t i = 0; i < freqs.size(); ++i)
        {
            const float f = freqs[i];
            float gain = 1.0f;
            
            switch (band.type)
            {
                case DecayRateBand::Bell:
                    gain = computeBellResponse(f, centerFreq, q);
                    break;
                case DecayRateBand::TiltLo:
                    gain = computeTiltLoResponse(f, centerFreq, q);
                    break;
                case DecayRateBand::TiltHi:
                    gain = computeTiltHiResponse(f, centerFreq, q);
                    break;
            }
            
            // Apply gain to decay multiplier
            const float effectiveMult = 1.0f + (mult - 1.0f) * gain;
            t60[i] *= effectiveMult;
        }
    }
    
    float computeBellResponse (float freq, float centerFreq, float q) const
    {
        const float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)fs;
        const float w0 = 2.0f * juce::MathConstants<float>::pi * centerFreq / (float)fs;
        const float alpha = std::sin(w0) / (2.0f * q);
        const float cosw = std::cos(w);
        const float cosw0 = std::cos(w0);
        
        const float num = 1.0f + alpha * alpha;
        const float den = 1.0f + alpha * alpha - 2.0f * alpha * cosw0 * cosw + alpha * alpha * cosw * cosw;
        
        return num / den;
    }
    
    float computeTiltLoResponse (float freq, float centerFreq, float q) const
    {
        // Low-shelf response for low-frequency decay shaping
        const float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)fs;
        const float w0 = 2.0f * juce::MathConstants<float>::pi * centerFreq / (float)fs;
        const float alpha = std::sin(w0) / (2.0f * q);
        const float cosw = std::cos(w);
        const float cosw0 = std::cos(w0);
        
        const float num = 1.0f + alpha * alpha + 2.0f * alpha * cosw0 * cosw;
        const float den = 1.0f + alpha * alpha - 2.0f * alpha * cosw0 * cosw;
        
        return num / den;
    }
    
    float computeTiltHiResponse (float freq, float centerFreq, float q) const
    {
        // High-shelf response for high-frequency decay shaping
        const float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)fs;
        const float w0 = 2.0f * juce::MathConstants<float>::pi * centerFreq / (float)fs;
        const float alpha = std::sin(w0) / (2.0f * q);
        const float cosw = std::cos(w);
        const float cosw0 = std::cos(w0);
        
        const float num = 1.0f + alpha * alpha - 2.0f * alpha * cosw0 * cosw;
        const float den = 1.0f + alpha * alpha + 2.0f * alpha * cosw0 * cosw;
        
        return num / den;
    }
    
    void mapT60ToLossCoeffs()
    {
        // Map T60 curve to per-line T60 values (not coefficients yet)
        // Each FDN line represents a different frequency range
        for (int line = 0; line < numFDNLines; ++line)
        {
            // Map line index to frequency range
            const float lineFreq = mapLineToFrequency(line);
            
            // Find closest T60 value in our curve
            float t60ForLine = baseT60Sec;
            if (!t60Curve.empty())
            {
                // Use log interpolation for better frequency mapping
                t60ForLine = logInterp(t60Freqs, t60Curve, lineFreq);
            }
            
            // Store T60 value (will be converted to feedback gains later)
            targetCoeffs[line] = t60ForLine;
        }
    }
    
    float mapLineToFrequency (int lineIndex) const
    {
        // Map FDN line index to representative frequency
        // Lines with longer delays typically represent lower frequencies
        const float minFreq = 100.0f;  // 100 Hz
        const float maxFreq = 8000.0f; // 8 kHz
        const float ratio = (float)lineIndex / (float)(numFDNLines - 1);
        return minFreq + ratio * (maxFreq - minFreq);
    }
    
    // Convert line delay to representative frequency
    static float lineFreqHzFromDelay (double lineDelaySec)
    {
        // Half-wave proxy; clamp to sensible band
        const double f = 1.0 / juce::jmax(1e-6, 2.0 * lineDelaySec);
        return (float)juce::jlimit(20.0, 20000.0, f);
    }
    
    // Log-space interpolation helper
    static float logInterp (const std::vector<float>& xHz, const std::vector<float>& y, float fHz)
    {
        if (xHz.empty() || y.empty()) return 1.0f;
        
        const float lf = std::log10(fHz);
        const float l0 = std::log10(xHz.front());
        const float l1 = std::log10(xHz.back());
        const float t = juce::jlimit(0.0f, 1.0f, (lf - l0) / (l1 - l0));
        
        // Find bin
        const int N = (int)xHz.size();
        const float idx = t * (N - 1);
        const int i0 = juce::jlimit(0, N-2, (int)std::floor(idx));
        const float frac = idx - (float)i0;
        
        return y[i0] * (1.0f - frac) + y[i0 + 1] * frac;
    }
    
    double fs = 48000.0;
    int numFDNLines = 8;
    float baseT60Sec = 1.8f;
    
    // Coefficient smoothing
    double smoothingTimeMs = 50.0;
    double tauSec = 0.05;
    
    // T60 curve computation
    std::vector<float> t60Curve;
    std::vector<float> t60Freqs;
    
    // Loss coefficients
    std::vector<float> targetCoeffs;
    std::vector<float> currentCoeffs;
};
} // namespace fieldverb
