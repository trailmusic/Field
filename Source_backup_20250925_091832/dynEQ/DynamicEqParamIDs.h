#pragma once
#include <JuceHeader.h>

namespace dynEq
{
namespace IDs
{
    static constexpr const char* enabled          = "dyn_enabled";
    static constexpr const char* qualityMode      = "dyn_qualityMode";     // 0=Zero,1=Natural,2=Linear
    static constexpr const char* oversample       = "dyn_oversample";      // 1x,2x,4x,8x
    static constexpr const char* analyzerOn       = "dyn_analyzerOn";
    static constexpr const char* analyzerPrePost  = "dyn_analyzerPrePost"; // 0=Pre,1=Post
    static constexpr const char* latencyMs        = "dyn_latencyMsReadout"; // read-only UI
    static constexpr const char* unmaskEnable     = "dyn_unmask_enable";
    static constexpr const char* unmaskTargetBus  = "dyn_unmask_targetBus";
}

namespace Band
{
    static constexpr const char* active      = "b_active";
    static constexpr const char* type        = "b_type";      // 0=Bell,1=LS,2=HS,3=HP,4=LP,5=Notch,6=BP,7=AllPass
    static constexpr const char* slope       = "b_slope";     // choice index (6..96 dB/oct options)
    static constexpr const char* channel     = "b_channel";   // 0=Stereo,1=M,2=S,3=L,4=R
    static constexpr const char* phase       = "b_phase";     // 0=Zero,1=Natural,2=Linear
    static constexpr const char* freqHz      = "b_freqHz";
    static constexpr const char* gainDb      = "b_gainDb";
    static constexpr const char* q           = "b_q";

    static constexpr const char* dynOn       = "b_dynOn";
    static constexpr const char* dynMode     = "b_dynMode";   // 0=Down,1=Up
    static constexpr const char* dynRangeDb  = "b_dynRangeDb";
    static constexpr const char* dynThreshDb = "b_dynThreshDb";
    static constexpr const char* dynAtkMs    = "b_dynAtkMs";
    static constexpr const char* dynRelMs    = "b_dynRelMs";
    static constexpr const char* dynHoldMs   = "b_dynHoldMs";
    static constexpr const char* dynLookAheadMs = "b_dynLookAheadMs"; // 0/3/6
    static constexpr const char* dynDetectorSrc = "b_dynDetectorSrc"; // 0=PreXY,1=PostXY,2=External1,3=External2
    static constexpr const char* dynDetHPHz  = "b_dynDetHPHz";
    static constexpr const char* dynDetLPHz  = "b_dynDetLPHz";
    static constexpr const char* dynDetQ     = "b_dynDetQ";
    static constexpr const char* dynTAmount  = "b_dynTAmount";
    static constexpr const char* dynSAmount  = "b_dynSAmount";

    static constexpr const char* specOn      = "b_specOn";
    static constexpr const char* specRangeDb = "b_specRangeDb"; // downward range (0..24 dB)
    static constexpr const char* specSelect  = "b_specSelect"; // 0..100
    static constexpr const char* specResol   = "b_specResol";  // 0=Low,1=Med,2=High
    static constexpr const char* specAdaptive= "b_specAdaptive";

    static constexpr const char* character   = "b_character";  // 0=Clean,1=Subtle,2=Warm
    static constexpr const char* charAmt     = "b_charAmt";    // 0..100

