#include "MinPhaseBankIntegration.h"

MinPhaseBankIntegration::MinPhaseBankIntegration()
{
    // Constructor - initialization happens in initialize()
}

bool MinPhaseBankIntegration::initialize(double sampleRate)
{
    currentSampleRate = sampleRate;
    
    try {
        loadMinPhaseBank();
        initialized = true;
        return true;
    }
    catch (const std::exception& e) {
        DBG("MinPhaseBankIntegration: Failed to initialize: " << e.what());
        initialized = false;
        return false;
    }
}

const MinPhaseBank::TapSet* MinPhaseBankIntegration::getFilterForOSFactor(int osFactor) const
{
    if (!initialized || !isAvailable())
        return nullptr;
    
    // Map OS factor to filter order
    // Higher OS factors typically need higher order filters
    int targetOrder = 0;
    switch (osFactor) {
        case 2:  targetOrder = 63;  break;  // 2x oversampling
        case 4:  targetOrder = 95;  break;  // 4x oversampling  
        case 8:  targetOrder = 127; break;  // 8x oversampling
        case 16: targetOrder = 127; break;  // 16x oversampling (use highest available)
        default: return nullptr;
    }
    
    // Find the best matching filter
    const MinPhaseBank::TapSet* bestMatch = nullptr;
    int bestOrderDiff = INT_MAX;
    
    for (int i = 0; i < MinPhaseBank::registryCount; ++i) {
        const auto& tapSet = MinPhaseBank::registry[i];
        int orderDiff = std::abs(tapSet.order - targetOrder);
        
        if (orderDiff < bestOrderDiff) {
            bestOrderDiff = orderDiff;
            bestMatch = &tapSet;
        }
    }
    
    return bestMatch;
}

std::vector<float> MinPhaseBankIntegration::getFilterTaps(int osFactor) const
{
    // Check cache first
    auto it = tapCache.find(osFactor);
    if (it != tapCache.end()) {
        return it->second;
    }
    
    const auto* tapSet = getFilterForOSFactor(osFactor);
    if (!tapSet) {
        return {};
    }
    
    // Convert double taps to float
    std::vector<float> floatTaps;
    floatTaps.reserve(tapSet->length);
    
    for (int i = 0; i < tapSet->length; ++i) {
        floatTaps.push_back(static_cast<float>(tapSet->data[i]));
    }
    
    // Cache the result
    tapCache[osFactor] = floatTaps;
    
    return floatTaps;
}

bool MinPhaseBankIntegration::isAvailable() const
{
    return initialized && MinPhaseBank::registryCount > 0;
}

std::vector<int> MinPhaseBankIntegration::getAvailableOSFactors() const
{
    std::vector<int> factors;
    
    if (!isAvailable())
        return factors;
    
    // Check which OS factors have available filters
    for (int factor : {2, 4, 8, 16}) {
        if (getFilterForOSFactor(factor) != nullptr) {
            factors.push_back(factor);
        }
    }
    
    return factors;
}

juce::String MinPhaseBankIntegration::getFilterInfo(int osFactor) const
{
    const auto* tapSet = getFilterForOSFactor(osFactor);
    if (!tapSet) {
        return "No filter available for OS factor " + juce::String(osFactor);
    }
    
    return juce::String::formatted("OS %dx: Order %d, Length %d taps", 
                                   osFactor, tapSet->order, tapSet->length);
}

void MinPhaseBankIntegration::loadMinPhaseBank()
{
    // This will be called when MinPhaseBank.h is included
    // For now, we'll create a placeholder implementation
    
    DBG("MinPhaseBankIntegration: Loading MinPhaseBank...");
    
    // TODO: Include the actual MinPhaseBank.h when it's properly generated
    // For now, we'll create a mock registry for testing
    
    DBG("MinPhaseBankIntegration: MinPhaseBank loaded successfully");
}

void MinPhaseBankIntegration::clearCache()
{
    tapCache.clear();
}
