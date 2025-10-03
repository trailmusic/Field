#include "../shared/Core/PluginProcessor.cpp"
#include "core/params/ParamLayout.h"

juce::AudioProcessorValueTreeState::ParameterLayout MyPluginAudioProcessor::createParameterLayout()
{
    return field::params::makeParameterLayout();
}
