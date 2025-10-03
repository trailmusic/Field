#include "ReverbParamMap.h"
#include "../DSP/ReverbParamIDs.h"

ReverbParamMap::ReverbParamMap()
{
    initializeParamMap();
}

void ReverbParamMap::initializeParamMap()
{
    // Core reverb parameters
    paramMap.map.set("decaySec", ReverbParamIDs::decaySec);
    paramMap.map.set("preDelayMs", ReverbParamIDs::preDelayMs);
    paramMap.map.set("erLevelDb", ReverbParamIDs::erLevelDb);
    paramMap.map.set("erTimeMs", ReverbParamIDs::erTimeMs);
    paramMap.map.set("erDensityPct", ReverbParamIDs::erDensityPct);
    paramMap.map.set("erWidthPct", ReverbParamIDs::erWidthPct);
    paramMap.map.set("erToTailPct", ReverbParamIDs::erToTailPct);
    paramMap.map.set("diffusionPct", ReverbParamIDs::diffusionPct);
    paramMap.map.set("densityPct", ReverbParamIDs::densityPct);
    paramMap.map.set("modDepthCents", ReverbParamIDs::modDepthCents);
    paramMap.map.set("modRateHz", ReverbParamIDs::modRateHz);
    paramMap.map.set("widthPct", ReverbParamIDs::widthPct);
    paramMap.map.set("rotationDeg", ReverbParamIDs::rotationDeg);
    paramMap.map.set("wetMix01", ReverbParamIDs::wetMix01);
    paramMap.map.set("bloomPct", ReverbParamIDs::bloomPct);
    paramMap.map.set("distancePct", ReverbParamIDs::distancePct);
    paramMap.map.set("freeze", ReverbParamIDs::freeze);
    paramMap.map.set("shimmerAmtPct", ReverbParamIDs::shimmerAmtPct);
    paramMap.map.set("shimmerInt", ReverbParamIDs::shimmerInt);
    paramMap.map.set("gateAmtPct", ReverbParamIDs::gateAmtPct);
    paramMap.map.set("outTrimDb", ReverbParamIDs::outTrimDb);
    
    // Ducking parameters
    paramMap.map.set("duckOn", ReverbParamIDs::duckOn);
    paramMap.map.set("duckMode", ReverbParamIDs::duckMode);
    paramMap.map.set("duckDepthDb", ReverbParamIDs::duckDepthDb);
    paramMap.map.set("duckThrDb", ReverbParamIDs::duckThrDb);
    paramMap.map.set("duckKneeDb", ReverbParamIDs::duckKneeDb);
    paramMap.map.set("duckRatio", ReverbParamIDs::duckRatio);
    paramMap.map.set("duckAtkMs", ReverbParamIDs::duckAtkMs);
    paramMap.map.set("duckRelMs", ReverbParamIDs::duckRelMs);
    paramMap.map.set("duckBandHz", ReverbParamIDs::duckBandHz);
    paramMap.map.set("duckBandQ", ReverbParamIDs::duckBandQ);
    paramMap.map.set("duckDetector", ReverbParamIDs::duckDetectorSrc);
    
    // Decay Rate EQ parameters
    paramMap.map.set("dreqXoverLoHz", ReverbParamIDs::dreqXoverLoHz);
    paramMap.map.set("dreqXoverHiHz", ReverbParamIDs::dreqXoverHiHz);
    paramMap.map.set("dreqApply", ReverbParamIDs::dreqApply);
    paramMap.map.set("followWidth", ReverbParamIDs::followWidth);
    paramMap.map.set("followWidthAmt", ReverbParamIDs::followWidthAmt);
    paramMap.map.set("followRot", ReverbParamIDs::followRot);
    paramMap.map.set("followRotAmt", ReverbParamIDs::followRotAmt);
}

juce::NamedValueSet ReverbParamMap::jsonToAPVTS(const juce::var& jsonParams) const
{
    juce::NamedValueSet apvtsParams;
    
    if (jsonParams.isObject())
    {
        for (const auto& kv : jsonParams.getDynamicObject()->getProperties())
        {
            const juce::String jsonKey = kv.name.toString();
            const juce::var jsonValue = kv.value;
            
            // Find the corresponding APVTS parameter ID
            if (paramMap.map.contains(jsonKey))
            {
                const juce::String apvtsKey = paramMap.map[jsonKey];
                apvtsParams.set(apvtsKey, jsonValue);
            }
        }
    }
    
    return apvtsParams;
}

juce::var ReverbParamMap::apvtsToJson(const juce::NamedValueSet& apvtsParams) const
{
    juce::var jsonParams = juce::var(new juce::DynamicObject());
    
    for (const auto& kv : apvtsParams)
    {
        const juce::String apvtsKey = kv.name.toString();
        const juce::var apvtsValue = kv.value;
        
        // Find the corresponding JSON key
        for (auto it = paramMap.map.begin(); it != paramMap.map.end(); ++it)
        {
            if (it.getValue() == apvtsKey)
            {
                jsonParams.getDynamicObject()->setProperty(it.getKey(), apvtsValue);
                break;
            }
        }
    }
    
    return jsonParams;
}
