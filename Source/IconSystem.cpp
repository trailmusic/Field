#include "IconSystem.h"

juce::Path IconSystem::createIcon (IconType type, float size)
{
    switch (type)
    {
        case Lock:           return createLockIcon (size);
        case Unlock:         return createUnlockIcon (size);
        case CogWheel:       return createCogWheelIcon (size);
        case Power:          return createPowerIcon (size);
        case Speaker:        return createSpeakerIcon (size);
        case Pan:            return createPanIcon (size);
        case Space:          return createSpaceIcon (size);
        case Width:          return createWidthIcon (size);
        case Tilt:           return createTiltIcon (size);
        case Mono:           return createMonoIcon (size);
        case HP:             return createHPIcon (size);
        case LP:             return createLPIcon (size);
        case Drive:          return createDriveIcon (size);
        case Mix:            return createMixIcon (size);
        case Air:            return createAirIcon (size);
        case Duck:           return createDuckIcon (size);
        case Link:           return createLinkIcon (size);
        case Snap:           return createSnapIcon (size);
        case Options:        return createOptionsIcon (size);
        case Bypass:         return createBypassIcon (size);
        case Stereo:         return createStereoIcon (size);
        case Split:          return createSplitIcon (size);
        case Save:           return createSaveIcon (size);
        case LeftArrow:      return createLeftArrowIcon (size);
        case RightArrow:     return createRightArrowIcon (size);
        case FullScreen:     return createFullScreenIcon (size);
        case ExitFullScreen: return createExitFullScreenIcon (size);
        case ColorPalette:   return createColorPaletteIcon (size);
        default:             return {};
    }
}

void IconSystem::drawIcon (juce::Graphics& g, IconType type,
                           juce::Rectangle<float> bounds, juce::Colour colour)
{
    const auto s = juce::jmin (bounds.getWidth(), bounds.getHeight());
    auto iconPath = createIcon (type, s);
    g.setColour (colour);
    g.fillPath (iconPath,
                juce::AffineTransform::translation (bounds.getX(), bounds.getY()));
}

// --- individual icons ---

juce::Path IconSystem::createLockIcon (float size)
{
    juce::Path path;
    const float scale = size / 16.0f;

    path.addRectangle (4 * scale, 8 * scale, 8 * scale, 6 * scale); // body
    path.addRectangle (6 * scale, 6 * scale, 4 * scale, 3 * scale); // shackle
    path.addRectangle (5 * scale, 7 * scale, 1 * scale, 2 * scale);
    path.addRectangle (10 * scale, 7 * scale, 1 * scale, 2 * scale);
    path.addEllipse   (7 * scale, 10 * scale, 2 * scale, 2 * scale); // keyhole
    path.addRectangle (7.5f * scale, 12 * scale, 1 * scale, 2 * scale);
    return path;
}

juce::Path IconSystem::createUnlockIcon (float size)
{
    juce::Path path;
    const float scale = size / 16.0f;

    path.addRectangle (4 * scale, 8 * scale, 8 * scale, 6 * scale);
    path.addRectangle (6 * scale, 6 * scale, 4 * scale, 2 * scale);
    path.addRectangle (5 * scale, 7 * scale, 1 * scale, 1 * scale);
    path.addRectangle (10 * scale, 7 * scale, 1 * scale, 1 * scale);
    path.addEllipse   (7 * scale, 10 * scale, 2 * scale, 2 * scale);
    path.addRectangle (7.5f * scale, 12 * scale, 1 * scale, 2 * scale);
    return path;
}

juce::Path IconSystem::createCogWheelIcon (float size)
{
    juce::Path path;
    const float scale   = size / 16.0f;
    const float cx      = 8 * scale;
    const float cy      = 8 * scale;
    const float radius  = 6 * scale;

    path.addEllipse (cx - radius, cy - radius, radius * 2, radius * 2);

    for (int i = 0; i < 8; ++i)
    {
        const float angle = i * juce::MathConstants<float>::pi / 4.0f;
        const float x     = cx + (radius + 1.5f * scale) * std::cos (angle);
        const float y     = cy + (radius + 1.5f * scale) * std::sin (angle);
        path.addEllipse (x - 1.5f * scale, y - 1.5f * scale, 3 * scale, 3 * scale);
    }

    path.addEllipse (cx - 2 * scale, cy - 2 * scale, 4 * scale, 4 * scale);
    return path;
}

juce::Path IconSystem::createPowerIcon (float size)
{
    juce::Path path;
    const float scale  = size / 16.0f;
    const float cx     = 8 * scale;
    const float cy     = 8 * scale;
    const float radius = 6 * scale;

    path.addEllipse (cx - radius, cy - radius, radius * 2, radius * 2);
    path.addRectangle (cx - 0.5f * scale, cy - 4 * scale, 1 * scale, 3 * scale);
    path.addEllipse   (cx - 2 * scale, cy - 1 * scale, 4 * scale, 4 * scale);
    return path;
}

