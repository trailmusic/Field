#include "ReverbPresetIntegration.h"

ReverbPresetIntegration::ReverbPresetIntegration()
{
    // Constructor - initialization happens in initializeReverbPresets()
}

void ReverbPresetIntegration::initializeReverbPresets(PresetStore& presetStore)
{
    if (initialized)
        return;
    
    // Load all reverb preset packs
    loadAllReverbPresetPacks(presetStore);
    
    initialized = true;
}

bool ReverbPresetIntegration::loadReverbPresetPack(const juce::File& jsonFile, PresetStore& presetStore)
{
    return presetLoader.loadPresetPack(jsonFile, presetStore);
}

void ReverbPresetIntegration::loadAllReverbPresetPacks(PresetStore& presetStore)
{
    // Load all reverb preset packs from Assets/Presets/Reverb/
    presetLoader.loadAllReverbPresets(presetStore);
    
    // Update loaded count
    loadedPresetCount = presetLoader.getLastLoadedPackCount();
}