    static constexpr const char* constOn     = "b_constOn";
    static constexpr const char* constRoot   = "b_constRoot";  // 0=Auto,1=Pitch,2=Note,3=Hz
    static constexpr const char* constHz     = "b_constHz";
    static constexpr const char* constNote   = "b_constNote";
    static constexpr const char* constCount  = "b_constCount"; // 2..16
    static constexpr const char* constSpread = "b_constSpread";
    static constexpr const char* constWeights= "b_constWeights"; // MemoryBlock
    static constexpr const char* constOddEven= "b_constOddEven"; // -100..100
    static constexpr const char* constTrack  = "b_constTrack";   // 0=Off,1=Mono,2=MIDI
    static constexpr const char* constGlideMs= "b_constGlideMs";
    static constexpr const char* constFormant= "b_constFormant";
}

inline void addDynamicEqParameters (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params, int maxBands = 24)
{
    using namespace juce;
    auto choice = [] (const String& id, const String& nm, const StringArray& opts, int def)
    {
        return std::make_unique<AudioParameterChoice>(ParameterID{id,1}, nm, opts, def);
    };
    auto boolp  = [] (const String& id, const String& nm, bool def)
    {
        return std::make_unique<AudioParameterBool>(ParameterID{id,1}, nm, def);
    };
    auto floatp = [] (const String& id, const String& nm, NormalisableRange<float> r, float def)
    {
        return std::make_unique<AudioParameterFloat>(ParameterID{id,1}, nm, r, def);
    };

    // Instance/global
    params.push_back (boolp  (IDs::enabled, "DynEQ Enable", true));
    params.push_back (choice (IDs::qualityMode, "DynEQ Quality", StringArray{ "Zero","Natural","Linear" }, 1));
    params.push_back (choice (IDs::oversample,  "DynEQ Oversample", StringArray{ "1x","2x","4x","8x" }, 0));
    params.push_back (boolp  (IDs::analyzerOn,  "DynEQ Analyzer", true));
    params.push_back (choice (IDs::analyzerPrePost, "DynEQ Analyzer Tap", StringArray{ "Pre","Post" }, 1));
    params.push_back (floatp (IDs::latencyMs, "DynEQ Latency (ms)", {0.f, 200.f, 0.f}, 0.f));
    params.push_back (boolp  (IDs::unmaskEnable, "DynEQ Unmask", false));
    params.push_back (floatp (IDs::unmaskTargetBus, "DynEQ Unmask Target", {-1.f, 64.f, 1.f}, -1.f));

    // Bands
    for (int i=0;i<maxBands;++i)
    {
        auto S = [i](const char* base){ return juce::String(base) + "_" + juce::String(i); };
        params.push_back (boolp  (S(Band::active),  "Dyn Band Active", false));
        params.push_back (choice (S(Band::type),    "Dyn Band Type", StringArray{ "Bell","LowShelf","HighShelf","HP","LP","Notch","BandPass","AllPass" }, 0));
        params.push_back (choice (S(Band::slope),   "Dyn Band Slope", StringArray{ "6","12","18","24","30","36","48","72","96" }, 3));
        params.push_back (choice (S(Band::channel), "Dyn Band Chan",  StringArray{ "Stereo","Mid","Side","Left","Right" }, 0));
        params.push_back (choice (S(Band::phase),   "Dyn Band Phase", StringArray{ "Zero","Natural","Linear" }, 1));
        params.push_back (floatp (S(Band::freqHz),  "Dyn Band Freq",  {20.f, 20000.f, 0.f, 0.25f}, 1000.f));
        params.push_back (floatp (S(Band::gainDb),  "Dyn Band Gain",  {-24.f, 24.f, 0.f}, 0.f));
        params.push_back (floatp (S(Band::q),       "Dyn Band Q",     {0.1f, 36.0f, 0.f, 0.3f}, 0.707f));

        params.push_back (boolp  (S(Band::dynOn),   "Dyn On", false));
        params.push_back (choice (S(Band::dynMode), "Dyn Mode", StringArray{ "Down","Up" }, 0));
        params.push_back (floatp (S(Band::dynRangeDb),  "Dyn Range", {-24.f, 24.f, 0.f}, -3.f));
        params.push_back (floatp (S(Band::dynThreshDb), "Dyn Thresh", {-80.f, 0.f, 0.f}, -24.f));
        params.push_back (floatp (S(Band::dynAtkMs),    "Dyn Attack", {0.1f, 200.f, 0.f}, 10.f));
        params.push_back (floatp (S(Band::dynRelMs),    "Dyn Release",{5.f, 1000.f, 0.f}, 120.f));
        params.push_back (floatp (S(Band::dynHoldMs),   "Dyn Hold",   {0.f, 500.f, 0.f}, 0.f));
        params.push_back (choice (S(Band::dynLookAheadMs), "Dyn LookAhead", StringArray{ "0","3","6" }, 0));
        params.push_back (choice (S(Band::dynDetectorSrc),"Dyn Detector",  StringArray{ "PreXY","PostXY","External1","External2" }, 1));
        params.push_back (floatp (S(Band::dynDetHPHz),   "Dyn Det HP", {20.f, 2000.f, 0.f, 0.3f}, 20.f));
        params.push_back (floatp (S(Band::dynDetLPHz),   "Dyn Det LP", {1000.f, 20000.f, 0.f, 0.3f}, 20000.f));
        params.push_back (floatp (S(Band::dynDetQ),      "Dyn Det Q",  {0.3f, 12.f, 0.f, 0.3f}, 0.7f));
        params.push_back (floatp (S(Band::dynTAmount),   "Dyn Transient", {0.f, 1.f, 0.f}, 0.5f));
        params.push_back (floatp (S(Band::dynSAmount),   "Dyn Sustain",   {0.f, 1.f, 0.f}, 0.5f));

        params.push_back (boolp  (S(Band::specOn),    "Spec On", false));
        params.push_back (floatp (S(Band::specRangeDb), "Spec Range", {0.f, 24.f, 0.f}, 3.f));
        params.push_back (floatp (S(Band::specSelect), "Spec Select", {0.f, 100.f, 0.f}, 50.f));
        params.push_back (choice (S(Band::specResol),  "Spec Res", StringArray{ "Low","Med","High" }, 1));
        params.push_back (boolp  (S(Band::specAdaptive), "Spec Adaptive", true));

        params.push_back (choice (S(Band::character), "Character", StringArray{ "Clean","Subtle","Warm" }, 0));
        params.push_back (floatp (S(Band::charAmt),   "Char Amt", {0.f, 100.f, 0.f}, 0.f));

        params.push_back (boolp  (S(Band::constOn),     "Const On", false));
        params.push_back (choice (S(Band::constRoot),   "Const Root", StringArray{ "Auto","Pitch","Note","Hz" }, 1));
        params.push_back (floatp (S(Band::constHz),     "Const Hz", {20.f, 4000.f, 0.f, 0.3f}, 110.f));
        params.push_back (floatp (S(Band::constNote),   "Const Note", {0.f, 127.f, 1.f}, 57.f));
        params.push_back (floatp (S(Band::constCount),  "Const Count", {2.f, 16.f, 1.f}, 6.f));
        params.push_back (floatp (S(Band::constSpread), "Const Spread", {0.f, 100.f, 0.f}, 25.f));
        params.push_back (floatp (S(Band::constOddEven),"Const OddEven", {-100.f, 100.f, 1.f}, 0.f));
        params.push_back (choice (S(Band::constTrack),  "Const Track", StringArray{ "Off","Mono","MIDI" }, 1));
        params.push_back (floatp (S(Band::constGlideMs),"Const Glide", {1.f, 500.f, 0.f}, 50.f));
        params.push_back (boolp  (S(Band::constFormant),"Const Formant", true));
    }
}
}


