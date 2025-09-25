#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PresetStore.h"

struct ParamMap
{
	juce::HashMap<juce::String, juce::String> map;
	ParamMap();
};

class NewPresetManager
{
public:
	NewPresetManager (juce::AudioProcessorValueTreeState& s, juce::UndoManager* um=nullptr);

	void setParamMap (const ParamMap& pm)
	{
		paramMap.map.clear();
		for (auto it = pm.map.begin(); it != pm.map.end(); ++it)
			paramMap.map.set (it.getKey(), it.getValue());
	}
	void applyPresetAtomic (const LibraryPreset& p);
	void auditionStart  (const LibraryPreset& p);
	void auditionCancel ();
	void auditionCommitToSlot (bool toA);
	void loadToSlot (const LibraryPreset& p, bool toA);

	const juce::ValueTree& slotA() const { return slotAState; }
	const juce::ValueTree& slotB() const { return slotBState; }

	LibraryPreset currentAsPreset (juce::String name,
						    juce::String category,
						    juce::StringArray tags,
						    juce::String description = {},
						    juce::String hint = {},
						    juce::String author = "User");

private:
	juce::AudioProcessorValueTreeState& apvts;
	juce::UndoManager* undo = nullptr;
	ParamMap paramMap;
	juce::ValueTree snapshotBeforeAudition, slotAState, slotBState;
	void setParam (juce::RangedAudioParameter* p, const juce::var& v);
};


