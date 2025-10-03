#pragma once
#include <JuceHeader.h>
#include "../../../shared/Presets/PresetManager.h"

// ===================== ReverbParamMap =========================================
/**
 * Parameter mapping for reverb presets to APVTS
 * Maps JSON preset parameters to Field's APVTS parameter IDs
 */
class ReverbParamMap
{
public:
    ReverbParamMap();
    
    // Get the parameter mapping for reverb presets
    const ParamMap& getParamMap() const { return paramMap; }
    
    // Convert JSON preset to APVTS parameters
    juce::NamedValueSet jsonToAPVTS(const juce::var& jsonParams) const;
    
    // Convert APVTS parameters to JSON preset
    juce::var apvtsToJson(const juce::NamedValueSet& apvtsParams) const;

private:
    ParamMap paramMap;
    
    // Initialize the parameter mapping
    void initializeParamMap();
};
