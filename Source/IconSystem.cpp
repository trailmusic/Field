#include "IconSystem.h"

juce::Path IconSystem::createIcon(IconType type, float size)
{
    switch (type)
    {
        case Lock: return createLockIcon(size);
        case Unlock: return createUnlockIcon(size);
        case CogWheel: return createCogWheelIcon(size);
        case Power: return createPowerIcon(size);
        case Speaker: return createSpeakerIcon(size);
        case Pan: return createPanIcon(size);
        case Space: return createSpaceIcon(size);
        case Width: return createWidthIcon(size);
        case Tilt: return createTiltIcon(size);
        case Mono: return createMonoIcon(size);
        case HP: return createHPIcon(size);
        case LP: return createLPIcon(size);
        case Drive: return createDriveIcon(size);
        case Mix: return createMixIcon(size);
        case Air: return createAirIcon(size);
        case Duck: return createDuckIcon(size);
        case Link: return createLinkIcon(size);
        case Snap: return createSnapIcon(size);
        case Options: return createOptionsIcon(size);
        case Bypass: return createBypassIcon(size);
        case Stereo: return createStereoIcon(size);
        case Split: return createSplitIcon(size);
        case Save: return createSaveIcon(size);
        case LeftArrow: return createLeftArrowIcon(size);
        case RightArrow: return createRightArrowIcon(size);
        case FullScreen: return createFullScreenIcon(size);
        case ExitFullScreen: return createExitFullScreenIcon(size);
        case ColorPalette: return createColorPaletteIcon(size);
        default: return juce::Path();
    }
}

void IconSystem::drawIcon(juce::Graphics& g, IconType type, juce::Rectangle<float> bounds, juce::Colour colour)
{
    auto iconPath = createIcon(type, juce::jmin(bounds.getWidth(), bounds.getHeight()));
    g.setColour(colour);
    g.fillPath(iconPath, juce::AffineTransform::translation(bounds.getX(), bounds.getY()));
}

juce::Path IconSystem::createLockIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Lock body
    path.addRectangle(4 * scale, 8 * scale, 8 * scale, 6 * scale);
    
    // Lock shackle
    path.addRectangle(6 * scale, 6 * scale, 4 * scale, 3 * scale);
    path.addRectangle(5 * scale, 7 * scale, 1 * scale, 2 * scale);
    path.addRectangle(10 * scale, 7 * scale, 1 * scale, 2 * scale);
    
    // Keyhole
    path.addEllipse(7 * scale, 10 * scale, 2 * scale, 2 * scale);
    path.addRectangle(7.5f * scale, 12 * scale, 1 * scale, 2 * scale);
    
    return path;
}

juce::Path IconSystem::createUnlockIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Lock body
    path.addRectangle(4 * scale, 8 * scale, 8 * scale, 6 * scale);
    
    // Lock shackle (open)
    path.addRectangle(6 * scale, 6 * scale, 4 * scale, 2 * scale);
    path.addRectangle(5 * scale, 7 * scale, 1 * scale, 1 * scale);
    path.addRectangle(10 * scale, 7 * scale, 1 * scale, 1 * scale);
    
    // Keyhole
    path.addEllipse(7 * scale, 10 * scale, 2 * scale, 2 * scale);
    path.addRectangle(7.5f * scale, 12 * scale, 1 * scale, 2 * scale);
    
    return path;
}

juce::Path IconSystem::createCogWheelIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    float centerX = 8 * scale;
    float centerY = 8 * scale;
    float radius = 6 * scale;
    
    // Main circle
    path.addEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
    
    // Gear teeth
    for (int i = 0; i < 8; ++i)
    {
        float angle = i * juce::MathConstants<float>::pi / 4.0f;
        float x = centerX + (radius + 1.5f * scale) * std::cos(angle);
        float y = centerY + (radius + 1.5f * scale) * std::sin(angle);
        path.addEllipse(x - 1.5f * scale, y - 1.5f * scale, 3 * scale, 3 * scale);
    }
    
    // Center hole
    path.addEllipse(centerX - 2 * scale, centerY - 2 * scale, 4 * scale, 4 * scale);
    
    return path;
}

