// FieldLookAndFeel.h
// -----------------------------------------------------------------------------
// DEV NOTES
// - Header aligned with refactored FieldLookAndFeel.cpp
// - Added `using Theme = FieldTheme;` so cpp can refer to FieldLNF::Theme.
// - Public API unchanged in spirit; drawLabel kept inline as before.
// - Colours/themes preserved; setGreenMode() still flips full palette.
// -----------------------------------------------------------------------------

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

// Theme palette (default "Ocean" accent). Other accents mentioned in comments.
struct FieldTheme
{
    // Lighter, more modern colours with better contrast
    juce::Colour base       { 0xFF3C3F45 }; // lighter base background
    juce::Colour panel      { 0xFF454951 }; // lighter panel background
    juce::Colour hl         { 0xFF5A5E66 }; // brighter highlight
    juce::Colour sh         { 0xFF2A2C30 }; // deeper shadow for contrast
    juce::Colour text       { 0xFFF0F2F5 }; // brighter text
    juce::Colour textMuted  { 0xFFB8BDC7 }; // lighter muted text

    // Enhanced shadows for depth
    juce::Colour shadowDark  { 0xFF1A1C20 }; // deep shadow
    juce::Colour shadowLight { 0xFF60646C }; // light shadow/highlight

    // Nature accent default: Ocean (#5AA9E6). Other options:
    // Moss #7FB069, Sand #C6AD8F, Copper #C5865C, Plum #8B6FA1
    juce::Colour accent     { 0xFF5AA9E6 };
    // Secondary accent for borders/accents needing neutral tone (dark charcoal)
    juce::Colour accentSecondary { 0xFF202226 };

    // Motion (purple-metal) system colours (used by Motion Engine UI)
    juce::Colour motionPanelTop { 0xFF7B81C1 }; // bluish-purple top
    juce::Colour motionPanelBot { 0xFF555A99 }; // deeper bluish-purple bottom
    juce::Colour motionBorder   { 0xFF4A4A8E }; // purple border

    // Upgraded Metallic System - Ocean-harmonized materials
    struct MetalStops { 
        juce::Colour top, bottom; 
        juce::Colour tint; 
        float tintAlpha; 
    };
    
    struct MetalTheme {
        MetalStops neutral  { juce::Colour (0xFF9CA4AD), juce::Colour (0xFF6E747C), juce::Colour (0x003D7BB8), 0.06f };
        MetalStops reverb   { juce::Colour (0xFFB87749), juce::Colour (0xFF7D4D2E), juce::Colour (0x00F2C39A), 0.10f }; // tint used as sheen in band
        MetalStops delay    { juce::Colour (0xFFC9CFB9), juce::Colour (0xFF8D927F), juce::Colour (0x004AA3FF), 0.05f };
        MetalStops motion   { juce::Colour (0xFF6D76B2), juce::Colour (0xFF434A86), juce::Colour (0x00C2D8FF), 0.06f };
        MetalStops band     { juce::Colour (0xFF6AA0D8), juce::Colour (0xFF3A6EA8), juce::Colour (0x000A0C0F), 0.12f }; // bottom multiply
        MetalStops phase    { juce::Colour (0xFF3E6BA3), juce::Colour (0xFF24466E), juce::Colour (0xFFC2D8FF), 0.06f }; // Deep Cobalt Interference
        MetalStops titanium { juce::Colour (0xFF7D858F), juce::Colour (0xFF3B4149), juce::Colour (0x003D7BB8), 0.08f };
    } metal;

    // EQ palette (defaults set in FieldLNF::setGreenMode(false))
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
        
        // Meter backgrounds and tracks
        juce::Colour trackBase    { 0xFF4A4E54 }; // Rich dark grey for track background
        juce::Colour trackActive  { 0xFF6A6E74 }; // Lighter grey for active track
        juce::Colour trackBorder  { 0xFF3A3E44 }; // Darker border for track definition
        
