#pragma once
#include <JuceHeader.h>

namespace ReverbEQParams
{
namespace IDs
{
    // Tone EQ (4-band post-reverb)
    static constexpr const char* toneEnabled = "rv_toneEnabled";
    static constexpr const char* toneQuality = "rv_toneQuality"; // 0=Zero,1=Natural,2=Linear
    static constexpr const char* toneOversample = "rv_toneOversample"; // 1x,2x,4x,8x
}

namespace ToneBand
{
    static constexpr const char* active = "tb_active";
    static constexpr const char* type = "tb_type"; // 0=Bell,1=LowShelf,2=HighShelf,3=HP,4=LP
    static constexpr const char* freqHz = "tb_freqHz";
    static constexpr const char* gainDb = "tb_gainDb";
    static constexpr const char* q = "tb_q";
    static constexpr const char* phase = "tb_phase"; // 0=Zero,1=Natural,2=Linear
}

namespace DecayBand
{
    static constexpr const char* active = "db_active";
    static constexpr const char* freqHz = "db_freqHz";
    static constexpr const char* q = "db_q";
    static constexpr const char* decayMult = "db_decayMult"; // 0.1x to 2.0x multiplier
    static constexpr const char* dynAmt = "db_dynAmt"; // 0-100% dynamic amount
}

// Parameter creation helpers
inline void addReverbEQParameters(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params)
{
    using namespace juce;
    
    auto choice = [](const String& id, const String& nm, const StringArray& opts, int def)
    {
        return std::make_unique<AudioParameterChoice>(ParameterID{id,1}, nm, opts, def);
    };
    auto boolp = [](const String& id, const String& nm, bool def)
    {
        return std::make_unique<AudioParameterBool>(ParameterID{id,1}, nm, def);
    };
    auto floatp = [](const String& id, const String& nm, NormalisableRange<float> r, float def)
    {
        return std::make_unique<AudioParameterFloat>(ParameterID{id,1}, nm, r, def);
    };

    // Tone EQ global
    params.push_back(boolp(IDs::toneEnabled, "Reverb Tone EQ", true));
    params.push_back(choice(IDs::toneQuality, "Reverb Tone Quality", StringArray{"Zero","Natural","Linear"}, 1));
    params.push_back(choice(IDs::toneOversample, "Reverb Tone Oversample", StringArray{"1x","2x","4x","8x"}, 0));

    // Tone EQ bands (4 bands)
    for (int i = 0; i < 4; ++i)
    {
        auto S = [i](const char* base) { return juce::String(base) + "_" + juce::String(i); };
        params.push_back(boolp(S(ToneBand::active), "Tone Band Active", false));
        params.push_back(choice(S(ToneBand::type), "Tone Band Type", StringArray{"Bell","LowShelf","HighShelf","HP","LP"}, 0));
        params.push_back(floatp(S(ToneBand::freqHz), "Tone Band Freq", {20.f, 20000.f, 0.f, 0.25f}, 1000.f));
        params.push_back(floatp(S(ToneBand::gainDb), "Tone Band Gain", {-24.f, 24.f, 0.f}, 0.f));
        params.push_back(floatp(S(ToneBand::q), "Tone Band Q", {0.1f, 36.0f, 0.f, 0.3f}, 0.707f));
        params.push_back(choice(S(ToneBand::phase), "Tone Band Phase", StringArray{"Zero","Natural","Linear"}, 1));
    }

    // Decay Rate EQ bands (3 bands)
    for (int i = 0; i < 3; ++i)
    {
        auto S = [i](const char* base) { return juce::String(base) + "_" + juce::String(i); };
        params.push_back(boolp(S(DecayBand::active), "Decay Band Active", false));
        params.push_back(floatp(S(DecayBand::freqHz), "Decay Band Freq", {20.f, 20000.f, 0.f, 0.25f}, 1000.f));
        params.push_back(floatp(S(DecayBand::q), "Decay Band Q", {0.1f, 36.0f, 0.f, 0.3f}, 0.707f));
        params.push_back(floatp(S(DecayBand::decayMult), "Decay Multiplier", {0.1f, 2.0f, 0.f}, 1.0f));
        params.push_back(floatp(S(DecayBand::dynAmt), "Decay Dyn Amount", {0.f, 100.f, 0.f}, 0.f));
    }
}
}