juce::Path IconSystem::createPowerIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    float centerX = 8 * scale;
    float centerY = 8 * scale;
    float radius = 6 * scale;
    
    // Power symbol circle
    path.addEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
    
    // Power symbol (vertical line with circle)
    path.addRectangle(centerX - 0.5f * scale, centerY - 4 * scale, 1 * scale, 3 * scale);
    path.addEllipse(centerX - 2 * scale, centerY - 1 * scale, 4 * scale, 4 * scale);
    
    return path;
}

juce::Path IconSystem::createSpeakerIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Speaker body
    path.addEllipse(2 * scale, 4 * scale, 8 * scale, 8 * scale);
    
    // Speaker cone
    path.addRectangle(10 * scale, 6 * scale, 4 * scale, 4 * scale);
    
    return path;
}

juce::Path IconSystem::createPanIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Left speaker
    path.addEllipse(1 * scale, 4 * scale, 6 * scale, 8 * scale);
    
    // Right speaker
    path.addEllipse(9 * scale, 4 * scale, 6 * scale, 8 * scale);
    
    return path;
}

juce::Path IconSystem::createSpaceIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Depth representation (concentric circles)
    path.addEllipse(3 * scale, 3 * scale, 10 * scale, 10 * scale);
    path.addEllipse(5 * scale, 5 * scale, 6 * scale, 6 * scale);
    path.addEllipse(7 * scale, 7 * scale, 2 * scale, 2 * scale);
    
    return path;
}

juce::Path IconSystem::createWidthIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Left arrow
    path.addTriangle(2 * scale, 8 * scale, 6 * scale, 4 * scale, 6 * scale, 12 * scale);
    
    // Right arrow
    path.addTriangle(14 * scale, 8 * scale, 10 * scale, 4 * scale, 10 * scale, 12 * scale);
    
    return path;
}

juce::Path IconSystem::createTiltIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Tilted line representing EQ tilt
    path.addLineSegment(juce::Line<float>(2 * scale, 12 * scale, 14 * scale, 4 * scale), 2 * scale);
    
    return path;
}

juce::Path IconSystem::createMonoIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Single speaker representing mono
    path.addEllipse(4 * scale, 4 * scale, 8 * scale, 8 * scale);
    
    return path;
}

juce::Path IconSystem::createHPIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // High-pass filter symbol (cutoff line)
    path.addRectangle(2 * scale, 8 * scale, 12 * scale, 2 * scale);
    path.addTriangle(2 * scale, 8 * scale, 8 * scale, 2 * scale, 14 * scale, 8 * scale);
    
    return path;
}

juce::Path IconSystem::createLPIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Low-pass filter symbol (cutoff line)
    path.addRectangle(2 * scale, 6 * scale, 12 * scale, 2 * scale);
    path.addTriangle(2 * scale, 14 * scale, 8 * scale, 8 * scale, 14 * scale, 14 * scale);
    
    return path;
}

juce::Path IconSystem::createDriveIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Distortion/saturation symbol
    path.addRectangle(2 * scale, 6 * scale, 12 * scale, 4 * scale);
    path.addLineSegment(juce::Line<float>(2 * scale, 8 * scale, 14 * scale, 8 * scale), 1 * scale);
    path.addLineSegment(juce::Line<float>(4 * scale, 6 * scale, 4 * scale, 10 * scale), 1 * scale);
    path.addLineSegment(juce::Line<float>(12 * scale, 6 * scale, 12 * scale, 10 * scale), 1 * scale);
    
    return path;
}

juce::Path IconSystem::createMixIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Mix symbol (two circles overlapping)
    path.addEllipse(3 * scale, 4 * scale, 6 * scale, 8 * scale);
    path.addEllipse(7 * scale, 4 * scale, 6 * scale, 8 * scale);
    
    return path;
}

juce::Path IconSystem::createAirIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Air/high frequency symbol (sparkles)
    path.addEllipse(4 * scale, 4 * scale, 2 * scale, 2 * scale);
    path.addEllipse(10 * scale, 6 * scale, 1.5f * scale, 1.5f * scale);
    path.addEllipse(6 * scale, 10 * scale, 1 * scale, 1 * scale);
    path.addEllipse(12 * scale, 10 * scale, 1.5f * scale, 1.5f * scale);
    
    return path;
}

juce::Path IconSystem::createDuckIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Ducking symbol (sidechain compression)
    path.addRectangle(2 * scale, 6 * scale, 12 * scale, 4 * scale);
    path.addTriangle(8 * scale, 2 * scale, 6 * scale, 6 * scale, 10 * scale, 6 * scale);
    
    return path;
}

