#pragma once

#include <array>

namespace MinPhaseBank
{
    // Simple placeholder structure for now
    struct TapSet
    {
        int order;
        const float* taps;
        int numTaps;
    };
    
    // Registry for lookup
    extern const TapSet* registry;
    extern const int registryCount;
}