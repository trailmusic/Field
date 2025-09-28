#pragma once
#include <JuceHeader.h>
#include "FieldTheme.h"

// Field Metallic System - Centralized metallic rendering
// Separated from FieldLookAndFeel for better organization

// Metallic system enums and helpers
enum class MetallicKind : int { 
    None=0, Neutral, Reverb, Delay, Band, Phase, Motion, XY 
};

// Helper function to detect metallic properties from component properties
inline MetallicKind metallicFromProps(const juce::NamedValueSet& props) 
{
    if ((bool)props.getWithDefault("phaseMetallic", false))  return MetallicKind::Phase;
    if ((bool)props.getWithDefault("reverbMetallic", false)) return MetallicKind::Reverb;
    if ((bool)props.getWithDefault("delayMetallic", false))  return MetallicKind::Delay;
    if ((bool)props.getWithDefault("bandMetallic", false))   return MetallicKind::Band;
    if ((bool)props.getWithDefault("motionMetallic", false)) return MetallicKind::Motion;
    if ((bool)props.getWithDefault("xyMetallic", false))     return MetallicKind::XY;
    if ((bool)props.getWithDefault("metallic", false))      return MetallicKind::Neutral;
    return MetallicKind::None;
}

// High-level area-based metallic system
inline void setAreaMetallic(juce::Component& component, MetallicKind areaKind) 
{
    auto& props = component.getProperties();
    
    // Always set base metallic property
    props.set("metallic", true);
    
    // Set specific area metallic property
    switch (areaKind) {
        case MetallicKind::Reverb:  props.set("reverbMetallic", true); break;
        case MetallicKind::Delay:   props.set("delayMetallic", true); break;
        case MetallicKind::Band:    props.set("bandMetallic", true); break;
        case MetallicKind::Phase:   props.set("phaseMetallic", true); break;
        case MetallicKind::Motion:  props.set("motionMetallic", true); break;
        case MetallicKind::XY:      props.set("xyMetallic", true); break;
        case MetallicKind::Neutral: /* only metallic=true */ break;
        case MetallicKind::None:    props.set("metallic", false); break;
    }
}

// Unified metallic helper for any component type
template<typename ComponentType>
inline void setAreaMetallicForCell(ComponentType& cell, MetallicKind areaKind) 
{
    setAreaMetallic(cell, areaKind);
}

// Metallic rendering system
class MetallicRenderer
{
public:
    // Generic metallic rendering
    static void paintMetal(juce::Graphics& g, const juce::Rectangle<float>& r,
                          const FieldTheme::MetalStops& metal, float corner = 8.0f);

    // Phase-specific metallic rendering (Deep Cobalt Interference)
    struct PhaseMetal {
        juce::Colour top, bottom;
        juce::Colour airyTint;   float airyAlpha;   // overlay
        juce::Colour bottomMul;  float bottomMulAlpha;
        float sheenAlpha;        // 0.10f default
    };
    
    static void paintPhaseMetal(juce::Graphics& g, const juce::Rectangle<float>& r,
                               const PhaseMetal& metal, float corner = 10.0f, float dpi = 1.0f);

    // Get metallic colors for a specific kind
    static FieldTheme::MetalStops getMetallicColors(const FieldTheme& theme, MetallicKind kind)
    {
        switch (kind) {
            case MetallicKind::Neutral: return theme.metal.neutral;
            case MetallicKind::Reverb:  return theme.metal.reverb;
            case MetallicKind::Delay:  return theme.metal.delay;
            case MetallicKind::Band:   return theme.metal.band;
            case MetallicKind::Phase:  return theme.metal.phase;
            case MetallicKind::Motion: return theme.metal.motion;
            case MetallicKind::XY:     return theme.metal.xy;
            default: return theme.metal.neutral;
        }
    }
};
