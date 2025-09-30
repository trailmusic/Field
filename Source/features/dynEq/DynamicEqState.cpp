#include "DynamicEqState.h"

using namespace juce;
using namespace dynEq;

static String bandNode (int idx) { return "band_" + String(idx); }

ValueTree BandState::toVT (int idx) const
{
    ValueTree t (bandNode(idx));
    auto setB = [&](const String& id, auto v){ t.setProperty (id, var(v), nullptr); };
    setB (Band::active, active); setB (Band::type, type); setB (Band::slope, slope); setB (Band::channel, channel); setB (Band::phase, phase);
    setB (Band::freqHz, freqHz); setB (Band::gainDb, gainDb); setB (Band::q, q);
    setB (Band::dynOn, dynOn); setB (Band::dynMode, dynMode); setB (Band::dynRangeDb, dynRangeDb); setB (Band::dynThreshDb, dynThreshDb);
    setB (Band::dynAtkMs, dynAtkMs); setB (Band::dynRelMs, dynRelMs); setB (Band::dynHoldMs, dynHoldMs); setB (Band::dynLookAheadMs, dynLookAheadMs);
    setB (Band::dynDetectorSrc, dynDetectorSrc); setB (Band::dynDetHPHz, dynDetHPHz); setB (Band::dynDetLPHz, dynDetLPHz); setB (Band::dynDetQ, dynDetQ);
    setB (Band::dynTAmount, dynTAmount); setB (Band::dynSAmount, dynSAmount);
    setB (Band::specOn, specOn); setB (Band::specSelect, specSelect); setB (Band::specResol, specResol); setB (Band::specAdaptive, specAdaptive);
    setB (Band::character, character); setB (Band::charAmt, charAmt);
    setB (Band::constOn, constOn); setB (Band::constRoot, constRoot); setB (Band::constHz, constHz); setB (Band::constNote, constNote);
    setB (Band::constCount, constCount); setB (Band::constSpread, constSpread);
    MemoryOutputStream mos; mos.writeInt ((int) constWeights.size()); for (auto v : constWeights) mos.writeFloat (v);
    t.setProperty (Band::constWeights, mos.getMemoryBlock(), nullptr);
    setB (Band::constOddEven, constOddEven); setB (Band::constTrack, constTrack); setB (Band::constGlideMs, constGlideMs); setB (Band::constFormant, constFormant);
    return t;
}

BandState BandState::fromVT (const ValueTree& t)
{
    BandState s;
    auto get = [&](const String& id, auto def){ return t.hasProperty(id) ? t[id] : var(def); };
    s.active=(bool)get(Band::active,s.active); s.type=(int)get(Band::type,s.type); s.slope=(int)get(Band::slope,s.slope); s.channel=(int)get(Band::channel,s.channel); s.phase=(int)get(Band::phase,s.phase);
    s.freqHz=(float)get(Band::freqHz,s.freqHz); s.gainDb=(float)get(Band::gainDb,s.gainDb); s.q=(float)get(Band::q,s.q);
    s.dynOn=(bool)get(Band::dynOn,s.dynOn); s.dynMode=(int)get(Band::dynMode,s.dynMode); s.dynRangeDb=(float)get(Band::dynRangeDb,s.dynRangeDb); s.dynThreshDb=(float)get(Band::dynThreshDb,s.dynThreshDb);
    s.dynAtkMs=(float)get(Band::dynAtkMs,s.dynAtkMs); s.dynRelMs=(float)get(Band::dynRelMs,s.dynRelMs); s.dynHoldMs=(float)get(Band::dynHoldMs,s.dynHoldMs); s.dynLookAheadMs=(float)get(Band::dynLookAheadMs,s.dynLookAheadMs);
    s.dynDetectorSrc=(int)get(Band::dynDetectorSrc,s.dynDetectorSrc); s.dynDetHPHz=(float)get(Band::dynDetHPHz,s.dynDetHPHz); s.dynDetLPHz=(float)get(Band::dynDetLPHz,s.dynDetLPHz); s.dynDetQ=(float)get(Band::dynDetQ,s.dynDetQ);
    s.dynTAmount=(float)get(Band::dynTAmount,s.dynTAmount); s.dynSAmount=(float)get(Band::dynSAmount,s.dynSAmount);
    s.specOn=(bool)get(Band::specOn,s.specOn); s.specSelect=(int)get(Band::specSelect,s.specSelect); s.specResol=(int)get(Band::specResol,s.specResol); s.specAdaptive=(bool)get(Band::specAdaptive,s.specAdaptive);
    s.character=(int)get(Band::character,s.character); s.charAmt=(float)get(Band::charAmt,s.charAmt);
    s.constOn=(bool)get(Band::constOn,s.constOn); s.constRoot=(int)get(Band::constRoot,s.constRoot); s.constHz=(float)get(Band::constHz,s.constHz); s.constNote=(int)get(Band::constNote,s.constNote);
    s.constCount=(int)get(Band::constCount,s.constCount); s.constSpread=(float)get(Band::constSpread,s.constSpread);
    if (auto mb = t.getProperty (Band::constWeights).getBinaryData()) { MemoryInputStream mis(*mb,false); int n=mis.readInt(); s.constWeights.clearQuick(); for(int i=0;i<n;++i) s.constWeights.add(mis.readFloat()); }
    s.constOddEven=(int)get(Band::constOddEven,s.constOddEven); s.constTrack=(int)get(Band::constTrack,s.constTrack); s.constGlideMs=(float)get(Band::constGlideMs,s.constGlideMs); s.constFormant=(bool)get(Band::constFormant,s.constFormant);
    return s;
}

ValueTree State::toVT() const
{
    ValueTree t ("DynamicEQ");
    auto setG=[&](const String& id, auto v){ t.setProperty (id, var(v), nullptr); };
    setG (IDs::enabled, enabled); setG (IDs::qualityMode, qualityMode); setG (IDs::oversample, oversample); setG (IDs::analyzerOn, analyzerOn); setG (IDs::analyzerPrePost, analyzerPrePost); setG (IDs::unmaskEnable, unmaskEnable); setG (IDs::unmaskTargetBus, unmaskTargetBus);
    for (int i=0;i<kMaxBands;++i) t.addChild (bands[i].toVT(i), -1, nullptr);
    return t;
}

State State::fromVT (const ValueTree& vts)
{
    State s;
    s.enabled=(bool)vts.getProperty(IDs::enabled,s.enabled); s.qualityMode=(int)vts.getProperty(IDs::qualityMode,s.qualityMode); s.oversample=(int)vts.getProperty(IDs::oversample,s.oversample); s.analyzerOn=(bool)vts.getProperty(IDs::analyzerOn,s.analyzerOn); s.analyzerPrePost=(int)vts.getProperty(IDs::analyzerPrePost,s.analyzerPrePost); s.unmaskEnable=(bool)vts.getProperty(IDs::unmaskEnable,s.unmaskEnable); s.unmaskTargetBus=(int)vts.getProperty(IDs::unmaskTargetBus,s.unmaskTargetBus);
    for (int i=0;i<vts.getNumChildren();++i)
    {
        auto c=vts.getChild(i); auto tag=c.getType().toString(); if (tag.startsWith("band_")) { int idx = tag.fromFirstOccurrenceOf("band_", false, false).getIntValue(); if (isPositiveAndBelow(idx, State::kMaxBands)) s.bands[idx]=BandState::fromVT(c); }
    }
    return s;
}