juce::Path IconSystem::createLinkIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Link symbol (chain links)
    path.addEllipse(3 * scale, 6 * scale, 4 * scale, 4 * scale);
    path.addEllipse(9 * scale, 6 * scale, 4 * scale, 4 * scale);
    path.addRectangle(7 * scale, 7 * scale, 2 * scale, 2 * scale);
    
    return path;
}

juce::Path IconSystem::createSnapIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    // Simple grid + magnet symbol for snap
    // Grid: two vertical and two horizontal lines
    path.addRectangle(3 * scale, 4 * scale, 10 * scale, 1 * scale);
    path.addRectangle(3 * scale, 8 * scale, 10 * scale, 1 * scale);
    path.addRectangle(3 * scale, 12 * scale, 10 * scale, 1 * scale);
    path.addRectangle(6 * scale, 3 * scale, 1 * scale, 10 * scale);
    path.addRectangle(10 * scale, 3 * scale, 1 * scale, 10 * scale);
    // Magnet: U-shape in top-left
    juce::Path magnet;
    magnet.addRectangle(2 * scale, 2 * scale, 2 * scale, 4 * scale);
    magnet.addRectangle(4 * scale, 2 * scale, 2 * scale, 4 * scale);
    magnet.addRectangle(2 * scale, 5 * scale, 4 * scale, 1 * scale);
    path.addPath(magnet);
    return path;
}

juce::Path IconSystem::createOptionsIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Three dots representing options menu
    path.addEllipse(4 * scale, 7 * scale, 2 * scale, 2 * scale);
    path.addEllipse(7 * scale, 7 * scale, 2 * scale, 2 * scale);
    path.addEllipse(10 * scale, 7 * scale, 2 * scale, 2 * scale);
    
    return path;
}

juce::Path IconSystem::createBypassIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Bypass symbol (straight line through)
    path.addRectangle(2 * scale, 7 * scale, 12 * scale, 2 * scale);
    
    return path;
}

juce::Path IconSystem::createStereoIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Stereo symbol (two speakers)
    path.addEllipse(2 * scale, 4 * scale, 5 * scale, 8 * scale);
    path.addEllipse(9 * scale, 4 * scale, 5 * scale, 8 * scale);
    
    return path;
}

juce::Path IconSystem::createSplitIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Split symbol (diverging lines)
    path.addLineSegment(juce::Line<float>(8 * scale, 2 * scale, 4 * scale, 8 * scale), 2 * scale);
    path.addLineSegment(juce::Line<float>(8 * scale, 2 * scale, 12 * scale, 8 * scale), 2 * scale);
    path.addEllipse(3 * scale, 10 * scale, 3 * scale, 3 * scale);
    path.addEllipse(10 * scale, 10 * scale, 3 * scale, 3 * scale);
    
    return path;
}

juce::Path IconSystem::createSaveIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Floppy disk save icon
    path.addRectangle(2 * scale, 4 * scale, 12 * scale, 10 * scale);
    path.addRectangle(4 * scale, 2 * scale, 8 * scale, 2 * scale);
    path.addRectangle(6 * scale, 6 * scale, 4 * scale, 4 * scale);
    
    return path;
} 

juce::Path IconSystem::createLeftArrowIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Better left arrow (pointing left) - more modern design
    // Arrow shaft
    path.startNewSubPath(12.0f * scale, 8.0f * scale);
    path.lineTo(6.0f * scale, 8.0f * scale);
    
    // Arrow head
    path.startNewSubPath(6.0f * scale, 8.0f * scale);
    path.lineTo(4.0f * scale, 6.0f * scale);
    path.lineTo(4.0f * scale, 10.0f * scale);
    path.closeSubPath();
    
    return path;
}

juce::Path IconSystem::createRightArrowIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Better right arrow (pointing right) - more modern design
    // Arrow shaft
    path.startNewSubPath(4.0f * scale, 8.0f * scale);
    path.lineTo(10.0f * scale, 8.0f * scale);
    
    // Arrow head
    path.startNewSubPath(10.0f * scale, 8.0f * scale);
    path.lineTo(12.0f * scale, 6.0f * scale);
    path.lineTo(12.0f * scale, 10.0f * scale);
    path.closeSubPath();
    
    return path;
}

