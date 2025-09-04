#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>

struct PresetMeta
{
	juce::Uuid     id;
	juce::String   name, category, subcategory, author;
	juce::String   description, hint;
	juce::StringArray tags;
	bool           isFactory { false };
	bool           isFavorite { false };
	int            timesUsed { 0 };
	int64_t        createdAt { 0 }, updatedAt { 0 }, lastUsedAt { 0 };
	int            schemaVersion { 1 }, engineVersion { 1 };
};

struct LibraryPreset
{
	PresetMeta            meta;
	juce::NamedValueSet   params;
};

class PresetStore
{
public:
	PresetStore();

	void setUserDir (juce::File dir);
	void setMetaDB  (juce::File metaDbFile);

	void addFactoryPreset (const LibraryPreset& p);
	void clearFactory();
	void scan();

	bool saveUserPreset (const LibraryPreset& p);
	bool overwriteUserPreset (const LibraryPreset& p);

	bool setFavorite (const juce::Uuid& id, bool fav);
	void bumpUsage (const juce::Uuid& id);

	juce::Array<LibraryPreset> getAll() const;
	juce::Optional<LibraryPreset> getById (const juce::Uuid& id) const;

	static juce::var toJson (const LibraryPreset& p);
	static juce::Optional<LibraryPreset> fromJson (const juce::var& v);

private:
	void loadMetaDB();
	void saveMetaDB() const;

	juce::File userDir, metaDbPath;
	juce::HashMap<juce::Uuid, LibraryPreset> byId;
	juce::Array<juce::Uuid> order;
	juce::DynamicObject::Ptr metaDB; // favorites/usage

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetStore)
};