        // Meter panel colors (richer greys with more contrast)
        juce::Colour panelLight   { 0xFF5A5E66 }; // Light grey panel
        juce::Colour panelMedium  { 0xFF454951 }; // Medium grey panel  
        juce::Colour panelDark    { 0xFF2A2C30 }; // Dark grey panel
        juce::Colour panelBorder  { 0xFF1A1C20 }; // Darkest border
    } meters;
};

class FieldLNF : public juce::LookAndFeel_V4
{
public:
    // Expose alias so cpp helpers can use FieldLNF::Theme
    using Theme = FieldTheme;

    enum class ThemeVariant { Ocean, Green, Pink, Yellow, Grey };
    static juce::String getThemeName (ThemeVariant v)
    {
        switch (v)
        {
            case ThemeVariant::Ocean:  return "Ocean";   // accent ~ Havelock Blue
            case ThemeVariant::Green:  return "Green";   // accent ~ Apple Green
            case ThemeVariant::Pink:   return "Pink";    // accent = Material Pink 500
            case ThemeVariant::Yellow: return "Amber";   // accent = Material Amber 500
            case ThemeVariant::Grey:   return "Slate";   // monochrome grey
            default: return "Unknown";
        }
    }

    explicit FieldLNF (FieldTheme t = {}) : theme (t)
    {
        setDefaultSansSerifTypefaceName ("Inter");
        // Initialize full default (colorful) palette including EQ colors
        setGreenMode (false);
    }

    // Apply theme colours to JUCE components
    void setupColours()
    {
        setColour (juce::ResizableWindow::backgroundColourId, theme.base);
        setColour (juce::Label::textColourId,                 theme.text);
        setColour (juce::Slider::textBoxTextColourId,         theme.text);
        setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
        setColour (juce::PopupMenu::backgroundColourId,       theme.panel);
        setColour (juce::PopupMenu::textColourId,             theme.text);
    }

