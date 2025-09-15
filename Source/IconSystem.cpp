#include "IconSystem.h"
using IT = IconSystem::IconType;

static inline juce::AffineTransform toCenter (juce::Rectangle<float> area, float size)
{
    return juce::AffineTransform::translation (area.getCentreX() - size * 0.5f,
                                               area.getCentreY() - size * 0.5f);
}

juce::Path IconSystem::createIcon (IconType type, float size)
{
    switch (type)
    {
        case Lock:           return createLockIcon (size);
        case Unlock:         return createUnlockIcon (size);
        case CogWheel:       return createCogWheelIcon (size);
        case Power:          return createPowerIcon (size);
        case Anchor:         return createAnchorIcon (size);
        case Retrig:         return createRetrigIcon (size);
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
        case Help:           return createHelpIcon (size);
        case X:              return createXIcon (size);
        case Snowflake:      return createSnowflakeIcon (size);
        case Note:           return createNoteIcon (size);
        case NoteDotted:     return createNoteDottedIcon (size);
        case Triplet3:       return createTriplet3Icon (size);
        case Droplet:        return createDropletIcon (size);
        case DropletSlash:   return createDropletSlashIcon (size);
        case SidechainKey:   return createSidechainKeyIcon (size);
        case Delta:          {
            juce::Path p; const float s = size / 16.0f; // upright triangle
            p.addTriangle (8 * s, 3 * s, 3 * s, 13 * s, 13 * s, 13 * s);
            return p;
        }
        default:             return {};
    }
}

void IconSystem::drawIcon (juce::Graphics& g, IT type, juce::Rectangle<float> area, juce::Colour colour)
{
    const float size = juce::jmin (area.getWidth(), area.getHeight());
    const float stroke = juce::jlimit (1.0f, 4.0f, size * 0.085f);

    auto P = createIcon (type, size);

    // shadow pass
    g.setColour (juce::Colours::black.withAlpha (0.18f));
    g.strokePath (P, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
                  toCenter (area.translated (0.7f, 1.0f), size));

    // main pass
    g.setColour (colour.withAlpha (0.98f));
    g.strokePath (P, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
                  toCenter (area, size));
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

juce::Path IconSystem::createXIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    // Two crossing rectangles for a clear X
    juce::Path a, b;
    a.addRectangle (7 * s, 3 * s, 2 * s, 10 * s);
    a.applyTransform (juce::AffineTransform::rotation (juce::MathConstants<float>::pi * 0.25f, 8 * s, 8 * s));
    b.addRectangle (7 * s, 3 * s, 2 * s, 10 * s);
    b.applyTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::pi * 0.25f, 8 * s, 8 * s));
    path.addPath (a);
    path.addPath (b);
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

juce::Path IconSystem::createAnchorIcon (float size)
{
    juce::Path p; 
    p.setUsingNonZeroWinding (false); // even-odd for ring “hole”

    const float s = size / 16.0f;
    auto R = [&] (float x){ return std::round(x) + 0.5f; }; // snap to pixel centers for crispness

    // --- Ring (donut) ---
    // Outer 6x6 circle centered near top; inner creates the hole.
    juce::Path ring;
    ring.addEllipse (R(5*s), R(1*s), R(6*s), R(6*s));                 // outer
    juce::Path inner; inner.addEllipse (R(6.75f*s), R(2.75f*s), R(2.5f*s), R(2.5f*s)); // inner
    ring.addPath (inner); // even-odd makes a hole

    // --- Shank (rounded) ---
    juce::Path shank;
    const float shankW = R(2*s), shankH = R(7.5f*s);
    shank.addRoundedRectangle (R(7*s), R(5.5f*s), shankW, shankH, R(1*s));

    // --- Stock (crossbar) ---
    juce::Path stock;
    stock.addRoundedRectangle (R(4.5f*s), R(9.5f*s), R(7*s), R(1.5f*s), R(0.75f*s));

    // --- Crown + Flukes (curved) ---
    juce::Path flukes;
    // Left fluke
    flukes.startNewSubPath (R(8*s),   R(12*s));
    flukes.cubicTo( R(6.5f*s), R(12*s),
                    R(5.25f*s),R(12.5f*s),
                    R(5*s),    R(14*s));     // inner curve down
    flukes.lineTo(  R(3.2f*s), R(13.25f*s)); // outer tip
    flukes.quadraticTo(R(4.75f*s), R(11.5f*s), R(6.5f*s), R(11*s)); // outer curve up
    flukes.closeSubPath();

    // Right fluke (mirror)
    flukes.startNewSubPath (R(8*s),   R(12*s));
    flukes.cubicTo( R(9.5f*s), R(12*s),
                    R(10.75f*s),R(12.5f*s),
                    R(11*s),   R(14*s));
    flukes.lineTo(  R(12.8f*s),R(13.25f*s));
    flukes.quadraticTo(R(11.25f*s), R(11.5f*s), R(9.5f*s), R(11*s));
    flukes.closeSubPath();

    // Small crown cap to visually join shank and flukes
    juce::Path crown;
    crown.addRoundedRectangle (R(6.5f*s), R(11*s), R(3*s), R(1*s), R(0.5f*s));

    // Combine
    p.addPath (ring);
    p.addPath (shank);
    p.addPath (stock);
    p.addPath (flukes);
    p.addPath (crown);

    return p;
}

