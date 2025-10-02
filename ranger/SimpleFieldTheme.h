#pragma once

#include <JuceHeader.h>

// Simplified Field Theme System for Field Ranger
// Based on Field's theme system but simplified for standalone use

struct SimpleFieldTheme
{
    // Base colors (Ocean theme by default)
    juce::Colour base       { 0xFF3C3F45 };
    juce::Colour panel      { 0xFF454951 };
    juce::Colour hl         { 0xFF5A5E66 };
    juce::Colour sh         { 0xFF2A2C30 };
    juce::Colour text       { 0xFFF0F2F5 };
    juce::Colour textMuted  { 0xFFB8BDC7 };
    
    // Enhanced shadows for depth
    juce::Colour shadowDark  { 0xFF1A1C20 };
    juce::Colour shadowLight { 0xFF60646C };
    
    // Primary accent (Ocean blue by default)
    juce::Colour accent     { 0xFF5AA9E6 };
    juce::Colour accentSecondary { 0xFF202226 };
    
    // Animation system colors
    struct AnimationTheme {
        juce::Colour bypassBlinkDark    { 0xFF8A8F96 };
        juce::Colour bypassBlinkBright  { 0xFFB8BDC7 };
        float blinkAlphaDark            { 0.35f };
        float blinkAlphaBright          { 0.18f };
        int blinkIntervalMs             { 250 };
        
        juce::Colour glowColor          { 0xFF5AA9E6 };
        float glowIntensity             { 0.8f };
        float glowRadius                { 8.0f };
        
        int animationFps                { 20 };
        bool enableAnimations           { true };
    } animation;
    
    // Metallic system colors
    struct MetalStops { 
        juce::Colour top, bottom; 
        juce::Colour tint; 
        float tintAlpha; 
    };
    
    struct MetalTheme {
        MetalStops neutral  { juce::Colour (0xFF9CA4AD), juce::Colour (0xFF6E747C), juce::Colour (0x00000000), 0.06f };
        MetalStops reverb   { juce::Colour (0xFFB87749), juce::Colour (0xFF7D4D2E), juce::Colour (0x00F2C39A), 0.10f };
        MetalStops delay    { juce::Colour (0xFFC9CFB9), juce::Colour (0xFF8D927F), juce::Colour (0x00000000), 0.05f };
        MetalStops motion   { juce::Colour (0xFF6D76B2), juce::Colour (0xFF434A86), juce::Colour (0x00000000), 0.06f };
        MetalStops band     { juce::Colour (0xFF6AA0D8), juce::Colour (0xFF3A6EA8), juce::Colour (0x00000000), 0.12f };
        MetalStops phase    { juce::Colour (0xFF3E6BA3), juce::Colour (0xFF24466E), juce::Colour (0x00000000), 0.06f };
        MetalStops xy       { juce::Colour (0xFF4B5560), juce::Colour (0xFF2E333A), juce::Colour (0x00000000), 0.05f };
    } metal;
    
    // EQ palette
    struct EqPalette {
        juce::Colour hp;
        juce::Colour lp;
        juce::Colour air;
        juce::Colour tilt;
        juce::Colour bass;
        juce::Colour scoop;
        juce::Colour monoShade;
    } eq;
    
    // Meter colors
    struct MeterColors {
        juce::Colour positive { 0xFF66BB6A };
        juce::Colour negative { 0xFFE57373 };
        juce::Colour safe     { 0xFF5AA9E6 };
        juce::Colour warning  { 0xFFFFC107 };
        juce::Colour error    { 0xFFE53935 };
        juce::Colour trackBase    { 0xFF3A3E44 };
        juce::Colour trackActive  { 0xFF4A4E54 };
        juce::Colour trackBorder  { 0xFF2A2C30 };
        juce::Colour panelLight   { 0xFF454951 };
        juce::Colour panelMedium  { 0xFF3A3E44 };
        juce::Colour panelDark    { 0xFF2A2C30 };
        juce::Colour panelBorder  { 0xFF1A1C20 };
    } meters;
    
