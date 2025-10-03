#include "../shared/Core/PluginProcessor.cpp"
#include "core/params/ParamLayout.h"

namespace field { namespace params {
    juce::AudioProcessorValueTreeState::ParameterLayout makeParameterLayout();
}}

FieldAudioProcessor::APVTS::ParameterLayout FieldAudioProcessor::createParameterLayout()
{
    return field::params::makeParameterLayout();
}
