#pragma once
#include <JuceHeader.h>
#include "../../../shared/Presets/PresetStore.h"
#include "../../../shared/Presets/PresetManager.h"
#include "ReverbPresetLoader.h"
#include "ReverbParamMap.h"

// ===================== ReverbPresetIntegration ===============================
/**
 * Integration point for reverb presets with the existing Field preset system
 * This class handles loading reverb presets and integrating them with PresetStore
 */
class ReverbPresetIntegration
{
public:
    ReverbPresetIntegration();
    ~ReverbPresetIntegration() = default;
    
    // Initialize reverb presets in the existing preset system
    void initializeReverbPresets(PresetStore& presetStore);
    
    // Get the parameter mapping for reverb presets
    const ParamMap& getReverbParamMap() const { return reverbParamMap.getParamMap(); }
    
    // Load specific reverb preset pack
    bool loadReverbPresetPack(const juce::File& jsonFile, PresetStore& presetStore);
    
    // Get integration status
    bool isInitialized() const { return initialized; }
    int getLoadedPresetCount() const { return loadedPresetCount; }

private:
    ReverbPresetLoader presetLoader;
    ReverbParamMap reverbParamMap;
    bool initialized = false;
    int loadedPresetCount = 0;
    
    // Helper methods
    void loadAllReverbPresetPacks(PresetStore& presetStore);
};
