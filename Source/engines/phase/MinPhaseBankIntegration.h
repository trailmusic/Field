#pragma once
#include <JuceHeader.h>
#include <array>
#include <memory>

namespace MinPhaseBank {
    struct TapSet {
        const double* data;
        int length;
        int order;
    };
    extern const TapSet registry[];
    extern const int registryCount;
}

class MinPhaseBankIntegration
{
public:
    MinPhaseBankIntegration();
    ~MinPhaseBankIntegration() = default;
    bool initialize(double sampleRate);
    const MinPhaseBank::TapSet* getFilterForOSFactor(int osFactor) const;
    std::vector<float> getFilterTaps(int osFactor) const;
    bool isAvailable() const;
    std::vector<int> getAvailableOSFactors() const;
    juce::String getFilterInfo(int osFactor) const;
private:
    bool initialized = false;
    double currentSampleRate = 0.0;
    mutable std::map<int, std::vector<float>> tapCache;
    void loadMinPhaseBank();
    void clearCache();
};