juce::Path IconSystem::createSpeakerIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse (2 * s, 4 * s, 8 * s, 8 * s);
    path.addRectangle (10 * s, 6 * s, 4 * s, 4 * s);
    return path;
}

juce::Path IconSystem::createPanIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse (1 * s, 4 * s, 6 * s, 8 * s);
    path.addEllipse (9 * s, 4 * s, 6 * s, 8 * s);
    return path;
}

juce::Path IconSystem::createSpaceIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse (3 * s, 3 * s, 10 * s, 10 * s);
    path.addEllipse (5 * s, 5 * s,  6 * s,  6 * s);
    path.addEllipse (7 * s, 7 * s,  2 * s,  2 * s);
    return path;
}

juce::Path IconSystem::createWidthIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addTriangle (2 * s, 8 * s, 6 * s, 4 * s, 6 * s, 12 * s);
    path.addTriangle (14 * s, 8 * s, 10 * s, 4 * s, 10 * s, 12 * s);
    return path;
}

juce::Path IconSystem::createTiltIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addLineSegment (juce::Line<float> (2 * s, 12 * s, 14 * s, 4 * s), 2 * s);
    return path;
}

juce::Path IconSystem::createMonoIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse (4 * s, 4 * s, 8 * s, 8 * s);
    return path;
}

juce::Path IconSystem::createHPIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addRectangle (2 * s, 8 * s, 12 * s, 2 * s);
    path.addTriangle  (2 * s, 8 * s, 8 * s, 2 * s, 14 * s, 8 * s);
    return path;
}

juce::Path IconSystem::createLPIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addRectangle (2 * s, 6 * s, 12 * s, 2 * s);
    path.addTriangle  (2 * s, 14 * s, 8 * s, 8 * s, 14 * s, 14 * s);
    return path;
}

juce::Path IconSystem::createDriveIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addRectangle (2 * s, 6 * s, 12 * s, 4 * s);
    path.addLineSegment (juce::Line<float> (2 * s, 8 * s, 14 * s, 8 * s), 1 * s);
    path.addLineSegment (juce::Line<float> (4 * s, 6 * s, 4 * s, 10 * s), 1 * s);
    path.addLineSegment (juce::Line<float> (12 * s, 6 * s, 12 * s, 10 * s), 1 * s);
    return path;
}

juce::Path IconSystem::createMixIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse (3 * s, 4 * s, 6 * s, 8 * s);
    path.addEllipse (7 * s, 4 * s, 6 * s, 8 * s);
    return path;
}

juce::Path IconSystem::createAirIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse ( 4 * s,  4 * s, 2 * s,   2 * s);
    path.addEllipse (10 * s,  6 * s, 1.5f * s, 1.5f * s);
    path.addEllipse ( 6 * s, 10 * s, 1 * s,   1 * s);
    path.addEllipse (12 * s, 10 * s, 1.5f * s, 1.5f * s);
    return path;
}

juce::Path IconSystem::createDuckIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addRectangle (2 * s, 6 * s, 12 * s, 4 * s);
    path.addTriangle  (8 * s, 2 * s, 6 * s, 6 * s, 10 * s, 6 * s);
    return path;
}

juce::Path IconSystem::createLinkIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse  (3 * s, 6 * s, 4 * s, 4 * s);
    path.addEllipse  (9 * s, 6 * s, 4 * s, 4 * s);
    path.addRectangle(7 * s, 7 * s, 2 * s, 2 * s);
    return path;
}

juce::Path IconSystem::createSnapIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;

    path.addRectangle (3 * s,  4 * s, 10 * s, 1 * s);
    path.addRectangle (3 * s,  8 * s, 10 * s, 1 * s);
    path.addRectangle (3 * s, 12 * s, 10 * s, 1 * s);
    path.addRectangle (6 * s,  3 * s, 1 * s,  10 * s);
    path.addRectangle (10 * s, 3 * s, 1 * s,  10 * s);

    juce::Path magnet;
    magnet.addRectangle (2 * s, 2 * s, 2 * s, 4 * s);
    magnet.addRectangle (4 * s, 2 * s, 2 * s, 4 * s);
    magnet.addRectangle (2 * s, 5 * s, 4 * s, 1 * s);
    path.addPath (magnet);

    return path;
}

juce::Path IconSystem::createOptionsIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse (4 * s, 7 * s, 2 * s, 2 * s);
    path.addEllipse (7 * s, 7 * s, 2 * s, 2 * s);
    path.addEllipse (10 * s, 7 * s, 2 * s, 2 * s);
    return path;
}

juce::Path IconSystem::createBypassIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addRectangle (2 * s, 7 * s, 12 * s, 2 * s);
    return path;
}

