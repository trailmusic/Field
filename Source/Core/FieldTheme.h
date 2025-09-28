#pragma once
#include <JuceHeader.h>

// Field Theme System - Centralized theme management
// Separated from FieldLookAndFeel for better organization

// Core theme palette with simplified structure
struct FieldTheme
{
    // Base colors
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
    
    // Animation system colors and effects
    struct AnimationTheme {
        // Bypass button blink animation
        juce::Colour bypassBlinkDark    { 0xFF8A8F96 }; // Darker phase of blink
        juce::Colour bypassBlinkBright  { 0xFFB8BDC7 }; // Brighter phase of blink
        float blinkAlphaDark            { 0.35f };      // Alpha for dark phase
        float blinkAlphaBright          { 0.18f };      // Alpha for bright phase
        int blinkIntervalMs             { 250 };        // Blink interval in milliseconds
        
        // Glow effects
        juce::Colour glowColor          { 0xFF5AA9E6 }; // Glow color (matches accent)
        float glowIntensity             { 0.8f };       // Glow intensity (0.0-1.0)
        float glowRadius                { 8.0f };       // Glow radius in pixels
        
        // Performance settings
        int animationFps                { 20 };         // Animation refresh rate
        bool enableAnimations           { true };       // Master animation toggle
    } animation;

    // Motion (purple-metal) system colours (used by Motion Engine UI)
    juce::Colour motionPanelTop { 0xFF7B81C1 }; // bluish-purple top
    juce::Colour motionPanelBot { 0xFF555A99 }; // deeper bluish-purple bottom
    juce::Colour motionBorder   { 0xFF4A4A8E }; // purple border

    // Metallic system colors
    struct MetalStops { 
        juce::Colour top, bottom; 
        juce::Colour tint; 
        float tintAlpha; 
    };
    
    struct MetalTheme {
        MetalStops neutral  { juce::Colour (0xFF9CA4AD), juce::Colour (0xFF6E747C), juce::Colour (0x003D7BB8), 0.06f };
        MetalStops reverb   { juce::Colour (0xFFB87749), juce::Colour (0xFF7D4D2E), juce::Colour (0x00F2C39A), 0.10f };
        MetalStops delay    { juce::Colour (0xFFC9CFB9), juce::Colour (0xFF8D927F), juce::Colour (0x004AA3FF), 0.05f };
        MetalStops motion   { juce::Colour (0xFF6D76B2), juce::Colour (0xFF434A86), juce::Colour (0x00C2D8FF), 0.06f };
        MetalStops band     { juce::Colour (0xFF6AA0D8), juce::Colour (0xFF3A6EA8), juce::Colour (0x000A0C0F), 0.12f };
        MetalStops phase    { juce::Colour (0xFF3E6BA3), juce::Colour (0xFF24466E), juce::Colour (0xFFC2D8FF), 0.06f };
        MetalStops xy       { juce::Colour (0xFF4B5560), juce::Colour (0xFF2E333A), juce::Colour (0xFF3D7BB8), 0.05f };
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

    // Meter colors - rich greys with high contrast
    struct MeterColors {
        // Correlation meter colors
        juce::Colour positive { 0xFF66BB6A }; // Rich green for positive correlation
        juce::Colour negative { 0xFFE57373 }; // Rich red for negative correlation

        // Level meter colors (warning/error states)
        juce::Colour safe     { 0xFF5AA9E6 }; // Ocean blue for safe levels
        juce::Colour warning  { 0xFFFFC107 }; // Amber for warning levels
        juce::Colour error    { 0xFFE53935 }; // Red for error/risk levels

        // Meter backgrounds and tracks (darker greys)
        juce::Colour trackBase    { 0xFF3A3E44 }; // Darker grey for track background
        juce::Colour trackActive  { 0xFF4A4E54 }; // Medium grey for active track
        juce::Colour trackBorder  { 0xFF2A2C30 }; // Darker border for track definition

        // Meter panel colors (darker greys with more contrast)
        juce::Colour panelLight   { 0xFF454951 }; // Darker light grey panel
        juce::Colour panelMedium  { 0xFF3A3E44 }; // Darker medium grey panel
        juce::Colour panelDark    { 0xFF2A2C30 }; // Dark grey panel
        juce::Colour panelBorder  { 0xFF1A1C20 }; // Darkest border
    } meters;
};

// Theme variants - all 5 original themes preserved
enum class ThemeVariant { Ocean, Green, Pink, Yellow, Grey };

// Theme manager class
class ThemeManager
{
public:
    static juce::String getThemeName(ThemeVariant variant)
    {
        switch (variant)
        {
            case ThemeVariant::Ocean: return "Ocean";
            case ThemeVariant::Green: return "Green";
            case ThemeVariant::Pink: return "Pink";
            case ThemeVariant::Yellow: return "Amber";
            case ThemeVariant::Grey: return "Grey";
            default: return "Unknown";
        }
    }

    static void applyTheme(FieldTheme& theme, ThemeVariant variant)
    {
        switch (variant)
        {
            case ThemeVariant::Green:
                applyGreenTheme(theme);
                break;
            case ThemeVariant::Pink:
                applyPinkTheme(theme);
                break;
            case ThemeVariant::Yellow:
                applyYellowTheme(theme);
                break;
            case ThemeVariant::Grey:
                applyGreyTheme(theme);
                break;
            case ThemeVariant::Ocean:
            default:
                applyOceanTheme(theme);
                break;
        }
    }

private:
    static void applyOceanTheme(FieldTheme& theme)
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

    static void applyGreenTheme(FieldTheme& theme)
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

    static void applyPinkTheme(FieldTheme& theme)
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

    static void applyYellowTheme(FieldTheme& theme)
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

    static void applyGreyTheme(FieldTheme& theme)
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
