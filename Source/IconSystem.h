#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class IconSystem
{
public:
    enum IconType
    {
        Lock,
        Unlock,
        CogWheel,
        Power,
        Anchor,
        Retrig,
        Speaker,
        Pan,
        Space,
        Width,
        Tilt,
        Mono,
        HP,
        LP,
        Drive,
        Mix,
        Air,
        Duck,
        Link,
        Snap,
        Options,
        Bypass,
        Stereo,
        Split,
        Save,
        LeftArrow,
        RightArrow,
        FullScreen,
        ExitFullScreen,
        ColorPalette,
        Help,
        X,
        Snowflake,
        Note,
        NoteDotted,
        Triplet3,
        Droplet,
        DropletSlash,
        SidechainKey,
        Delta
    };

    static juce::Path createIcon (IconType type, float size = 16.0f);
    static void drawIcon (juce::Graphics& g, IconType type,
                          juce::Rectangle<float> bounds,
                          juce::Colour colour = juce::Colours::white);

private:
    static juce::Path createLockIcon         (float size);
    static juce::Path createUnlockIcon       (float size);
    static juce::Path createCogWheelIcon     (float size);
    static juce::Path createPowerIcon        (float size);
    static juce::Path createAnchorIcon       (float size);
    static juce::Path createRetrigIcon       (float size);
    static juce::Path createSpeakerIcon      (float size);
    static juce::Path createPanIcon          (float size);
    static juce::Path createSpaceIcon        (float size);
    static juce::Path createWidthIcon        (float size);
    static juce::Path createTiltIcon         (float size);
    static juce::Path createMonoIcon         (float size);
    static juce::Path createHPIcon           (float size);
    static juce::Path createLPIcon           (float size);
    static juce::Path createDriveIcon        (float size);
    static juce::Path createMixIcon          (float size);
    static juce::Path createAirIcon          (float size);
    static juce::Path createDuckIcon         (float size);
    static juce::Path createLinkIcon         (float size);
    static juce::Path createSnapIcon         (float size);
    static juce::Path createOptionsIcon      (float size);
    static juce::Path createBypassIcon       (float size);
    static juce::Path createStereoIcon       (float size);
    static juce::Path createSplitIcon        (float size);
    static juce::Path createSaveIcon         (float size);
    static juce::Path createLeftArrowIcon    (float size);
    static juce::Path createRightArrowIcon   (float size);
    static juce::Path createFullScreenIcon   (float size);
    static juce::Path createExitFullScreenIcon (float size);
    static juce::Path createColorPaletteIcon (float size);
    static juce::Path createHelpIcon (float size);
    static juce::Path createXIcon (float size);
    static juce::Path createSnowflakeIcon (float size);
    static juce::Path createNoteIcon (float size);
    static juce::Path createNoteDottedIcon (float size);
    static juce::Path createTriplet3Icon (float size);
    static juce::Path createDropletIcon (float size);
    static juce::Path createDropletSlashIcon (float size);
    static juce::Path createSidechainKeyIcon (float size);
};