    // New: set named theme variant (Ocean, Green, Pink, Yellow, Grey)
    void setTheme (ThemeVariant variant)
    {
        switch (variant)
        {
            case ThemeVariant::Green:
            {
                // Green monochromatic palette
                theme.base        = juce::Colour (0xFF0D1F0D);
                theme.panel       = juce::Colour (0xFF1E2F1E);
                theme.text        = juce::Colour (0xFFE8F4E8);
                theme.textMuted   = juce::Colour (0xFFB0C5B0);
                theme.accent      = juce::Colour (0xFF5AA95A);
                theme.hl          = juce::Colour (0xFF2E4F2E);
                theme.sh          = juce::Colour (0xFF0D1E0D);
                theme.shadowDark  = juce::Colour (0xFF0D1E0D);
                theme.shadowLight = juce::Colour (0xFF4E6F4E);

                theme.accentSecondary = juce::Colour (0xFF202226);
                theme.eq.hp        = juce::Colour (0xFF6FBF73);
                theme.eq.lp        = juce::Colour (0xFF66BB6A);
                theme.eq.air       = juce::Colour (0xFFA5D6A7);
                theme.eq.tilt      = juce::Colour (0xFF81C784);
                theme.eq.bass      = juce::Colour (0xFF43A047);
                theme.eq.scoop     = juce::Colour (0xFF98EE99);
                theme.eq.monoShade = juce::Colour (0xFF0D1E0D).withAlpha (0.18f);
                // Motion colours tuned for Green mode (keep purple bias slightly cooler)
                theme.motionPanelTop = juce::Colour (0xFF6F76B5);
                theme.motionPanelBot = juce::Colour (0xFF4B5096);
                theme.motionBorder   = juce::Colour (0xFF45458A);
                break;
            }
            case ThemeVariant::Pink:
            {
                // Keep neutral surfaces, swap accent to pink
                theme.base        = juce::Colour (0xFF3C3F45);
                theme.panel       = juce::Colour (0xFF454951);
                theme.text        = juce::Colour (0xFFF0F2F5);
                theme.textMuted   = juce::Colour (0xFFB8BDC7);
                theme.accent      = juce::Colour (0xFFE91E63); // Pink
                theme.hl          = juce::Colour (0xFF5A5E66);
                theme.sh          = juce::Colour (0xFF2A2C30);
                theme.shadowDark  = juce::Colour (0xFF1A1C20);
                theme.shadowLight = juce::Colour (0xFF60646C);

                // Pink-centric EQ palette
                theme.accentSecondary = juce::Colour (0xFF202226);
                theme.eq.hp        = juce::Colour (0xFFF06292); // light rose
                theme.eq.lp        = juce::Colour (0xFFC2185B); // deep pink
                theme.eq.air       = juce::Colour (0xFFFFC1E3); // light pink
                theme.eq.tilt      = juce::Colour (0xFFFF8A80); // soft coral
                theme.eq.bass      = juce::Colour (0xFFEC407A); // vivid pink
                theme.eq.scoop     = juce::Colour (0xFFBA68C8); // magenta/plum
                theme.eq.monoShade = juce::Colour (0xFF2A2C30).withAlpha (0.15f);
                // Motion colours for Pink mode (cooler violet to balance pink)
                theme.motionPanelTop = juce::Colour (0xFF877FC8);
                theme.motionPanelBot = juce::Colour (0xFF5E58A8);
                theme.motionBorder   = juce::Colour (0xFF504A9A);
                break;
            }
            case ThemeVariant::Yellow:
            {
                theme.base        = juce::Colour (0xFF3C3F45);
                theme.panel       = juce::Colour (0xFF454951);
                theme.text        = juce::Colour (0xFFF0F2F5);
                theme.textMuted   = juce::Colour (0xFFB8BDC7);
                theme.accent      = juce::Colour (0xFFFFC107); // Amber/Yellow
                theme.hl          = juce::Colour (0xFF5A5E66);
                theme.sh          = juce::Colour (0xFF2A2C30);
                theme.shadowDark  = juce::Colour (0xFF1A1C20);
                theme.shadowLight = juce::Colour (0xFF60646C);

                theme.accentSecondary = juce::Colour (0xFF202226);
                // Amber-centric EQ palette
                theme.eq.hp        = juce::Colour (0xFFFFD54F); // lighter amber
                theme.eq.lp        = juce::Colour (0xFFFFB300); // deeper amber
                theme.eq.air       = juce::Colour (0xFFFFF59D); // pale yellow
                theme.eq.tilt      = juce::Colour (0xFFFFCA28); // amber 400
                theme.eq.bass      = juce::Colour (0xFFFFA000); // amber 700
                theme.eq.scoop     = juce::Colour (0xFFFFB74D); // orange/amber
                theme.eq.monoShade = juce::Colour (0xFF2A2C30).withAlpha (0.15f);
                // Motion colours for Yellow mode (a bit more saturated purple for contrast)
                theme.motionPanelTop = juce::Colour (0xFF7E74C9);
                theme.motionPanelBot = juce::Colour (0xFF5A50A9);
                theme.motionBorder   = juce::Colour (0xFF4F479B);
                break;
            }
            case ThemeVariant::Grey:
            {
                // Monochromatic grey
                theme.base        = juce::Colour (0xFF2E3034);
                theme.panel       = juce::Colour (0xFF3A3D43);
                theme.text        = juce::Colour (0xFFE6E8EB);
                theme.textMuted   = juce::Colour (0xFFB3B8BF);
                theme.accent      = juce::Colour (0xFF9EA3AA);
                theme.hl          = juce::Colour (0xFF5A5D63);
                theme.sh          = juce::Colour (0xFF202226);
                theme.shadowDark  = juce::Colour (0xFF141518);
                theme.shadowLight = juce::Colour (0xFF5F646B);

                theme.accentSecondary = juce::Colour (0xFF202226);
                // EQ greys
                theme.eq.hp        = juce::Colour (0xFFB0B5BC);
                theme.eq.lp        = juce::Colour (0xFFA5ABB3);
                theme.eq.air       = juce::Colour (0xFFE6E8EB);
                theme.eq.tilt      = juce::Colour (0xFFD0D4D9);
                theme.eq.bass      = juce::Colour (0xFF9EA3AA);
                theme.eq.scoop     = juce::Colour (0xFFC7CCD3);
                theme.eq.monoShade = juce::Colour (0xFF202226).withAlpha (0.16f);
                // Motion colours for Grey mode (muted violet)
                theme.motionPanelTop = juce::Colour (0xFF8086B0);
                theme.motionPanelBot = juce::Colour (0xFF5D628A);
                theme.motionBorder   = juce::Colour (0xFF52577F);
                break;
            }
            case ThemeVariant::Ocean:
            default:
            {
                // Standard blue palette
                theme.base        = juce::Colour (0xFF3C3F45);
                theme.panel       = juce::Colour (0xFF454951);
                theme.text        = juce::Colour (0xFFF0F2F5);
                theme.textMuted   = juce::Colour (0xFFB8BDC7);
                theme.accent      = juce::Colour (0xFF3D7BB8); // Darker Ocean blue
                theme.hl          = juce::Colour (0xFF5A5E66);
                theme.sh          = juce::Colour (0xFF2A2C30);
                theme.shadowDark  = juce::Colour (0xFF1A1C20);
                theme.shadowLight = juce::Colour (0xFF60646C);

                theme.accentSecondary = juce::Colour (0xFF202226);
                theme.eq.hp        = juce::Colour (0xFF2B7BC7); // HP: darker blue
                theme.eq.lp        = juce::Colour (0xFF1A5F9E); // LP: darker blue
                theme.eq.air       = juce::Colour (0xFFFFF59D); // Air: soft yellow
                theme.eq.tilt      = juce::Colour (0xFFFFA726); // Tilt: orange
                theme.eq.bass      = juce::Colour (0xFF66BB6A); // Bass: green
                theme.eq.scoop     = juce::Colour (0xFFAB47BC); // Scoop: plum/purple
                theme.eq.monoShade = juce::Colour (0xFF2A2C30).withAlpha (0.15f);
                // Motion colours for Ocean mode (default bluish-purple)
                theme.motionPanelTop = juce::Colour (0xFF7B81C1);
                theme.motionPanelBot = juce::Colour (0xFF555A99);
                theme.motionBorder   = juce::Colour (0xFF4A4A8E);
                break;
            }
        }
        currentVariant = variant;
        setupColours();
    }

