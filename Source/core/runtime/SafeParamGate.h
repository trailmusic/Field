#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace field { namespace core { namespace runtime {

struct SafeParamGate
{
	static bool getBool (juce::AudioProcessor& proc, const juce::String& id, bool fallback) noexcept
	{
		if (auto* apvts = findAPVTS(proc))
			if (auto* p = apvts->getParameter (id))
				return p->getValue() >= 0.5f;
		return fallback;
	}

	static float getFloat (juce::AudioProcessor& proc, const juce::String& id, float fallback) noexcept
	{
		if (auto* apvts = findAPVTS(proc))
			if (auto* p = apvts->getParameter (id))
				return p->getValue();
		return fallback;
	}

    static int getInt (juce::AudioProcessor& proc, const juce::String& id, int fallback) noexcept
    {
        if (auto* apvts = findAPVTS(proc))
            if (auto* p = apvts->getParameter (id))
                return (int) std::lround (p->getValue());
        return fallback;
    }

private:
    static juce::AudioProcessorValueTreeState* findAPVTS (juce::AudioProcessor& proc) noexcept
	{
        // Preferred: processor exposes getAPVTS()
        struct APVTSGetter { virtual juce::AudioProcessorValueTreeState& getAPVTS() = 0; };
        if (auto* g = dynamic_cast<APVTSGetter*>(&proc)) return &g->getAPVTS();
		return nullptr;
	}
};

}}} // namespace field::core::runtime
