#include "PresetSystem.h"
#include <juce_core/juce_core.h>

PresetManager::PresetManager()
{
    // Add default preset
    Preset defaultPreset;
    defaultPreset.name = "Default";
    defaultPreset.category = "Factory";
    defaultPreset.subcategory = "Basic";
    defaultPreset.description = "Clean, neutral starting point";
    defaultPreset.author = "Field";
    defaultPreset.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.0f},
        {"tilt", 0.0f},
        {"mono_hz", 0.0f},
        {"hp_hz", 20.0f},
        {"lp_hz", 20000.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"bass_db", 0.0f},
        {"scoop", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.0f},
        {"ducking", 0.0f},
        {"tilt_freq", 1000.0f},
        {"scoop_freq", 800.0f},
        {"bass_freq", 200.0f},
        {"air_freq", 8000.0f},
        {"space_algo", 0.0f},
        {"split_mode", 0.0f}
    };
    presets.push_back(defaultPreset);
    
    // VOCAL PRESETS
    Preset vocalCenterGlow;
    vocalCenterGlow.name = "Vocal — Center Glow";
    vocalCenterGlow.category = "Factory";
    vocalCenterGlow.subcategory = "Vocal";
    vocalCenterGlow.description = "Intimate pop vocal: gently forward, a touch of air, stable center.";
    vocalCenterGlow.author = "Field";
    vocalCenterGlow.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.20f},
        {"tilt", 1.5f},
        {"mono_hz", 80.0f},
        {"hp_hz", 80.0f},
        {"lp_hz", 18000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.15f},
        {"ducking", 0.0f}
    };
    presets.push_back(vocalCenterGlow);
    
    Preset vocalAiryPopWide;
    vocalAiryPopWide.name = "Vocal — Airy Pop Wide";
    vocalAiryPopWide.category = "Factory";
    vocalAiryPopWide.subcategory = "Vocal";
    vocalAiryPopWide.description = "Modern sheen with tasteful width and pre-delay clarity.";
    vocalAiryPopWide.author = "Field";
    vocalAiryPopWide.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.50f},
        {"tilt", 2.5f},
        {"mono_hz", 100.0f},
        {"hp_hz", 90.0f},
        {"lp_hz", 18000.0f},
        {"sat_drive_db", 3.0f},
        {"sat_mix", 0.15f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.20f},
        {"ducking", 0.0f}
    };
    presets.push_back(vocalAiryPopWide);
    
    // DRUM PRESETS
    Preset kickMonoAnchor;
    kickMonoAnchor.name = "Kick — Mono Anchor";
    kickMonoAnchor.category = "Factory";
    kickMonoAnchor.subcategory = "Drums";
    kickMonoAnchor.description = "Solid low-end focus; near-mono lows with minimal space.";
    kickMonoAnchor.author = "Field";
    kickMonoAnchor.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.60f},
        {"tilt", -0.5f},
        {"mono_hz", 200.0f},
        {"hp_hz", 30.0f},
        {"lp_hz", 14000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.05f},
        {"ducking", 0.0f}
    };
    presets.push_back(kickMonoAnchor);
    
    Preset snarePunchCenter;
    snarePunchCenter.name = "Snare — Punch Center";
    snarePunchCenter.category = "Factory";
    snarePunchCenter.subcategory = "Drums";
    snarePunchCenter.description = "Focused crack with subtle sheen; sits forward without harshness.";
    snarePunchCenter.author = "Field";
    snarePunchCenter.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.00f},
        {"tilt", 1.0f},
        {"mono_hz", 120.0f},
        {"hp_hz", 120.0f},
        {"lp_hz", 18000.0f},
        {"sat_drive_db", 4.0f},
        {"sat_mix", 0.20f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.10f},
        {"ducking", 0.0f}
    };
    presets.push_back(snarePunchCenter);
    
    Preset drumOHNaturalSpread;
    drumOHNaturalSpread.name = "Drum OH — Natural Spread";
    drumOHNaturalSpread.category = "Factory";
    drumOHNaturalSpread.subcategory = "Drums";
    drumOHNaturalSpread.description = "Open stereo picture with gentle front positioning.";
    drumOHNaturalSpread.author = "Field";
    drumOHNaturalSpread.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.60f},
        {"tilt", 1.5f},
        {"mono_hz", 80.0f},
        {"hp_hz", 120.0f},
        {"lp_hz", 18000.0f},
        {"sat_drive_db", 3.0f},
        {"sat_mix", 0.15f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.25f},
        {"ducking", 0.0f}
    };
    presets.push_back(drumOHNaturalSpread);
    
    // BASS PRESETS
    Preset bassTightCenter;
    bassTightCenter.name = "Bass — Tight Center";
    bassTightCenter.category = "Factory";
    bassTightCenter.subcategory = "Bass";
    bassTightCenter.description = "Centered, weighty bass with controlled highs and mild grit.";
    bassTightCenter.author = "Field";
    bassTightCenter.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.70f},
        {"tilt", -1.5f},
        {"mono_hz", 160.0f},
        {"hp_hz", 35.0f},
        {"lp_hz", 12000.0f},
        {"sat_drive_db", 5.0f},
        {"sat_mix", 0.25f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.10f},
        {"ducking", 0.0f}
    };
    presets.push_back(bassTightCenter);
    
    Preset bass808DeepCenter;
    bass808DeepCenter.name = "808 — Deep Center";
    bass808DeepCenter.category = "Factory";
    bass808DeepCenter.subcategory = "Bass";
    bass808DeepCenter.description = "Centered sub weight with slight grit; keep highs tamed.";
    bass808DeepCenter.author = "Field";
    bass808DeepCenter.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.50f},
        {"tilt", -2.0f},
        {"mono_hz", 220.0f},
        {"hp_hz", 30.0f},
        {"lp_hz", 12000.0f},
        {"sat_drive_db", 4.0f},
        {"sat_mix", 0.20f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.15f},
        {"ducking", 0.0f}
    };
    presets.push_back(bass808DeepCenter);
    
    // GUITAR PRESETS
    Preset guitarDoubleWide;
    guitarDoubleWide.name = "Guitar — Double Wide";
    guitarDoubleWide.category = "Factory";
    guitarDoubleWide.subcategory = "Guitar";
    guitarDoubleWide.description = "Big stereo guitars; polished but not harsh.";
    guitarDoubleWide.author = "Field";
    guitarDoubleWide.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.80f},
        {"tilt", 1.0f},
        {"mono_hz", 80.0f},
        {"hp_hz", 90.0f},
        {"lp_hz", 18000.0f},
        {"sat_drive_db", 3.0f},
        {"sat_mix", 0.20f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.20f},
        {"ducking", 0.0f}
    };
    presets.push_back(guitarDoubleWide);
    
    Preset guitarLeftFront;
    guitarLeftFront.name = "Guitar — Left Front";
    guitarLeftFront.category = "Factory";
    guitarLeftFront.subcategory = "Guitar";
    guitarLeftFront.description = "Classic stage-left placement with mild width and edge.";
    guitarLeftFront.author = "Field";
    guitarLeftFront.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.20f},
        {"tilt", 0.5f},
        {"mono_hz", 90.0f},
        {"hp_hz", 90.0f},
        {"lp_hz", 18000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.15f},
        {"air_db", 0.0f},
        {"pan", -0.45f},
        {"depth", 0.15f},
        {"ducking", 0.0f}
    };
    presets.push_back(guitarLeftFront);
    
    Preset guitarRightBackWash;
    guitarRightBackWash.name = "Guitar — Right Back Wash";
    guitarRightBackWash.category = "Factory";
    guitarRightBackWash.subcategory = "Guitar";
    guitarRightBackWash.description = "Supportive right-side bed with extra space and softer highs.";
    guitarRightBackWash.author = "Field";
    guitarRightBackWash.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.60f},
        {"tilt", -0.5f},
        {"mono_hz", 80.0f},
        {"hp_hz", 90.0f},
        {"lp_hz", 16000.0f},
        {"sat_drive_db", 1.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", 0.50f},
        {"depth", 0.45f},
        {"ducking", 0.0f}
    };
    presets.push_back(guitarRightBackWash);
    
    // PIANO PRESETS
    Preset pianoStageLeft;
    pianoStageLeft.name = "Piano — Stage Left";
    pianoStageLeft.category = "Factory";
    pianoStageLeft.subcategory = "Piano";
    pianoStageLeft.description = "Natural left bias, moderate width, present but not bright.";
    pianoStageLeft.author = "Field";
    pianoStageLeft.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.30f},
        {"tilt", 0.0f},
        {"mono_hz", 70.0f},
        {"hp_hz", 60.0f},
        {"lp_hz", 18000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", -0.30f},
        {"depth", 0.20f},
        {"ducking", 0.0f}
    };
    presets.push_back(pianoStageLeft);
    
    Preset pianoStudioNarrow;
    pianoStudioNarrow.name = "Piano — Studio Narrow";
    pianoStudioNarrow.category = "Factory";
    pianoStudioNarrow.subcategory = "Piano";
    pianoStudioNarrow.description = "Controlled imaging for dense mixes; still forward.";
    pianoStudioNarrow.author = "Field";
    pianoStudioNarrow.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.90f},
        {"tilt", 0.0f},
        {"mono_hz", 120.0f},
        {"hp_hz", 80.0f},
        {"lp_hz", 18000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.10f},
        {"ducking", 0.0f}
    };
    presets.push_back(pianoStudioNarrow);
    
    // SYNTH PRESETS
    Preset padFarWide;
    padFarWide.name = "Pad — Far & Wide";
    padFarWide.category = "Factory";
    padFarWide.subcategory = "Synth";
    padFarWide.description = "Cinematic bed: deep, wide, darker tilt to sit behind vocals.";
    padFarWide.author = "Field";
    padFarWide.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.90f},
        {"tilt", -1.5f},
        {"mono_hz", 60.0f},
        {"hp_hz", 60.0f},
        {"lp_hz", 16000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.15f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.60f},
        {"ducking", 0.0f}
    };
    presets.push_back(padFarWide);
    
    Preset leadSpotlightPop;
    leadSpotlightPop.name = "Lead — Spotlight Pop";
    leadSpotlightPop.category = "Factory";
    leadSpotlightPop.subcategory = "Synth";
    leadSpotlightPop.description = "Forward and glossy; bright tilt, tasteful saturation.";
    leadSpotlightPop.author = "Field";
    leadSpotlightPop.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.40f},
        {"tilt", 2.0f},
        {"mono_hz", 80.0f},
        {"hp_hz", 110.0f},
        {"lp_hz", 19000.0f},
        {"sat_drive_db", 6.0f},
        {"sat_mix", 0.30f},
        {"air_db", 0.0f},
        {"pan", 0.10f},
        {"depth", 0.10f},
        {"ducking", 0.0f}
    };
    presets.push_back(leadSpotlightPop);
    
    Preset padShimmerFrontWide;
    padShimmerFrontWide.name = "Pad — Shimmer Front Wide";
    padShimmerFrontWide.category = "Factory";
    padShimmerFrontWide.subcategory = "Synth";
    padShimmerFrontWide.description = "Brighter, closer pad to lift choruses without clutter.";
    padShimmerFrontWide.author = "Field";
    padShimmerFrontWide.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.60f},
        {"tilt", 1.5f},
        {"mono_hz", 70.0f},
        {"hp_hz", 90.0f},
        {"lp_hz", 19000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.15f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.30f},
        {"ducking", 0.0f}
    };
    presets.push_back(padShimmerFrontWide);
    
    // STRINGS PRESETS
    Preset stringsBackLeftHall;
    stringsBackLeftHall.name = "Strings — Back Left Hall";
    stringsBackLeftHall.category = "Factory";
    stringsBackLeftHall.subcategory = "Strings";
    stringsBackLeftHall.description = "Left-biased section sitting behind the band; soft top.";
    stringsBackLeftHall.author = "Field";
    stringsBackLeftHall.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.40f},
        {"tilt", -1.0f},
        {"mono_hz", 70.0f},
        {"hp_hz", 90.0f},
        {"lp_hz", 16000.0f},
        {"sat_drive_db", 1.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", -0.20f},
        {"depth", 0.50f},
        {"ducking", 0.0f}
    };
    presets.push_back(stringsBackLeftHall);
    
    Preset stringsChamberFront;
    stringsChamberFront.name = "Strings — Chamber Front";
    stringsChamberFront.category = "Factory";
    stringsChamberFront.subcategory = "Strings";
    stringsChamberFront.description = "Closer, more intimate section with controlled width.";
    stringsChamberFront.author = "Field";
    stringsChamberFront.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.20f},
        {"tilt", -0.5f},
        {"mono_hz", 80.0f},
        {"hp_hz", 100.0f},
        {"lp_hz", 16000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.25f},
        {"ducking", 0.0f}
    };
    presets.push_back(stringsChamberFront);
    
    // PERCUSSION PRESETS
    Preset percNarrowScatter;
    percNarrowScatter.name = "Perc — Narrow Scatter";
    percNarrowScatter.category = "Factory";
    percNarrowScatter.subcategory = "Percussion";
    percNarrowScatter.description = "Keeps transient percussion tidy; minor space and sheen.";
    percNarrowScatter.author = "Field";
    percNarrowScatter.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.80f},
        {"tilt", 0.5f},
        {"mono_hz", 80.0f},
        {"hp_hz", 120.0f},
        {"lp_hz", 17000.0f},
        {"sat_drive_db", 2.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.20f},
        {"ducking", 0.0f}
    };
    presets.push_back(percNarrowScatter);
    
    // FX PRESETS
    Preset fxWideBrightSweep;
    fxWideBrightSweep.name = "FX — Wide Bright Sweep";
    fxWideBrightSweep.category = "Factory";
    fxWideBrightSweep.subcategory = "FX";
    fxWideBrightSweep.description = "Edgy transitions: ultra-wide, bright tilt, oversampled saturation.";
    fxWideBrightSweep.author = "Field";
    fxWideBrightSweep.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.90f},
        {"tilt", 3.0f},
        {"mono_hz", 60.0f},
        {"hp_hz", 120.0f},
        {"lp_hz", 20000.0f},
        {"sat_drive_db", 7.0f},
        {"sat_mix", 0.35f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.35f},
        {"ducking", 0.0f}
    };
    presets.push_back(fxWideBrightSweep);
    
    Preset lofiDarkCorner;
    lofiDarkCorner.name = "Lo-Fi — Dark Corner";
    lofiDarkCorner.category = "Factory";
    lofiDarkCorner.subcategory = "FX";
    lofiDarkCorner.description = "Moody side placement with darker tone and heavier saturation.";
    lofiDarkCorner.author = "Field";
    lofiDarkCorner.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.80f},
        {"tilt", -3.0f},
        {"mono_hz", 90.0f},
        {"hp_hz", 120.0f},
        {"lp_hz", 8000.0f},
        {"sat_drive_db", 10.0f},
        {"sat_mix", 0.50f},
        {"air_db", 0.0f},
        {"pan", -0.55f},
        {"depth", 0.40f},
        {"ducking", 0.0f}
    };
    presets.push_back(lofiDarkCorner);
    
    Preset radioNarrowMid;
    radioNarrowMid.name = "Radio — Narrow Mid";
    radioNarrowMid.category = "Factory";
    radioNarrowMid.subcategory = "FX";
    radioNarrowMid.description = "Band-limited, center-focused preset for throwback FX.";
    radioNarrowMid.author = "Field";
    radioNarrowMid.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.60f},
        {"tilt", -1.0f},
        {"mono_hz", 120.0f},
        {"hp_hz", 200.0f},
        {"lp_hz", 5000.0f},
        {"sat_drive_db", 6.0f},
        {"sat_mix", 0.40f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.10f},
        {"ducking", 0.0f}
    };
    presets.push_back(radioNarrowMid);
    
    Preset ambienceBackWall;
    ambienceBackWall.name = "Ambience — Back Wall";
    ambienceBackWall.category = "Factory";
    ambienceBackWall.subcategory = "FX";
    ambienceBackWall.description = "Deeply recessed ambience bed; soft highs, wide space.";
    ambienceBackWall.author = "Field";
    ambienceBackWall.parameters = {
        {"gain_db", 0.0f},
        {"width", 1.70f},
        {"tilt", -2.0f},
        {"mono_hz", 60.0f},
        {"hp_hz", 60.0f},
        {"lp_hz", 14000.0f},
        {"sat_drive_db", 1.0f},
        {"sat_mix", 0.10f},
        {"air_db", 0.0f},
        {"pan", 0.0f},
        {"depth", 0.70f},
        {"ducking", 0.0f}
    };
    presets.push_back(ambienceBackWall);
    
    // SPACE CHARACTER PRESETS (for Space knob only)
    Preset spaceRoom;
    spaceRoom.name = "Space — Room";
    spaceRoom.category = "Factory";
    spaceRoom.subcategory = "Space";
    spaceRoom.description = "Natural room ambience with moderate depth and warmth.";
    spaceRoom.author = "Field";
    spaceRoom.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.5f},
        {"tilt", 0.5f},
        {"mono_hz", 0.0f},
        {"hp_hz", 0.0f},
        {"lp_hz", 1.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"pan", 0.5f},
        {"depth", 0.25f},
        {"ducking", 0.0f}
    };
    presets.push_back(spaceRoom);
    
    Preset spaceHall;
    spaceHall.name = "Space — Hall";
    spaceHall.category = "Factory";
    spaceHall.subcategory = "Space";
    spaceHall.description = "Large hall reverb with extended decay and natural diffusion.";
    spaceHall.author = "Field";
    spaceHall.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.5f},
        {"tilt", 0.5f},
        {"mono_hz", 0.0f},
        {"hp_hz", 0.0f},
        {"lp_hz", 1.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"pan", 0.5f},
        {"depth", 0.45f},
        {"ducking", 0.0f}
    };
    presets.push_back(spaceHall);
    
    Preset spacePlate;
    spacePlate.name = "Space — Plate";
    spacePlate.category = "Factory";
    spacePlate.subcategory = "Space";
    spacePlate.description = "Classic plate reverb with metallic character and bright sheen.";
    spacePlate.author = "Field";
    spacePlate.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.5f},
        {"tilt", 0.5f},
        {"mono_hz", 0.0f},
        {"hp_hz", 0.0f},
        {"lp_hz", 1.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"pan", 0.5f},
        {"depth", 0.35f},
        {"ducking", 0.0f}
    };
    presets.push_back(spacePlate);
    
    Preset spaceChamber;
    spaceChamber.name = "Space — Chamber";
    spaceChamber.category = "Factory";
    spaceChamber.subcategory = "Space";
    spaceChamber.description = "Intimate chamber reverb with controlled early reflections.";
    spaceChamber.author = "Field";
    spaceChamber.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.5f},
        {"tilt", 0.5f},
        {"mono_hz", 0.0f},
        {"hp_hz", 0.0f},
        {"lp_hz", 1.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"pan", 0.5f},
        {"depth", 0.20f},
        {"ducking", 0.0f}
    };
    presets.push_back(spaceChamber);
    
    Preset spaceSpring;
    spaceSpring.name = "Space — Spring";
    spaceSpring.category = "Factory";
    spaceSpring.subcategory = "Space";
    spaceSpring.description = "Vintage spring reverb with characteristic metallic twang.";
    spaceSpring.author = "Field";
    spaceSpring.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.5f},
        {"tilt", 0.5f},
        {"mono_hz", 0.0f},
        {"hp_hz", 0.0f},
        {"lp_hz", 1.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"pan", 0.5f},
        {"depth", 0.30f},
        {"ducking", 0.0f}
    };
    presets.push_back(spaceSpring);
    
    Preset spaceReverse;
    spaceReverse.name = "Space — Reverse";
    spaceReverse.category = "Factory";
    spaceReverse.subcategory = "Space";
    spaceReverse.description = "Reverse reverb effect with swelling ambience.";
    spaceReverse.author = "Field";
    spaceReverse.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.5f},
        {"tilt", 0.5f},
        {"mono_hz", 0.0f},
        {"hp_hz", 0.0f},
        {"lp_hz", 1.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"pan", 0.5f},
        {"depth", 0.55f},
        {"ducking", 0.0f}
    };
    presets.push_back(spaceReverse);
    
    Preset spaceGated;
    spaceGated.name = "Space — Gated";
    spaceGated.category = "Factory";
    spaceGated.subcategory = "Space";
    spaceGated.description = "Gated reverb with tight, punchy ambience.";
    spaceGated.author = "Field";
    spaceGated.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.5f},
        {"tilt", 0.5f},
        {"mono_hz", 0.0f},
        {"hp_hz", 0.0f},
        {"lp_hz", 1.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"pan", 0.5f},
        {"depth", 0.15f},
        {"ducking", 0.0f}
    };
    presets.push_back(spaceGated);
    
    Preset spaceShimmer;
    spaceShimmer.name = "Space — Shimmer";
    spaceShimmer.category = "Factory";
    spaceShimmer.subcategory = "Space";
    spaceShimmer.description = "Ethereal shimmer reverb with harmonic enhancement.";
    spaceShimmer.author = "Field";
    spaceShimmer.parameters = {
        {"gain_db", 0.0f},
        {"width", 0.5f},
        {"tilt", 0.5f},
        {"mono_hz", 0.0f},
        {"hp_hz", 0.0f},
        {"lp_hz", 1.0f},
        {"sat_drive_db", 0.0f},
        {"sat_mix", 1.0f},
        {"air_db", 0.0f},
        {"pan", 0.5f},
        {"depth", 0.50f},
        {"ducking", 0.0f}
    };
    presets.push_back(spaceShimmer);
}

