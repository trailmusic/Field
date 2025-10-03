#pragma once
#include <JuceHeader.h>
#include "ReverbPresetIntegration.h"

// ===================== ReverbPresetIntegrationExample ========================
/**
 * Example of how to integrate reverb presets with the existing Field preset system
 * This shows the minimal code needed to add reverb presets to the existing UI
 */
class ReverbPresetIntegrationExample
{
public:
    // Example: How to initialize reverb presets in PluginEditor
    static void initializeReverbPresetsInEditor(PresetStore& presetStore, NewPresetManager& presetManager)
    {
        // 1. Initialize reverb preset integration
        static ReverbPresetIntegration reverbIntegration;
        reverbIntegration.initializeReverbPresets(presetStore);
        
        // 2. Set the reverb parameter mapping in the preset manager
        presetManager.setParamMap(reverbIntegration.getReverbParamMap());
        
        // That's it! The existing presetField button will now show reverb presets
        // alongside other presets in the PresetCommandPalette
    }
    
    // Example: How to load a specific reverb preset pack
    static bool loadSpecificReverbPack(const juce::File& jsonFile, PresetStore& presetStore)
    {
        static ReverbPresetIntegration reverbIntegration;
        return reverbIntegration.loadReverbPresetPack(jsonFile, presetStore);
    }
    
    // Example: How to check if reverb presets are loaded
    static bool areReverbPresetsLoaded()
    {
        static ReverbPresetIntegration reverbIntegration;
        return reverbIntegration.isInitialized();
    }
};