juce::Path IconSystem::createRetrigIcon (float size)
{
    juce::Path p; 
    p.setUsingNonZeroWinding (false); // even-odd for ring gap if needed

    const float s = size / 16.0f;
    auto R = [&] (float x){ return std::round(x) + 0.5f; };

    // --- Ring segment (pie) ---
    // Outer 12x12 circle with inner radius for a clean ring; arc spans ~290° leaving a reset gap near top.
    const float cx = R(8*s), cy = R(8*s);
    const float outer = R(12*s);
    const float start = juce::MathConstants<float>::pi * 0.18f;  // ~32°
    const float end   = juce::MathConstants<float>::twoPi * 0.97f; // ~350°
    juce::Path ring;
    ring.addPieSegment (R(2*s), R(2*s), outer, outer, start, end, 0.58f /* inner radius as fraction */);

    // --- Arrowhead at arc end ---
    const float theta = end;
    const float rMid  = R(8*s);
    const float ax = cx + rMid * std::cos(theta);
    const float ay = cy + rMid * std::sin(theta);
    juce::Path arrow;
    const float ah = R(2.6f*s); // arrowhead length
    const float aw = R(1.8f*s); // width
    // Build a small triangle pointing tangentially
    const float tx = -std::sin(theta);
    const float ty =  std::cos(theta);
    juce::Point<float> tip (ax + ah * std::cos(theta), ay + ah * std::sin(theta));
    juce::Point<float> baseL (ax - 0.6f*ah * std::cos(theta) + 0.5f*aw * tx,
                              ay - 0.6f*ah * std::sin(theta) + 0.5f*aw * ty);
    juce::Point<float> baseR (ax - 0.6f*ah * std::cos(theta) - 0.5f*aw * tx,
                              ay - 0.6f*ah * std::sin(theta) - 0.5f*aw * ty);
    arrow.startNewSubPath (tip); arrow.lineTo (baseL); arrow.lineTo (baseR); arrow.closeSubPath();

    // --- Beat tick at 12 o’clock (reset marker) ---
    juce::Path tick;
    tick.addRectangle (R(7.4f*s), R(0.8f*s), R(1.2f*s), R(3.0f*s));

    // --- Inner “hit dot” (nice to pulse on retrig) ---
    juce::Path dot;
    dot.addEllipse (R(7.2f*s), R(7.2f*s), R(1.6f*s), R(1.6f*s));

    // Combine
    p.addPath (ring);
    p.addPath (arrow);
    p.addPath (tick);
    p.addPath (dot);

    return p;
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

juce::Path IconSystem::createHelpIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    // Outer circle
    path.addEllipse (2 * s, 2 * s, 12 * s, 12 * s);
    // Question mark constructed from simple rectangles/lines
    juce::Path q;
    // dot
    q.addRectangle (7 * s, 12 * s, 2 * s, 2 * s);
    // stem
    q.addRectangle (7 * s, 9 * s, 2 * s, 3 * s);
    // hook (three small rectangles to suggest a curve)
    q.addRectangle (7 * s, 8 * s, 3 * s, 1 * s);
    q.addRectangle (9 * s, 7 * s, 1 * s, 1 * s);
    q.addRectangle (6 * s, 6 * s, 4 * s, 1 * s);
    path.addPath (q);
    return path;
}

