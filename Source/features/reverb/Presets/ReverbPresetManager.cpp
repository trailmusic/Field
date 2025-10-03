#include "ReverbPresetManager.h"

ReverbPresetManager::ReverbPresetManager()
{
    // Initialize with default values
    packAuthor = "Field Audio";
    packVersion = "1.0";
    packCount = 0;
}

bool ReverbPresetManager::loadPresetPack(const juce::File& jsonFile)
{
    if (!jsonFile.existsAsFile())
        return false;
    
    return loadPresetPack(jsonFile.loadFileAsString());
}

bool ReverbPresetManager::loadPresetPack(const juce::String& jsonContent)
{
    clearPresets();
    
    auto json = juce::JSON::parse(jsonContent);
    if (json.isVoid())
        return false;
    
    // Parse pack metadata
    packAuthor = json.getProperty("author", "Unknown").toString();
    packVersion = json.getProperty("version", "1.0").toString();
    packCount = (int)json.getProperty("count", 0);
    
    // Parse presets
    auto presetsArray = json.getProperty("presets", juce::var());
    if (presetsArray.isArray())
    {
        for (auto& presetVar : *presetsArray.getArray())
        {
            if (presetVar.isObject())
            {
                PresetData preset;
                preset.name = presetVar.getProperty("name", "Untitled").toString();
                preset.model = presetVar.getProperty("model", "Room").toString();
                
                // Parse tags
                auto tagsVar = presetVar.getProperty("tags", juce::var());
                if (tagsVar.isArray())
                {
                    for (auto& tag : *tagsVar.getArray())
                        preset.tags.add(tag.toString());
                }
                
                // Store parameter data
                preset.params = presetVar.getProperty("params", juce::var());
                preset.toneEQ = presetVar.getProperty("toneEQ", juce::var());
                preset.decayRateEQ = presetVar.getProperty("decayRateEQ", juce::var());
                preset.ducking = presetVar.getProperty("ducking", juce::var());
                
                presets.add(preset);
            }
        }
    }
    
    return presets.size() > 0;
}

void ReverbPresetManager::clearPresets()
{
    presets.clear();
    packAuthor = "Field Audio";
    packVersion = "1.0";
    packCount = 0;
}

ReverbPresetManager::PresetInfo ReverbPresetManager::getPresetInfo(int index) const
{
    PresetInfo info;
    
    if (juce::isPositiveAndBelow(index, presets.size()))
    {
        const auto& preset = presets[index];
        info.name = preset.name;
        info.model = preset.model;
        info.tags = preset.tags;
        info.hasToneEQ = !preset.toneEQ.isVoid();
        info.hasDecayRateEQ = !preset.decayRateEQ.isVoid();
        info.hasDucking = !preset.ducking.isVoid();
    }
    
    return info;
}

juce::String ReverbPresetManager::getPresetName(int index) const
{
    if (juce::isPositiveAndBelow(index, presets.size()))
        return presets[index].name;
    return {};
}

juce::StringArray ReverbPresetManager::getPresetTags(int index) const
{
    if (juce::isPositiveAndBelow(index, presets.size()))
        return presets[index].tags;
    return {};
}

bool ReverbPresetManager::applyPreset(int index, ReverbParams& params)
{
    if (!juce::isPositiveAndBelow(index, presets.size()))
        return false;
    
    const auto& preset = presets[index];
    
    // Apply model defaults first
    applyModel(params, preset.model);
    
    // Parse and apply preset parameters
    params = parseParams(preset.params);
    
    // Apply additional EQ and ducking settings
    if (!preset.toneEQ.isVoid())
        applyToneEQ(preset.toneEQ, params);
    
    if (!preset.decayRateEQ.isVoid())
        applyDecayRateEQ(preset.decayRateEQ, params);
    
    if (!preset.ducking.isVoid())
        applyDucking(preset.ducking, params);
    
    return true;
}

bool ReverbPresetManager::applyPreset(const juce::String& name, ReverbParams& params)
{
    for (int i = 0; i < presets.size(); ++i)
    {
        if (presets[i].name == name)
            return applyPreset(i, params);
    }
    return false;
}

void ReverbPresetManager::applyModel(ReverbParams& params, const juce::String& modelName)
{
    // Use the ModelMacros.h functionality
    std::string modelStr = modelName.toStdString();
    std::transform(modelStr.begin(), modelStr.end(), modelStr.begin(), ::tolower);
    
    if (modelStr == "plate")
    {
        params.decaySec = 2.2f;
        params.erTimeMs = 45.0f;
        params.erDensity = 65.0f;
        params.diffusion = 80.0f;
        params.modDepthCents = 6.0f;
        params.modRateHz = 0.35f;
    }
    else if (modelStr == "hall")
    {
        params.decaySec = 4.5f;
        params.erTimeMs = 65.0f;
        params.erDensity = 55.0f;
        params.diffusion = 90.0f;
        params.modDepthCents = 8.0f;
        params.modRateHz = 0.25f;
    }
    else if (modelStr == "chamber")
    {
        params.decaySec = 1.8f;
        params.erTimeMs = 55.0f;
        params.erDensity = 70.0f;
        params.diffusion = 78.0f;
        params.modDepthCents = 5.0f;
        params.modRateHz = 0.30f;
    }
    else // room
    {
        params.decaySec = 0.9f;
        params.erTimeMs = 35.0f;
        params.erDensity = 75.0f;
        params.diffusion = 65.0f;
        params.modDepthCents = 3.0f;
        params.modRateHz = 0.40f;
    }
}

juce::Array<int> ReverbPresetManager::findPresetsByTag(const juce::String& tag) const
{
    juce::Array<int> results;
    
    for (int i = 0; i < presets.size(); ++i)
    {
        if (presets[i].tags.contains(tag))
            results.add(i);
    }
    
    return results;
}

