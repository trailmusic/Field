#pragma once

namespace ReverbParamIDs
{
    // Core routing
    static constexpr const char* enabled         = "reverb_enabled";
    static constexpr const char* killDry         = "reverb_kill_dry";

    // Structure / space
    static constexpr const char* preDelayMs      = "preDelayMs";
    static constexpr const char* decaySec        = "decaySec";
    static constexpr const char* sizePct         = "sizePct";

    // Early reflections
    static constexpr const char* erLevelDb       = "erLevelDb";
    static constexpr const char* erDensityPct    = "erDensityPct";
    static constexpr const char* erWidthPct      = "erWidthPct";
    static constexpr const char* erTimeMs        = "erTimeMs";
    static constexpr const char* erToTailPct     = "erToTailPct";

    // Diffusion / density
    static constexpr const char* diffusionPct    = "diffusionPct";
    static constexpr const char* densityPct      = "densityPct";

    // Modulation
    static constexpr const char* modDepthCents   = "modDepthCents";
    static constexpr const char* modRateHz       = "modRateHz";

    // Stereo + rotation (single static; motion follows external engine)
    static constexpr const char* widthPct        = "widthPct";
    static constexpr const char* rotationDeg     = "rotationDeg";

    // Motion follow (from global Motion Engine)
    static constexpr const char* followWidth     = "followWidth";
    static constexpr const char* followWidthAmt  = "followWidthAmt";
    static constexpr const char* followRot       = "followRot";
    static constexpr const char* followRotAmt    = "followRotAmt";

    // Mix & specials
    static constexpr const char* wetMix01        = "wetMix01";
    static constexpr const char* bloomPct        = "bloomPct";
    static constexpr const char* distancePct     = "distancePct";
    static constexpr const char* freeze          = "freeze";
    static constexpr const char* shimmerAmtPct   = "shimmerAmtPct";
    static constexpr const char* shimmerInt      = "shimmerInt";
    static constexpr const char* gateAmtPct      = "gateAmtPct";
    static constexpr const char* outTrimDb       = "outTrimDb";

    // Reverb EQ routing (your 4-band dyna-EQ pane)
    static constexpr const char* dreqXoverLoHz   = "dreqXoverLoHz";
    static constexpr const char* dreqXoverHiHz   = "dreqXoverHiHz";
    static constexpr const char* dreqApply       = "dreqApply"; // 0=Pre,1=Post,2=ER,3=Tail

    // Ducking (moved to floating module)
    static constexpr const char* duckOn          = "duckOn";
    static constexpr const char* duckMode        = "duckMode";   // 0=General,1=Vocal,2=DrumBus,3=Guitar,4=Keys
    static constexpr const char* duckDepthDb     = "duckDepthDb";
    static constexpr const char* duckAtkMs       = "duckAtkMs";
    static constexpr const char* duckRelMs        = "duckRelMs";
    static constexpr const char* duckThrDb        = "duckThrDb";
    static constexpr const char* duckRatio        = "duckRatio";
    static constexpr const char* duckKneeDb       = "duckKneeDb";
    static constexpr const char* duckBandHz       = "duckBandHz";
    static constexpr const char* duckBandQ        = "duckBandQ";
    static constexpr const char* duckDetectorSrc  = "duckDetectorSrc"; // 0=Dry,1=ER,2=Tail,3=Wet

    // Removed (legacy): widthStartPct, widthEndPct, rotStartDeg, rotEndDeg, duckLaMs, duckRmsMs
}


