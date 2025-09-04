#include "PresetRegistry.h"
#include "dsp/DelayPresetLibrary.h"
using namespace juce;

static PresetEntry makePreset(
    String name, String category, String author, String desc, String hint,
    StringArray tags, std::initializer_list<std::pair<const char*, var>> kv)
{
    PresetEntry p; p.id = Uuid(); p.name=name; p.category=category; p.author=author;
    p.desc=desc; p.hint=hint; p.tags = tags; p.isFactory=true;
    for (auto& it : kv) p.params.set (it.first, it.second);
    return p;
}

static void addFactoryPresets(juce::Array<PresetEntry>& out)
{
    out.ensureStorageAllocated(32);

    // 1) Lead Vox Safety Echo
    out.add (makePreset(
        "Lead Vox Safety Echo", "Vocals", "Factory",
        "Always-on vocal support—clean 1/8 that adds depth without blur.",
        "Set first; it should feel like part of the vocal.",
        { "vocal","support","clean","1/8","ducked","digital" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 250.0}, {"timeDiv", 8}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 22.0}, {"wet", 0.16}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 20.0}, {"stereoSpreadPct", 20.0}, {"width", 0.90},
            {"modRateHz", 0.35}, {"modDepthMs", 2.5}, {"wowflutter", 0.0}, {"jitterPct", 1.5},
            {"hpHz", 140.0}, {"lpHz", 9500.0}, {"tiltDb", 0.0}, {"sat", 0.05},
            {"diffusion", 0.0}, {"diffuseSizeMs", 18.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.55},
            {"duckAttackMs", 10.0}, {"duckReleaseMs", 180.0}, {"duckThresholdDb", -24.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 2) Vocal Throw — Dotted 1/4
    out.add (makePreset(
        "Vocal Throw — Dotted 1/4", "Vocals", "Factory",
        "Big dotted-quarter throws that tuck under the phrase via input-ducking.",
        "Automate wet for end-of-line throws.",
        { "vocal","throw","dotted","1/4","wide","ducked","digital" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 375.0}, {"timeDiv", 4}, {"gridFlavor", 1}, {"tempoBpm", 120.0},
            {"feedbackPct", 52.0}, {"wet", 0.28}, {"killDry", false}, {"freeze", false},
            {"pingpong", true}, {"crossfeedPct", 25.0}, {"stereoSpreadPct", 30.0}, {"width", 1.0},
            {"modRateHz", 0.25}, {"modDepthMs", 1.5}, {"wowflutter", 0.0}, {"jitterPct", 1.0},
            {"hpHz", 160.0}, {"lpHz", 10000.0}, {"tiltDb", 0.0}, {"sat", 0.08},
            {"diffusion", 0.05}, {"diffuseSizeMs", 16.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.70},
            {"duckAttackMs", 6.0}, {"duckReleaseMs", 260.0}, {"duckThresholdDb", -20.0},
            {"duckRatio", 3.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 3) Rockabilly Slapback
    out.add (makePreset(
        "Rockabilly Slapback", "Vocals", "Factory",
        "Punchy tape slap ~100 ms; minimal feedback; touch of saturation.",
        "Great on lead vox & snare.",
        { "slap","rock","tape","short","mono" },
        {
            {"enabled", true}, {"mode", 2}, {"sync", false}, {"timeMs", 110.0}, {"timeDiv", 8}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 12.0}, {"wet", 0.12}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 0.0}, {"stereoSpreadPct", 10.0}, {"width", 0.85},
            {"modRateHz", 0.60}, {"modDepthMs", 1.6}, {"wowflutter", 0.35}, {"jitterPct", 1.5},
            {"hpHz", 180.0}, {"lpHz", 8000.0}, {"tiltDb", -0.5}, {"sat", 0.45},
            {"diffusion", 0.00}, {"diffuseSizeMs", 14.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.20},
            {"duckAttackMs", 8.0}, {"duckReleaseMs", 120.0}, {"duckThresholdDb", -18.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 4) Tape Slap + Micro Chorus
    out.add (makePreset(
        "Tape Slap + Micro Chorus", "Vocals", "Factory",
        "Short tape slap with subtle mod for width—never seasick.",
        "Nudge time 70–110 ms to taste.",
        { "slap","tape","chorus","short","vocal","guitar" },
        {
            {"enabled", true}, {"mode", 2}, {"sync", false}, {"timeMs", 85.0}, {"timeDiv", 16}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 18.0}, {"wet", 0.14}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 0.0}, {"stereoSpreadPct", 18.0}, {"width", 0.95},
            {"modRateHz", 0.90}, {"modDepthMs", 2.2}, {"wowflutter", 0.30}, {"jitterPct", 1.0},
            {"hpHz", 150.0}, {"lpHz", 9500.0}, {"tiltDb", 0.0}, {"sat", 0.30},
            {"diffusion", 0.00}, {"diffuseSizeMs", 16.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.25},
            {"duckAttackMs", 8.0}, {"duckReleaseMs", 140.0}, {"duckThresholdDb", -22.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 4.0}, {"duckLinkGlobal", true}
        }
    ));

    // 5) Analog Ping-Pong Dotted 1/8
    out.add (makePreset(
        "Analog Ping-Pong Dotted 1/8", "Guitars", "Factory",
        "BBD-style dotted-eighth ping-pong; musical width without fizz.",
        "Automate FEEDBACK for builds.",
        { "analog","guitar","synth","ping-pong","dotted","1/8" },
        {
            {"enabled", true}, {"mode", 1}, {"sync", true}, {"timeMs", 350.0}, {"timeDiv", 8}, {"gridFlavor", 1}, {"tempoBpm", 120.0},
            {"feedbackPct", 48.0}, {"wet", 0.24}, {"killDry", false}, {"freeze", false},
            {"pingpong", true}, {"crossfeedPct", 30.0}, {"stereoSpreadPct", 25.0}, {"width", 1.0},
            {"modRateHz", 0.35}, {"modDepthMs", 3.0}, {"wowflutter", 0.00}, {"jitterPct", 3.0},
            {"hpHz", 120.0}, {"lpHz", 7500.0}, {"tiltDb", 0.0}, {"sat", 0.35},
            {"diffusion", 0.10}, {"diffuseSizeMs", 14.0},
            {"duckSource", 2}, {"duckPost", true}, {"duckDepth", 0.50},
            {"duckAttackMs", 12.0}, {"duckReleaseMs", 200.0}, {"duckThresholdDb", -24.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 6) Quarter Note Crowd
    out.add (makePreset(
        "Quarter Note Crowd", "Utility", "Factory",
        "Clean quarter repeats that sit behind a busy mix.",
        "Push width to 1.00 for a wider bed.",
        { "digital","1/4","clean","utility" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 500.0}, {"timeDiv", 4}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 38.0}, {"wet", 0.22}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 10.0}, {"stereoSpreadPct", 20.0}, {"width", 0.95},
            {"modRateHz", 0.25}, {"modDepthMs", 1.2}, {"wowflutter", 0.0}, {"jitterPct", 0.5},
            {"hpHz", 120.0}, {"lpHz", 12000.0}, {"tiltDb", 0.0}, {"sat", 0.05},
            {"diffusion", 0.05}, {"diffuseSizeMs", 16.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.35},
            {"duckAttackMs", 10.0}, {"duckReleaseMs", 180.0}, {"duckThresholdDb", -24.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 7) Triplet Trap Tail
    out.add (makePreset(
        "Triplet Trap Tail", "Vocals", "Factory",
        "1/8T repeats for modern vocal tails; a touch darker.",
        "Great under ad-libs.",
        { "vocal","1/8T","triplet","dark","modern" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 250.0}, {"timeDiv", 8}, {"gridFlavor", 2}, {"tempoBpm", 120.0},
            {"feedbackPct", 42.0}, {"wet", 0.24}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 10.0}, {"stereoSpreadPct", 20.0}, {"width", 0.95},
            {"modRateHz", 0.30}, {"modDepthMs", 1.8}, {"wowflutter", 0.0}, {"jitterPct", 1.0},
            {"hpHz", 180.0}, {"lpHz", 11000.0}, {"tiltDb", -1.5}, {"sat", 0.08},
            {"diffusion", 0.00}, {"diffuseSizeMs", 16.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.50},
            {"duckAttackMs", 8.0}, {"duckReleaseMs", 200.0}, {"duckThresholdDb", -22.0},
            {"duckRatio", 2.5}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 8) Cloud Wash (Diffused Echo)
    out.add (makePreset(
        "Cloud Wash (Diffused)", "Ambient", "Factory",
        "High-feedback 1/8 with diffusion for a soft, reverb-like bed.",
        "Blend under pads or piano.",
        { "ambient","diffused","1/8","wash","soft" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 250.0}, {"timeDiv", 8}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 55.0}, {"wet", 0.30}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 15.0}, {"stereoSpreadPct", 30.0}, {"width", 1.0},
            {"modRateHz", 0.20}, {"modDepthMs", 1.0}, {"wowflutter", 0.0}, {"jitterPct", 0.5},
            {"hpHz", 120.0}, {"lpHz", 14000.0}, {"tiltDb", 0.0}, {"sat", 0.10},
            {"diffusion", 0.35}, {"diffuseSizeMs", 26.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.30},
            {"duckAttackMs", 10.0}, {"duckReleaseMs", 240.0}, {"duckThresholdDb", -24.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 9) Vintage Space Echo (Long)
    out.add (makePreset(
        "Vintage Space Echo (Long)", "FX", "Factory",
        "Long tape echo with wobble + gentle diffusion for classic dub tails.",
        "Ride FEEDBACK live.",
        { "tape","long","dub","wobble","fx" },
        {
            {"enabled", true}, {"mode", 2}, {"sync", false}, {"timeMs", 430.0}, {"timeDiv", 4}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 60.0}, {"wet", 0.22}, {"killDry", false}, {"freeze", false},
            {"pingpong", true}, {"crossfeedPct", 20.0}, {"stereoSpreadPct", 25.0}, {"width", 0.95},
            {"modRateHz", 0.40}, {"modDepthMs", 3.2}, {"wowflutter", 0.45}, {"jitterPct", 4.0},
            {"hpHz", 100.0}, {"lpHz", 12000.0}, {"tiltDb", -0.5}, {"sat", 0.40},
            {"diffusion", 0.15}, {"diffuseSizeMs", 22.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.20},
            {"duckAttackMs", 12.0}, {"duckReleaseMs", 240.0}, {"duckThresholdDb", -26.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 10) Crystal Clean 1/16
    out.add (makePreset(
        "Crystal Clean 1/16", "Utility", "Factory",
        "Tight digital 1/16 for rhythmic sparkle without smear.",
        "Great on arps and hats.",
        { "digital","clean","1/16","sparkle" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 125.0}, {"timeDiv", 16}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 28.0}, {"wet", 0.18}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 0.0}, {"stereoSpreadPct", 18.0}, {"width", 1.0},
            {"modRateHz", 0.20}, {"modDepthMs", 0.8}, {"wowflutter", 0.0}, {"jitterPct", 0.5},
            {"hpHz", 100.0}, {"lpHz", 16000.0}, {"tiltDb", 0.0}, {"sat", 0.02},
            {"diffusion", 0.00}, {"diffuseSizeMs", 14.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.20},
            {"duckAttackMs", 12.0}, {"duckReleaseMs", 180.0}, {"duckThresholdDb", -28.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 4.0}, {"duckLinkGlobal", true}
        }
    ));

    // 11) Stereo Haas Widener
    out.add (makePreset(
        "Stereo Haas Widener", "Imaging", "Factory",
        "Very short unsynced echo to widen without chorus smear.",
        "Keep wet low (10–15%).",
        { "haas","short","wide","imaging" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", false}, {"timeMs", 20.0}, {"timeDiv", 64}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 0.0}, {"wet", 0.12}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 0.0}, {"stereoSpreadPct", 40.0}, {"width", 1.0},
            {"modRateHz", 0.00}, {"modDepthMs", 0.0}, {"wowflutter", 0.0}, {"jitterPct", 0.0},
            {"hpHz", 150.0}, {"lpHz", 18000.0}, {"tiltDb", 0.0}, {"sat", 0.00},
            {"diffusion", 0.00}, {"diffuseSizeMs", 14.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.00},
            {"duckAttackMs", 10.0}, {"duckReleaseMs", 120.0}, {"duckThresholdDb", -28.0},
            {"duckRatio", 1.0}, {"duckLookaheadMs", 0.0}, {"duckLinkGlobal", true}
        }
    ));

    // 12) Lo-Fi Phone Delay
    out.add (makePreset(
        "Lo-Fi Phone Delay", "FX", "Factory",
        "Band-limited analog 1/8 with character and grit.",
        "Blend in parallel on buses.",
        { "lofi","analog","band-limited","1/8","fx" },
        {
            {"enabled", true}, {"mode", 1}, {"sync", true}, {"timeMs", 250.0}, {"timeDiv", 8}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 34.0}, {"wet", 0.22}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 10.0}, {"stereoSpreadPct", 15.0}, {"width", 0.9},
            {"modRateHz", 0.35}, {"modDepthMs", 2.0}, {"wowflutter", 0.20}, {"jitterPct", 5.0},
            {"hpHz", 400.0}, {"lpHz", 3500.0}, {"tiltDb", -1.0}, {"sat", 0.50},
            {"diffusion", 0.05}, {"diffuseSizeMs", 16.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.30},
            {"duckAttackMs", 8.0}, {"duckReleaseMs", 160.0}, {"duckThresholdDb", -22.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 4.0}, {"duckLinkGlobal", true}
        }
    ));

    // 13) Arp Sparkle 1/16T
    out.add (makePreset(
        "Arp Sparkle 1/16T", "Keys", "Factory",
        "Triplet 1/16 for motion on arps and plucks; very clean.",
        "Pair with a gentle high-shelf in the mix.",
        { "keys","arp","triplet","1/16","clean" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 83.3}, {"timeDiv", 16}, {"gridFlavor", 2}, {"tempoBpm", 120.0},
            {"feedbackPct", 22.0}, {"wet", 0.16}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 0.0}, {"stereoSpreadPct", 18.0}, {"width", 1.0},
            {"modRateHz", 0.50}, {"modDepthMs", 1.5}, {"wowflutter", 0.0}, {"jitterPct", 0.5},
            {"hpHz", 200.0}, {"lpHz", 16000.0}, {"tiltDb", 0.5}, {"sat", 0.02},
            {"diffusion", 0.00}, {"diffuseSizeMs", 14.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.20},
            {"duckAttackMs", 10.0}, {"duckReleaseMs", 140.0}, {"duckThresholdDb", -26.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 4.0}, {"duckLinkGlobal", true}
        }
    ));

    // 14) Dub Feedback Ride
    out.add (makePreset(
        "Dub Feedback Ride", "FX", "Factory",
        "Feedback-heavy analog quarter for live rides and drops.",
        "Map FEEDBACK to a knob or MIDI.",
        { "dub","analog","1/4","feedback","fx" },
        {
            {"enabled", true}, {"mode", 1}, {"sync", true}, {"timeMs", 500.0}, {"timeDiv", 4}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 72.0}, {"wet", 0.32}, {"killDry", false}, {"freeze", false},
            {"pingpong", true}, {"crossfeedPct", 35.0}, {"stereoSpreadPct", 20.0}, {"width", 0.95},
            {"modRateHz", 0.30}, {"modDepthMs", 2.2}, {"wowflutter", 0.10}, {"jitterPct", 2.0},
            {"hpHz", 120.0}, {"lpHz", 9000.0}, {"tiltDb", 0.0}, {"sat", 0.50},
            {"diffusion", 0.20}, {"diffuseSizeMs", 18.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.20},
            {"duckAttackMs", 10.0}, {"duckReleaseMs", 240.0}, {"duckThresholdDb", -24.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 15) Ambient Bloom
    out.add (makePreset(
        "Ambient Bloom", "Ambient", "Factory",
        "Long unsynced echoes with diffusion for dreamy pads and guitars.",
        "Use after reverb for even softer bloom.",
        { "ambient","long","diffused","pads","guitar" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", false}, {"timeMs", 680.0}, {"timeDiv", 4}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 58.0}, {"wet", 0.28}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 10.0}, {"stereoSpreadPct", 30.0}, {"width", 1.0},
            {"modRateHz", 0.18}, {"modDepthMs", 1.2}, {"wowflutter", 0.0}, {"jitterPct", 1.0},
            {"hpHz", 80.0}, {"lpHz", 12000.0}, {"tiltDb", -0.5}, {"sat", 0.08},
            {"diffusion", 0.45}, {"diffuseSizeMs", 30.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.25},
            {"duckAttackMs", 12.0}, {"duckReleaseMs", 280.0}, {"duckThresholdDb", -26.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 16) Drum Bus Ghost 1/8
    out.add (makePreset(
        "Drum Bus Ghost 1/8", "Drums", "Factory",
        "Tight 1/8 ambience that breathes with the kit via strong ducking.",
        "Great on overheads or full drum bus.",
        { "drums","1/8","ghost","ducked","utility" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 250.0}, {"timeDiv", 8}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 24.0}, {"wet", 0.12}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 10.0}, {"stereoSpreadPct", 10.0}, {"width", 0.9},
            {"modRateHz", 0.20}, {"modDepthMs", 1.0}, {"wowflutter", 0.0}, {"jitterPct", 1.0},
            {"hpHz", 200.0}, {"lpHz", 9000.0}, {"tiltDb", 0.0}, {"sat", 0.06},
            {"diffusion", 0.05}, {"diffuseSizeMs", 16.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.75},
            {"duckAttackMs", 4.0}, {"duckReleaseMs", 120.0}, {"duckThresholdDb", -18.0},
            {"duckRatio", 4.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 17) Snare Throw 1/2 Bar
    out.add (makePreset(
        "Snare Throw 1/2 Bar", "Drums", "Factory",
        "Wide half-note throws for big transitions; moderate ducking.",
        "Trigger on fills and endings.",
        { "snare","throw","1/2","wide","fx" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 1000.0}, {"timeDiv", 2}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 62.0}, {"wet", 0.26}, {"killDry", false}, {"freeze", false},
            {"pingpong", true}, {"crossfeedPct", 25.0}, {"stereoSpreadPct", 25.0}, {"width", 1.0},
            {"modRateHz", 0.25}, {"modDepthMs", 1.8}, {"wowflutter", 0.0}, {"jitterPct", 1.5},
            {"hpHz", 250.0}, {"lpHz", 10000.0}, {"tiltDb", 0.0}, {"sat", 0.10},
            {"diffusion", 0.05}, {"diffuseSizeMs", 18.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.40},
            {"duckAttackMs", 8.0}, {"duckReleaseMs", 220.0}, {"duckThresholdDb", -22.0},
            {"duckRatio", 2.2}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 18) Guitar Rhythmic Dotted 1/8
    out.add (makePreset(
        "Guitar Rhythmic Dotted 1/8", "Guitars", "Factory",
        "Classic dotted-eighth rhythm echo, slightly saturated.",
        "Sync your picking to the repeats.",
        { "guitar","rhythmic","dotted","1/8","analog" },
        {
            {"enabled", true}, {"mode", 1}, {"sync", true}, {"timeMs", 350.0}, {"timeDiv", 8}, {"gridFlavor", 1}, {"tempoBpm", 120.0},
            {"feedbackPct", 35.0}, {"wet", 0.20}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 15.0}, {"stereoSpreadPct", 20.0}, {"width", 0.95},
            {"modRateHz", 0.45}, {"modDepthMs", 2.8}, {"wowflutter", 0.05}, {"jitterPct", 2.0},
            {"hpHz", 130.0}, {"lpHz", 9000.0}, {"tiltDb", 0.0}, {"sat", 0.25},
            {"diffusion", 0.08}, {"diffuseSizeMs", 16.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.30},
            {"duckAttackMs", 10.0}, {"duckReleaseMs", 180.0}, {"duckThresholdDb", -24.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 19) Keys Tape Flutter — Dotted 1/4
    out.add (makePreset(
        "Keys Tape Flutter — Dotted 1/4", "Keys", "Factory",
        "Fluttery tape dotted-quarter for nostalgic keys/pads.",
        "Layer under a plate verb.",
        { "keys","tape","dotted","1/4","flutter" },
        {
            {"enabled", true}, {"mode", 2}, {"sync", true}, {"timeMs", 375.0}, {"timeDiv", 4}, {"gridFlavor", 1}, {"tempoBpm", 120.0},
            {"feedbackPct", 46.0}, {"wet", 0.24}, {"killDry", false}, {"freeze", false},
            {"pingpong", false}, {"crossfeedPct", 10.0}, {"stereoSpreadPct", 25.0}, {"width", 1.0},
            {"modRateHz", 0.30}, {"modDepthMs", 3.2}, {"wowflutter", 0.50}, {"jitterPct", 3.5},
            {"hpHz", 100.0}, {"lpHz", 12000.0}, {"tiltDb", -0.5}, {"sat", 0.30},
            {"diffusion", 0.10}, {"diffuseSizeMs", 20.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.30},
            {"duckAttackMs", 12.0}, {"duckReleaseMs", 220.0}, {"duckThresholdDb", -24.0},
            {"duckRatio", 2.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));

    // 20) EDM Pre-Drop Throw — Whole Note
    out.add (makePreset(
        "EDM Pre-Drop Throw — Whole", "FX", "Factory",
        "Huge whole-note throw that ducks hard under the build.",
        "Turn on Ping-Pong for extra width.",
        { "edm","throw","1/1","huge","ducked","digital" },
        {
            {"enabled", true}, {"mode", 0}, {"sync", true}, {"timeMs", 2000.0}, {"timeDiv", 1}, {"gridFlavor", 0}, {"tempoBpm", 120.0},
            {"feedbackPct", 50.0}, {"wet", 0.22}, {"killDry", false}, {"freeze", false},
            {"pingpong", true}, {"crossfeedPct", 25.0}, {"stereoSpreadPct", 25.0}, {"width", 1.0},
            {"modRateHz", 0.20}, {"modDepthMs", 1.2}, {"wowflutter", 0.0}, {"jitterPct", 1.0},
            {"hpHz", 120.0}, {"lpHz", 11000.0}, {"tiltDb", 0.0}, {"sat", 0.05},
            {"diffusion", 0.00}, {"diffuseSizeMs", 16.0},
            {"duckSource", 0}, {"duckPost", true}, {"duckDepth", 0.60},
            {"duckAttackMs", 8.0}, {"duckReleaseMs", 300.0}, {"duckThresholdDb", -22.0},
            {"duckRatio", 3.0}, {"duckLookaheadMs", 5.0}, {"duckLinkGlobal", true}
        }
    ));
}

static std::unique_ptr<PropertiesFile> makeProps()
{
    PropertiesFile::Options o;
    o.applicationName = "Field"; o.filenameSuffix = "settings"; o.folderName = "Field/YourPlugin";
    o.storageFormat = PropertiesFile::storeAsXML; o.millisecondsBeforeSaving = 200;
    return std::make_unique<PropertiesFile>(o);
}

PresetRegistry::PresetRegistry()
{
    props = makeProps();
    loadCompiledFactory();
    tryLoadUserJson();
    loadFavorites();
}

static void addFromDelayPresets(Array<PresetEntry>& out)
{
    const auto& lib = DelayPresets::all();
    for (const auto& m : lib)
    {
        PresetEntry p; p.id = Uuid(); p.isFactory = true; p.author = "Factory";
        p.name = m.name; p.desc = m.desc; p.hint = m.hint; p.category = "Delay";
        p.tags = m.tags;
        const auto& dp = m.params;
        p.params.set("enabled", dp.enabled);
        p.params.set("mode", dp.mode);
        p.params.set("sync", dp.sync);
        p.params.set("timeMs", dp.timeMs);
        p.params.set("timeDiv", dp.timeDiv);
        p.params.set("gridFlavor", dp.gridFlavor);
        p.params.set("tempoBpm", dp.tempoBpm);
        p.params.set("feedbackPct", dp.feedbackPct);
        p.params.set("wet", dp.wet);
        p.params.set("killDry", dp.killDry);
        p.params.set("freeze", dp.freeze);
        p.params.set("pingpong", dp.pingpong);
        p.params.set("crossfeedPct", dp.crossfeedPct);
        p.params.set("stereoSpreadPct", dp.stereoSpreadPct);
        p.params.set("width", dp.width);
        p.params.set("modRateHz", dp.modRateHz);
        p.params.set("modDepthMs", dp.modDepthMs);
        p.params.set("wowflutter", dp.wowflutter);
        p.params.set("jitterPct", dp.jitterPct);
        p.params.set("hpHz", dp.hpHz);
        p.params.set("lpHz", dp.lpHz);
        p.params.set("tiltDb", dp.tiltDb);
        p.params.set("sat", dp.sat);
        p.params.set("diffusion", dp.diffusion);
        p.params.set("diffuseSizeMs", dp.diffuseSizeMs);
        p.params.set("duckSource", dp.duckSource);
        p.params.set("duckPost", dp.duckPost);
        p.params.set("duckDepth", dp.duckDepth);
        p.params.set("duckAttackMs", dp.duckAttackMs);
        p.params.set("duckReleaseMs", dp.duckReleaseMs);
        p.params.set("duckThresholdDb", dp.duckThresholdDb);
        p.params.set("duckRatio", dp.duckRatio);
        p.params.set("duckLookaheadMs", dp.duckLookaheadMs);
        p.params.set("duckLinkGlobal", dp.duckLinkGlobal);
        out.add (std::move(p));
    }
}

void PresetRegistry::loadCompiledFactory()
{
    list.clearQuick();
    // Always include the full compiled 20-pack
    addFactoryPresets (list);
    // Optionally merge any presets from DelayPresets::all()
    addFromDelayPresets (list);
    // De-duplicate by name (keep first occurrence)
    juce::StringArray seen;
    juce::Array<PresetEntry> dedup;
    for (auto& p : list)
    {
        if (! seen.contains (p.name)) { seen.add (p.name); dedup.add (p); }
    }
    list.swapWith (dedup);
    DBG("[PresetRegistry] factory presets loaded: " << list.size());
}

void PresetRegistry::tryLoadUserJson()
{
    auto f = File::getSpecialLocation(File::userApplicationDataDirectory)
             .getChildFile("Field/Presets/delay_presets.json");
    if (!f.existsAsFile()) return;
    auto v = JSON::parse (f.loadFileAsString());
    if (v.isVoid() || !v.isArray()) return;
    for (auto& it : *v.getArray())
    {
        if (!it.isObject()) continue;
        if (auto* d = it.getDynamicObject())
        {
            PresetEntry p; p.id = Uuid(); p.isFactory = false; p.author = "User";
            p.name = d->getProperty("name").toString();
            p.category = d->getProperty("category").toString();
            p.desc = d->getProperty("desc").toString();
            p.hint = d->getProperty("hint").toString();
            if (auto* arr = d->getProperty("tags").getArray()) for (auto& t : *arr) p.tags.add (t.toString());
            if (auto* pv = d->getProperty("params").getDynamicObject())
                for (auto& kv : pv->getProperties()) p.params.set (kv.name.toString(), kv.value);
            if (p.name.isNotEmpty()) list.add (std::move(p));
        }
    }
}

void PresetRegistry::reloadUserJson()
{
    // remove previous non-factory entries
    for (int i = list.size(); --i >= 0; ) if (! list.getReference(i).isFactory) list.remove (i);
    tryLoadUserJson();
    loadFavorites();
}

void PresetRegistry::reloadAll()
{
    list.clearQuick();
    loadCompiledFactory();
    tryLoadUserJson();
    loadFavorites();
    DBG("[PresetRegistry] total after reload (factory+user): " << list.size());
}

void PresetRegistry::loadFavorites()
{
    auto fav = props->getValue ("preset_favorites");
    if (fav.isNotEmpty())
    {
        StringArray ids; ids.addTokens (fav, ",", "\""); ids.trim(); ids.removeEmptyStrings();
        for (auto& p : list) if (ids.contains (p.id.toString())) p.isFavorite = true;
    }
}

void PresetRegistry::saveFavorites()
{
    StringArray ids; for (auto& p : list) if (p.isFavorite) ids.add (p.id.toString());
    props->setValue ("preset_favorites", ids.joinIntoString(",")); props->saveIfNeeded();
}

void PresetRegistry::setFavorite(const Uuid& id, bool fav)
{
    for (auto& p : list) if (p.id == id) p.isFavorite = fav; saveFavorites();
}

bool PresetRegistry::getFavorite(const Uuid& id) const
{
    for (auto& p : list) if (p.id == id) return p.isFavorite; return false;
}


