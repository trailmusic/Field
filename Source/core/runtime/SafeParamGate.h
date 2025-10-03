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
		// Preferred: expose APVTS via processor properties in ctor
		if (auto* var = proc.getProperties().getVarPointer("apvts"))
			if (auto* ptr = var->getNativeObject())
				if (auto* apvts = reinterpret_cast<juce::AudioProcessorValueTreeState*>(ptr))
					return apvts;
		return nullptr;
	}
};

}}} // namespace field::core::runtime
