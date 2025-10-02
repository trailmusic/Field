#pragma once
/*
====================================================================================================
 ReverbTypes.h — Shared type definitions for reverb system
----------------------------------------------------------------------------------------------------
 Purpose
    - Central place for shared types used across reverb components
    - Prevents circular dependencies between headers
    - Contains DecayRateProfile, DecayRateBand, ToneEq, etc.

 Usage
    - Include this header in any file that needs reverb type definitions
    - Keep this file lightweight and focused on types only
====================================================================================================
*/

#include <JuceHeader.h>
#include <vector>

// ===================== Decay-Rate EQ profile (from UI) ======================
struct DecayRateBand
{
    enum Type { Bell = 0, TiltLo = 1, TiltHi = 2 } type = Bell;
    float freqHz = 1000.f;
    float mult   = 1.0f;   // 0.5× .. 2.0× (maps to per-band loss in tank)
    float q      = 0.707f; // shape
};

struct DecayRateProfile
{
    std::vector<DecayRateBand> bands; // <= 3 typical
};

// ===================== Tone EQ (static) =====================================
struct ToneEqBand
{
    enum Kind { Peak, LowShelf, HighShelf } kind = Peak;
    float freqHz = 1000.f;
    float q      = 0.707f;   // shelf slope via Q
    float gainDb = 0.0f;
};

struct ToneEq
{
    std::vector<ToneEqBand> bands; // small (<=3)
    enum Apply { Pre, Post, EROnly, TailOnly } apply = Post;
};
