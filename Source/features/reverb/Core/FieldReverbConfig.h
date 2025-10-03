#pragma once
/*
====================================================================================================
 FieldReverbConfig.h — Global feature switches and shared compile flags
----------------------------------------------------------------------------------------------------
 What & Why
    - Central place to flip Phase 2 (FDN) on/off without touching engine source.
    - Single include to keep compile-time options consistent across targets.

 Usage
    - Define FIELD_REVERB_PHASE2=1 in Projucer/CMake to enable the FDN tank path.
    - Define FIELD_ENABLE_SIMD=1 to include SIMD stubs (safe fallbacks provided).

 Safety
    - All switches default to OFF (Phase 1 behavior).
====================================================================================================
*/

#ifndef FIELD_REVERB_PHASE2
  #define FIELD_REVERB_PHASE2 0
#endif

#ifndef FIELD_ENABLE_SIMD
  #define FIELD_ENABLE_SIMD 0
#endif

// Tunables (kept here so builders can tweak without code spelunking)
#ifndef FIELD_REVERB_DEFAULT_IR_SECONDS
  #define FIELD_REVERB_DEFAULT_IR_SECONDS 10
#endif
