#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

inline juce::AudioProcessor::BusesProperties makeStereoBuses()
{
	using AP = juce::AudioProcessor;
	return AP::BusesProperties()
		.withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
		.withOutput ("Output", juce::AudioChannelSet::stereo(), true);
}