juce::Array<int> ReverbPresetManager::findPresetsByModel(const juce::String& model) const
{
    juce::Array<int> results;
    
    for (int i = 0; i < presets.size(); ++i)
    {
        if (presets[i].model.equalsIgnoreCase(model))
            results.add(i);
    }
    
    return results;
}

juce::Array<int> ReverbPresetManager::searchPresets(const juce::String& query) const
{
    juce::Array<int> results;
    const juce::String lowerQuery = query.toLowerCase();
    
    for (int i = 0; i < presets.size(); ++i)
    {
        const auto& preset = presets[i];
        
        // Search in name
        if (preset.name.toLowerCase().contains(lowerQuery))
        {
            results.add(i);
            continue;
        }
        
        // Search in tags
        for (const auto& tag : preset.tags)
        {
            if (tag.toLowerCase().contains(lowerQuery))
            {
                results.add(i);
                break;
            }
        }
    }
    
    return results;
}

ReverbParams ReverbPresetManager::parseParams(const juce::var& paramsData) const
{
    ReverbParams params;
    
    if (paramsData.isObject())
    {
        // Core parameters
        params.decaySec = (float)paramsData.getProperty("decaySec", 2.4);
        params.preDelayMs = (float)paramsData.getProperty("preDelayMs", 0.0);
        params.erLevelDb = (float)paramsData.getProperty("erLevelDb", -10.0);
        params.erTimeMs = (float)paramsData.getProperty("erTimeMs", 50.0);
        params.erDensity = (float)paramsData.getProperty("erDensityPct", 65.0);
        params.erWidthPct = (float)paramsData.getProperty("erWidthPct", 80.0);
        params.erToTailPct = (float)paramsData.getProperty("erToTailPct", 60.0);
        params.diffusion = (float)paramsData.getProperty("diffusionPct", 80.0);
        params.density = (float)paramsData.getProperty("densityPct", 70.0);
        params.modDepthCents = (float)paramsData.getProperty("modDepthCents", 6.0);
        params.modRateHz = (float)paramsData.getProperty("modRateHz", 0.35);
        params.widthPct = (float)paramsData.getProperty("widthPct", 100.0);
        params.rotStartDeg = (float)paramsData.getProperty("rotationDeg", 0.0);
        // Note: wetMix01, bloomPct, distancePct, outTrimDb don't exist in ReverbParams
        // These would need to be mapped to existing fields or handled separately
        params.freeze = (bool)paramsData.getProperty("freeze", false);
        params.shimmerAmtPct = (float)paramsData.getProperty("shimmerAmtPct", 0.0);
        params.shimmerIntervalMode = (int)paramsData.getProperty("shimmerInt", 0);
        params.gateAmtPct = (float)paramsData.getProperty("gateAmtPct", 0.0);
        // outTrimDb would need to be handled separately as it's not in ReverbParams
        
        // Ducking parameters
        params.duckOn = (bool)paramsData.getProperty("duckOn", false);
        params.duckMode = (int)paramsData.getProperty("duckMode", 0);
        params.duckDepthDb = (float)paramsData.getProperty("duckDepthDb", 0.0);
        params.duckThrDb = (float)paramsData.getProperty("duckThrDb", -20.0);
        params.duckKneeDb = (float)paramsData.getProperty("duckKneeDb", 0.0);
        params.duckRatio = (float)paramsData.getProperty("duckRatio", 1.0);
        params.duckAtkMs = (float)paramsData.getProperty("duckAtkMs", 10.0);
        params.duckRelMs = (float)paramsData.getProperty("duckRelMs", 100.0);
        params.duckBandHz = (float)paramsData.getProperty("duckBandHz", 1000.0);
        params.duckBandQ = (float)paramsData.getProperty("duckBandQ", 1.0);
        params.duckDetector = (int)paramsData.getProperty("duckDetector", 0);
    }
    
    return params;
}

void ReverbPresetManager::applyToneEQ(const juce::var& toneEQData, ReverbParams& params) const
{
    // Tone EQ application would be implemented here
    // This would map to the Tone EQ system in the reverb engine
    juce::ignoreUnused(toneEQData, params);
}

void ReverbPresetManager::applyDecayRateEQ(const juce::var& decayRateEQData, ReverbParams& params) const
{
    // Decay Rate EQ application would be implemented here
    // This would map to the Decay Rate EQ system in the reverb engine
    juce::ignoreUnused(decayRateEQData, params);
}

void ReverbPresetManager::applyDucking(const juce::var& duckingData, ReverbParams& params) const
{
    // Ducking application would be implemented here
    // This would map to the ducking system in the reverb engine
    juce::ignoreUnused(duckingData, params);
}

ReverbPresetManager::ModelDefaults ReverbPresetManager::getModelDefaults(const juce::String& modelName) const
{
    ModelDefaults defaults;
    
    if (modelName.equalsIgnoreCase("plate"))
    {
        defaults = {55.0f, 2.2f, 45.0f, 65.0f, 80.0f, 6.0f, 0.35f, 1.00f, 0.85f};
    }
    else if (modelName.equalsIgnoreCase("hall"))
    {
        defaults = {85.0f, 4.5f, 65.0f, 55.0f, 90.0f, 8.0f, 0.25f, 1.05f, 0.80f};
    }
    else if (modelName.equalsIgnoreCase("chamber"))
    {
        defaults = {50.0f, 1.8f, 55.0f, 70.0f, 78.0f, 5.0f, 0.30f, 0.95f, 0.90f};
    }
    else // room
    {
        defaults = {35.0f, 0.9f, 35.0f, 75.0f, 65.0f, 3.0f, 0.40f, 0.90f, 0.95f};
    }
    
    return defaults;
}