void PresetManager::loadPresets()
{
    loadPresetsFromFile();
}

void PresetManager::savePreset(const juce::String& name, const juce::String& category, 
                              const juce::String& subcategory, const juce::String& description)
{
    if (parameterGetter)
    {
        Preset newPreset(name, category, subcategory, description, "User", true);
        newPreset.parameters = parameterGetter();
        
        // Remove existing preset with same name if it exists
        presets.erase(std::remove_if(presets.begin(), presets.end(),
                                    [&name](const Preset& p) { return p.name == name; }),
                     presets.end());
        
        presets.push_back(newPreset);
        savePresetsToFile();
    }
}

void PresetManager::deletePreset(const juce::String& name)
{
    presets.erase(std::remove_if(presets.begin(), presets.end(),
                                [&name](const Preset& p) { return p.name == name && p.isUserPreset; }),
                 presets.end());
    savePresetsToFile();
}

void PresetManager::applyPreset(const juce::String& name)
{
    if (parameterSetter)
    {
        auto preset = getPreset(name);
        for (const auto& param : preset.parameters)
        {
            parameterSetter(param.first, param.second);
        }
    }
}

juce::StringArray PresetManager::getCategories() const
{
    juce::StringArray categories;
    for (const auto& preset : presets)
    {
        if (!categories.contains(preset.category))
            categories.add(preset.category);
    }
    categories.sort(true);
    return categories;
}

