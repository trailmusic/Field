#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

struct PresetEntry
{
    juce::Uuid           id;
    juce::String         name, category, author, desc, hint;
    juce::StringArray    tags;
    juce::NamedValueSet  params;
    bool                 isFactory { true };
    bool                 isFavorite { false };
};

class PresetRegistry
{
public:
    PresetRegistry();
    const juce::Array<PresetEntry>& all() const { return list; }

    void setFavorite (const juce::Uuid& id, bool fav);
    bool getFavorite (const juce::Uuid& id) const;
    void reloadUserJson();
    void reloadAll();

private:
    juce::Array<PresetEntry> list;
    std::unique_ptr<juce::PropertiesFile> props;

    void loadCompiledFactory();
    void tryLoadUserJson();
    void loadFavorites();
    void saveFavorites();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetRegistry)
};


