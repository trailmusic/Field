#pragma once

#include <JuceHeader.h>
#include <array>
#include <memory>

// Forward declaration for MinPhaseBank integration
namespace MinPhaseBank {
    struct TapSet { 
        const double* data; 
        int length; 
        int order; 
    };
    
    extern const TapSet registry[];
    extern const int registryCount;
}

/**
 * MinPhaseBank Integration for Field Plugin
 * =========================================
 * 
 * This class provides integration between Field Ranger's MinPhaseBank.h
 * and Field's oversampling system. It allows the plugin to use
 * professionally designed minimum-phase FIR filters for oversampling.
 */
class MinPhaseBankIntegration
{
public:
    MinPhaseBankIntegration();
    ~MinPhaseBankIntegration() = default;
    
    /**
     * Initialize the MinPhaseBank integration
     * @param sampleRate Current sample rate
     * @return true if initialization successful
     */
    bool initialize(double sampleRate);
    
    /**
     * Get the best available filter for the given oversampling factor
     * @param osFactor Oversampling factor (2, 4, 8, 16)
     * @return Pointer to TapSet if found, nullptr otherwise
     */
    const MinPhaseBank::TapSet* getFilterForOSFactor(int osFactor) const;
    
    /**
     * Get filter taps as float array for JUCE oversampling
     * @param osFactor Oversampling factor
     * @return Vector of filter taps, empty if not found
     */
    std::vector<float> getFilterTaps(int osFactor) const;
    
    /**
     * Check if MinPhaseBank is available and loaded
     * @return true if MinPhaseBank is available
     */
    bool isAvailable() const;
    
    /**
     * Get available oversampling factors
     * @return Vector of available OS factors
     */
    std::vector<int> getAvailableOSFactors() const;
    
    /**
     * Get filter information for debugging
     * @param osFactor Oversampling factor
     * @return String with filter info
     */
    juce::String getFilterInfo(int osFactor) const;

private:
    bool initialized = false;
    double currentSampleRate = 0.0;
    
    // Cache for filter taps converted to float
    mutable std::map<int, std::vector<float>> tapCache;
    
    void loadMinPhaseBank();
    void clearCache();
};