juce::StringArray PresetManager::getSubcategories(const juce::String& category) const
{
    juce::StringArray subcategories;
    for (const auto& preset : presets)
    {
        if (preset.category == category && !subcategories.contains(preset.subcategory))
            subcategories.add(preset.subcategory);
    }
    subcategories.sort(true);
    return subcategories;
}

juce::StringArray PresetManager::getPresetNames(const juce::String& category, const juce::String& subcategory) const
{
    juce::StringArray names;
    for (const auto& preset : presets)
    {
        bool categoryMatch = category.isEmpty() || preset.category == category;
        bool subcategoryMatch = subcategory.isEmpty() || preset.subcategory == subcategory;
        
        if (categoryMatch && subcategoryMatch)
            names.add(preset.name);
    }
    names.sort(true);
    return names;
}

juce::StringArray PresetManager::searchPresets(const juce::String& searchTerm) const
{
    juce::StringArray results;
    juce::String searchLower = searchTerm.toLowerCase();
    
    for (const auto& preset : presets)
    {
        if (preset.name.toLowerCase().contains(searchLower) ||
            preset.description.toLowerCase().contains(searchLower) ||
            preset.author.toLowerCase().contains(searchLower) ||
            preset.category.toLowerCase().contains(searchLower) ||
            preset.subcategory.toLowerCase().contains(searchLower))
        {
            results.add(preset.name);
        }
    }
    return results;
}