juce::Path IconSystem::createSnowflakeIcon (float size)
{
    juce::Path path;
    const float s = size / 16.0f;
    const float cx = 8 * s, cy = 8 * s;
    const float r = 6 * s;
    // Radial spokes
    for (int i = 0; i < 6; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (i / 6.0f); // 0,60,120,180,240,300 degrees
        const float x1 = cx + r * std::cos (a);
        const float y1 = cy + r * std::sin (a);
        path.addLineSegment (juce::Line<float> (cx, cy, x1, y1), 1.2f * s);
    }
    // Small ticks on ends
    for (int i = 0; i < 6; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (i / 6.0f);
        const float x1 = cx + r * std::cos (a);
        const float y1 = cy + r * std::sin (a);
        const float tx = -std::sin (a), ty = std::cos (a);
        path.addLineSegment (juce::Line<float> (x1 - 2 * s * tx, y1 - 2 * s * ty, x1 + 2 * s * tx, y1 + 2 * s * ty), 1.0f * s);
    }
    return path;
}

// --- musical note family ---
static void addNotehead(juce::Path& p, float cx, float cy, float rx, float ry)
{
    juce::Path e; e.addEllipse (cx - rx, cy - ry, rx * 2.0f, ry * 2.0f); p.addPath (e);
}
static void addStem(juce::Path& p, float x, float yTop, float yBot, float w)
{
    juce::Rectangle<float> r (x - w * 0.5f, yTop, w, yBot - yTop); p.addRectangle (r);
}

juce::Path IconSystem::createNoteIcon (float size)
{
    juce::Path p; const float s = size;
    const float cx = s * 0.52f, cy = s * 0.60f;
    addNotehead (p, cx, cy, s * 0.20f, s * 0.15f);
    addStem     (p, cx + s * 0.20f, cy - s * 0.35f, cy - s * 0.02f, s * 0.08f);
    return p;
}

juce::Path IconSystem::createNoteDottedIcon (float size)
{
    auto p = createNoteIcon (size);
    const float s = size; float r = s * 0.07f;
    juce::Path dot; dot.addEllipse (s * 0.78f - r, s * 0.58f - r, 2 * r, 2 * r);
    p.addPath (dot);
    return p;
}

juce::Path IconSystem::createTriplet3Icon (float size)
{
    juce::Path p; const float s = size;
    for (int i = 0; i < 3; ++i)
    {
        float t = (i - 1) * 0.28f;
        addNotehead (p, s * (0.50f + t), s * (0.60f - std::abs (t) * 0.12f), s * 0.13f, s * 0.10f);
    }
    juce::Path br; float xL = s * 0.24f, xR = s * 0.76f, y = s * 0.36f;
    br.startNewSubPath (xL, y); br.lineTo (xL, y + s * 0.10f);
    br.startNewSubPath (xR, y); br.lineTo (xR, y + s * 0.10f);
    br.startNewSubPath (xL, y); br.lineTo (xR, y);
    p.addPath (br);
    return p;
}

// --- droplet family ---
juce::Path IconSystem::createDropletIcon (float size)
{
    juce::Path p; const float s = size;
    float cx = s * 0.5f, cy = s * 0.58f, r = s * 0.22f;
    p.startNewSubPath (cx, cy - r * 1.5f);
    p.cubicTo (cx + r, cy - r, cx + r * 0.9f, cy + r * 0.5f, cx, cy + r);
    p.cubicTo (cx - r * 0.9f, cy + r * 0.5f, cx - r, cy - r, cx, cy - r * 1.5f);
    return p;
}

juce::Path IconSystem::createDropletSlashIcon (float size)
{
    auto p = createDropletIcon (size);
    const float s = size; float x0 = s * 0.25f, y0 = s * 0.78f, x1 = s * 0.78f, y1 = s * 0.25f;
    juce::Path slash; slash.addLineSegment ({ { x0, y0 }, { x1, y1 } }, s * 0.10f);
    p.addPath (slash);
    return p;
}

juce::Path IconSystem::createSidechainKeyIcon (float size)
{
    juce::Path p; const float s = size;
    // keyhole
    p.addEllipse (s * 0.62f, s * 0.26f, s * 0.20f, s * 0.24f);
    p.addRectangle (s * 0.69f, s * 0.48f, s * 0.06f, s * 0.22f);
    // incoming mini-wave
    juce::Path w; float x = s * 0.15f, y = s * 0.56f, dx = s * 0.10f, a = s * 0.08f;
    w.startNewSubPath (x, y);
    for (int i = 0; i < 3; ++i)
    {
        w.cubicTo (x + dx * 0.5f, y - a, x + dx * 0.5f, y + a, x + dx, y);
        x += dx;
    }
    p.addPath (w);
    return p;
}
