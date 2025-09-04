#pragma once
#include <juce_core/juce_core.h>
#include "DelayEngine.h"

struct DelayPresetMeta
{
	juce::String      name;
	juce::String      desc;
	juce::String      hint;
	juce::StringArray tags;
	DelayParams       params;
};

namespace DelayPresets
{
	// Build-once, then cached
	const std::vector<DelayPresetMeta>& all();
	void resetCache();
	const DelayPresetMeta*              findByName (const juce::String& name);
	juce::StringArray                   names();

	template <typename Sample>
	bool applyTo (DelayEngine<Sample>& engine, const juce::String& name)
	{
		if (auto* p = findByName (name)) { engine.setParameters (p->params); return true; }
		return false;
	}
}