Preset PresetManager::getPreset(const juce::String& name) const
{
    for (const auto& preset : presets)
    {
        if (preset.name == name)
            return preset;
    }
    return Preset();
}

void PresetManager::setParameterGetter(std::function<std::map<juce::String, float>()> getter)
{
    parameterGetter = getter;
}

void PresetManager::setParameterSetter(std::function<void(const juce::String&, float)> setter)
{
    parameterSetter = setter;
}

void PresetManager::createDefaultPresets()
{
    // Clear existing presets
    presets.clear();
    
    // Studio Presets
    presets.push_back(Preset("Default", "Studio", "General", "Clean, balanced starting point", "Field"));
    presets.push_back(Preset("Wide Stereo", "Studio", "Width", "Expansive stereo field", "Field"));
    presets.push_back(Preset("Narrow Focus", "Studio", "Width", "Tight, focused stereo image", "Field"));
    presets.push_back(Preset("Center Focus", "Studio", "Width", "Emphasizes center content", "Field"));
    
    // Mixing Presets
    presets.push_back(Preset("Drum Bus", "Mixing", "Drums", "Wide, punchy drum processing", "Field"));
    presets.push_back(Preset("Vocal Space", "Mixing", "Vocals", "Airy vocal enhancement", "Field"));
    presets.push_back(Preset("Bass Mono", "Mixing", "Bass", "Solid bass foundation", "Field"));
    presets.push_back(Preset("Guitar Width", "Mixing", "Guitars", "Expansive guitar processing", "Field"));
    
    // Mastering Presets
    presets.push_back(Preset("Master Wide", "Mastering", "General", "Wide mastering chain", "Field"));
    presets.push_back(Preset("Master Punch", "Mastering", "Impact", "Punchy mastering", "Field"));
    presets.push_back(Preset("Master Air", "Mastering", "Clarity", "Airy mastering", "Field"));
    
    // Creative Presets
    presets.push_back(Preset("Extreme Width", "Creative", "Experimental", "Maximum stereo expansion", "Field"));
    presets.push_back(Preset("Hard Pan", "Creative", "Panning", "Ableton-style hard panning", "Field"));
    presets.push_back(Preset("Space Echo", "Creative", "Effects", "Echo-like spatial effect", "Field"));
    
    // Set default parameter values for each preset
    for (auto& preset : presets)
    {
        if (preset.name == "Default")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 1.0f}, {"tilt", 0.0f}, {"mono_hz", 0.0f},
                                {"hp_hz", 60.0f}, {"lp_hz", 16000.0f}, {"sat_drive_db", 6.0f}, {"sat_mix", 0.5f},
                                {"air_db", 0.0f}, {"pan", 0.0f}, {"depth", 0.2f}, {"ducking", 0.0f}};
        }
        else if (preset.name == "Wide Stereo")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 2.5f}, {"tilt", 0.0f}, {"mono_hz", 80.0f},
                                {"hp_hz", 40.0f}, {"lp_hz", 18000.0f}, {"sat_drive_db", 3.0f}, {"sat_mix", 0.3f},
                                {"air_db", 2.0f}, {"pan", 0.0f}, {"depth", 0.4f}, {"ducking", 0.3f}};
        }
        else if (preset.name == "Narrow Focus")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 0.3f}, {"tilt", 0.0f}, {"mono_hz", 0.0f},
                                {"hp_hz", 100.0f}, {"lp_hz", 12000.0f}, {"sat_drive_db", 8.0f}, {"sat_mix", 0.7f},
                                {"air_db", 0.0f}, {"pan", 0.0f}, {"depth", 0.1f}, {"ducking", 0.5f}};
        }
        else if (preset.name == "Center Focus")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 0.1f}, {"tilt", 0.0f}, {"mono_hz", 0.0f},
                                {"hp_hz", 80.0f}, {"lp_hz", 15000.0f}, {"sat_drive_db", 12.0f}, {"sat_mix", 0.8f},
                                {"air_db", 0.0f}, {"pan", 0.0f}, {"depth", 0.05f}, {"ducking", 0.8f}};
        }
        else if (preset.name == "Drum Bus")
        {
            preset.parameters = {{"gain_db", 2.0f}, {"width", 1.8f}, {"tilt", 1.0f}, {"mono_hz", 120.0f},
                                {"hp_hz", 30.0f}, {"lp_hz", 20000.0f}, {"sat_drive_db", 4.0f}, {"sat_mix", 0.4f},
                                {"air_db", 1.5f}, {"pan", 0.0f}, {"depth", 0.3f}, {"ducking", 0.2f}};
        }
        else if (preset.name == "Vocal Space")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 1.2f}, {"tilt", -1.0f}, {"mono_hz", 0.0f},
                                {"hp_hz", 80.0f}, {"lp_hz", 18000.0f}, {"sat_drive_db", 2.0f}, {"sat_mix", 0.2f},
                                {"air_db", 3.0f}, {"pan", 0.0f}, {"depth", 0.2f}, {"ducking", 0.1f}};
        }
        else if (preset.name == "Bass Mono")
        {
            preset.parameters = {{"gain_db", 1.0f}, {"width", 0.5f}, {"tilt", 2.0f}, {"mono_hz", 200.0f},
                                {"hp_hz", 20.0f}, {"lp_hz", 8000.0f}, {"sat_drive_db", 6.0f}, {"sat_mix", 0.6f},
                                {"air_db", 0.0f}, {"pan", 0.0f}, {"depth", 0.1f}, {"ducking", 0.4f}};
        }
        else if (preset.name == "Guitar Width")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 2.0f}, {"tilt", 0.5f}, {"mono_hz", 100.0f},
                                {"hp_hz", 60.0f}, {"lp_hz", 16000.0f}, {"sat_drive_db", 3.0f}, {"sat_mix", 0.3f},
                                {"air_db", 1.0f}, {"pan", 0.0f}, {"depth", 0.3f}, {"ducking", 0.2f}};
        }
        else if (preset.name == "Master Wide")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 1.5f}, {"tilt", 0.0f}, {"mono_hz", 60.0f},
                                {"hp_hz", 20.0f}, {"lp_hz", 20000.0f}, {"sat_drive_db", 2.0f}, {"sat_mix", 0.2f},
                                {"air_db", 1.5f}, {"pan", 0.0f}, {"depth", 0.2f}, {"ducking", 0.1f}};
        }
        else if (preset.name == "Master Punch")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 1.2f}, {"tilt", 1.0f}, {"mono_hz", 80.0f},
                                {"hp_hz", 30.0f}, {"lp_hz", 18000.0f}, {"sat_drive_db", 4.0f}, {"sat_mix", 0.4f},
                                {"air_db", 0.5f}, {"pan", 0.0f}, {"depth", 0.15f}, {"ducking", 0.3f}};
        }
        else if (preset.name == "Master Air")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 1.3f}, {"tilt", -0.5f}, {"mono_hz", 40.0f},
                                {"hp_hz", 20.0f}, {"lp_hz", 20000.0f}, {"sat_drive_db", 1.0f}, {"sat_mix", 0.1f},
                                {"air_db", 2.5f}, {"pan", 0.0f}, {"depth", 0.1f}, {"ducking", 0.1f}};
        }
        else if (preset.name == "Extreme Width")
        {
            preset.parameters = {{"gain_db", -2.0f}, {"width", 4.5f}, {"tilt", 0.0f}, {"mono_hz", 120.0f},
                                {"hp_hz", 30.0f}, {"lp_hz", 20000.0f}, {"sat_drive_db", 4.0f}, {"sat_mix", 0.4f},
                                {"air_db", 3.0f}, {"pan", 0.0f}, {"depth", 0.6f}, {"ducking", 0.2f}};
        }
        else if (preset.name == "Hard Pan")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 0.7f}, {"tilt", 0.0f}, {"mono_hz", 0.0f},
                                {"hp_hz", 60.0f}, {"lp_hz", 16000.0f}, {"sat_drive_db", 2.0f}, {"sat_mix", 0.2f},
                                {"air_db", 1.0f}, {"pan", 0.9f}, {"depth", 0.1f}, {"ducking", 0.4f}};
        }
        else if (preset.name == "Space Echo")
        {
            preset.parameters = {{"gain_db", 0.0f}, {"width", 1.8f}, {"tilt", 0.0f}, {"mono_hz", 0.0f},
                                {"hp_hz", 40.0f}, {"lp_hz", 12000.0f}, {"sat_drive_db", 3.0f}, {"sat_mix", 0.3f},
                                {"air_db", 1.5f}, {"pan", 0.0f}, {"depth", 0.8f}, {"ducking", 0.6f}};
        }
    }
}