juce::Path IconSystem::createFullScreenIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Full screen icon (expand arrows pointing outward)
    // Top left arrow
    path.startNewSubPath(4.0f * scale, 4.0f * scale);
    path.lineTo(2.0f * scale, 4.0f * scale);
    path.lineTo(2.0f * scale, 2.0f * scale);
    path.startNewSubPath(2.0f * scale, 4.0f * scale);
    path.lineTo(4.0f * scale, 2.0f * scale);
    
    // Top right arrow
    path.startNewSubPath(12.0f * scale, 4.0f * scale);
    path.lineTo(14.0f * scale, 4.0f * scale);
    path.lineTo(14.0f * scale, 2.0f * scale);
    path.startNewSubPath(14.0f * scale, 4.0f * scale);
    path.lineTo(12.0f * scale, 2.0f * scale);
    
    // Bottom left arrow
    path.startNewSubPath(4.0f * scale, 12.0f * scale);
    path.lineTo(2.0f * scale, 12.0f * scale);
    path.lineTo(2.0f * scale, 14.0f * scale);
    path.startNewSubPath(2.0f * scale, 12.0f * scale);
    path.lineTo(4.0f * scale, 14.0f * scale);
    
    // Bottom right arrow
    path.startNewSubPath(12.0f * scale, 12.0f * scale);
    path.lineTo(14.0f * scale, 12.0f * scale);
    path.lineTo(14.0f * scale, 14.0f * scale);
    path.startNewSubPath(14.0f * scale, 12.0f * scale);
    path.lineTo(12.0f * scale, 14.0f * scale);
    
    return path;
}

juce::Path IconSystem::createExitFullScreenIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Exit full screen icon (contract arrows pointing inward)
    // Top left arrow
    path.startNewSubPath(2.0f * scale, 2.0f * scale);
    path.lineTo(4.0f * scale, 2.0f * scale);
    path.lineTo(4.0f * scale, 4.0f * scale);
    path.startNewSubPath(4.0f * scale, 2.0f * scale);
    path.lineTo(2.0f * scale, 4.0f * scale);
    
    // Top right arrow
    path.startNewSubPath(14.0f * scale, 2.0f * scale);
    path.lineTo(12.0f * scale, 2.0f * scale);
    path.lineTo(12.0f * scale, 4.0f * scale);
    path.startNewSubPath(12.0f * scale, 2.0f * scale);
    path.lineTo(14.0f * scale, 4.0f * scale);
    
    // Bottom left arrow
    path.startNewSubPath(2.0f * scale, 14.0f * scale);
    path.lineTo(4.0f * scale, 14.0f * scale);
    path.lineTo(4.0f * scale, 12.0f * scale);
    path.startNewSubPath(4.0f * scale, 14.0f * scale);
    path.lineTo(2.0f * scale, 12.0f * scale);
    
    // Bottom right arrow
    path.startNewSubPath(14.0f * scale, 14.0f * scale);
    path.lineTo(12.0f * scale, 14.0f * scale);
    path.lineTo(12.0f * scale, 12.0f * scale);
    path.startNewSubPath(12.0f * scale, 14.0f * scale);
    path.lineTo(14.0f * scale, 12.0f * scale);
    
    return path;
}

juce::Path IconSystem::createColorPaletteIcon(float size)
{
    juce::Path path;
    float scale = size / 16.0f;
    
    // Color palette icon - a rounded rectangle with color circles
    // Main palette body
    path.addRoundedRectangle(2.0f * scale, 4.0f * scale, 12.0f * scale, 8.0f * scale, 1.0f * scale);
    
    // Color circles (representing paint colors)
    // Red circle
    path.addEllipse(4.0f * scale, 6.0f * scale, 2.0f * scale, 2.0f * scale);
    
    // Green circle
    path.addEllipse(7.0f * scale, 6.0f * scale, 2.0f * scale, 2.0f * scale);
    
    // Blue circle
    path.addEllipse(10.0f * scale, 6.0f * scale, 2.0f * scale, 2.0f * scale);
    
    // Yellow circle
    path.addEllipse(4.0f * scale, 9.0f * scale, 2.0f * scale, 2.0f * scale);
    
    // Purple circle
    path.addEllipse(7.0f * scale, 9.0f * scale, 2.0f * scale, 2.0f * scale);
    
    // Orange circle
    path.addEllipse(10.0f * scale, 9.0f * scale, 2.0f * scale, 2.0f * scale);
    
    return path;
} 