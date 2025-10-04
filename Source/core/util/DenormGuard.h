#pragma once
#include <juce_core/juce_core.h>

// RAII FTZ/DAZ guard using JUCE's ScopedNoDenormals
struct DenormGuard
{
    juce::ScopedNoDenormals scoped;
};