void PresetManager::savePresetsToFile()
{
    auto userPresetsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                          .getChildFile("Field/Presets");
    userPresetsDir.createDirectory();
    
    auto presetsFile = userPresetsDir.getChildFile("user_presets.json");
    juce::var presetsArray = juce::var(juce::Array<juce::var>());
    
    for (const auto& preset : presets)
    {
        if (preset.isUserPreset)
        {
                auto presetObj = new juce::DynamicObject();
    presetObj->setProperty("name", preset.name);
    presetObj->setProperty("category", preset.category);
    presetObj->setProperty("subcategory", preset.subcategory);
    presetObj->setProperty("description", preset.description);
    presetObj->setProperty("author", preset.author);
    presetObj->setProperty("isUserPreset", preset.isUserPreset);
            
                auto paramsObj = new juce::DynamicObject();
    for (const auto& param : preset.parameters)
    {
        paramsObj->setProperty(param.first, param.second);
    }
    presetObj->setProperty("parameters", paramsObj);
            
            presetsArray.append(presetObj);
        }
    }
    
    presetsFile.replaceWithText(presetsArray.toString());
}

void PresetManager::loadPresetsFromFile()
{
    auto userPresetsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                          .getChildFile("Field/Presets");
    auto presetsFile = userPresetsDir.getChildFile("user_presets.json");
    
    if (presetsFile.existsAsFile())
    {
        auto json = juce::JSON::parse(presetsFile);
        if (json.isArray())
        {
            for (const auto& presetVar : *json.getArray())
            {
                if (presetVar.isObject())
                {
                    Preset preset;
                    preset.name = presetVar["name"].toString();
                    preset.category = presetVar["category"].toString();
                    preset.subcategory = presetVar["subcategory"].toString();
                    preset.description = presetVar["description"].toString();
                    preset.author = presetVar["author"].toString();
                    preset.isUserPreset = presetVar["isUserPreset"];
                    
                    auto paramsVar = presetVar["parameters"];
                    if (paramsVar.isObject())
                    {
                                            auto properties = paramsVar.getDynamicObject()->getProperties();
                    for (int i = 0; i < properties.size(); ++i)
                    {
                        auto paramName = properties.getName(i).toString();
                        preset.parameters[paramName] = properties.getValueAt(i);
                    }
                    }
                    
                    presets.push_back(preset);
                }
            }
        }
    }
}

