#pragma once
/*
====================================================================================================
 SimdBiquad.h — SIMD-friendly layout & scalar fallback
----------------------------------------------------------------------------------------------------
 Purpose
    - Provide a drop-in biquad process path you can switch to later.
    - Keeps API stable while we stay scalar in Phase 1/2.

 Usage
    - If FIELD_ENABLE_SIMD=1 and you include the right headers, add an AVX2/NEON specialization.

 Notes
    - Structure-of-Arrays (SoA) to vectorize multiple channels or multiple filters in parallel.
====================================================================================================
*/
#include <JuceHeader.h>
#include "FieldReverbConfig.h"

namespace simdverb
{
struct BiquadSoA
{
    // coeffs per instance
    float b0{}, b1{}, b2{}, a1{}, a2{};
    // states per channel
    std::vector<float> z1, z2;

    void resize (int channels) { z1.assign (channels, 0.0f); z2.assign (channels, 0.0f); }

    void setCoeffs (float B0, float B1, float B2, float A1, float A2)
    { b0=B0; b1=B1; b2=B2; a1=A1; a2=A2; }

    void processBlock (juce::AudioBuffer<float>& buf)
    {
        const int C = buf.getNumChannels();
        const int N = buf.getNumSamples();
        if ((int) z1.size() != C) resize (C);

    #if FIELD_ENABLE_SIMD
        // TODO: add AVX2/NEON path
    #endif
        for (int c=0;c<C;++c)
        {
            float z1c = z1[c], z2c = z2[c];
            float* d = buf.getWritePointer (c);
            for (int i=0;i<N;++i)
            {
                const float x = d[i];
                const float y = b0*x + z1c;
                z1c = b1*x - a1*y + z2c;
                z2c = b2*x - a2*y;
                d[i] = y;
            }
            z1[c]=z1c; z2[c]=z2c;
        }
    }
};
} // namespace simdverb
