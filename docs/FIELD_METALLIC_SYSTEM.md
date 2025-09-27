# Field Metallic System Documentation

Last updated: 2025-01-27
Scope: Ocean-harmonized metallic rendering system for Field audio plugin UI components.

---

## Overview

The Field metallic system provides sophisticated, Ocean-brand-harmonized material rendering for UI components. All metallic surfaces use the centralized `FieldLNF::paintMetal()` function with proper caching and performance optimization.

---

## Core System

### Metallic Theme Structure

```cpp
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
    MetalStops phase    { juce::Colour (0xFF5B93CF), juce::Colour (0xFF355F97), juce::Colour (0x000A0C0F), 0.12f };
    MetalStops titanium { juce::Colour (0xFF7D858F), juce::Colour (0xFF3B4149), juce::Colour (0x003D7BB8), 0.08f };
} metal;
```

### Rendering Function

```cpp
static void paintMetal (juce::Graphics& g, const juce::Rectangle<float>& r,
                       const FieldTheme::MetalStops& m, float corner = 8.0f);
```

---

## Material Variants

### 1. Neutral Steel (Default)
- **Usage**: Global default, frames, non-module metal
- **Colors**: `#9CA4AD → #6E747C`
- **Tint**: Ocean primary `#3D7BB8` @ 6%
- **Properties**: `metallic` + `theme.metal.neutral`

### 2. Reverb Copper/Burnished
- **Usage**: Reverb controls, large knobs, mode switches
- **Colors**: `#B87749 → #7D4D2E`
- **Tint**: Hot spot `#F2C39A` @ 10% in sheen band
- **Properties**: `reverbMetallic` + `theme.metal.reverb`

### 3. Delay Champagne Nickel
- **Usage**: Delay panel body, time display bezel, feedback meter bed
- **Colors**: `#C9CFB9 → #8D927F`
- **Tint**: Ocean highlight `#4AA3FF` @ 5%
- **Properties**: `delayMetallic` + `theme.metal.delay`

### 4. Motion Indigo Anodized
- **Usage**: Motion panel ring, animation rails, depth bezel
- **Colors**: `#6D76B2 → #434A86`
- **Tint**: Airy lift `#C2D8FF` @ 6%
- **Properties**: `motionPurpleBorder` + `theme.metal.motion`

### 5. Band/Phase Ocean Anodized
- **Usage**: EQ container, phase widgets, band controls
- **Colors**: `#6AA0D8 → #3A6EA8` (Band), `#5B93CF → #355F97` (Phase)
- **Tint**: Depth multiply `#0A0C0F` @ 12% on bottom 25%
- **Properties**: `bandMetallic`/`phaseMetallic` + `theme.metal.band`/`theme.metal.phase`

### 6. Dark Titanium (Pro/Advanced)
- **Usage**: Pro pages, advanced panes, focused states
- **Colors**: `#7D858F → #3B4149`
- **Tint**: Ocean primary `#3D7BB8` @ 8%
- **Properties**: `theme.metal.titanium`

---

## Rendering Features

### Base Gradient
- Sophisticated top-to-bottom lighting with proper material-specific colors
- Consistent gradient direction across all materials

### Sheen Band
- 10% white highlight in upper third (y = 0.28 × height)
- Height: 10-24px with 6-10px feather
- Creates realistic specular reflection

### Brush Lines
- Vertical micro-lines for authenticity
- Irregular brightness (4-5% alpha, varying every 12 lines)
- 1-2px spacing with 0.5-1.0px thickness variation

### Grain Texture
- Fine monochrome noise for realism
- 4-5% black alpha overlay
- Consistent across all materials

### Vignette Effects
- Edge darkening for depth perception
- 12-16% black alpha on edges
- Radius follows corner radius

---

## Performance Optimization

### Caching Strategy
- All metallic textures cached per size/scale
- Reuse instead of regenerating per paint
- Minimal repaint regions

### Rendering Order
1. Base gradient
2. Optional tint overlay
3. Sheen band
4. Brush lines
5. Grain texture
6. Vignette effects
7. Borders

### Memory Management
- Pre-allocated geometry for hot paths
- No per-frame allocations in paint
- Efficient clip region management

---

## Usage Guidelines

### Component Integration
```cpp
// In KnobCell or other components
if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
{
    if (reverbMetal)
        FieldLNF::paintMetal(g, rr, lf->theme.metal.reverb, rad);
    else if (delayMetal)
        FieldLNF::paintMetal(g, rr, lf->theme.metal.delay, rad);
    // ... other material variants
}
```

### Property Flags
- `metallic`: Neutral steel (default)
- `reverbMetallic`: Copper/burnished
- `delayMetallic`: Champagne nickel
- `bandMetallic`: Ocean anodized
- `phaseMetallic`: Ocean anodized (darker)
- `motionPurpleBorder`: Indigo anodized

### Performance Best Practices
- Use `FieldLNF::paintMetal()` for all metallic rendering
- Cache textures per size/scale
- Minimize repaint regions
- Set `setOpaque(true)` for fully painted components

---

## Compliance

### FIELD_UI_RULES Compliance
- ✅ Centralized paint logic in LNF helpers
- ✅ Proper texture caching
- ✅ Minimal repaint regions
- ✅ Theme-derived colors only
- ✅ Performance-optimized rendering

### ui_performance_audit.md Compliance
- ✅ No heavy per-pixel/random work
- ✅ Textures cached and reused
- ✅ Ocean-harmonized metallic rendering
- ✅ Minimal repaint regions
- ✅ Proper memory management

---

## Migration Notes

### From Legacy System
- Replace old metallic color definitions with `theme.metal.*` variants
- Use `FieldLNF::paintMetal()` instead of custom metallic rendering
- Update property flags to match new system
- Ensure proper caching and performance optimization

### Backward Compatibility
- Legacy property flags still recognized
- Graceful fallback to neutral steel
- No breaking changes to existing components

---

## Future Enhancements

### Planned Features
- Dynamic material switching based on context
- Advanced lighting models
- Custom material definitions
- Performance profiling integration

### Monitoring
- Track rendering performance across materials
- Monitor memory usage for texture caching
- Validate visual consistency across modules

---

## Notes
- Keep this document updated when material variants change
- Document any performance optimizations or new features
- Maintain compliance with FIELD_UI_RULES and ui_performance_audit.md