// PresetComboBox implementation
PresetComboBox::PresetComboBox()
    : presetManager(nullptr), showFavoritesOnly(false), inSearchMode(false), searchPlaceholder("Search presets...")
{
    addListener(this);
    loadFavorites();
    setEditableText(true);
}

void PresetComboBox::setPresetManager(PresetManager* manager)
{
    presetManager = manager;
    updatePresetList();
}

void PresetComboBox::refreshPresets()
{
    updatePresetList();
}

void PresetComboBox::setCategory(const juce::String& category)
{
    currentCategory = category;
    updatePresetList();
}

void PresetComboBox::setSubcategory(const juce::String& subcategory)
{
    currentSubcategory = subcategory;
    updatePresetList();
}

void PresetComboBox::searchPresets(const juce::String& searchTerm)
{
    currentSearch = searchTerm;
    updatePresetList();
}

void PresetComboBox::toggleFavorite(const juce::String& presetName)
{
    if (favoritePresets.find(presetName) != favoritePresets.end()) {
        favoritePresets.erase(presetName);
    } else {
        favoritePresets.insert(presetName);
    }
    saveFavorites();
    updatePresetList();
}

bool PresetComboBox::isFavorite(const juce::String& presetName) const
{
    return favoritePresets.find(presetName) != favoritePresets.end();
}

void PresetComboBox::setShowFavoritesOnly(bool showFavorites)
{
    showFavoritesOnly = showFavorites;
    updatePresetList();
}

void PresetComboBox::setTagFilter(const juce::String& tag)
{
    currentTagFilter = tag;
    updatePresetList();
}

void PresetComboBox::clearFilters()
{
    currentSearch.clear();
    currentTagFilter.clear();
    showFavoritesOnly = false;
    updatePresetList();
}

void PresetComboBox::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (onPresetSelected && getSelectedId() > 0) {
        selectedPresetName = getText();
        if (selectedPresetName.startsWith("★ ")) {
            selectedPresetName = selectedPresetName.substring(2); // Remove star prefix
        }
        exitSearchMode();
        onPresetSelected(selectedPresetName);
    }
}

void PresetComboBox::enterSearchMode()
{
    inSearchMode = true;
    selectedPresetName = getText();
    if (selectedPresetName.startsWith("★ ")) {
        selectedPresetName = selectedPresetName.substring(2); // Remove star prefix
    }
    setText(searchPlaceholder, juce::dontSendNotification);
    setEditableText(true);
}

void PresetComboBox::exitSearchMode()
{
    inSearchMode = false;
    if (selectedPresetName.isNotEmpty()) {
        // Restore the selected preset name
        juce::String displayName = selectedPresetName;
        if (isFavorite(selectedPresetName)) {
            displayName = "★ " + displayName;
        }
        setText(displayName, juce::dontSendNotification);
    }
    setEditableText(false);
}

void PresetComboBox::updateSearchResults(const juce::String& searchText)
{
    if (inSearchMode && searchText != searchPlaceholder) {
        searchPresets(searchText);
    }
}

void PresetComboBox::mouseDown(const juce::MouseEvent& e)
{
    if (!inSearchMode) {
        enterSearchMode();
    }
    juce::ComboBox::mouseDown(e);
}

void PresetComboBox::textEditorTextChanged(juce::TextEditor& editor)
{
    if (inSearchMode && editor.getText() != searchPlaceholder) {
        updateSearchResults(editor.getText());
    }
}

void PresetComboBox::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    // Select first item if available
    if (getNumItems() > 0) {
        setSelectedId(1, juce::sendNotification);
    }
    exitSearchMode();
}

