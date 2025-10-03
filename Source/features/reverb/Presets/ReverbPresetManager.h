#pragma once
#include <JuceHeader.h>
#include "../Core/ReverbEngine.h"
#include "ModelMacros.h"

// ===================== ReverbPresetManager ===================================
/**
 * Manages reverb presets and model application
 * Handles JSON preset loading, model application, and parameter mapping
 */
class ReverbPresetManager
{
public:
    struct PresetInfo
    {
        juce::String name;
        juce::String model;
        juce::StringArray tags;
        bool hasToneEQ = false;
        bool hasDecayRateEQ = false;
        bool hasDucking = false;
    };
    
    ReverbPresetManager();
    ~ReverbPresetManager() = default;
    
    // Preset loading and management
    bool loadPresetPack(const juce::File& jsonFile);
    bool loadPresetPack(const juce::String& jsonContent);
    void clearPresets();
    
    // Preset access
    int getNumPresets() const { return presets.size(); }
    PresetInfo getPresetInfo(int index) const;
    juce::String getPresetName(int index) const;
    juce::StringArray getPresetTags(int index) const;
    
    // Preset application
    bool applyPreset(int index, ReverbParams& params);
    bool applyPreset(const juce::String& name, ReverbParams& params);
    
    // Model application
    void applyModel(ReverbParams& params, const juce::String& modelName);
    
    // Search and filtering
    juce::Array<int> findPresetsByTag(const juce::String& tag) const;
    juce::Array<int> findPresetsByModel(const juce::String& model) const;
    juce::Array<int> searchPresets(const juce::String& query) const;
    
    // Preset pack info
    juce::String getPackAuthor() const { return packAuthor; }
    juce::String getPackVersion() const { return packVersion; }
    int getPackCount() const { return packCount; }

private:
    struct PresetData
    {
        juce::String name;
        juce::String model;
        juce::StringArray tags;
        juce::var params;
        juce::var toneEQ;
        juce::var decayRateEQ;
        juce::var ducking;
    };
    
    juce::Array<PresetData> presets;
    juce::String packAuthor;
    juce::String packVersion;
    int packCount = 0;
    
    // Helper methods
    ReverbParams parseParams(const juce::var& paramsData) const;
    void applyToneEQ(const juce::var& toneEQData, ReverbParams& params) const;
    void applyDecayRateEQ(const juce::var& decayRateEQData, ReverbParams& params) const;
    void applyDucking(const juce::var& duckingData, ReverbParams& params) const;
    
    // Model defaults
    struct ModelDefaults
    {
        float sizePct, decaySec, erTimeMs, erDensityPct, diffusionPct;
        float modDepthCents, modRateHz, dreqLoMult, dreqHiMult;
    };
    
    ModelDefaults getModelDefaults(const juce::String& modelName) const;
};
