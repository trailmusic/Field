#include "MinPhaseBank.h"

namespace MinPhaseBank
{
    // Placeholder data - in a real implementation, this would be generated
    // by the Field Ranger console tools
    static const float dummyTaps63[] = {0.0f, 0.0f, 0.0f}; // 3 dummy taps
    static const float dummyTaps95[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // 5 dummy taps
    static const float dummyTaps127[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // 7 dummy taps
    
    static const TapSet dummyRegistry[] = {
        {63, dummyTaps63, 3},
        {95, dummyTaps95, 5},
        {127, dummyTaps127, 7}
    };
    
    const TapSet* registry = dummyRegistry;
    const int registryCount = 3;
}