void PresetComboBox::textEditorEscapeKeyPressed(juce::TextEditor& editor)
{
    exitSearchMode();
}

void PresetComboBox::textEditorFocusLost(juce::TextEditor& editor)
{
    exitSearchMode();
}

void PresetComboBox::updatePresetList()
{
    clear();
    
    if (!presetManager) return;
    
    int itemId = 1;
    
    // Add category headers and presets
    juce::StringArray categories = presetManager->getCategories();
    
    for (const auto& category : categories) {
        if (!currentCategory.isEmpty() && category != currentCategory) continue;
        
        // Add category header
        addSectionHeading(category);
        
        juce::StringArray subcategories = presetManager->getSubcategories(category);
        
        for (const auto& subcategory : subcategories) {
            if (!currentSubcategory.isEmpty() && subcategory != currentSubcategory) continue;
            
            // Add subcategory header
            addSectionHeading("  " + subcategory);
            
            juce::StringArray presetNames = presetManager->getPresetNames(category, subcategory);
            
            for (const auto& presetName : presetNames) {
                Preset preset = presetManager->getPreset(presetName);
                
                if (!presetMatchesFilters(preset)) continue;
                
                // Add favorite star if applicable
                juce::String displayName = presetName;
                if (isFavorite(presetName)) {
                    displayName = "★ " + displayName;
                }
                
                addItem(displayName, itemId++);
            }
        }
    }
    
    // Set default selection
    if (getNumItems() > 0) {
        setSelectedId(1, juce::dontSendNotification);
    }
}

void PresetComboBox::loadFavorites()
{
    auto file = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Field/favorites.txt");
    
    if (file.existsAsFile()) {
        juce::StringArray lines;
        file.readLines(lines);
        
        for (const auto& line : lines) {
            if (line.isNotEmpty()) {
                favoritePresets.insert(line.trim());
            }
        }
    }
}

void PresetComboBox::saveFavorites()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("Field");
    if (!dir.exists()) {
        dir.createDirectory();
    }
    
    auto file = dir.getChildFile("favorites.txt");
    juce::StringArray lines;
    
    for (const auto& favorite : favoritePresets) {
        lines.add(favorite);
    }
    
    file.replaceWithText(lines.joinIntoString("\n"));
}

bool PresetComboBox::presetMatchesFilters(const Preset& preset) const
{
    // Check search filter
    if (currentSearch.isNotEmpty()) {
        bool matchesSearch = preset.name.containsIgnoreCase(currentSearch) ||
                           preset.description.containsIgnoreCase(currentSearch) ||
                           preset.category.containsIgnoreCase(currentSearch) ||
                           preset.subcategory.containsIgnoreCase(currentSearch);
        if (!matchesSearch) return false;
    }
    
    // Check favorites filter
    if (showFavoritesOnly && !isFavorite(preset.name)) {
        return false;
    }
    
    // Check tag filter (using subcategory as tags for now)
    if (currentTagFilter.isNotEmpty() && preset.subcategory != currentTagFilter) {
        return false;
    }
    
    return true;
}

// SavePresetButton implementation
SavePresetButton::SavePresetButton() : juce::Button("Save Preset")
{
    setTooltip("Save current settings as a preset");
}

void SavePresetButton::paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    
    // Draw raised shadow effect for depth
    g.setColour(juce::Colour(0x40000000)); // Semi-transparent black shadow
    g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 3.0f);
    
    // Background with light gradient (design system, toned down)
    juce::Colour base = juce::Colour(0xFF3A3D45);
    juce::Colour top  = base.brighter(0.10f);
    juce::Colour bot  = base.darker(0.10f);
    juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Border for definition
    g.setColour(isButtonDown ? juce::Colour(0xFF2A2A2A) : 
               isMouseOver ? juce::Colour(0xFF4A4A4A) : 
               juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    
    // Save icon - grey color
    juce::Colour iconColor = juce::Colour(0xFF888888);
    IconSystem::drawIcon(g, IconSystem::Save, bounds.reduced(4.0f), iconColor);
}

void SavePresetButton::mouseDown(const juce::MouseEvent&)
{
    if (onSavePreset)
        onSavePreset();
}

// PresetDropdownWithSubmenu implementation
PresetDropdownWithSubmenu::PresetDropdownWithSubmenu()
    : presetManager(nullptr), showFavoritesOnly(false), inSearchMode(false), 
      searchPlaceholder("Search presets..."), submenuVisible(false)
{
    juce::ComboBox::addListener(this);
    loadFavorites();
    setEditableText(true);
}

PresetDropdownWithSubmenu::~PresetDropdownWithSubmenu()
{
    juce::ComboBox::removeListener(this);
}

void PresetDropdownWithSubmenu::setPresetManager(PresetManager* manager)
{
    presetManager = manager;
    updatePresetList();
}

void PresetDropdownWithSubmenu::refreshPresets()
{
    updatePresetList();
}

void PresetDropdownWithSubmenu::setCategory(const juce::String& category)
{
    currentCategory = category;
    updatePresetList();
}

void PresetDropdownWithSubmenu::setSubcategory(const juce::String& subcategory)
{
    currentSubcategory = subcategory;
    updatePresetList();
}

void PresetDropdownWithSubmenu::searchPresets(const juce::String& searchTerm)
{
    currentSearch = searchTerm;
    updatePresetList();
}

void PresetDropdownWithSubmenu::toggleFavorite(const juce::String& presetName)
{
    if (favoritePresets.find(presetName) != favoritePresets.end()) {
        favoritePresets.erase(presetName);
    } else {
        favoritePresets.insert(presetName);
    }
    saveFavorites();
    updatePresetList();
}

bool PresetDropdownWithSubmenu::isFavorite(const juce::String& presetName) const
{
    return favoritePresets.find(presetName) != favoritePresets.end();
}

void PresetDropdownWithSubmenu::setShowFavoritesOnly(bool showFavorites)
{
    showFavoritesOnly = showFavorites;
    updatePresetList();
}

void PresetDropdownWithSubmenu::setTagFilter(const juce::String& tag)
{
    currentTagFilter = tag;
    updatePresetList();
}

void PresetDropdownWithSubmenu::clearFilters()
{
    currentSearch.clear();
    currentTagFilter.clear();
    showFavoritesOnly = false;
    updatePresetList();
}

void PresetDropdownWithSubmenu::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (onPresetSelected && getSelectedId() > 0) {
        selectedPresetName = getText();
        if (selectedPresetName.startsWith("★ ")) {
            selectedPresetName = selectedPresetName.substring(2); // Remove star prefix
        }
        exitSearchMode();
        onPresetSelected(selectedPresetName);
    }
}

