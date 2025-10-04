#include "ReverbParamMap.h"
#include "core/params/ParamIDs.h"

ReverbParamMap::ReverbParamMap()
{
    initializeParamMap();
}

void ReverbParamMap::initializeParamMap()
{
    // Core reverb parameters (string IDs)
    paramMap.map.set("decaySec", "decaySec");
    paramMap.map.set("preDelayMs", "preDelayMs");
    paramMap.map.set("erLevelDb", "erLevelDb");
    paramMap.map.set("erTimeMs", "erTimeMs");
    paramMap.map.set("erDensityPct", "erDensityPct");
    paramMap.map.set("erWidthPct", "erWidthPct");
    paramMap.map.set("erToTailPct", "erToTailPct");
    paramMap.map.set("diffusionPct", "diffusionPct");
    paramMap.map.set("densityPct", "densityPct");
    paramMap.map.set("modDepthCents", "modDepthCents");
    paramMap.map.set("modRateHz", "modRateHz");
    paramMap.map.set("widthPct", "widthPct");
    paramMap.map.set("rotationDeg", "rotationDeg");
    paramMap.map.set("wetMix01", "wetMix01");
    paramMap.map.set("bloomPct", "bloomPct");
    paramMap.map.set("distancePct", "distancePct");
    paramMap.map.set("freeze", "freeze");
    paramMap.map.set("shimmerAmtPct", "shimmerAmtPct");
    paramMap.map.set("shimmerInt", "shimmerInt");
    paramMap.map.set("gateAmtPct", "gateAmtPct");
    paramMap.map.set("outTrimDb", "outTrimDb");
    
    // Ducking parameters
    paramMap.map.set("duckOn", "duckOn");
    paramMap.map.set("duckMode", "duckMode");
    paramMap.map.set("duckDepthDb", "duckDepthDb");
    paramMap.map.set("duckThrDb", "duckThrDb");
    paramMap.map.set("duckKneeDb", "duckKneeDb");
    paramMap.map.set("duckRatio", "duckRatio");
    paramMap.map.set("duckAtkMs", "duckAtkMs");
    paramMap.map.set("duckRelMs", "duckRelMs");
    paramMap.map.set("duckBandHz", "duckBandHz");
    paramMap.map.set("duckBandQ", "duckBandQ");
    paramMap.map.set("duckDetector", "duckDetectorSrc");
    
    // Decay Rate EQ parameters
    paramMap.map.set("dreqXoverLoHz", "dreqXoverLoHz");
    paramMap.map.set("dreqXoverHiHz", "dreqXoverHiHz");
    paramMap.map.set("dreqApply", "dreqApply");
    paramMap.map.set("followWidth", "followWidth");
    paramMap.map.set("followWidthAmt", "followWidthAmt");
    paramMap.map.set("followRot", "followRot");
    paramMap.map.set("followRotAmt", "followRotAmt");
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
