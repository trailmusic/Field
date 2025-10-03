#include "ReverbPresetLoader.h"

ReverbPresetLoader::ReverbPresetLoader()
{
    lastPackAuthor = "Field Audio";
    lastPackVersion = "1.0";
    lastPackCount = 0;
}

bool ReverbPresetLoader::loadPresetPack(const juce::File& jsonFile, PresetStore& presetStore)
{
    if (!jsonFile.existsAsFile())
        return false;
    
    return loadPresetPack(jsonFile.loadFileAsString(), presetStore);
}

bool ReverbPresetLoader::loadPresetPack(const juce::String& jsonContent, PresetStore& presetStore)
{
    auto json = juce::JSON::parse(jsonContent);
    if (json.isVoid())
        return false;
    
    // Parse pack metadata
    lastPackAuthor = json.getProperty("author", "Field Audio").toString();
    lastPackVersion = json.getProperty("version", "1.0").toString();
    lastPackCount = (int)json.getProperty("count", 0);
    
    // Parse presets
    auto presetsArray = json.getProperty("presets", juce::var());
    if (presetsArray.isArray())
    {
        for (const auto& presetVar : *presetsArray.getArray())
        {
            if (presetVar.isObject())
            {
                LibraryPreset libraryPreset = convertJsonPresetToLibraryPreset(presetVar, lastPackAuthor);
                presetStore.addFactoryPreset(libraryPreset);
            }
        }
    }
    
    return lastPackCount > 0;
}

void ReverbPresetLoader::loadAllReverbPresets(PresetStore& presetStore)
{
    auto presetFiles = findReverbPresetFiles();
    
    for (const auto& file : presetFiles)
    {
        loadPresetPack(file, presetStore);
    }
}

LibraryPreset ReverbPresetLoader::convertJsonPresetToLibraryPreset(const juce::var& jsonPreset, const juce::String& packAuthor) const
{
    LibraryPreset libraryPreset;
    
    // Set metadata
    libraryPreset.meta.name = jsonPreset.getProperty("name", "Untitled").toString();
    libraryPreset.meta.author = packAuthor;
    libraryPreset.meta.category = "Reverb";
    libraryPreset.meta.subcategory = jsonPreset.getProperty("model", "Room").toString();
    libraryPreset.meta.description = jsonPreset.getProperty("description", "").toString();
    libraryPreset.meta.hint = jsonPreset.getProperty("hint", "").toString();
    libraryPreset.meta.isFactory = true;
    libraryPreset.meta.isFavorite = false;
    libraryPreset.meta.timesUsed = 0;
    libraryPreset.meta.createdAt = juce::Time::getCurrentTime().toMilliseconds();
    libraryPreset.meta.updatedAt = libraryPreset.meta.createdAt;
    libraryPreset.meta.lastUsedAt = 0;
    libraryPreset.meta.schemaVersion = 1;
    libraryPreset.meta.engineVersion = 1;
    
    // Generate unique ID
    libraryPreset.meta.id = juce::Uuid(generatePresetId(libraryPreset.meta.name, packAuthor));
    
    // Extract tags
    libraryPreset.meta.tags = extractTags(jsonPreset.getProperty("tags", juce::var()));
    
    // Convert parameters
    auto paramsVar = jsonPreset.getProperty("params", juce::var());
    if (paramsVar.isObject())
    {
        libraryPreset.params = paramMap.jsonToAPVTS(paramsVar);
    }
    
    return libraryPreset;
}

juce::StringArray ReverbPresetLoader::extractTags(const juce::var& tagsVar) const
{
    juce::StringArray tags;
    
    if (tagsVar.isArray())
    {
        for (const auto& tag : *tagsVar.getArray())
        {
            tags.add(tag.toString());
        }
    }
    
    return tags;
}

juce::String ReverbPresetLoader::generatePresetId(const juce::String& name, const juce::String& packAuthor) const
{
    // Generate a deterministic ID based on name and pack author
    const juce::String combined = name + "_" + packAuthor;
    return juce::String(combined.hashCode64());
}

juce::Array<juce::File> ReverbPresetLoader::findReverbPresetFiles() const
{
    juce::Array<juce::File> presetFiles;
    
    // Look in Assets/Presets/Reverb/ directory
    const auto reverbPresetsDir = juce::File::getCurrentWorkingDirectory()
                                  .getChildFile("Assets")
                                  .getChildFile("Presets")
                                  .getChildFile("Reverb");
    
    if (reverbPresetsDir.exists() && reverbPresetsDir.isDirectory())
    {
        auto files = reverbPresetsDir.findChildFiles(juce::File::findFiles, false, "*.json");
        presetFiles.addArray(files);
    }
    
    return presetFiles;
}