void PresetDropdownWithSubmenu::enterSearchMode()
{
    inSearchMode = true;
    selectedPresetName = getText();
    if (selectedPresetName.startsWith("★ ")) {
        selectedPresetName = selectedPresetName.substring(2); // Remove star prefix
    }
    setText(searchPlaceholder, juce::dontSendNotification);
    setEditableText(true);
}

void PresetDropdownWithSubmenu::exitSearchMode()
{
    inSearchMode = false;
    if (selectedPresetName.isNotEmpty()) {
        // Restore the selected preset name
        juce::String displayName = selectedPresetName;
        if (isFavorite(selectedPresetName)) {
            displayName = "★ " + displayName;
        }
        setText(displayName, juce::dontSendNotification);
    }
    setEditableText(false);
}

void PresetDropdownWithSubmenu::updateSearchResults(const juce::String& searchText)
{
    if (inSearchMode && searchText != searchPlaceholder) {
        searchPresets(searchText);
    }
}

void PresetDropdownWithSubmenu::mouseDown(const juce::MouseEvent& e)
{
    if (!inSearchMode) {
        enterSearchMode();
    }
    juce::ComboBox::mouseDown(e);
}

void PresetDropdownWithSubmenu::textEditorTextChanged(juce::TextEditor& editor)
{
    if (inSearchMode && editor.getText() != searchPlaceholder) {
        updateSearchResults(editor.getText());
    }
}

void PresetDropdownWithSubmenu::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    // Select first item if available
    if (getNumItems() > 0) {
        setSelectedId(1, juce::sendNotification);
    }
    exitSearchMode();
}

void PresetDropdownWithSubmenu::textEditorEscapeKeyPressed(juce::TextEditor& editor)
{
    exitSearchMode();
}

void PresetDropdownWithSubmenu::textEditorFocusLost(juce::TextEditor& editor)
{
    exitSearchMode();
}

void PresetDropdownWithSubmenu::paint(juce::Graphics& g)
{
    // Custom paint method for visual distinction
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.setColour(juce::Colours::white);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Draw border
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    
    // Draw text with custom formatting
    if (getSelectedId() > 0) {
        juce::String text = getText();
        g.setColour(juce::Colours::black);
        g.setFont(14.0f);
        g.drawText(text, bounds.reduced(8.0f), juce::Justification::centredLeft);
    }
}

void PresetDropdownWithSubmenu::updatePresetList()
{
    clear();
    
    if (!presetManager) return;
    
    int itemId = 1;
    
    // Add category headers and presets with visual distinction
    juce::StringArray categories = presetManager->getCategories();
    
    for (const auto& category : categories) {
        if (!currentCategory.isEmpty() && category != currentCategory) continue;
        
        // Add category header with darker background
        addSectionHeading("=== " + category + " ===");
        
        juce::StringArray subcategories = presetManager->getSubcategories(category);
        
        for (const auto& subcategory : subcategories) {
            if (!currentSubcategory.isEmpty() && subcategory != currentSubcategory) continue;
            
            // Add subcategory header with medium background
            addSectionHeading("  -- " + subcategory + " --");
            
            juce::StringArray presetNames = presetManager->getPresetNames(category, subcategory);
            
            for (const auto& presetName : presetNames) {
                Preset preset = presetManager->getPreset(presetName);
                
                if (!presetMatchesFilters(preset)) continue;
                
                // Add favorite star if applicable
                juce::String displayName = presetName;
                if (isFavorite(presetName)) {
                    displayName = "★ " + displayName;
                }
                
                addItem(displayName, itemId++);
            }
        }
    }
    
    // Set default selection
    if (getNumItems() > 0) {
        setSelectedId(1, juce::dontSendNotification);
    }
}

void PresetDropdownWithSubmenu::loadFavorites()
{
    auto file = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Field/favorites.txt");
    
    if (file.existsAsFile()) {
        juce::StringArray lines;
        file.readLines(lines);
        
        for (const auto& line : lines) {
            if (line.isNotEmpty()) {
                favoritePresets.insert(line.trim());
            }
        }
    }
}

void PresetDropdownWithSubmenu::saveFavorites()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("Field");
    if (!dir.exists()) {
        dir.createDirectory();
    }
    
    auto file = dir.getChildFile("favorites.txt");
    juce::StringArray lines;
    
    for (const auto& favorite : favoritePresets) {
        lines.add(favorite);
    }
    
    file.replaceWithText(lines.joinIntoString("\n"));
}

bool PresetDropdownWithSubmenu::presetMatchesFilters(const Preset& preset) const
{
    // Check search filter
    if (currentSearch.isNotEmpty()) {
        bool matchesSearch = preset.name.containsIgnoreCase(currentSearch) ||
                           preset.description.containsIgnoreCase(currentSearch) ||
                           preset.category.containsIgnoreCase(currentSearch) ||
                           preset.subcategory.containsIgnoreCase(currentSearch);
        if (!matchesSearch) return false;
    }
    
    // Check favorites filter
    if (showFavoritesOnly && !isFavorite(preset.name)) {
        return false;
    }
    
    // Check tag filter (using subcategory as tags for now)
    if (currentTagFilter.isNotEmpty() && preset.subcategory != currentTagFilter) {
        return false;
    }
    
    return true;
}

void PresetDropdownWithSubmenu::showSubmenu(const Preset& preset)
{
    createSubmenuComponent(preset);
    if (submenuComponent) {
        addAndMakeVisible(submenuComponent.get());
        submenuVisible = true;
        resized();
    }
}

void PresetDropdownWithSubmenu::hideSubmenu()
{
    if (submenuComponent) {
        removeChildComponent(submenuComponent.get());
        submenuComponent.reset();
        submenuVisible = false;
        resized();
    }
}

void PresetDropdownWithSubmenu::createSubmenuComponent(const Preset& preset)
{
    // Create a custom component for the submenu
    submenuComponent = std::make_unique<juce::Component>();
    
    // This is a placeholder - in a full implementation, you would create
    // a detailed submenu component with preset information, favorite button, etc.
    submenuComponent->setSize(300, 200);
    
    // For now, we'll just create a simple text display
    auto* label = new juce::Label("presetInfo", 
        "Name: " + preset.name + "\n"
        "Category: " + preset.category + "\n"
        "Subcategory: " + preset.subcategory + "\n"
        "Description: " + preset.description + "\n"
        "Author: " + preset.author);
    
    label->setBounds(10, 10, 280, 180);
    submenuComponent->addAndMakeVisible(label);
} 