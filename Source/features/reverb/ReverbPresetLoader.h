#pragma once
#include <JuceHeader.h>
#include "../../shared/Presets/PresetStore.h"
#include "ReverbParamMap.h"

// ===================== ReverbPresetLoader ====================================
/**
 * Loads reverb presets from JSON files and integrates with existing PresetStore
 * Handles the conversion from JSON preset format to LibraryPreset format
 */
class ReverbPresetLoader
{
public:
    ReverbPresetLoader();
    ~ReverbPresetLoader() = default;
    
    // Load reverb preset packs
    bool loadPresetPack(const juce::File& jsonFile, PresetStore& presetStore);
    bool loadPresetPack(const juce::String& jsonContent, PresetStore& presetStore);
    
    // Load all reverb preset packs from Assets directory
    void loadAllReverbPresets(PresetStore& presetStore);
    
    // Get preset pack info
    juce::String getLastLoadedPackAuthor() const { return lastPackAuthor; }
    juce::String getLastLoadedPackVersion() const { return lastPackVersion; }
    int getLastLoadedPackCount() const { return lastPackCount; }

private:
    ReverbParamMap paramMap;
    juce::String lastPackAuthor;
    juce::String lastPackVersion;
    int lastPackCount = 0;
    
    // Helper methods
    LibraryPreset convertJsonPresetToLibraryPreset(const juce::var& jsonPreset, const juce::String& packAuthor) const;
    juce::StringArray extractTags(const juce::var& tagsVar) const;
    juce::String generatePresetId(const juce::String& name, const juce::String& packAuthor) const;
    
    // Preset pack discovery
    juce::Array<juce::File> findReverbPresetFiles() const;
};