juce::Path IconSystem::createStereoIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addEllipse (2 * s, 4 * s, 5 * s, 8 * s);
    path.addEllipse (9 * s, 4 * s, 5 * s, 8 * s);
    return path;
}

juce::Path IconSystem::createSplitIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addLineSegment (juce::Line<float> (8 * s, 2 * s,  4 * s, 8 * s), 2 * s);
    path.addLineSegment (juce::Line<float> (8 * s, 2 * s, 12 * s, 8 * s), 2 * s);
    path.addEllipse     (3 * s, 10 * s, 3 * s, 3 * s);
    path.addEllipse     (10 * s, 10 * s, 3 * s, 3 * s);
    return path;
}

juce::Path IconSystem::createSaveIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.addRectangle (2 * s, 4 * s, 12 * s, 10 * s);
    path.addRectangle (4 * s, 2 * s, 8 * s,  2 * s);
    path.addRectangle (6 * s, 6 * s, 4 * s,  4 * s);
    return path;
}

juce::Path IconSystem::createLeftArrowIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.startNewSubPath (12.0f * s, 8.0f * s);
    path.lineTo          ( 6.0f * s, 8.0f * s);
    path.startNewSubPath ( 6.0f * s, 8.0f * s);
    path.lineTo          ( 4.0f * s, 6.0f * s);
    path.lineTo          ( 4.0f * s, 10.0f * s);
    path.closeSubPath();
    return path;
}

juce::Path IconSystem::createRightArrowIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    path.startNewSubPath ( 4.0f * s, 8.0f * s);
    path.lineTo          (10.0f * s, 8.0f * s);
    path.startNewSubPath (10.0f * s, 8.0f * s);
    path.lineTo          (12.0f * s, 6.0f * s);
    path.lineTo          (12.0f * s, 10.0f * s);
    path.closeSubPath();
    return path;
}

juce::Path IconSystem::createFullScreenIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;

    path.startNewSubPath (4 * s, 4 * s);  path.lineTo (2 * s, 4 * s); path.lineTo (2 * s, 2 * s);
    path.startNewSubPath (2 * s, 4 * s);  path.lineTo (4 * s, 2 * s);

    path.startNewSubPath (12 * s, 4 * s); path.lineTo (14 * s, 4 * s); path.lineTo (14 * s, 2 * s);
    path.startNewSubPath (14 * s, 4 * s); path.lineTo (12 * s, 2 * s);

    path.startNewSubPath (4 * s, 12 * s); path.lineTo (2 * s, 12 * s); path.lineTo (2 * s, 14 * s);
    path.startNewSubPath (2 * s, 12 * s); path.lineTo (4 * s, 14 * s);

    path.startNewSubPath (12 * s, 12 * s); path.lineTo (14 * s, 12 * s); path.lineTo (14 * s, 14 * s);
    path.startNewSubPath (14 * s, 12 * s); path.lineTo (12 * s, 14 * s);

    return path;
}

juce::Path IconSystem::createExitFullScreenIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;

    path.startNewSubPath (2 * s, 2 * s);  path.lineTo (4 * s, 2 * s); path.lineTo (4 * s, 4 * s);
    path.startNewSubPath (4 * s, 2 * s);  path.lineTo (2 * s, 4 * s);

    path.startNewSubPath (14 * s, 2 * s); path.lineTo (12 * s, 2 * s); path.lineTo (12 * s, 4 * s);
    path.startNewSubPath (12 * s, 2 * s); path.lineTo (14 * s, 4 * s);

    path.startNewSubPath (2 * s, 14 * s); path.lineTo (4 * s, 14 * s); path.lineTo (4 * s, 12 * s);
    path.startNewSubPath (4 * s, 14 * s); path.lineTo (2 * s, 12 * s);

    path.startNewSubPath (14 * s, 14 * s); path.lineTo (12 * s, 14 * s); path.lineTo (12 * s, 12 * s);
    path.startNewSubPath (12 * s, 14 * s); path.lineTo (14 * s, 12 * s);

    return path;
}

juce::Path IconSystem::createColorPaletteIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;

    path.addRoundedRectangle (2.0f * s, 4.0f * s, 12.0f * s, 8.0f * s, 1.0f * s);

    path.addEllipse ( 4.0f * s, 6.0f * s, 2.0f * s, 2.0f * s);
    path.addEllipse ( 7.0f * s, 6.0f * s, 2.0f * s, 2.0f * s);
    path.addEllipse (10.0f * s, 6.0f * s, 2.0f * s, 2.0f * s);

    path.addEllipse ( 4.0f * s, 9.0f * s, 2.0f * s, 2.0f * s);
    path.addEllipse ( 7.0f * s, 9.0f * s, 2.0f * s, 2.0f * s);
    path.addEllipse (10.0f * s, 9.0f * s, 2.0f * s, 2.0f * s);

    return path;
}
