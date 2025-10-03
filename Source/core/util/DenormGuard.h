#pragma once
#include <juce_core/juce_core.h>

struct DenormGuard
{
	DenormGuard()  { juce::ScopedNoDenormals::setDisabled (true); }
	~DenormGuard() { juce::ScopedNoDenormals::setDisabled (false); }
};