    // Toggle green monochrome palette. Re-applies setupColours().
    void setGreenMode (bool enabled)
    {
        setTheme (enabled ? ThemeVariant::Green : ThemeVariant::Ocean);
    }

    // --- Custom primitives ----------------------------------------------------
    void drawNeoPanel (juce::Graphics& g, juce::Rectangle<float> r, float radius = 16.0f) const;
    // Rotation pad renderer: energy circle + rotated M'/S' basis + orthonormal S-curve
    void drawRotationPad (juce::Graphics& g, juce::Rectangle<float> bounds,
                          float rotationDeg, float asymmetry,
                          juce::Colour accent, juce::Colour text, juce::Colour panel) const;
    // ComboBox overrides to allow icon-only dropdown (chevron) rendering
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;

    // PopupMenu overrides so ComboBox menus match option/phase menu styling
    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override;
    void drawPopupMenuSeparator  (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area,
                                     const juce::String& sectionName) override;
    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                             bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                             bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                             const juce::Drawable* icon, const juce::Colour* textColour) override;
    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override;

    // Configure per-item tints for PopupMenu items (applies to next/active menus using this LNF)
    void setPopupItemTints (const juce::Array<juce::Colour>& tints)
    {
        popupItemTints = tints;
    }

    // Shared painter for cell panels (KnobCell, SwitchCell) to ensure identical look
    void paintCellPanel (juce::Graphics& g, juce::Component& c, bool showBorder, bool hover) const
    {
        auto r = c.getLocalBounds().toFloat();
        const float rad = 8.0f;

        g.setColour (theme.panel);
        g.fillRoundedRectangle (r.reduced (3.0f), rad);

        juce::DropShadow ds1 (theme.shadowDark.withAlpha (0.35f), 12, { -1, -1 });
        juce::DropShadow ds2 (theme.shadowLight.withAlpha (0.25f),  6, { -1, -1 });
        ds1.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());
        ds2.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());

        g.setColour (theme.sh.withAlpha (0.18f));
        g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);

        if (showBorder)
        {
            auto border = r.reduced (2.0f);
            g.setColour (theme.accentSecondary);
            if (hover)
            {
                for (int i = 1; i <= 6; ++i)
                {
                    const float t = (float) i / 6.0f;
                    const float expand = 2.0f + t * 8.0f;
                    g.setColour (theme.accentSecondary.withAlpha ((1.0f - t) * 0.22f));
                    g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.35f, 2.0f);
                }
            }
            g.setColour (theme.accentSecondary);
            g.drawRoundedRectangle (border, rad, 1.5f);
        }
    }

    // Label styling (inline to keep behaviour identical to your original header)
    void drawLabel (juce::Graphics& g, juce::Label& l) override
    {
        g.fillAll (juce::Colours::transparentBlack);
        g.setColour (theme.textMuted);
        g.setFont (getLabelFont (l).boldened());
        g.drawFittedText (l.getText(), l.getLocalBounds(), juce::Justification::centred, 1);
    }

    // --- Slider overrides -----------------------------------------------------
    void drawRotarySlider (juce::Graphics&, int, int, int, int,
                           float, float, float, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int, int, int, int,
                           float, float, float,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    int  getSliderThumbRadius (juce::Slider&) override;

    // Toggle button (used for qLinkButton) with accent/grey states and optional inverted logic
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool isMouseOverButton, bool isButtonDown) override;
    // TextButton / generic button background (e.g., Learn/Stop)
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;

    // Gain-specific rotary with delta overlay (called by your custom GainSlider)
    void drawGainSlider (juce::Graphics& g, int x, int y, int w, int h,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, float gainDb);

    // Centered knob label helper (kept public for reuse)
    void drawKnobLabel (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text);

    // Top tabs pill background (active/inactive) to keep styling centralized
    void drawTabPill (juce::Graphics& g, juce::Rectangle<float> r, bool active) const;

    // Upgraded Metallic Rendering System
    static void paintMetal (juce::Graphics& g, const juce::Rectangle<float>& r,
                           const FieldTheme::MetalStops& m, float corner = 8.0f);

    // Phase-Specific Metallic System (Deep Cobalt Interference)
    struct PhaseMetal {
        juce::Colour top, bottom;
        juce::Colour airyTint;   float airyAlpha;   // overlay
        juce::Colour bottomMul;  float bottomMulAlpha;
        float sheenAlpha;        // 0.10f default
    };
    
    static void paintPhaseMetal (juce::Graphics& g, const juce::Rectangle<float>& r,
                                const PhaseMetal& m, float corner = 10.0f, float dpi = 1.0f);

    // Active theme (mutable for runtime palette switching)
    FieldTheme theme;
    ThemeVariant currentVariant { ThemeVariant::Ocean };

private:
    // Popup menu per-item tinting support (applies to ComboBox popups using this LNF)
    juce::Array<juce::Colour> popupItemTints;
    mutable int popupPaintIndex { 0 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FieldLNF)
};