    // Update metallic colors to be theme-aware
    void updateMetallicColors()
    {
        // Neutral metallic - based on panel colors
        metal.neutral.top = panel.brighter(0.15f);
        metal.neutral.bottom = panel.darker(0.15f);
        metal.neutral.tint = accent.withAlpha(0.0f);
        
        // Reverb metallic - warm orange tones
        metal.reverb.top = juce::Colour(0xFFB87749);
        metal.reverb.bottom = juce::Colour(0xFF7D4D2E);
        metal.reverb.tint = accent.withAlpha(0.1f);
        
        // Delay metallic - cool green tones  
        metal.delay.top = juce::Colour(0xFFC9CFB9);
        metal.delay.bottom = juce::Colour(0xFF8D927F);
        metal.delay.tint = accent.withAlpha(0.05f);
        
        // Motion metallic - purple tones
        metal.motion.top = juce::Colour(0xFF6D76B2);
        metal.motion.bottom = juce::Colour(0xFF434A86);
        metal.motion.tint = accent.withAlpha(0.06f);
        
        // Band metallic - accent-based blue tones
        metal.band.top = accent.brighter(0.3f);
        metal.band.bottom = accent.darker(0.2f);
        metal.band.tint = accent.withAlpha(0.12f);
        
        // Phase metallic - deeper accent tones
        metal.phase.top = accent.darker(0.1f);
        metal.phase.bottom = accent.darker(0.3f);
        metal.phase.tint = accent.withAlpha(0.06f);
        
        // XY metallic - neutral grey tones
        metal.xy.top = panel.brighter(0.05f);
        metal.xy.bottom = panel.darker(0.1f);
        metal.xy.tint = accent.withAlpha(0.05f);
    }
};

// Theme variants
enum class SimpleThemeVariant { Ocean, Green, Pink, Yellow, Grey };

// Simple theme manager
class SimpleThemeManager
{
public:
    static juce::String getThemeName(SimpleThemeVariant variant)
    {
        switch (variant)
        {
            case SimpleThemeVariant::Ocean: return "Ocean";
            case SimpleThemeVariant::Green: return "Green";
            case SimpleThemeVariant::Pink: return "Pink";
            case SimpleThemeVariant::Yellow: return "Amber";
            case SimpleThemeVariant::Grey: return "Grey";
            default: return "Unknown";
        }
    }

    static void applyTheme(SimpleFieldTheme& theme, SimpleThemeVariant variant)
    {
        switch (variant)
        {
            case SimpleThemeVariant::Green:
                applyGreenTheme(theme);
                break;
            case SimpleThemeVariant::Pink:
                applyPinkTheme(theme);
                break;
            case SimpleThemeVariant::Yellow:
                applyYellowTheme(theme);
                break;
            case SimpleThemeVariant::Grey:
                applyGreyTheme(theme);
                break;
            case SimpleThemeVariant::Ocean:
            default:
                applyOceanTheme(theme);
                break;
        }
        
        // Update metallic colors to be theme-aware
        theme.updateMetallicColors();
    }

private:
    static void applyOceanTheme(SimpleFieldTheme& theme)
    {
        // Default Ocean theme (already set in struct defaults)
        theme.accent = juce::Colour(0xFF3D7BB8);
        theme.eq.hp = juce::Colour(0xFF2B7BC7);
        theme.eq.lp = juce::Colour(0xFF1A5F9E);
        theme.eq.air = juce::Colour(0xFFFFF59D);
        theme.eq.tilt = juce::Colour(0xFFFFA726);
        theme.eq.bass = juce::Colour(0xFF66BB6A);
        theme.eq.scoop = juce::Colour(0xFFAB47BC);
        theme.eq.monoShade = juce::Colour(0xFF2A2C30).withAlpha(0.15f);
    }

    static void applyGreenTheme(SimpleFieldTheme& theme)
    {
        theme.base = juce::Colour(0xFF0D1F0D);
        theme.panel = juce::Colour(0xFF1E2F1E);
        theme.text = juce::Colour(0xFFE8F4E8);
        theme.textMuted = juce::Colour(0xFFB0C5B0);
        theme.accent = juce::Colour(0xFF5AA95A);
        theme.hl = juce::Colour(0xFF2E4F2E);
        theme.sh = juce::Colour(0xFF0D1E0D);
        theme.shadowDark = juce::Colour(0xFF0D1E0D);
        theme.shadowLight = juce::Colour(0xFF4E6F4E);
        theme.accentSecondary = juce::Colour(0xFF202226);
        
        // Green EQ palette
        theme.eq.hp = juce::Colour(0xFF6FBF73);
        theme.eq.lp = juce::Colour(0xFF66BB6A);
        theme.eq.air = juce::Colour(0xFFA5D6A7);
        theme.eq.tilt = juce::Colour(0xFF81C784);
        theme.eq.bass = juce::Colour(0xFF43A047);
        theme.eq.scoop = juce::Colour(0xFF98EE99);
        theme.eq.monoShade = juce::Colour(0xFF0D1E0D).withAlpha(0.18f);
    }

    static void applyPinkTheme(SimpleFieldTheme& theme)
    {
        // Keep neutral surfaces, swap accent to pink
        theme.base = juce::Colour(0xFF3C3F45);
        theme.panel = juce::Colour(0xFF454951);
        theme.text = juce::Colour(0xFFF0F2F5);
        theme.textMuted = juce::Colour(0xFFB8BDC7);
        theme.accent = juce::Colour(0xFFE91E63); // Pink
        theme.hl = juce::Colour(0xFF5A5E66);
        theme.sh = juce::Colour(0xFF2A2C30);
        theme.shadowDark = juce::Colour(0xFF1A1C20);
        theme.shadowLight = juce::Colour(0xFF60646C);
        theme.accentSecondary = juce::Colour(0xFF202226);
        
        // Pink-centric EQ palette
        theme.eq.hp = juce::Colour(0xFFF06292); // light rose
        theme.eq.lp = juce::Colour(0xFFC2185B); // deep pink
        theme.eq.air = juce::Colour(0xFFFFC1E3); // light pink
        theme.eq.tilt = juce::Colour(0xFFFF8A80); // soft coral
        theme.eq.bass = juce::Colour(0xFFEC407A); // vivid pink
        theme.eq.scoop = juce::Colour(0xFFBA68C8); // magenta/plum
        theme.eq.monoShade = juce::Colour(0xFF2A2C30).withAlpha(0.15f);
    }

    static void applyYellowTheme(SimpleFieldTheme& theme)
    {
        theme.base = juce::Colour(0xFF3C3F45);
        theme.panel = juce::Colour(0xFF454951);
        theme.text = juce::Colour(0xFFF0F2F5);
        theme.textMuted = juce::Colour(0xFFB8BDC7);
        theme.accent = juce::Colour(0xFFFFC107); // Amber/Yellow
        theme.hl = juce::Colour(0xFF5A5E66);
        theme.sh = juce::Colour(0xFF2A2C30);
        theme.shadowDark = juce::Colour(0xFF1A1C20);
        theme.shadowLight = juce::Colour(0xFF60646C);
        theme.accentSecondary = juce::Colour(0xFF202226);
        
        // Amber-centric EQ palette
        theme.eq.hp = juce::Colour(0xFFFFD54F); // lighter amber
        theme.eq.lp = juce::Colour(0xFFFFB300); // deeper amber
        theme.eq.air = juce::Colour(0xFFFFF59D); // pale yellow
        theme.eq.tilt = juce::Colour(0xFFFFCA28); // amber 400
        theme.eq.bass = juce::Colour(0xFFFFA000); // amber 700
        theme.eq.scoop = juce::Colour(0xFFFFB74D); // orange/amber
        theme.eq.monoShade = juce::Colour(0xFF2A2C30).withAlpha(0.15f);
    }

    static void applyGreyTheme(SimpleFieldTheme& theme)
    {
        theme.base = juce::Colour(0xFF2E3034);
        theme.panel = juce::Colour(0xFF3A3D43);
        theme.text = juce::Colour(0xFFE6E8EB);
        theme.textMuted = juce::Colour(0xFFB3B8BF);
        theme.accent = juce::Colour(0xFF9EA3AA);
        theme.hl = juce::Colour(0xFF5A5D63);
        theme.sh = juce::Colour(0xFF202226);
        theme.shadowDark = juce::Colour(0xFF141518);
        theme.shadowLight = juce::Colour(0xFF5F646B);
        theme.accentSecondary = juce::Colour(0xFF202226);
        
        // Grey EQ palette
        theme.eq.hp = juce::Colour(0xFFB0B5BC);
        theme.eq.lp = juce::Colour(0xFFA5ABB3);
        theme.eq.air = juce::Colour(0xFFE6E8EB);
        theme.eq.tilt = juce::Colour(0xFFD0D4D9);
        theme.eq.bass = juce::Colour(0xFF9EA3AA);
        theme.eq.scoop = juce::Colour(0xFFC7CCD3);
        theme.eq.monoShade = juce::Colour(0xFF202226).withAlpha(0.16f);
    }
};

