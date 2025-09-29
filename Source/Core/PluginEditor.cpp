#include "PluginProcessor.h"
#include "PluginEditor.h"
// XYPaneAdapter removed - XYTab now contains XYPad directly
#include "ui/Managers/PaneManager.h"
#include "reverb/ReverbParamIDs.h"
#include "ui/Design/Layout.h"
#include "dsp/DelayPresetLibrary.h"
#include "reverb/ui/ReverbGraphics.h"
#include "reverb/ui/ReverbDynEQPane.h"
#include "ui/Controls/ControlGridMetrics.h"

//==============================================================

// Reusable tinted PopupMenu LookAndFeel + helper
struct TintMenuLNFEx : public juce::LookAndFeel_V4
{
    juce::Colour defaultTint { juce::Colours::skyblue };
    juce::Array<juce::Colour> itemTints;
    bool hideChecks = true;
    mutable int paintIndex = 0; // reset on background draw

    void drawPopupMenuBackground (juce::Graphics& g, int w, int h) override
    {
        paintIndex = 0;
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
        // Use configured colours when available; avoid hardcoded hex
        auto bg = findColour (juce::PopupMenu::backgroundColourId);
        auto text = findColour (juce::PopupMenu::textColourId);
        g.setColour (bg);
        g.fillRect (r);
        g.setColour (text.withAlpha (0.06f));
        g.drawRoundedRectangle (r.reduced (1.0f), 5.0f, 1.0f);
    }

    void drawPopupMenuSeparator (juce::Graphics& g, const juce::Rectangle<int>& area)
    {
        auto r = area.toFloat().reduced (10.0f, 0.0f);
        auto text = findColour (juce::PopupMenu::textColourId);
        g.setColour (text.withAlpha (0.10f));
        g.fillRect (juce::Rectangle<float> (r.getX(), r.getCentreY() - 0.5f, r.getWidth(), 1.0f));
    }

    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::String& title,
                                     const juce::Rectangle<int>& area)
    {
        auto r = area.toFloat().reduced (8.0f, 4.0f);
        auto text = findColour (juce::PopupMenu::textColourId);
        g.setColour (text.withAlpha (0.60f));
        g.setFont (juce::Font (juce::FontOptions (12.5f)).withExtraKerningFactor (0.02f).boldened());
        g.drawFittedText (title.toUpperCase(), r.toNearestInt(), juce::Justification::centredLeft, 1);
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool /*isActive*/, bool isHighlighted, bool isTicked,
                            bool /*hasSubMenu*/, const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* /*icon*/, const juce::Colour* textColour) override
    {
        if (isSeparator) { drawPopupMenuSeparator (g, area); return; }

        auto r = area.toFloat().reduced (4.0f, 2.0f);
        const juce::Colour tint = (paintIndex >= 0 && paintIndex < itemTints.size())
                                  ? itemTints.getReference (paintIndex++)
                                  : defaultTint;

        if (isHighlighted || isTicked)
        {
            g.setColour (tint.withAlpha (isHighlighted ? 0.90f : 0.65f));
            g.fillRoundedRectangle (r, 4.0f);
        auto text = findColour (juce::PopupMenu::textColourId);
        g.setColour (text.withAlpha (0.10f));
            g.drawRoundedRectangle (r, 4.0f, 1.0f);
        }

        auto ta = r.reduced (hideChecks ? 8.0f : 22.0f, 0.0f);
        g.setColour (textColour ? *textColour : findColour (juce::PopupMenu::textColourId).withAlpha (0.95f));
        g.setFont (juce::Font (juce::FontOptions (14.0f)));
        g.drawFittedText (text, ta.toNearestInt(), juce::Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            g.setColour (findColour (juce::PopupMenu::textColourId).withAlpha (0.55f));
            g.setFont (juce::Font (juce::FontOptions (13.0f)));
            auto rt = ta.removeFromRight (60).toNearestInt();
            g.drawFittedText (shortcutKeyText, rt, juce::Justification::centredRight, 1);
        }
    }
};

// Small per-button LNF to draw an up/down chevron on the Group 2 toggle
struct BottomChevronLNF : public FieldLNF
{
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                               bool isOver, bool isDown) override
    {
        auto r = button.getLocalBounds().toFloat().reduced (2.0f);
        // Background consistent with our buttons: gradient panel + outline
        auto top = theme.panel.brighter (0.10f);
        auto bot = theme.panel.darker   (0.10f);
        g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (theme.sh);
        g.drawRoundedRectangle (r, 6.0f, 1.0f);

        // Subtle elevation on hover/down
        if (isOver || isDown)
        {
            juce::DropShadow ds1 (theme.shadowDark.withAlpha (0.20f), 8, { -1, -1 });
            juce::DropShadow ds2 (theme.shadowLight.withAlpha (0.18f), 5, { -1, -1 });
            ds1.drawForRectangle (g, r.getSmallestIntegerContainer());
            ds2.drawForRectangle (g, r.getSmallestIntegerContainer());
        }
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isOver, bool /*isDown*/) override
    {
        // Chevron points up when not engaged, down when engaged
        const bool engaged = button.getToggleState();
        auto col = isOver ? theme.accent : theme.textMuted;
        g.setColour (col);

        auto b = button.getLocalBounds().toFloat().reduced (button.getHeight() * 0.22f, button.getHeight() * 0.30f);
        const float cx = b.getCentreX();
        const float cy = b.getCentreY();
        const float half = b.getWidth() * 0.40f;   // wider span for a flatter chevron
        const float vAmp = b.getHeight() * 0.18f;  // smaller vertical excursion for flat look

        juce::Path p;
        if (engaged)
        {
            // Down chevron (V)
            p.startNewSubPath (cx - half, cy - vAmp);
            p.lineTo          (cx,        cy + vAmp);
            p.lineTo          (cx + half, cy - vAmp);
        }
        else
        {
            // Up chevron (^)
            p.startNewSubPath (cx - half, cy + vAmp);
            p.lineTo          (cx,        cy - vAmp);
            p.lineTo          (cx + half, cy + vAmp);
        }
        g.strokePath (p, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
};

template <typename BuildFn, typename ResultFn>
static void showTintedMenu (juce::Component& anchor, const TintMenuLNFEx& configuredLnf,
                            BuildFn&& build, ResultFn&& onResult)
{
    auto lnfHold = std::make_shared<TintMenuLNFEx>();
    // Copy relevant configuration
    lnfHold->defaultTint = configuredLnf.defaultTint;
    lnfHold->hideChecks  = configuredLnf.hideChecks;
    lnfHold->itemTints   = configuredLnf.itemTints;
    // Copy colours that might have been set on configuredLnf
    lnfHold->setColour (juce::PopupMenu::textColourId,
                        configuredLnf.findColour (juce::PopupMenu::textColourId));
    lnfHold->setColour (juce::PopupMenu::highlightedBackgroundColourId,
                        configuredLnf.findColour (juce::PopupMenu::highlightedBackgroundColourId));
    lnfHold->setColour (juce::PopupMenu::highlightedTextColourId,
                        configuredLnf.findColour (juce::PopupMenu::highlightedTextColourId));

    juce::PopupMenu m; m.setLookAndFeel (lnfHold.get());

    build (m, *lnfHold);

    auto* parent = anchor.getTopLevelComponent();
    juce::PopupMenu::Options opt;
    opt = opt.withTargetComponent (&anchor)
             .withParentComponent (parent)
             .withMinimumWidth (juce::jmax (160, anchor.getWidth()));

    m.showMenuAsync (opt, [lnfHold, onResult] (int r) { onResult (r); });
}

//==============================================================

//==============================================================
// ToggleSwitch (compact, slow animation, keeps original visual)

//==============================================================
// VerticalSlider3D implementation
//==============================================================

//==============================================================
// ControlContainer (panel with subtle depth + title)
ControlContainer::ControlContainer() { setWantsKeyboardFocus (false); }

void ControlContainer::setTitle (const juce::String& t) { containerTitle = t; repaint(); }

void ControlContainer::paint (juce::Graphics& g)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto panel = lf ? lf->theme.panel : juce::Colour (0xFF3A3D45);
    const auto text  = lf ? lf->theme.text  : juce::Colour (0xFFF0F2F5);
    const auto accent= lf ? lf->theme.accent: juce::Colour (0xFF5AA9E6);

    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;

    if (showBorder)
    {
        // Use custom background color if set, otherwise use theme panel color
        juce::Colour bgColor = useCustomBackgroundColour ? backgroundColour : panel;
        g.setColour (bgColor);
        g.fillRoundedRectangle (r.reduced (3.0f), rad);

        // depth
        juce::DropShadow ds1 ((lf ? lf->theme.shadowDark  : juce::Colour (0xFF1A1C20)).withAlpha (0.6f), 20, { -2, -2 });
        juce::DropShadow ds2 ((lf ? lf->theme.shadowLight : juce::Colour (0xFF60646C)).withAlpha (0.4f),  8, { -1, -1 });
        auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
        ds1.drawForRectangle (g, ri);
        ds2.drawForRectangle (g, ri);

        // inner rim
        g.setColour ((lf ? lf->theme.sh : juce::Colour (0xFF2A2C30)).withAlpha (0.3f));
        g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 1.0f);
    }

    if (showBorder)
    {
        auto border = r.reduced (3.0f);
        // hover halo
        const bool over = isMouseOverOrDragging();
        if (over || hoverActive)
        {
            g.setColour (accent.withAlpha (0.5f));
            g.drawRoundedRectangle (border.expanded (2.0f), rad, 2.0f);
        }
        g.setColour (useCustomBorderColour ? borderColour : accent);
        g.drawRoundedRectangle (border, rad, 1.0f);
    }

    if (containerTitle.isNotEmpty() && showBorder)
    {
        auto title = r.reduced (10.0f).removeFromTop (25);
        g.setColour (text);
        g.setFont (juce::Font (juce::FontOptions (14.0f).withStyle ("Bold")));

        // optional icon
        IconSystem::IconType icon = IconSystem::Speaker;
        if      (containerTitle == "FIELD")  icon = IconSystem::Space;
        else if (containerTitle == "VOLUME") icon = IconSystem::Speaker;
        else if (containerTitle == "EQ")     icon = IconSystem::Tilt;

        IconSystem::drawIcon (g, icon, title.removeFromLeft (20).reduced (2.0f), text);
        g.drawText (containerTitle, title, juce::Justification::centredLeft);
    }
}

//==============================================================
// XYPad (visual upgrade preserved; layout/edges match original)
void XYPad::pushWaveformSample (double l, double r)
{
    waveformL[(size_t) waveformWriteIndex] = l;
    waveformR[(size_t) waveformWriteIndex] = r;
    waveformWriteIndex = (waveformWriteIndex + 1) % waveformBufferSize;
    hasWaveformData = true;

    // Avoid cross-thread repaint; Editor timer will repaint at ~30 Hz.
    // (No repaint here.)
}

int XYPad::getBallAtPosition (juce::Point<float> pos, juce::Rectangle<float> b)
{
    if (!isSplitMode) return 0;

    const float gainScale = juce::jmap (gainValue, -24.0f, 24.0f, 0.5f, 2.0f);
    const float hitR = 15.0f * gainScale;

    juce::Point<float> L (b.getX() + leftPt  * b.getWidth(),  b.getY() + (1.0f - pt.second) * b.getHeight());
    juce::Point<float> R (b.getX() + rightPt * b.getWidth(),  b.getY() + (1.0f - pt.second) * b.getHeight());

    if (pos.getDistanceFrom (L) < hitR) return 1;
    if (pos.getDistanceFrom (R) < hitR) return 2;
    return 0;
}

void XYPad::drag (const juce::MouseEvent& e)
{
    auto r = getLocalBounds().toFloat().reduced (40.0f);
    float x01 = juce::jlimit (0.0f, 1.0f, (e.position.x - r.getX()) / r.getWidth());
    float y01 = juce::jlimit (0.0f, 1.0f, 1.0f - (e.position.y - r.getY()) / r.getHeight());

    if (snapEnabled)
    {
        x01 = std::round (x01 * 20.0f) / 20.0f;
        y01 = std::round (y01 * 10.0f) / 10.0f;
    }

    if (isSplitMode)
    {
        if (isLinked)
        {
            leftPt = rightPt = x01;
            pt.second = y01;
            if (onSplitChange) onSplitChange (leftPt, rightPt, y01);
        }
        else
        {
            if (activeBall == 0) activeBall = getBallAtPosition (e.position, r);
            if (activeBall == 1) { leftPt  = x01; pt.second = y01; if (onBallChange) onBallChange (1, leftPt,  y01); }
            if (activeBall == 2) { rightPt = x01; pt.second = y01; if (onBallChange) onBallChange (2, rightPt, y01); }
            if (onSplitChange) onSplitChange (leftPt, rightPt, pt.second);
        }
    }
    else
    {
        pt = { x01, y01 };
        if (onChange) onChange (x01, y01);
    }

    repaint();
}
void XYPad::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;

    // panel
    if (auto* lfPanel = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        g.setColour (lfPanel->theme.panel);
    else
        g.setColour (juce::Colours::darkgrey);
    g.fillRoundedRectangle (r.reduced (3.0f), rad);

    // depth (softer to avoid visible top/bottom bars)
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    juce::DropShadow ds1 ((lf ? lf->theme.shadowDark  : juce::Colours::black).withAlpha (0.35f), 12, { -1, -1 });
    juce::DropShadow ds2 ((lf ? lf->theme.shadowLight : juce::Colours::grey).withAlpha (0.25f),  6, { -1, -1 });
    auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
    ds1.drawForRectangle (g, ri);
    ds2.drawForRectangle (g, ri);

    // rim (lighter)
    g.setColour ((lf ? lf->theme.sh : juce::Colours::black).withAlpha (0.18f));
    g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);

    // hover halo
    const auto accent = (lf ? lf->theme.accent : juce::Colours::lightblue);
    auto border = r.reduced (2.0f);
    const bool over = isMouseOverOrDragging();
    g.setColour (accent);
    g.drawRoundedRectangle (border, rad, 2.0f);
    if (over || hoverActive)
    {
        for (int i = 1; i <= 8; ++i)
        {
            const float t = (float) i / 8.0f;
            const float expand = 3.0f + t * 10.0f;
            g.setColour (accent.withAlpha ((1.0f - t) * (isGreenMode ? 0.25f : 0.22f)));
            g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.4f, 2.0f);
        }
        g.setColour (accent);
        g.drawRoundedRectangle (border, rad, 2.0f);
    }

    // Standard border treatment: accent border (reduced brightness for XY pad)
    g.setColour (accent.withAlpha (0.3f));
    g.drawRoundedRectangle (r, rad, 1.0f);

    // inner content
    auto padBounds = r.reduced (40.0f);

    drawWaveformBackground (g, padBounds);
    drawGrid              (g, padBounds);
    drawFrequencyRegions  (g, padBounds);
    drawEQCurves          (g, padBounds);
    drawBalls             (g, padBounds);
    drawImagingOverlays   (g, padBounds);

    // center crosshair (subtle)
    g.setColour ((lf ? lf->theme.textMuted : juce::Colours::white).withAlpha (0.4f));
    g.drawLine (r.getCentreX(), r.getY() + 40, r.getCentreX(), r.getBottom() - 40, 1.5f);
    g.drawLine (r.getX() + 40, r.getCentreY(), r.getRight() - 40, r.getCentreY(), 1.5f);
}

void XYPad::drawImagingOverlays (juce::Graphics& g, juce::Rectangle<float> b)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto gridCol = lf ? lf->theme.hl.withAlpha (0.30f)    : juce::Colours::grey.withAlpha (0.30f);
    auto textCol = lf ? lf->theme.textMuted.withAlpha(.8f): juce::Colours::lightgrey.withAlpha(.8f);
    auto acc     = lf ? lf->theme.accent.withAlpha(0.85f) : juce::Colours::lightblue.withAlpha(0.85f);

    auto xAtHz = [&] (float hz)
    {
        const float minHz = 20.0f, maxHz = 20000.0f;
        float t = (float) (std::log10 (juce::jlimit(minHz, maxHz, hz) / minHz) / std::log10 (maxHz / minHz));
        return juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
    };

    // XO LO and XO HI lines moved to Band tab - no longer drawn in XY Pad

    // 2) True M/S rotation renderer (energy circle + rotated basis + S-curve)
    if (lf)
    {
        // Slightly smaller ring: 45% of pad height is the radius
        const float radius = b.getHeight() * 0.45f;
        const float side   = radius * 2.0f;
        auto rotRect = juce::Rectangle<float> (0.0f, 0.0f, side, side).withCentre (b.getCentre());
        lf->drawRotationPad (g, rotRect, rotationDeg, asym,
                             lf->theme.accent, lf->theme.text, lf->theme.panel);
    }

    // SHUF visuals moved to Band tab
}
// ---- grid / frequency regions / EQ / balls ----
// Minimal implementations to satisfy drawing helpers used by XYPad::paint
void XYPad::drawGrid (juce::Graphics& g, juce::Rectangle<float> b)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto border = lf ? lf->theme.sh : juce::Colours::darkgrey;
    const auto grid   = lf ? lf->theme.hl.withAlpha (0.22f) : juce::Colours::grey.withAlpha (0.22f);
    const auto sub    = lf ? lf->theme.hl.withAlpha (0.10f) : juce::Colours::grey.withAlpha (0.10f);

    g.setColour (border);
    g.drawRoundedRectangle (b, 6.0f, 1.0f);

    // Pan subgrid (every 5 units across -50..0..+50)
    // Map -50..+50 to left..right; ticks every 5
    for (int p = -50; p <= 50; p += 5)
    {
        const float t = (float) (p + 50) / 100.0f; // 0..1
        const float x = juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
        g.setColour ((p % 10 == 0) ? grid : sub);
        g.drawLine (x, b.getY(), x, b.getBottom(), (p % 10 == 0) ? 1.0f : 0.6f);
        // Top/bottom tick marks every 5 for extra legibility
        if (p % 10 != 0)
        {
            const float tickH = 6.0f;
            g.setColour (sub);
            g.drawLine (x, b.getY(),              x, b.getY() + tickH,        1.0f);
            g.drawLine (x, b.getBottom() - tickH, x, b.getBottom(),           1.0f);
        }
        if (p % 10 == 0)
        {
            // labels at top
            juce::String lbl;
            if (p < 0)      lbl = juce::String (std::abs(p)) + "L";
            else if (p > 0) lbl = juce::String (p) + "R";
            else            lbl = "0";
            g.setColour (lf ? lf->theme.textMuted.withAlpha (0.8f) : juce::Colours::lightgrey.withAlpha (0.8f));
            g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")));
            g.drawText (lbl, juce::Rectangle<int> ((int) (x - 14), (int) (b.getY() - 14), 28, 12), juce::Justification::centred);
        }
    }
    // A few horizontal guides (quarters)
    for (int j = 1; j < 4; ++j)
    {
        const float y = juce::jmap ((float) j, 0.0f, 4.0f, b.getY(), b.getBottom());
        g.setColour (j == 2 ? grid : sub);
        g.drawLine (b.getX(), y, b.getRight(), y, j == 2 ? 1.0f : 0.6f);
    }

    // Frequency scale markers (low→high left-to-right)
    g.setColour (lf ? lf->theme.textMuted.withAlpha (0.35f) : juce::Colours::white.withAlpha (0.35f));
    const float yLabel = b.getBottom() + 12.0f;
    auto drawHz = [&] (float hz)
    {
        const float t = (float) (std::log10 (hz / 20.0f) / 3.0);
        const float x = juce::jmap (juce::jlimit (0.0f, 1.0f, t), 0.0f, 1.0f, b.getX(), b.getRight());
        g.drawLine (x, b.getBottom(), x, b.getBottom() - 6.0f, 1.0f);
        juce::String label;
        if      (hz >= 1000.0f) label = juce::String (hz / 1000.0f, 1) + "k";
        else                    label = juce::String ((int) hz);
        g.drawText (label, juce::Rectangle<int> ((int) x - 20, (int) yLabel, 40, 12), juce::Justification::centred);
    };
    for (float hz : { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f })
        drawHz (hz);
}

void XYPad::drawFrequencyRegions (juce::Graphics& g, juce::Rectangle<float> b)
{
    // Shaded log-spaced bands to differentiate Hz regions
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto base1 = lf ? lf->theme.base : juce::Colours::darkgrey;
    auto base2 = lf ? lf->theme.panel: juce::Colours::grey;
    base1 = base1.withAlpha (0.06f);
    base2 = base2.withAlpha (0.10f);

    const float minHz = 20.0f, maxHz = 20000.0f;
    auto xAtHz = [&] (float hz)
    {
        const float t = (float) (std::log10 (juce::jlimit (minHz, maxHz, hz) / minHz) / std::log10 (maxHz / minHz));
        return juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
    };

    // Define region boundaries (approx): 20, 60, 200, 800, 3k, 8k, 20k
    float marks[] = { 20.0f, 60.0f, 200.0f, 800.0f, 3000.0f, 8000.0f, 20000.0f };
    for (int i = 0; i < 6; ++i)
    {
        float x1 = xAtHz (marks[i]);
        float x2 = xAtHz (marks[i+1]);
        auto region = juce::Rectangle<float> (x1, b.getY(), x2 - x1, b.getHeight());
        g.setColour ((i % 2 == 0) ? base1 : base2);
        g.fillRect (region);
    }
}

void XYPad::drawWaveformBackground (juce::Graphics& g, juce::Rectangle<float> b)
{
    if (!hasWaveformData) return;
    const int N = waveformBufferSize;
    const int stride = 2; // downsample for slower, more readable motion
    const int P = juce::jmax (2, (N - 1) / stride + 1);
    const float dx = b.getWidth() / (float) (P - 1);

    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto leftCol  = lf ? lf->theme.accent.withAlpha (0.40f) : juce::Colours::lightblue.withAlpha (0.40f);
    const auto rightCol = lf ? lf->theme.text.withAlpha (0.35f)   : juce::Colours::white.withAlpha (0.35f);

    auto drawBuffer = [&] (const std::array<double, waveformBufferSize>& buf, juce::Colour col)
    {
        juce::Path p;
        p.preallocateSpace (P * 3);
        float x = b.getX();
        // Left-to-right: oldest on left, newest on right
        const int startIdx = waveformWriteIndex; // oldest sample position
        int pointIndex = 0;
        for (int i = 0; i < N; i += stride)
        {
            const int idx = (startIdx + i) % N;
            const float y = juce::jmap ((float) buf[(size_t) idx], -1.0f, 1.0f, b.getBottom(), b.getY());
            if (pointIndex == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
            x += dx;
            ++pointIndex;
        }
        // glow: outer soft + core line
        g.setColour (col.withAlpha (0.15f));
        g.strokePath (p, juce::PathStrokeType (10.0f));
        g.setColour (col.withAlpha (0.30f));
        g.strokePath (p, juce::PathStrokeType (5.0f));
        g.setColour (col.withAlpha (0.75f));
        g.strokePath (p, juce::PathStrokeType (1.2f));
    };

    drawBuffer (waveformL, leftCol);
    drawBuffer (waveformR, rightCol);
}

namespace VizEQ {
struct Biquad {
    double b0=1, b1=0, b2=0, a1=0, a2=0;
    inline void normalize(double a0){ b0/=a0; b1/=a0; b2/=a0; a1/=a0; a2/=a0; }
    inline double magDB(double w) const {
        const double c1=std::cos(w), s1=std::sin(w);
        const double c2=std::cos(2*w), s2=std::sin(2*w);
        const double NR=b0 + b1*c1 + b2*c2;
        const double NI=     b1*s1 + b2*s2;
        const double DR=1.0 + a1*c1 + a2*c2;
        const double DI=     a1*s1 + a2*s2;
        const double m2=(NR*NR+NI*NI)/(DR*DR+DI*DI);
        return 20.0*std::log10(std::max(1e-12, std::sqrt(m2)));
    }
};
constexpr double kPI = juce::MathConstants<double>::pi;
constexpr double kSqrt2Inv = 0.7071067811865476;
inline Biquad lowpassRBJ(double Fs, double f0, double Q=kSqrt2Inv){
    const double w0=2.0*kPI*f0/Fs, c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*Q);
    double b0=(1.0-c)*0.5, b1=1.0-c, b2=(1.0-c)*0.5;
    double a0=1.0+alpha, a1=-2.0*c, a2=1.0-alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline Biquad highpassRBJ(double Fs, double f0, double Q=kSqrt2Inv){
    const double w0=2.0*kPI*f0/Fs, c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*Q);
    double b0=(1.0+c)*0.5, b1=-(1.0+c), b2=(1.0+c)*0.5;
    double a0=1.0+alpha, a1=-2.0*c, a2=1.0-alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline Biquad lowshelfRBJ(double Fs,double f0,double GdB,double S=1.0){
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/2.0 * std::sqrt((A + 1.0/A)*(1.0/S - 1.0) + 2.0);
    const double twoRtA_alpha = 2.0*std::sqrt(A)*alpha;
    double b0=A*((A+1)-(A-1)*c + twoRtA_alpha);
    double b1=2*A*((A-1)-(A+1)*c);
    double b2=A*((A+1)-(A-1)*c - twoRtA_alpha);
    double a0=   (A+1)+(A-1)*c + twoRtA_alpha;
    double a1=-2*((A-1)+(A+1)*c);
    double a2=   (A+1)+(A-1)*c - twoRtA_alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline Biquad highshelfRBJ(double Fs,double f0,double GdB,double S=1.0){
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/2.0 * std::sqrt((A + 1.0/A)*(1.0/S - 1.0) + 2.0);
    const double twoRtA_alpha = 2.0*std::sqrt(A)*alpha;
    double b0=A*((A+1)+(A-1)*c + twoRtA_alpha);
    double b1=-2*A*((A-1)+(A+1)*c);
    double b2=A*((A+1)+(A-1)*c - twoRtA_alpha);
    double a0=   (A+1)-(A-1)*c + twoRtA_alpha;
    double a1= 2*((A-1)-(A+1)*c);
    double a2=   (A+1)-(A-1)*c - twoRtA_alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline Biquad peakingRBJ_Q(double Fs,double f0,double GdB,double Q){
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*std::max(1e-6,Q));
    double b0=1.0+alpha*A, b1=-2.0*c, b2=1.0-alpha*A;
    double a0=1.0+alpha/A, a1=-2.0*c, a2=1.0-alpha/A;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b; }
inline double softPix(double dB, double knee=6.0, double dBmax=18.0){
    const double s = (1.0 - std::exp(-std::abs(dB)/knee)) * dBmax;
    return dB>=0.0 ? s : -s;
}
} // namespace VizEQ
void XYPad::drawEQCurves (juce::Graphics& g, juce::Rectangle<float> b)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto hpLpCol   = lf ? lf->theme.eq.hp     : juce::Colours::lightblue;
    const auto airCol    = lf ? lf->theme.eq.air    : juce::Colours::white;
    const auto tiltCol   = lf ? lf->theme.eq.tilt   : juce::Colours::orange;
    const auto bassCol   = lf ? lf->theme.eq.bass   : juce::Colours::green;
    const auto scoopCol  = lf ? lf->theme.eq.scoop  : juce::Colours::purple;
    const auto monoShade = lf ? lf->theme.eq.monoShade : juce::Colours::black.withAlpha (0.15f);

    // Shade mono region
    if (monoHzValue > 20.0f)
    {
        const float minHz = 20.0f, maxHz = 20000.0f;
        const float t = (float) (std::log10 (juce::jlimit (minHz, maxHz, monoHzValue) / minHz) / std::log10 (maxHz / minHz));
        const float xMono = juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
        g.setColour (monoShade);
        g.fillRect (juce::Rectangle<float> (b.getX(), b.getY(), xMono - b.getX(), b.getHeight()));
    }

    // High-res sampling along width for smooth curves
    const int N = juce::jmax (192, (int) b.getWidth());
    auto freqAt = [] (float t01)
    {
        const float minHz = 20.0f, maxHz = 20000.0f;
        return minHz * std::pow (maxHz / minHz, t01);
    };

    auto toY = [&] (float db)
    {
        // Slightly more exaggerated for visibility
        const float scale = 6.0f;
        return juce::jlimit (b.getY(), b.getBottom(), b.getCentreY() - db * scale);
    };

    juce::Path hpLp, hpFill, lpFill, airP, airFill, tiltP, tiltFill, bassP, bassFill, scoopP, scoopFill;
    // Reserve to reduce per-frame allocations in paint
    const int reservePts = juce::jmax (N * 3, 256);
    hpLp.preallocateSpace (reservePts);
    hpFill.preallocateSpace (reservePts);
    lpFill.preallocateSpace (reservePts);
    airP.preallocateSpace (reservePts);
    airFill.preallocateSpace (reservePts);
    tiltP.preallocateSpace (reservePts);
    tiltFill.preallocateSpace (reservePts);
    bassP.preallocateSpace (reservePts);
    bassFill.preallocateSpace (reservePts);
    scoopP.preallocateSpace (reservePts);
    scoopFill.preallocateSpace (reservePts);
    for (int i = 0; i < N; ++i)
    {
        const float t01 = (float) i / (float) (N - 1);
        const float x    = juce::jmap (t01, b.getX(), b.getRight());
        const float hz   = freqAt (t01);

        // Build RBJ biquads for current parameters
        const double Fs = vizSampleRate > 0.0 ? vizSampleRate : 48000.0;
        // Skip HP/LP visual influence when they are at neutral extremes to avoid phantom curvature at 20/20000
        const bool hpNeutral = hpValue <= 20.0f;
        const bool lpNeutral = lpValue >= 20000.0f;
        // Shelves use S; Tilt optionally inherits S
        auto bBass  = VizEQ::lowshelfRBJ (Fs, juce::jlimit (20.0f, 20000.0f, bassFreqValue),  bassValue,  shelfShapeS);
        auto bAir   = VizEQ::highshelfRBJ(Fs, juce::jlimit (20.0f, 20000.0f, airFreqValue),   airValue,   shelfShapeS);
        const float tiltS = tiltUsesShelfS ? shelfShapeS : 0.90f;
        auto bTiltLo= VizEQ::lowshelfRBJ (Fs, juce::jlimit (20.0f, 20000.0f, tiltFreqValue), +0.5f*tiltValue, tiltS);
        auto bTiltHi= VizEQ::highshelfRBJ(Fs, juce::jlimit (20.0f, 20000.0f, tiltFreqValue), -0.5f*tiltValue, tiltS);
        
        // HP/LP use Q (global or per-filter); neutral at extremes uses sqrt2 for flatness
        const double qHP = qLink ? (double) filterQGlobal : (double) hpQ;
        const double qLP = qLink ? (double) filterQGlobal : (double) lpQ;
        auto bHP    = hpNeutral ? VizEQ::highpassRBJ(Fs, 20.0, VizEQ::kSqrt2Inv)
                                : VizEQ::highpassRBJ (Fs, juce::jlimit (20.0f, 20000.0f, hpValue), qHP);
        auto bLP    = lpNeutral ? VizEQ::lowpassRBJ (Fs, 20000.0, VizEQ::kSqrt2Inv)
                                : VizEQ::lowpassRBJ  (Fs, juce::jlimit (20.0f, 20000.0f, lpValue), qLP);
        // Peak bell for scoop/boost: adapt Q from shelf shape S (wider at low S, tighter at high S)
        const double qPeak = juce::jlimit (0.5, 2.0, juce::jmap ((double) shelfShapeS, 0.25, 1.50, 0.5, 2.0));
        auto bPeak  = VizEQ::peakingRBJ_Q(Fs, juce::jlimit (20.0f, 20000.0f, scoopFreqValue), scoopValue, qPeak);

        const double w = 2.0 * juce::MathConstants<double>::pi * (double)hz / Fs;
        const float hpDb   = hpNeutral ? 0.0f : (float) bHP.magDB (w);
        const float lpDb   = lpNeutral ? 0.0f : (float) bLP.magDB (w);
        const float airDb  = (float) bAir.magDB(w);
        const float bassDb = (float) bBass.magDB(w);
        const float tiltDb = (float) (bTiltLo.magDB(w) + bTiltHi.magDB(w));
        const float scoopDb= (float) bPeak.magDB(w);

        const float yHP  = toY ((float) VizEQ::softPix (hpDb + lpDb));
        const float yAir = toY ((float) VizEQ::softPix (airDb));
        const float yTlt = toY ((float) VizEQ::softPix (tiltDb));
        const float yBas = toY ((float) VizEQ::softPix (bassDb));
        const float yScp = toY ((float) VizEQ::softPix (scoopDb));

        if (i == 0)
        {
            hpLp .startNewSubPath (x, yHP);
            airP .startNewSubPath (x, yAir);
            tiltP.startNewSubPath (x, yTlt);
            bassP.startNewSubPath (x, yBas);
            scoopP.startNewSubPath (x, yScp);
            // start fill paths along top to draw vertical gradient later
            hpFill .startNewSubPath (x, yHP);
            lpFill .startNewSubPath (x, yHP);
            airFill.startNewSubPath (x, yAir);
            tiltFill.startNewSubPath (x, yTlt);
            bassFill.startNewSubPath (x, yBas);
            scoopFill.startNewSubPath (x, yScp);
        }
        else
        {
            hpLp .lineTo (x, yHP);
            airP .lineTo (x, yAir);
            tiltP.lineTo (x, yTlt);
            bassP.lineTo (x, yBas);
            scoopP.lineTo (x, yScp);
            hpFill .lineTo (x, yHP);
            lpFill .lineTo (x, yHP);
            airFill.lineTo (x, yAir);
            tiltFill.lineTo (x, yTlt);
            bassFill.lineTo (x, yBas);
            scoopFill.lineTo (x, yScp);
        }
    }

    auto stroke = [&] (const juce::Path& path, juce::Colour base)
    {
        g.setColour (base.withAlpha (0.12f)); g.strokePath (path, juce::PathStrokeType (10.0f));
        g.setColour (base.withAlpha (0.28f)); g.strokePath (path, juce::PathStrokeType (5.0f));
        g.setColour (base.withAlpha (0.95f)); g.strokePath (path, juce::PathStrokeType (2.0f));
    };

    // Close fills to bottom for gradient area and draw subtle vertical gradients per-curve
    auto fillGradient = [&] (juce::Path& topPath, juce::Colour base)
    {
        juce::Path fill = topPath;
        fill.lineTo (b.getRight(), b.getBottom());
        fill.lineTo (b.getX(),     b.getBottom());
        fill.closeSubPath();
        juce::ColourGradient grad (base.withAlpha (0.25f), b.getX(), b.getY(), base.withAlpha (0.02f), b.getX(), b.getBottom(), false);
        g.setGradientFill (grad);
        g.fillPath (fill);
    };

    fillGradient (hpFill,  hpLpCol);
    fillGradient (bassFill,bassCol);
    fillGradient (airFill, airCol);
    fillGradient (tiltFill,tiltCol);
    fillGradient (scoopFill,scoopCol);

    stroke (hpLp,  hpLpCol);
    stroke (bassP, bassCol);
    stroke (airP,  airCol);
    // dashed tilt
    {
        juce::Path dashed;
        const float dashes[] = { 6.0f, 4.0f };
        juce::PathStrokeType (2.0f).createDashedStroke (dashed, tiltP, dashes, 2);
        g.setColour (tiltCol.withAlpha (0.12f)); g.strokePath (dashed, juce::PathStrokeType (10.0f));
        g.setColour (tiltCol.withAlpha (0.25f)); g.strokePath (dashed, juce::PathStrokeType (5.0f));
        g.setColour (tiltCol.withAlpha (0.95f)); g.strokePath (dashed, juce::PathStrokeType (2.0f));
    }
    stroke (scoopP, scoopCol);

    // Mono cutoff visualization (filter-accurate shading with stronger distinction and side-curve)
    if (monoHzValue > 20.0f)
    {
        auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        const auto baseEq = lf2 ? lf2->theme.eq.hp : juce::Colours::lightblue;

        const float minHz = 20.0f, maxHz = 20000.0f;
        const float Fc = juce::jlimit (minHz, maxHz, monoHzValue);

        // Slope order from 6/12/24 -> 1/2/4
        const int order = (monoSlopeDbPerOct <= 6 ? 1 : monoSlopeDbPerOct <= 12 ? 2 : 4);
        const juce::Colour tint = (order == 1 ? baseEq.brighter (0.25f)
                                              : order == 2 ? baseEq
                                                           : baseEq.darker (0.25f));

        auto xAtHz = [&] (float hz)
        {
            const float t = (float) (std::log10 (hz / minHz) / std::log10 (maxHz / minHz));
            return juce::jmap (juce::jlimit (0.0f, 1.0f, t), 0.0f, 1.0f, b.getX(), b.getRight());
        };

        // Shade by Butterworth magnitude: |H(jw)| = 1/sqrt(1+(w/wc)^(2N))
        // Increase distinction between orders by non-linear alpha mapping
        const int cols = juce::jmax (192, (int) b.getWidth());
        juce::Path sideCurve; bool sideStarted = false;
        for (int i = 0; i < cols; ++i)
        {
            const float t01 = (float) i / (float) (cols - 1);
            const float hz  = 20.0f * std::pow (1000.0f, t01 * 3.0f);
            const float ratio = hz / Fc;
            const float mag = 1.0f / std::sqrt (1.0f + std::pow (juce::jmax (ratio, 1.0e-6f), (float) (2 * order)));

            // Mono weight ~ |H_lp|. Use exponent and scaling per-order to exaggerate visual separation
            const float monoWeight = mag; // 0..1
            const float shape = (order == 1 ? 0.85f : order == 2 ? 1.10f : 1.45f);
            const float alpha = juce::jlimit (0.0f, 1.0f, 0.06f + 0.70f * std::pow (monoWeight, shape));

            const float x = xAtHz (hz);
            g.setColour (tint.withAlpha (alpha * (hz <= Fc ? 0.85f : 0.6f)));
            g.fillRect (juce::Rectangle<float> (x, b.getY(), 2.0f, b.getHeight()));

            // Optional dashed curve: stereo width multiplier ~ |1 - H_lp|
            const float sideWeight = juce::jlimit (0.0f, 1.0f, 1.0f - mag);
            const float y = b.getY() + (1.0f - sideWeight) * b.getHeight();
            if (!sideStarted) { sideCurve.startNewSubPath (x, y); sideStarted = true; }
            else              { sideCurve.lineTo (x, y); }
        }

        const float xFc = xAtHz (Fc);
        g.setColour (tint.withAlpha (0.80f));
        g.drawLine (xFc, b.getY(), xFc, b.getBottom(), 1.4f);

        // Draw dashed side curve on top
        {
            juce::Path dashed;
            const float dashes[] = { 6.0f, 4.0f };
            juce::PathStrokeType (2.0f).createDashedStroke (dashed, sideCurve, dashes, 2);
            g.setColour (tint.withAlpha (0.55f));
            g.strokePath (dashed, juce::PathStrokeType (1.8f));
        }
    }

    // dB scale labels on left (match curve pixel mapping using softPix)
    {
        g.setColour ((lf ? lf->theme.textMuted : juce::Colours::lightgrey).withAlpha (0.8f));
        g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
        const float dBs[] = { +18.0f, +12.0f, +6.0f, 0.0f, -6.0f, -12.0f, -18.0f };
        for (float d : dBs)
        {
            // Use the same visual mapping as curves so peaks align with tick longitudes
            const float y = toY ((float) VizEQ::softPix ((double) d, 6.0, 18.0));
            g.drawText (juce::String ((int) d) + " dB", juce::Rectangle<int> ((int) b.getX() - 44, (int) (y - 7), 40, 14), juce::Justification::centredRight);
            // small tick
            g.fillRect (juce::Rectangle<float> (b.getX() - 6.0f, y - 0.5f, 4.0f, 1.0f));
        }
    }
}

void XYPad::drawBalls (juce::Graphics& g, juce::Rectangle<float> b)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto accent = lf ? lf->theme.accent : juce::Colours::lightblue;
    const auto text   = lf ? lf->theme.text   : juce::Colours::white;

    const float r = 8.0f;
    const float cx = b.getX() + pt.first * b.getWidth();
    const float cy = b.getY() + (1.0f - pt.second) * b.getHeight();

    if (!isSplitMode)
    {
        g.setColour ((lf ? lf->theme.shadowDark : juce::Colours::black).withAlpha (0.4f));
        g.fillEllipse (cx - r + 2.0f, cy - r + 2.0f, r * 2.0f, r * 2.0f);
        g.setColour (accent);
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);
        g.setColour (text.withAlpha (0.7f));
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.2f);
        // Reverb depth rings: subtle expanding rings based on spaceValue
        if (spaceValue > 0.001f)
        {
            const float maxRadius = r * (1.0f + 1.5f * spaceValue);
            const int rings = 3;
            for (int i = 1; i <= rings; ++i)
            {
                const float t = (float) i / (float) rings;
                const float rr = juce::jmap (t, 0.0f, 1.0f, r * 1.2f, maxRadius);
                g.setColour (accent.withAlpha (0.18f * (1.0f - t)));
                g.drawEllipse (cx - rr, cy - rr, rr * 2.0f, rr * 2.0f, 1.2f);
            }
        }
        return;
    }

    // Split mode: left/right balls
    const float lx = b.getX() + leftPt  * b.getWidth();
    const float rx = b.getX() + rightPt * b.getWidth();
    const float y  = cy;

    g.setColour ((lf ? lf->theme.shadowDark : juce::Colours::black).withAlpha (0.4f));
    g.setColour ((lf ? lf->theme.shadowDark : juce::Colours::black).withAlpha (0.4f));
    g.fillEllipse (lx - r + 2.0f, y - r + 2.0f, r * 2.0f, r * 2.0f);
    g.fillEllipse (rx - r + 2.0f, y - r + 2.0f, r * 2.0f, r * 2.0f);

    g.setColour (accent);
    g.fillEllipse (lx - r, y - r, r * 2.0f, r * 2.0f);
    g.setColour (text.withAlpha (0.7f));
    g.fillEllipse (rx - r, y - r, r * 2.0f, r * 2.0f);

    g.setColour (text.withAlpha (0.7f));
    g.drawEllipse (lx - r, y - r, r * 2.0f, r * 2.0f, 1.2f);
    g.drawEllipse (rx - r, y - r, r * 2.0f, r * 2.0f, 1.2f);
}
// ------------------------------------------------
/* ===================== Editor ===================== */
MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor (MyPluginAudioProcessor& p)
: AudioProcessorEditor (&p), proc (p), presetManager (proc.apvts, nullptr), bypassButton(lnf)
{
    // Log: Editor constructor started
    // TEMPORARILY DISABLE file logging to test if this is causing the crash
    // juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    // f.appendText("Editor Ctor: STARTED\n", false, false, "\n");
    
    // Initialize components using dedicated methods
    initializePresetSystem();
    initializeManagers();
    initializeSizeConstraints();
    initializeUIComponents();
    initializeButtonCallbacks();
    initializeParameterAttachments();
    finalizeInitialization();

    // Constructor now uses dedicated initialization methods
}

// Initialization Methods
void MyPluginAudioProcessorEditor::initializePresetSystem()
{
    // Populate factory presets directly from app-data JSON (single source of truth)
    {
        const auto presetsFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                  .getChildFile ("Field/Presets/delay_presets.json");
        if (presetsFile.existsAsFile())
        {
            const auto text = presetsFile.loadFileAsString();
            const auto root = juce::JSON::parse (text);
            auto importOne = [this](const juce::var& v)
            {
                if (! v.isObject()) return;
                if (auto* obj = v.getDynamicObject())
                {
                    LibraryPreset pr;
                    pr.meta.name        = obj->getProperty ("name").toString();
                    pr.meta.description = obj->getProperty ("desc").toString();
                    pr.meta.hint        = obj->getProperty ("hint").toString();
                    pr.meta.author      = "Factory";
                    pr.meta.category    = "Delay";
                    if (auto tags = obj->getProperty ("tags"); tags.isArray())
                        for (auto& t : *tags.getArray()) pr.meta.tags.add (t.toString());
                    if (auto params = obj->getProperty ("params"); params.isObject())
                        if (auto* pv = params.getDynamicObject())
                            for (auto& kv : pv->getProperties())
                                pr.params.set (kv.name.toString(), kv.value);
                    if (pr.meta.name.isNotEmpty()) presetStore.addFactoryPreset (pr);
                }
            };
            if (root.isArray())
            {
                for (const auto& it : *root.getArray()) importOne (it);
            }
            else if (auto* d = root.getDynamicObject())
            {
                auto arr = d->getProperty ("presets");
                if (arr.isArray()) for (const auto& it : *arr.getArray()) importOne (it);
            }
        }
    }
    presetStore.scan();
    DBG("[PresetStore] after add+scan: " << presetStore.getAll().size() << " presets");
}

void MyPluginAudioProcessorEditor::initializeManagers()
{
    // Initialize managers
    layoutManager = std::make_unique<LayoutManager>(*this);
    eventManager = std::make_unique<EventManager>(*this);
    attachmentManager = std::make_unique<AttachmentManager>(*this);
}

void MyPluginAudioProcessorEditor::initializeSizeConstraints()
{
    // Build knob cells once after all sliders/labels are created
    buildCells();
    
    // Calculate minimum size based on layout requirements
    const float s = 1.0f;
    const int lPx  = Layout::dp ((float) Layout::knobPx (Layout::Knob::L),  s);
    const int xlPx = Layout::dp ((float) Layout::knobPx (Layout::Knob::XL), s);
    const int swW  = Layout::dp ((int) (Layout::ALGO_SWITCH_W * Layout::ALGO_SWITCH_W_RATIO), s);
    // Calculate minimum width needed for all controls
    const int numItems = 1 + 1 + 1 + 5 + 3; // pan, space, switch, duck(5), gain/drive/mix(3)
    const int gaps = numItems - 1;
    const int gapS = Layout::dp (Layout::GAP_S, s);
    // Compute minimum delay card width based on 7 columns using same cell width and inner gap
    const int cellW_delay_min = lPx + Layout::dp (8, s);
    const int delayColsMin = 7;
    const int delayCardWMin = delayColsMin * cellW_delay_min + gapS * (delayColsMin - 1) + Layout::dp (Layout::GAP, s);
    // Compute minimum motion grid width (second divider + gap + 4 motion cells)
    const int motionColsMin = 4;
    const int motionCellWMin = lPx + Layout::dp (8, s);
    const int motionDividerWMin = Layout::dp (8, s);
    const int motionAreaWMin = motionDividerWMin + Layout::dp (Layout::GAP, s) + motionColsMin * motionCellWMin;
    const int calculatedMinWidth = xlPx + lPx + swW + 5*lPx + 3*lPx + gaps * gapS
                                   + Layout::dp (Layout::PAD, s) * 2
                                   + delayCardWMin + Layout::dp (Layout::GAP, s)
                                   + motionAreaWMin;
    
    // Calculate minimum height based on current architecture: visuals + 2x16 controls
    const int headerH = Layout::dp (50, s);
    const int xyMinH = Layout::dp (Layout::XY_MIN_H, s);
    const int metersH = Layout::dp (84, s);
    const int bottomReserveMin = Layout::dp (6, s) + Layout::dp (22, s);
    const int gapH = Layout::dp (Layout::GAP, s);
    const auto gridMetrics = ControlGridMetrics::compute (baseWidth, baseHeight);
    const int controlsHMin = gridMetrics.controlsH; // 2 rows of the flat grid
    const int calculatedMinHeight = headerH + juce::jmax (xyMinH, metersH) + gapH + controlsHMin + Layout::dp (Layout::PAD, s) + bottomReserveMin;
    
    // Store resize constraints
    // Allow some narrowing vs content width, but never below a conservative floor
    const int minWidthAllowed = juce::jmax (800, (int) std::round ((float) baseWidth * 0.5f));
    const int minWidthFloor   = Layout::BP_WIDE; // breakpoint for wide layouts (protects 4x16 flats)
    const int proposedMinW    = juce::jmin (calculatedMinWidth, minWidthAllowed);
    this->minWidth = juce::jmax (minWidthFloor, proposedMinW);
    this->minHeight = calculatedMinHeight;
    this->maxWidth = 3000;
    this->maxHeight = 2000;
    
    // Set minimum size constraints
    setResizeLimits (minWidth, minHeight, maxWidth, maxHeight);
    
    // Set initial size (respecting minimums). Prefer baseWidth over full content width.
    const int initialWidth = juce::jmax (baseWidth, minWidth);
    const int initialHeight = juce::jmax (baseHeight, calculatedMinHeight);
    setSize (initialWidth, initialHeight);
}

void MyPluginAudioProcessorEditor::initializeUIComponents()
{
    // Initial layout deferred until layoutReady is true
    // Defer one tick to ensure LookAndFeel and attachments settle, then repaint
    juce::MessageManager::callAsync ([this]
    {
        // OLD REVERB SYSTEM REMOVED
        // OLD REVERB SYSTEM REMOVED
        repaint();
    });
    lnf.theme.accent = juce::Colour (0xFF5AA9E6); // ocean default
    lnf.setupColours();
    setLookAndFeel (&lnf);

    // Drive editor heartbeat at 30 Hz (UI updates throttled within timer)
    startTimerHz (30);
    uiTimerHzCurrent = 30;
    addMouseListener (this, true); // receive events from all children
}

void MyPluginAudioProcessorEditor::initializeButtonCallbacks()
{
    // Options menu (oversampling) with per-mode tint
    addAndMakeVisible (optionsButton);
    optionsButton.onClick = [this]
    {
        const bool wasOn = optionsButton.getToggleState();
        optionsButton.setToggleState (true, juce::dontSendNotification);
        optionsButton.repaint();

        TintMenuLNFEx menuLnf; menuLnf.defaultTint = lnf.theme.accent; menuLnf.hideChecks = true;
        menuLnf.setColour (juce::PopupMenu::textColourId, lnf.theme.text);

        int curIdx = 0, numChoices = 1;
        if (auto* rp = proc.apvts.getParameter ("os_mode"))
            if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp)) { curIdx = cp->getIndex(); numChoices = cp->choices.size(); }

        showTintedMenu (optionsButton, menuLnf,
            // BUILD
            [this, curIdx, numChoices] (juce::PopupMenu& m, TintMenuLNFEx& lnfEx)
            {
                m.addSectionHeader ("Oversampling");
                struct Row { int id; const char* text; juce::Colour tint; bool enabled; };
                juce::Array<Row> rows;
                rows.add ({ 1, "1x (Off)",          lnf.theme.textMuted, true });
                rows.add ({ 2, "2x (High Quality)", lnf.theme.eq.bass.withAlpha (0.95f),   numChoices > 1 });   // vibrant green
                rows.add ({ 3, "4x (Ultra)",        lnf.theme.accent.withHue   (lnf.theme.accent.getHue() + 0.08f).withSaturation (0.9f), numChoices > 2 });
                rows.add ({ 4, "8x (Max)",          lnf.theme.accent.withHue   (lnf.theme.accent.getHue() - 0.08f).withBrightness (0.95f), numChoices > 3 });
                rows.add ({ 5, "16x (Extreme)",     lnf.theme.eq.scoop.withAlpha (0.95f), numChoices > 4 });    // bold magenta/plum

                lnfEx.itemTints.clear();
                for (int i = 0; i < rows.size(); ++i)
                {
                    m.addItem (rows[i].id, rows[i].text, rows[i].enabled, i == curIdx);
                    lnfEx.itemTints.add (rows[i].tint);
                }
            },
            // RESULT
            [this, wasOn, numChoices] (int r)
            {
                // restore button active state to real OS>1×
                if (auto* rp = proc.apvts.getParameter ("os_mode"))
                    if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp))
                        optionsButton.setToggleState (cp->getIndex() > 0, juce::dontSendNotification);
                    else optionsButton.setToggleState (wasOn, juce::dontSendNotification);
                optionsButton.repaint();

                if (r == 0) return;
                if (r < 1 || r > numChoices) return;
                if (auto* p = proc.apvts.getParameter ("os_mode"))
                {
                    const float norm = numChoices > 1 ? (float) (r - 1) / (float) (numChoices - 1) : 0.0f;
                    p->beginChangeGesture(); p->setValueNotifyingHost (norm); p->endChangeGesture();
                }
            });
    };

    // Tint Options button based on oversampling choice
    auto applyOptionsTint = [this]
    {
        int sel = 0;
        if (auto* rp = proc.apvts.getParameter ("os_mode"))
            if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (rp)) sel = cp->getIndex();
        // Map index to label and tint
        juce::Colour tint = lnf.theme.textMuted;
        juce::String label = "1x";
        switch (sel)
        {
            case 0: tint = lnf.theme.textMuted; label = "1x"; break;
            case 1: tint = lnf.theme.eq.bass.withAlpha (0.95f); label = "2x"; break;
            case 2: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.08f).withSaturation (0.9f); label = "4x"; break;
            case 3: tint = lnf.theme.accent.withHue (lnf.theme.accent.getHue() - 0.08f).withBrightness (0.95f); label = "8x"; break;
            case 4: tint = lnf.theme.eq.scoop.withAlpha (0.95f); label = "16x"; break;
        }
        optionsButton.getProperties().set ("accentOverrideARGB", (int) tint.getARGB());
        optionsButton.getProperties().set ("iconOverrideARGB", (int) tint.getARGB());
        optionsButton.getProperties().set ("labelText", label);
        optionsButton.setToggleState (true, juce::dontSendNotification);
        optionsButton.repaint();
    };
    applyOptionsTint();
    osSelect.onChange = [this, applyOptionsTint]
    {
        applyOptionsTint();
    };
    // Also listen to APVTS os_mode directly to avoid drift
    if (!osModeParamAttach)
    {
        if (auto* p = proc.apvts.getParameter ("os_mode"))
        {
            osModeParamAttach = std::make_unique<juce::ParameterAttachment>(*p, [applyOptionsTint](float){ applyOptionsTint(); }, nullptr);
        }
    }

    // Help button → FAQ dialog
    addAndMakeVisible (helpButton);
    helpButton.onClick = [this]
    {
        struct HelpFAQComponent : public juce::Component
        {
            HelpFAQComponent (FieldLNF& l) : lnf(l)
            {
                addAndMakeVisible (text);
                text.setReadOnly (true);
                text.setMultiLine (true);
                text.setScrollbarsShown (true);
                text.setCaretVisible (false);
                text.setFont (juce::Font (juce::FontOptions (14.0f)));
                text.setText (
                    "FIELD — FAQ\n\n"
                    "Q: How do I change color modes?\n"
                    "A: Click the palette button in the header to cycle Ocean → Green → Pink → Yellow → Grey.\n\n"
                    "Q: Where are colors defined?\n"
                    "A: All colors live in FieldLookAndFeel (FieldLNF::theme). Components never hardcode colors.\n\n"
                    "Q: Why don't knobs move when I resize?\n"
                    "A: Sizing happens in resized() only; layout is responsive via Layout::dp().\n\n"
                    "Q: How do I reset a control?\n"
                    "A: Double-click most knobs/sliders to reset to default.\n\n"
                    "Q: Where are presets saved?\n"
                    "A: In your user data folder under the plugin's presets directory.\n\n"
                );
            }
            void resized() override
            {
                text.setBounds (getLocalBounds().reduced (12));
            }
            void paint (juce::Graphics& g) override
            {
                g.fillAll (lnf.theme.panel);
                g.setColour (lnf.theme.sh);
                g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 6.0f, 1.0f);
            }
            juce::TextEditor text;
            FieldLNF& lnf;
        };

        auto* content = new HelpFAQComponent (lnf);
        content->setSize (600, 400);

        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned (content);
        opts.dialogTitle = "Help / FAQ";
        opts.componentToCentreAround = this;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        (void) opts.launchAsync();
    };

    // Bypass (attach to param if present)
    addAndMakeVisible (bypassButton);
    bypassButton.onClick = [this]
    {
        if (auto* p = proc.apvts.getParameter ("bypass"))
            p->setValueNotifyingHost (bypassButton.getToggleState() ? 1.0f : 0.0f);
    };

    // History panel removed

    // Header Undo/Redo removed

    // Color mode cycle (Ocean → Green → Pink → Yellow → Grey)
    addAndMakeVisible (colorModeButton);
    addAndMakeVisible (tooltipsButton);
    colorModeButton.setTooltip (ThemeManager::getThemeName (lnf.currentVariant));
    colorModeButton.onClick = [this]
    {
        // Determine current by accent; rotate deterministically through variants
        using TV = ThemeVariant;
        static ThemeVariant order[] = { ThemeVariant::Ocean, ThemeVariant::Green, ThemeVariant::Pink, ThemeVariant::Yellow, ThemeVariant::Grey };
        auto currentAccent = lnf.theme.accent.getARGB();
        int idx = 0;
        if (currentAccent == juce::Colour (0xFF5AA9E6).getARGB()) idx = 0; // Ocean
        else if (currentAccent == juce::Colour (0xFF5AA95A).getARGB()) idx = 1; // Green
        else if (currentAccent == juce::Colour (0xFFE91E63).getARGB()) idx = 2; // Pink
        else if (currentAccent == juce::Colour (0xFFFFC107).getARGB()) idx = 3; // Yellow
        else if (currentAccent == juce::Colour (0xFF9EA3AA).getARGB()) idx = 4; // Grey
        idx = (idx + 1) % 5;
        lnf.setTheme (order[idx]);
        colorModeButton.setTooltip (ThemeManager::getThemeName (order[idx]));
        // Propagate to components that cache green flag
        const bool greenNow = (order[idx] == ThemeVariant::Green);
        // OLD REVERB SYSTEM REMOVED
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setGreenMode(greenNow);
        }
        repaint();
    };

    tooltipsButton.setTooltip ("Tooltip Assistant");
    tooltipsButton.setToggleState (tooltipAssistantOn_, juce::dontSendNotification);
    // Tooltip button logic delegated to EventManager
    tooltipsButton.onClick = [this] { /* Delegated to EventManager */ };

    // Full screen (top-level window kiosk toggle; restore original bounds)
    addAndMakeVisible (fullScreenButton);
    fullScreenButton.onClick = [this]
    {
        const bool on = fullScreenButton.getToggleState();

        if (auto* tlw = getTopLevelComponent())
        {
            if (auto* rw = dynamic_cast<juce::ResizableWindow*>(tlw))
            {
                if (on)
                {
                    // Save current window bounds to restore later
                    savedBounds = rw->getBounds();
                    rw->setFullScreen (true);
                }
                else
                {
                    rw->setFullScreen (false);
                    if (!savedBounds.isEmpty())
                        rw->setBounds (savedBounds);
                }
                return;
            }
        }

        // Fallback: if no top-level resizable window is accessible, do nothing to avoid bad states
        // Reset the toggle to off if we couldn't enter fullscreen safely
        if (on)
            fullScreenButton.setToggleState (false, juce::dontSendNotification);
    };

    // Link + Snap
    addAndMakeVisible (linkButton);
    linkButton.onClick = [this]
    {
        linkButton.setToggleState (!linkButton.getToggleState(), juce::dontSendNotification);
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setLinked(linkButton.getToggleState());
        }
    };
    addAndMakeVisible (snapButton);
    snapButton.setToggleState (false, juce::dontSendNotification); // default OFF per your note
    snapButton.onClick = [this]
    {
        const bool on = !snapButton.getToggleState();
        snapButton.setToggleState (on, juce::dontSendNotification);
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setSnapEnabled(on);
        }
    };

    // Presets UI
    // Preset field + arrows
    addAndMakeVisible (presetField);
    presetField.setButtonText ("Search presets…");
    presetField.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetField.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (prevPresetButton);
    addAndMakeVisible (nextPresetButton);

    prevPresetButton.onClick = [this]
    {
        // no-op: legacy preset combo removed
    };
    nextPresetButton.onClick = [this]
    {
        // no-op: legacy preset combo removed
    };
    // Open command-palette when clicking the preset field; arrows remain for prev/next only
    presetField.onClick = [this]
    {
        static PresetRegistry presetRegistry; // lifetime across openings
        presetRegistry.reloadAll();
        PresetCommandPalette::show(
            presetRegistry, presetField,
            // Apply
            [this](const PresetEntry& e){
                // Convert NamedValueSet to APVTS via PresetManager
                LibraryPreset tmp; tmp.meta.id = e.id; tmp.meta.name = e.name; tmp.params = e.params;
                presetManager.applyPresetAtomic (tmp);
                presetNameLabel.setText (e.name, juce::dontSendNotification);
            },
            // Load to slot
            [this](const PresetEntry& e, bool toA){ LibraryPreset tmp; tmp.params = e.params; presetManager.loadToSlot (tmp, toA); },
            // Star (persist favorite)
            [reg=&presetRegistry](const PresetEntry& e, bool fav){ reg->setFavorite (e.id, fav); },
            // Save As
            [this](juce::String name, juce::StringArray tags, juce::String cat){ auto pr = presetManager.currentAsPreset (name, cat, tags, "User preset", "", "You"); presetStore.saveUserPreset (pr); presetStore.scan(); },
            presetField.getButtonText()
        );
    };

    // Removed savePresetButton handler
    // A/B + copy
    addAndMakeVisible (abButtonA);
    addAndMakeVisible (abButtonB);
    addAndMakeVisible (copyButton);
    abButtonA.setToggleState (true, juce::dontSendNotification);
    abButtonB.setToggleState (false, juce::dontSendNotification);
    abButtonA.onClick = [this]{ if (!abButtonA.getToggleState()) toggleABState(); };
    abButtonB.onClick = [this]{ if (!abButtonB.getToggleState()) toggleABState(); };
    
    // Set metallic properties for A/B buttons to enable Button Switch styling
    setAreaMetallicForCell (abButtonA, MetallicKind::Band);
    setAreaMetallicForCell (abButtonB, MetallicKind::Band);
    copyButton.onClick = [this]
    {
        juce::PopupMenu m; m.addItem (1, "Copy A to B"); m.addItem (2, "Copy B to A");
        m.showMenuAsync (juce::PopupMenu::Options(), [this](int r)
        {
            if (r == 1) { copyState (true);  pasteState (false); }
            if (r == 2) { copyState (false); pasteState (true);  }
        });
    };

    // Split toggle
    addAndMakeVisible (splitToggle);
    splitToggle.onToggleChange = [this] (bool split)
    {
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setSplitMode(split);
        }
        linkButton.setVisible (split);
        panKnob.setVisible (!split);
        panValue.setVisible (!split);
        panKnobLeft .setVisible (split);
        panKnobRight.setVisible (split);
        panValueLeft .setVisible (split);
        panValueRight.setVisible (split);
        resized();
    };
    splitToggle.setToggleState (false, juce::dontSendNotification);
    linkButton.setVisible (false);
    // Multi-pane dock (XY, Dynamic EQ, Imager) + shade overlay
    panes = std::make_unique<PaneManager> (proc, proc.apvts.state, &lnf);
    addAndMakeVisible (panes.get());
    panes->setSampleRate (proc.getSampleRate());
    // keep-warm removed; no pane warm-up needed
    // Default to XY view on startup
    panes->setActive (PaneID::XY, true);
    // Spectrum removed; Dynamic EQ pane owns its analyzer styling

    // xyShade functionality removed - handled by PaneManager

    // Add meter components
    addAndMakeVisible (corrMeter);
    addAndMakeVisible (lrMeters);
    addAndMakeVisible (ioMeters);

    // Containers
    addAndMakeVisible (mainControlsContainer); mainControlsContainer.setTitle (""); mainControlsContainer.setShowBorder (false);
    addAndMakeVisible (panKnobContainer);      panKnobContainer.setTitle ("");     panKnobContainer.setShowBorder (true);
    addAndMakeVisible (volumeContainer);       volumeContainer.setTitle ("");   volumeContainer.setShowBorder (true);
    // Delay container will be removed; lay out directly on right side
    // addAndMakeVisible (delayContainer);        delayContainer.setTitle ("");     delayContainer.setShowBorder (true);
    // Row containers for EQ/Image are no longer used
    addAndMakeVisible (MainContentContainer);  MainContentContainer.setTitle ("");    MainContentContainer.setShowBorder (false);
    addAndMakeVisible (rightSlidersContainer);   rightSlidersContainer.setTitle ("");     rightSlidersContainer.setShowBorder (false);
    addAndMakeVisible (metersContainer);       metersContainer.setTitle ("");         metersContainer.setShowBorder (false);
    
    
    // Add 3D vertical sliders to rightSlidersContainer
    rightSlidersContainer.addAndMakeVisible (inputSlider);
    rightSlidersContainer.addAndMakeVisible (outputSlider);
    rightSlidersContainer.addAndMakeVisible (mixSlider);
    
    
    
    // Configure sliders (ranges and values already set in VerticalSlider3D constructor)
    inputSlider.setTextValueSuffix (" dB");
    inputSlider.setLookAndFeel (&lnf);
    
    outputSlider.setTextValueSuffix (" dB");
    outputSlider.setLookAndFeel (&lnf);
    
    mixSlider.setRange (0.0, 100.0, 0.1);  // Mix slider has different range
    mixSlider.setValue (100.0);
    mixSlider.setTextValueSuffix (" %");
    mixSlider.setLookAndFeel (&lnf);
    
    // Add small units display to sliders
    inputSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    outputSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 15);
    
    // Set component names for handle labels
    inputSlider.setName ("inputSlider");
    outputSlider.setName ("outputSlider");
    mixSlider.setName ("mixSlider");
    
    // Style labels with knobcell color background
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto sh = lf->theme.meters.panelDark;
    
    auto labelStyle = [&](juce::Label& label) {
        label.setColour (juce::Label::backgroundColourId, sh.withAlpha (0.15f));
        label.setColour (juce::Label::textColourId, juce::Colours::white);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (10.0f, juce::Font::bold));
    };
    

    // Width group (image row, bottom-right): invisible container + placeholder slots for spanning grid
    addChildComponent (widthGroupContainer);
    widthGroupContainer.setTitle ("");
    widthGroupContainer.setShowBorder (true);
    widthGroupContainer.setVisible (false);
    widthGroupContainer.setInterceptsMouseClicks (false, false);
    
    // Gain+Drive+Mix group (volume row): invisible container to horizontally arrange GAIN/DRIVE/MIX (all XL)
    addChildComponent (gainMixGroupContainer);
    gainMixGroupContainer.setTitle ("");
    gainMixGroupContainer.setShowBorder (false);
    gainMixGroupContainer.setVisible (false);
    gainMixGroupContainer.setInterceptsMouseClicks (false, false);
    addChildComponent (gainMixSlot1); gainMixSlot1.setInterceptsMouseClicks (false, false); gainMixSlot1.setVisible (false);
    addChildComponent (gainMixSlot2); gainMixSlot2.setInterceptsMouseClicks (false, false); gainMixSlot2.setVisible (false);

    // Ducking group (Depth, Attack, Release, Threshold) – invisible container
    addChildComponent (duckGroupContainer);
    duckGroupContainer.setTitle ("");
    duckGroupContainer.setShowBorder (false);
    duckGroupContainer.setVisible (false);
    duckGroupContainer.setInterceptsMouseClicks (false, false);
    addChildComponent (duckSlot1); duckSlot1.setInterceptsMouseClicks (false, false); duckSlot1.setVisible (false);
    addChildComponent (duckSlot2); duckSlot2.setInterceptsMouseClicks (false, false); duckSlot2.setVisible (false);
    addChildComponent (duckSlot3); duckSlot3.setInterceptsMouseClicks (false, false); duckSlot3.setVisible (false);
    addChildComponent (widthGroupSlot1); widthGroupSlot1.setInterceptsMouseClicks (false, false); widthGroupSlot1.setVisible (false);
    addChildComponent (widthGroupSlot2); widthGroupSlot2.setInterceptsMouseClicks (false, false); widthGroupSlot2.setVisible (false);
    addChildComponent (widthGroupSlot3); widthGroupSlot3.setInterceptsMouseClicks (false, false); widthGroupSlot3.setVisible (false);

    // Volume row unified group (pan, reverb, algo switch, duck group, gain/drive/mix)
    addChildComponent (volGroupContainer);
    volGroupContainer.setTitle("");
    volGroupContainer.setShowBorder(false);
    volGroupContainer.setVisible(false);
    volGroupContainer.setInterceptsMouseClicks(false, false);
    addChildComponent (volSlot1); volSlot1.setInterceptsMouseClicks(false,false); volSlot1.setVisible(false);
    addChildComponent (volSlot2); volSlot2.setInterceptsMouseClicks(false,false); volSlot2.setVisible(false);
    addChildComponent (volSlot3); volSlot3.setInterceptsMouseClicks(false,false); volSlot3.setVisible(false);
    addChildComponent (volSlot4); volSlot4.setInterceptsMouseClicks(false,false); volSlot4.setVisible(false);
    addChildComponent (volSlot5); volSlot5.setInterceptsMouseClicks(false,false); volSlot5.setVisible(false);
    addChildComponent (volSlot6); volSlot6.setInterceptsMouseClicks(false,false); volSlot6.setVisible(false);
    addChildComponent (volSlot7); volSlot7.setInterceptsMouseClicks(false,false); volSlot7.setVisible(false);

    // Split-pan overlay container
    addAndMakeVisible (panSplitContainer);
    panSplitContainer.setVisible (false);
    panSplitContainer.setInterceptsMouseClicks (false, false);

    // Sliders/knobs
    // NOTE: These names/IDs match your processor (original)
    auto style = [this](juce::Slider& s, bool main = false)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        // Align with FieldLNF ticks: π → π + 2π
        constexpr float kStart = juce::MathConstants<float>::pi;
        constexpr float kEnd   = juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi;
        s.setRotaryParameters (kStart, kEnd, true);
        s.setLookAndFeel (&lnf);
    };

    // main rotary
    for (juce::Slider* slider : { &width,&tilt,&monoHz,&hpHz,&lpHz,&satDrive,&satMix,&air,&bass,&scoop,
                              &widthLo,&widthMid,&widthHi,&xoverLoHz,&xoverHiHz,&rotationDeg,&asymmetry,
                              &shelfShapeS,&filterQ })
    {
        addAndMakeVisible (*slider);
        style (*slider);
        slider->setVelocityBasedMode (true);
        slider->setVelocityModeParameters (0.85, 1, 0.0, true);
        slider->setMouseDragSensitivity (140);
        slider->addListener (this);
    }
    addAndMakeVisible (gain); style (gain); gain.addListener (this);

    // micro sliders (freq) + HP/LP Q minis
    for (juce::Slider* slider : { &tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider, &hpQSlider, &lpQSlider })
    {
        addAndMakeVisible (*slider);
        slider->setSliderStyle (juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider->setMouseDragSensitivity (140);
        slider->setVelocityBasedMode (false);
        slider->setSliderSnapsToMousePosition (false);
        slider->setDoubleClickReturnValue (true, 0.0);
        slider->setLookAndFeel (&lnf);
        slider->addListener (this);
    }

    // link buttons (will be placed inside a cell later)
    addAndMakeVisible (tiltLinkSButton);
    addAndMakeVisible (qLinkButton);
    tiltLinkSButton.setLookAndFeel (&lnf);
    qLinkButton.setLookAndFeel (&lnf);
    tiltLinkSButton.setButtonText ("Tilt uses S");
    qLinkButton.setButtonText (""); // visual icon-only; state-driven paint


    // pan + reverb + ducking
    addAndMakeVisible (panKnob);      style (panKnob, true);     panKnob.setRange (-1.0, 1.0, 0.01); panKnob.setOverlayEnabled (false); panKnob.addListener (this);
    addAndMakeVisible (panKnobLeft);  style (panKnobLeft, true); panKnobLeft.setRange (-1.0, 1.0, 0.01); panKnobLeft.setOverlayEnabled (true);  panKnobLeft.setLabel ("L"); panKnobLeft.addListener (this);
    addAndMakeVisible (panKnobRight); style (panKnobRight, true);panKnobRight.setRange(-1.0, 1.0, 0.01); panKnobRight.setOverlayEnabled (true); panKnobRight.setLabel ("R"); panKnobRight.addListener (this);

    panKnob.setVisible (true);
    panKnobLeft.setVisible (false);
    panKnobRight.setVisible (false);

    // Legacy Group 1 reverb depth knob is no longer used; keep hidden

    // values
    for (juce::Label* l : { &gainValue,&widthValue,&tiltValue,&monoValue,&hpValue,&lpValue,&satDriveValue,&satMixValue,&airValue,&bassValue,&scoopValue,&shelfShapeValue,&filterQValue,
                             &panValue,&panValueLeft,&panValueRight,
                             &tiltFreqValue,&scoopFreqValue,&bassFreqValue,&airFreqValue,
                             &widthLoValue,&widthMidValue,&widthHiValue,&xoverLoValue,&xoverHiValue,
                             &rotationValue,&asymValue,
                             &delayTimeValue,&delayFeedbackValue,&delayWetValue,&delaySpreadValue,&delayWidthValue,&delayModRateValue,&delayModDepthValue,&delayWowflutterValue,&delayJitterValue,
                             &delayHpValue,&delayLpValue,&delayTiltValue,&delaySatValue,&delayDiffusionValue,&delayDiffuseSizeValue,
                             &delayDuckDepthValue,&delayDuckAttackValue,&delayDuckReleaseValue,&delayDuckThresholdValue,&delayDuckRatioValue,&delayDuckLookaheadValue })
    {
        addAndMakeVisible (*l);
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (15.0f * scaleFactor).withStyle ("Bold")));
        l->setColour (juce::Label::textColourId, lnf.theme.text);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    // Live EQ curve updates while turning knobs
    auto addLiveRepaint = [this](juce::Slider& s)
    {
        s.onValueChange = [this]
        {
            if (auto* xyTab = panes->getXYTab()) {
                xyTab->setTiltValue((float) tilt.getValue());
                xyTab->setHPValue((float) hpHz.getValue());
                xyTab->setLPValue((float) lpHz.getValue());
                xyTab->setAirValue((float) air.getValue());
            }
            if (auto* xyTab = panes->getXYTab()) {
                xyTab->setBassValue((float) bass.getValue());
                xyTab->setScoopValue((float) scoop.getValue());
                xyTab->setTiltFreqValue((float) tiltFreqSlider.getValue());
                xyTab->setScoopFreqValue((float) scoopFreqSlider.getValue());
                xyTab->setBassFreqValue((float) bassFreqSlider.getValue());
            }
            if (auto* xyTab = panes->getXYTab()) {
                xyTab->setAirFreqValue((float) airFreqSlider.getValue());
                xyTab->setMonoValue((float) monoHz.getValue());
                xyTab->repaint();
            }
        };
    };
    for (juce::Slider* slider : { &tilt,&hpHz,&lpHz,&air,&bass,&scoop,&tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider,&monoHz,&shelfShapeS,&filterQ,&hpQSlider,&lpQSlider })
    {
        addLiveRepaint (*slider);
        slider->onDragStart = [this]{ proc.setIsEditing (true); };
        slider->onDragEnd   = [this]{ proc.setIsEditing (false); };
    }

    // slider names (for LNF knob labels)
    gain.setName ("GAIN"); width.setName ("WIDTH"); tilt.setName ("TILT"); monoHz.setName ("MONO");
    hpHz.setName ("HP Hz"); lpHz.setName ("LP Hz"); satDrive.setName ("DRIVE"); satMix.setName ("MIX");
    air.setName ("AIR"); bass.setName ("BASS"); scoop.setName ("SCOOP");
    shelfShapeS.setName ("Shape"); filterQ.setName ("Q");
    panKnob.setName ("PAN"); panKnobLeft.setName ("PAN L"); panKnobRight.setName ("PAN R");
    tiltFreqSlider.setName ("TILT F"); scoopFreqSlider.setName ("SCOOP F"); bassFreqSlider.setName ("BASS F"); airFreqSlider.setName ("AIR F");
    // Imaging knob labels
    widthLo.setName ("W LO"); widthMid.setName ("W MID"); widthHi.setName ("W HI");
    xoverLoHz.setName ("XO LO"); xoverHiHz.setName ("XO HI");
    rotationDeg.setName ("ROT"); asymmetry.setName ("ASYM");

    // HP/LP value label precision: integer Hz
    hpHz.setNumDecimalPlacesToDisplay (0);
    lpHz.setNumDecimalPlacesToDisplay (0);

    // Imaging static text labels under knobs (hidden to avoid duplication with value labels)
    for (auto* l : { &widthLoName,&widthMidName,&widthHiName,&xoverLoName,&xoverHiName,&rotationName,&asymName,&shufLoName,&shufHiName,&shufXName })
    {
        l->setVisible (false);
        l->setInterceptsMouseClicks (false, false);
    }
    widthLoName.setText ("", juce::dontSendNotification);
    widthMidName.setText("", juce::dontSendNotification);
    widthHiName.setText ("", juce::dontSendNotification);
    xoverLoName.setText ("", juce::dontSendNotification);
    xoverHiName.setText ("", juce::dontSendNotification);
    rotationName.setText("", juce::dontSendNotification);
    asymName.setText   ("", juce::dontSendNotification);
    shufLoName.setText ("", juce::dontSendNotification);
    shufHiName.setText ("", juce::dontSendNotification);
    shufXName.setText  ("", juce::dontSendNotification);
    // Delay controls initialization
    for (juce::Slider* slider : { &delayTime, &delayFeedback, &delayWet, &delaySpread, &delayWidth, &delayModRate, &delayModDepth, &delayWowflutter, &delayJitter, &delayPreDelay,
                                  &delayHp, &delayLp, &delayTilt, &delaySat, &delayDiffusion, &delayDiffuseSize,
                                  &delayDuckDepth, &delayDuckAttack, &delayDuckRelease, &delayDuckThreshold, &delayDuckRatio, &delayDuckLookahead })
    {
        addAndMakeVisible (*slider);
        style (*slider);
        slider->addListener (this);
    }
    
    // Delay combo boxes
    for (juce::ComboBox* combo : { &delayMode, &delayTimeDiv, &delayDuckSource, &delayGridFlavor, &delayFilterType })
    {
        addAndMakeVisible (*combo);
        combo->setLookAndFeel (&lnf);
        combo->addListener (this);
        // Metallic styling now handled in DelayControlsPane
    }
    
    // Delay toggle buttons
    for (juce::ToggleButton* button : { &delayEnabled, &delaySync, &delayKillDry, &delayFreeze, &delayPingpong, &delayDuckPost, &delayDuckLinkGlobal })
    {
        addAndMakeVisible (*button);
        button->setLookAndFeel (&lnf);
        button->addListener (this);
        // Metallic styling now handled in DelayControlsPane
    }

    // Delay row-1 explicit captions (ensure text visible in SwitchCell)
    delayEnabled.setButtonText (""); delayEnabled.setToggleState (delayEnabled.getToggleState(), juce::dontSendNotification);
    delaySync.setButtonText ("");      delaySync.setToggleState (delaySync.getToggleState(), juce::dontSendNotification);
    delayPingpong.setButtonText (""); delayPingpong.setToggleState (delayPingpong.getToggleState(), juce::dontSendNotification);
    delayFreeze.setButtonText ("");  delayFreeze.setToggleState (delayFreeze.getToggleState(), juce::dontSendNotification);
    delayKillDry.setButtonText (""); delayKillDry.setToggleState (delayKillDry.getToggleState(), juce::dontSendNotification);
    delayMode.setTextWhenNothingSelected ("Mode");
    // Populate Delay Mode items from APVTS choice parameter so popup shows options
    if (auto* delayModeParam = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("delay_mode")))
    {
        delayMode.clear();
        for (int i = 0; i < delayModeParam->choices.size(); ++i)
            delayMode.addItem (delayModeParam->choices[i], i + 1);
        // Reflect current value without firing callbacks; attachment will keep in sync
        delayMode.setSelectedId (delayModeParam->getIndex() + 1, juce::dontSendNotification);
        // Per-item tints (Digital, Analog, Tape)
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            juce::Array<juce::Colour> tints;
            tints.add (lnf.theme.accent.withAlpha (0.85f));                                   // Digital: accent
            tints.add (lnf.theme.eq.tilt.withAlpha (0.90f));                                   // Analog: orange tilt
            tints.add (lnf.theme.eq.bass.withHue (lnf.theme.eq.bass.getHue() - 0.10f));        // Tape: warmer green
            lf->setPopupItemTints (tints);
        }
        delayMode.getProperties().set ("tintedSelected", true);
    }

    // Populate Delay Duck Source items from APVTS choice parameter so popup shows options
    if (auto* duckSrcParam = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("delay_duck_source")))
    {
        delayDuckSource.clear();
        for (int i = 0; i < duckSrcParam->choices.size(); ++i)
            delayDuckSource.addItem (duckSrcParam->choices[i], i + 1);
        delayDuckSource.setSelectedId (duckSrcParam->getIndex() + 1, juce::dontSendNotification);
        // Per-item tints (Pre, Post, External)
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            juce::Array<juce::Colour> tints;
            tints.add (lnf.theme.eq.hp.withAlpha (0.95f));                                     // Pre: HP blue
            tints.add (lnf.theme.accent.withHue (lnf.theme.accent.getHue() + 0.12f));          // Post: cyan shift
            tints.add (lnf.theme.eq.scoop.withAlpha (0.95f));                                  // External: plum
            lf->setPopupItemTints (tints);
        }
        delayDuckSource.getProperties().set ("tintedSelected", true);
    }
    
    // Delay value labels
    for (juce::Label* l : { &delayTimeValue, &delayFeedbackValue, &delayWetValue, &delaySpreadValue, &delayWidthValue, &delayModRateValue, &delayModDepthValue, &delayWowflutterValue, &delayJitterValue, &delayPreDelayValue,
                            &delayHpValue, &delayLpValue, &delayTiltValue, &delaySatValue, &delayDiffusionValue, &delayDiffuseSizeValue,
                            &delayDuckDepthValue, &delayDuckAttackValue, &delayDuckReleaseValue, &delayDuckThresholdValue, &delayDuckRatioValue, &delayDuckLookaheadValue })
    {
        addAndMakeVisible (*l);
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (juce::FontOptions (13.0f * scaleFactor).withStyle ("Bold")));
        l->setColour (juce::Label::textColourId, lnf.theme.text);
        l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    }
    
    // Motion value labels
    // Motion value labels removed - now handled by MotionControlsPane
    
    // Delay name labels
    for (auto* l : { &delayTimeName, &delayFeedbackName, &delayWetName, &delaySpreadName, &delayWidthName, &delayModRateName, &delayModDepthName, &delayWowflutterName, &delayJitterName, &delayPreDelayName,
                     &delayHpName, &delayLpName, &delayTiltName, &delaySatName, &delayDiffusionName, &delayDiffuseSizeName,
                     &delayDuckDepthName, &delayDuckAttackName, &delayDuckReleaseName, &delayDuckThresholdName, &delayDuckRatioName, &delayDuckLookaheadName })
    {
        l->setVisible (false);
        l->setInterceptsMouseClicks (false, false);
        l->setText ("", juce::dontSendNotification);
    }
    
    // Set delay control names
    delayTime.setName ("TIME"); delayFeedback.setName ("FB"); delayWet.setName ("WET"); delaySpread.setName ("SPREAD");
    delayWidth.setName ("WIDTH"); delayModRate.setName ("MOD RATE"); delayModDepth.setName ("MOD DEPTH");
    delayWowflutter.setName ("WOW"); delayJitter.setName ("JITTER");
    delayHp.setName ("HP"); delayLp.setName ("LP"); delayTilt.setName ("TILT"); delaySat.setName ("SAT");
    delayDiffusion.setName ("DIFF"); delayDiffuseSize.setName ("SIZE");
    delayDuckDepth.setName ("DEPTH"); delayDuckAttack.setName ("ATT"); delayDuckRelease.setName ("REL");
    delayDuckThreshold.setName ("THR"); delayDuckRatio.setName ("DUCK RAT"); delayDuckLookahead.setName ("LA");
    delayPreDelay.setName ("PRE");
    
    // Motion control names removed - now handled by MotionControlsPane

    // seed value labels with current values
    sliderValueChanged (&width);
    sliderValueChanged (&tilt);
    sliderValueChanged (&monoHz);
    sliderValueChanged (&hpHz);
    sliderValueChanged (&lpHz);
    sliderValueChanged (&satDrive);
    sliderValueChanged (&satMix);
    sliderValueChanged (&air);
    sliderValueChanged (&bass);
    sliderValueChanged (&scoop);
    sliderValueChanged (&panKnob);
    sliderValueChanged (&panKnobLeft);
    sliderValueChanged (&panKnobRight);
    sliderValueChanged (&tiltFreqSlider);
    sliderValueChanged (&scoopFreqSlider);
    sliderValueChanged (&bassFreqSlider);
    sliderValueChanged (&airFreqSlider);
    sliderValueChanged (&gain);
    updateMutedKnobVisuals();

    // Mono Maker slope & audition controls
    // Keep legacy ComboBox hidden for APVTS attachment; drive it from switch
    addChildComponent (monoSlopeChoice);
    monoSlopeChoice.addItem ("6",  1);
    monoSlopeChoice.addItem ("12", 2);
    monoSlopeChoice.addItem ("24", 3);

    // Center group controls moved to XYControlsPane (complete implementation there)
    if (!monoSlopeSwitch)
        monoSlopeSwitch = std::make_unique<MonoSlopeSwitch>();
    addAndMakeVisible (*monoSlopeSwitch);
    monoSlopeSwitch->onChange = [this](int idx)
    {
        monoSlopeChoice.setSelectedItemIndex (juce::jlimit (0, 2, idx), juce::NotificationType::sendNotification);
    };
    addAndMakeVisible (monoAuditionButton);
    monoAuditionButton.setButtonText ("AUD");

    // Parameter attachments delegated to AttachmentManager
    if (attachmentManager) {
        attachmentManager->attachAllParameters();
    }

    // All children created; allow layout from now on
    layoutReady = true;

    // OLD REVERB SYSTEM REMOVED - Now using new reverb system in Source/reverb/

    // header divider
    addAndMakeVisible (splitDivider);
    splitDivider.setVisible (true);

    // XY callbacks -> AVTS
    auto refreshXYOverlays = [this]
    {
        if (auto* xyTab = panes->getXYTab()) {
            xyTab->setMixValue   (proc.apvts.getRawParameterValue ("sat_mix")->load());
            xyTab->setDriveValue (proc.apvts.getRawParameterValue ("sat_drive_db")->load());
            xyTab->setTiltValue  (proc.apvts.getRawParameterValue ("tilt")->load());
            xyTab->setHPValue    (proc.apvts.getRawParameterValue ("hp_hz")->load());
            xyTab->setLPValue    (proc.apvts.getRawParameterValue ("lp_hz")->load());
            xyTab->setAirValue   (proc.apvts.getRawParameterValue ("air_db")->load());
            xyTab->setBassValue  (proc.apvts.getRawParameterValue ("bass_db")->load());
            xyTab->setScoopValue (proc.apvts.getRawParameterValue ("scoop")->load());
            xyTab->setTiltFreqValue  (proc.apvts.getRawParameterValue ("tilt_freq")->load());
            xyTab->setScoopFreqValue (proc.apvts.getRawParameterValue ("scoop_freq")->load());
            xyTab->setBassFreqValue  (proc.apvts.getRawParameterValue ("bass_freq")->load());
            xyTab->setAirFreqValue   (proc.apvts.getRawParameterValue ("air_freq")->load());
            xyTab->setWidthValue (proc.apvts.getRawParameterValue ("width")->load());
            xyTab->setPanValue   (proc.apvts.getRawParameterValue ("pan")->load());
            xyTab->setGainValue  (proc.apvts.getRawParameterValue ("gain_db")->load());
        }
    };

    if (auto* xyTab = panes->getXYTab()) {
        xyTab->onChange = [this, refreshXYOverlays](float x01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (0.0f); split->endChangeGesture(); }
        if (auto* pan   = proc.apvts.getParameter ("pan"))        { pan  ->beginChangeGesture(); pan  ->setValueNotifyingHost (x01);  pan  ->endChangeGesture(); }
        if (auto* dep   = proc.apvts.getParameter ("depth"))      { dep  ->beginChangeGesture(); dep  ->setValueNotifyingHost (y01);  dep  ->endChangeGesture(); }
        refreshXYOverlays();
    };
        xyTab->onSplitChange = [this, refreshXYOverlays](float l01, float r01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (1.0f); split->endChangeGesture(); }
        if (auto* pL = proc.apvts.getParameter ("pan_l")) { pL->beginChangeGesture(); pL->setValueNotifyingHost (l01); pL->endChangeGesture(); }
        if (auto* pR = proc.apvts.getParameter ("pan_r")) { pR->beginChangeGesture(); pR->setValueNotifyingHost (r01); pR->endChangeGesture(); }
        if (auto* dep= proc.apvts.getParameter ("depth")) { dep->beginChangeGesture(); dep->setValueNotifyingHost (y01); dep->endChangeGesture(); }
        refreshXYOverlays();
    };
    }
    // listeners for split overlay % etc.
    panKnobLeft.addListener (this);
    panKnobRight.addListener (this);
    // Parameter attachments now handled by AttachmentManager

    // Center group attachments
    // Center group parameter attachments moved to XYControlsPane (complete implementation there)

    // EQ and bypass parameter attachments now handled by AttachmentManager
    
    // Delay parameter attachments now handled by AttachmentManager

            // Motion includes removed - now handled by MotionControlsPane
            
            // Motion ComboBoxes, Buttons, and Sliders removed - now handled by MotionControlsPane
            
            // Motion ComboBox tinting removed - now handled by MotionControlsPane
            
            // Motion controls are now handled by MotionControlsPane - no longer created here
            
            // Motion parameter attachments removed - now handled by MotionControlsPane

    // parameter listeners (host→UI)
    // OLD REVERB SYSTEM REMOVED
    proc.apvts.addParameterListener ("split_mode", this);
    proc.apvts.addParameterListener ("pan",        this);
    proc.apvts.addParameterListener ("depth",      this);
    proc.apvts.addParameterListener ("mono_slope_db_oct", this);
    // EQ shape/Q visual linkage (live updates)
    proc.apvts.addParameterListener ("eq_shelf_shape", this);
    proc.apvts.addParameterListener ("eq_q_link",      this);
    proc.apvts.addParameterListener ("eq_filter_q",    this);
    proc.apvts.addParameterListener ("hp_q",           this);
    // Motion panner selection removed - now handled by MotionControlsPane
    proc.apvts.addParameterListener ("lp_q",           this);
    proc.apvts.addParameterListener ("tilt_link_s",    this);
    // Imaging overlays
    proc.apvts.addParameterListener ("xover_lo_hz",    this);
    proc.apvts.addParameterListener ("xover_hi_hz",    this);
    proc.apvts.addParameterListener ("rotation_deg",   this);
    proc.apvts.addParameterListener ("asymmetry",      this);

    // audio callbacks -> panes
    proc.onAudioSample   = [this](float L, float R) { if (panes) panes->onAudioSample (L, R); };
    proc.onAudioBlock    = [this](const float* L, const float* R, int n) { if (panes) panes->onAudioBlock (L, R, n); };
    proc.onAudioBlockPre = [this](const float* L, const float* R, int n) { if (panes) panes->onAudioBlockPre (L, R, n); };
    // XYPad sample rate now handled by XYTab

    // Keybindings for panes and keep-warm
    struct LocalKeyListener : public juce::KeyListener {
        PaneManager* mgr;
        explicit LocalKeyListener (PaneManager* m) : mgr (m) {}
        bool keyPressed (const juce::KeyPress& k, juce::Component*) override
        {
            if (!mgr) return false;
            if (k.getTextCharacter()=='1') { mgr->setActive (PaneID::XY, true);       return true; }
            if (k.getTextCharacter()=='2') { mgr->setActive (PaneID::DynEQ, true); return true; }
            if (k.getTextCharacter()=='3') { mgr->setActive (PaneID::Imager, true);   return true; }
            // keep-warm toggle removed (K/k)
            return false;
        }
    };
    keyListener.reset (new LocalKeyListener (panes.get()));
    addKeyListener (keyListener.get());

    // divider line component
    addAndMakeVisible (splitDivider);

    // sync XY with current values
    syncXYPadWithParameters();

    // pointer cursors on interactive children
    applyGlobalCursorPolicy();

    startTimerHz (20);

    // Image row group
    addChildComponent (imgGroupContainer);
    imgGroupContainer.setTitle("");
    imgGroupContainer.setShowBorder(false);
    imgGroupContainer.setVisible(false);
    imgGroupContainer.setInterceptsMouseClicks(false, false);
    // Row 2 group (Reverb, switch, Ducking group, Delay group)
    addChildComponent (volGroupContainer2);
    volGroupContainer2.setTitle("");
    volGroupContainer2.setShowBorder(false);
    volGroupContainer2.setVisible(false);
    volGroupContainer2.setInterceptsMouseClicks(false, false);
    // Mono group container
    addAndMakeVisible (monoGroupContainer);
    monoGroupContainer.setTitle ("");
    monoGroupContainer.setShowBorder (true);
    // Reverb group container
    // Reverb row container no longer used; layout directly
    
    // Re-enable resized() call since crash is happening after constructor
    resized();
}

void MyPluginAudioProcessorEditor::buildCells()
{
    // Row 1
    if (!widthCell)   widthCell   = std::make_unique<KnobCell>(width,    widthValue,    "WIDTH");
    if (!widthLoCell) widthLoCell = std::make_unique<KnobCell>(widthLo,  widthLoValue,  "W LO");
    if (!widthMidCell)widthMidCell= std::make_unique<KnobCell>(widthMid, widthMidValue, "W MID");
    if (!widthHiCell) widthHiCell = std::make_unique<KnobCell>(widthHi,  widthHiValue,  "W HI");
    if (!gainCell)    gainCell    = std::make_unique<KnobCell>(gain,     gainValue,     "GAIN");
    if (!satDriveCell)satDriveCell= std::make_unique<KnobCell>(satDrive, satDriveValue, "DRIVE");
    if (!satMixCell)  satMixCell  = std::make_unique<KnobCell>(satMix,   satMixValue,   "MIX");
    if (!monoCell)    monoCell    = std::make_unique<KnobCell>(monoHz,   monoValue,     "MONO");
    // Legacy spaceCell (REVERB) removed from Group 1 row; Reverb amount lives in Group 2 as WET

    if (!bassCell)     bassCell     = std::make_unique<KnobCell>(bass,  bassValue,  "BASS");
    if (!airCell)      airCell      = std::make_unique<KnobCell>(air,   airValue,   "AIR");
    if (!tiltCell)     tiltCell     = std::make_unique<KnobCell>(tilt,  tiltValue,  "TILT");
    if (!scoopCell)    scoopCell    = std::make_unique<KnobCell>(scoop, scoopValue, "SCOOP");
    if (!hpCell)       hpCell       = std::make_unique<KnobCell>(hpHz,  hpValue,    "HP Hz");
    if (!lpCell)       lpCell       = std::make_unique<KnobCell>(lpHz,  lpValue,    "LP Hz");

    if (!xoverLoCell)  xoverLoCell  = std::make_unique<KnobCell>(xoverLoHz, xoverLoValue, "XO LO");
    if (!xoverHiCell)  xoverHiCell  = std::make_unique<KnobCell>(xoverHiHz, xoverHiValue, "XO HI");
    if (!rotationCell) rotationCell = std::make_unique<KnobCell>(rotationDeg, rotationValue, "ROT");
    if (!asymCell)     asymCell     = std::make_unique<KnobCell>(asymmetry,   asymValue,     "ASYM");
    // SHUF cells moved to Band tab

    if (!delayTimeCell)      delayTimeCell       = std::make_unique<KnobCell>(delayTime,      delayTimeValue,      "TIME");
    if (!delayFeedbackCell)  delayFeedbackCell   = std::make_unique<KnobCell>(delayFeedback,  delayFeedbackValue,  "FB");
    if (!delayWetCell)       delayWetCell        = std::make_unique<KnobCell>(delayWet,       delayWetValue,       "WET");
    if (!delaySpreadCell)    delaySpreadCell     = std::make_unique<KnobCell>(delaySpread,    delaySpreadValue,    "SPREAD");
    if (!delayWidthCell)     delayWidthCell      = std::make_unique<KnobCell>(delayWidth,     delayWidthValue,     "WIDTH");
    if (!delayModRateCell)   delayModRateCell    = std::make_unique<KnobCell>(delayModRate,   delayModRateValue,   "RATE");
    if (!delayModDepthCell)  delayModDepthCell   = std::make_unique<KnobCell>(delayModDepth,  delayModDepthValue,  "DEPTH");
    if (!delayWowflutterCell)delayWowflutterCell = std::make_unique<KnobCell>(delayWowflutter,delayWowflutterValue,"WOW");
    if (!delayJitterCell)    delayJitterCell     = std::make_unique<KnobCell>(delayJitter,    delayJitterValue,    "JITTER");
    if (!delayHpCell)        delayHpCell         = std::make_unique<KnobCell>(delayHp,        delayHpValue,        "HP");
    if (!delayLpCell)        delayLpCell         = std::make_unique<KnobCell>(delayLp,        delayLpValue,        "LP");
    if (!delayTiltCell)      delayTiltCell       = std::make_unique<KnobCell>(delayTilt,      delayTiltValue,      "TILT");
    if (!delaySatCell)       delaySatCell        = std::make_unique<KnobCell>(delaySat,       delaySatValue,       "SAT");
    if (!delayDiffusionCell) delayDiffusionCell  = std::make_unique<KnobCell>(delayDiffusion, delayDiffusionValue, "DIFF");
    if (!delayDiffuseSizeCell)delayDiffuseSizeCell= std::make_unique<KnobCell>(delayDiffuseSize, delayDiffuseSizeValue, "SIZE");
    if (!delayDuckDepthCell) delayDuckDepthCell  = std::make_unique<KnobCell>(delayDuckDepth, delayDuckDepthValue, "DEPTH");
    if (!delayDuckAttackCell)delayDuckAttackCell = std::make_unique<KnobCell>(delayDuckAttack,delayDuckAttackValue,"ATT");
    if (!delayDuckReleaseCell)delayDuckReleaseCell=std::make_unique<KnobCell>(delayDuckRelease,delayDuckReleaseValue,"REL");
    if (!delayJitterCell) delayJitterCell = std::make_unique<KnobCell>(delayJitter, delayJitterValue, "JITTER");
    if (!delayDuckRatioCell) delayDuckRatioCell = std::make_unique<KnobCell>(delayDuckRatio, delayDuckRatioValue, "RAT");
    
    // Log: Editor constructor complete
    // TEMPORARILY DISABLE file logging to test if this is causing the crash
    // juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    // f.appendText("Editor Ctor: COMPLETE\n", false, false, "\n");
}


MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    // Editor destructor - restore crash logging for debugging
    juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
    f.appendText("Editor Destructor: STARTED\n", false, false, "\n");
    
    // Cancel AsyncUpdater to prevent use-after-free
    // Motion binding removed - now handled by MotionControlsPane
    f.appendText("Editor Destructor: AsyncUpdater cancelled\n", false, false, "\n");
    
    // Detach APVTS attachments BEFORE any controls are destroyed
    // Parameter attachments now handled by AttachmentManager
    if (attachmentManager) {
        attachmentManager->detachAllParameters();
    }
    // Motion attachments removed - now handled by MotionControlsPane
    f.appendText("Editor Destructor: APVTS attachments cleared\n", false, false, "\n");

    // Stop editor timer early
    stopTimer();
    f.appendText("Editor Destructor: Editor timer stopped\n", false, false, "\n");

    // Remove key listener safely
    if (keyListener)
    {
        removeKeyListener (keyListener.get());
        keyListener.reset();
    }
    f.appendText("Editor Destructor: Key listener removed\n", false, false, "\n");

    // Clear audio->UI callbacks to prevent use-after-free from audio thread
    proc.onAudioSample   = nullptr;
    proc.onAudioBlock    = nullptr;
    proc.onAudioBlockPre = nullptr;
    f.appendText("Editor Destructor: Audio callbacks cleared\n", false, false, "\n");

    // Remove all parameter listeners that were added in the ctor
    // OLD REVERB SYSTEM REMOVED
    proc.apvts.removeParameterListener ("split_mode", this);
    proc.apvts.removeParameterListener ("pan",        this);
    proc.apvts.removeParameterListener ("depth",      this);
    proc.apvts.removeParameterListener ("mono_slope_db_oct", this);
    proc.apvts.removeParameterListener ("eq_shelf_shape", this);
    proc.apvts.removeParameterListener ("eq_q_link",      this);
    proc.apvts.removeParameterListener ("eq_filter_q",    this);
    proc.apvts.removeParameterListener ("hp_q",           this);
    proc.apvts.removeParameterListener (motion::id::panner_select, this);
    proc.apvts.removeParameterListener ("lp_q",           this);
    proc.apvts.removeParameterListener ("tilt_link_s",    this);
    proc.apvts.removeParameterListener ("xover_lo_hz",    this);
    proc.apvts.removeParameterListener ("xover_hi_hz",    this);
    proc.apvts.removeParameterListener ("rotation_deg",   this);
    proc.apvts.removeParameterListener ("asymmetry",      this);
    f.appendText("Editor Destructor: Parameter listeners removed\n", false, false, "\n");

    // Detach UI listeners from knobs
    panKnobLeft.removeListener (this);
    panKnobRight.removeListener (this);
    f.appendText("Editor Destructor: UI listeners removed\n", false, false, "\n");

    // Ensure PaneManager timers and children are torn down before editor memory goes away
    panes.reset();
    f.appendText("Editor Destructor: PaneManager reset\n", false, false, "\n");

    // ensure A holds final state if user ended on B
    if (!isStateA) { saveCurrentState(); stateA = stateB; }
    f.appendText("Editor Destructor: State saved\n", false, false, "\n");

    setLookAndFeel (nullptr);
    f.appendText("Editor Destructor: LookAndFeel detached\n", false, false, "\n");
    
    f.appendText("Editor Destructor: COMPLETE\n", false, false, "\n");
}
void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Paint method - removed logging to prevent file I/O on every paint
    
    // background gradient (toned down and tinted with accent color)
    auto full = getLocalBounds();
    juce::Colour top    = lnf.theme.sh.interpolatedWith(lnf.theme.accent, 0.08f); // Subtle accent tint
    juce::Colour mid    = lnf.theme.hl.darker(0.15f).interpolatedWith(lnf.theme.accent, 0.12f); // Accent tint on middle
    juce::Colour bottom = lnf.theme.sh.interpolatedWith(lnf.theme.accent, 0.08f); // Subtle accent tint
    juce::ColourGradient bg (top, (float) full.getCentreX(), (float) full.getY(),
                             bottom, (float) full.getCentreX(), (float) full.getBottom(), false);
    bg.addColour (0.85, mid);
    g.setGradientFill (bg);
    g.fillAll();

    // logo + tagline
    auto header = getLocalBounds().removeFromTop ((int) (100 * scaleFactor));
    const int leftInset = Layout::dp (20, scaleFactor);
    auto logoArea = juce::Rectangle<int> (header.getX() + leftInset, header.getY() + Layout::dp (4, scaleFactor),
                                          header.getWidth(), (int) (30 * scaleFactor + 2));
    
    // Enhanced FIELD logo with shadow and glow effects
    drawHeaderFieldLogo(g, logoArea.toFloat());

    // version - position after the actual logo width
    const juce::String ver = " v" + juce::String (JUCE_STRINGIFY (JucePlugin_VersionString));
    juce::Font vfont (juce::FontOptions (juce::jmax (9, (int) std::round (8 * scaleFactor))));
    g.setFont (vfont);
    g.setColour (lnf.theme.textMuted);
    
    // Calculate actual logo width and position version after it
    const float actualLogoWidth = juce::jmin(logoArea.getHeight() * 0.8f, 30.0f) * 2.5f;
    const int vx = logoArea.getX() + (int) actualLogoWidth + Layout::dp (8, scaleFactor);
    const int vy = logoArea.getY() + (logoArea.getHeight() - vfont.getHeight()) * 0.5f + 1;
    g.drawText (ver, juce::Rectangle<int> (vx, vy, 120, (int) vfont.getHeight() + 2), juce::Justification::centredLeft);

    // tagline
    g.setColour (lnf.theme.textMuted);
    g.setFont (juce::Font (juce::FontOptions (13.0f * scaleFactor).withStyle ("Bold")));
    g.drawText ("Spatial Audio Processor",
                juce::Rectangle<int> (logoArea.getX(), logoArea.getBottom() + Layout::dp (2, scaleFactor),
                                     header.getWidth(), (int) (14 * scaleFactor + 2)),
                juce::Justification::centredLeft);

    // resize handle
    auto bounds = getLocalBounds();
    auto resizeArea = bounds.removeFromRight (20).removeFromBottom (20);
    g.setColour (lnf.theme.textMuted);
    for (int i = 0; i < 3; ++i)
    {
        int off = i * 4;
        g.drawLine (resizeArea.getRight() - 8 - off, resizeArea.getBottom() - 4 - off,
                    resizeArea.getRight() - 4 - off, resizeArea.getBottom() - 8 - off, 1.0f);
    }
}

void MyPluginAudioProcessorEditor::drawHeaderFieldLogo (juce::Graphics& g, juce::Rectangle<float> area) const
{
    // Calculate logo size for header (smaller than shade overlay)
    const float logoHeight = juce::jmin(area.getHeight() * 0.8f, 30.0f);
    const float logoWidth = logoHeight * 2.5f; // FIELD is wider than tall
    
    // Center the logo in the header area
    const float logoX = area.getX();
    const float logoY = area.getCentreY() - logoHeight * 0.5f;
    const auto logoRect = juce::Rectangle<float>(logoX, logoY, logoWidth, logoHeight);
    
    // Create bold font for header logo
    juce::Font logoFont(juce::FontOptions(logoHeight * 0.8f).withStyle("Bold"));
    g.setFont(logoFont);
    
    // Enhanced shadow system for header (stronger effects)
    const int shadowLayers = 8; // Increased for stronger effect
    for (int i = shadowLayers; i > 0; --i)
    {
        const float shadowOffset = (float)i * 2.0f; // Increased offset for stronger effect
        const float shadowAlpha = (1.0f - (float)i / shadowLayers) * 0.7f; // Increased alpha for stronger effect
        
        // Outer accent glow
        g.setColour(lnf.theme.accent.withAlpha(shadowAlpha * 0.8f));
        g.drawText("FIELD", logoRect.translated(shadowOffset, shadowOffset), 
                  juce::Justification::centredLeft);
        
        // Dark shadow for depth
        g.setColour(juce::Colours::black.withAlpha(shadowAlpha * 0.9f));
        g.drawText("FIELD", logoRect.translated(shadowOffset * 0.5f, shadowOffset * 0.5f), 
                  juce::Justification::centredLeft);
    }
    
    // Enhanced gradient effect for header (stronger)
    juce::ColourGradient logoGradient(
        lnf.theme.accent.brighter(0.6f), logoRect.getX(), logoRect.getY(),
        lnf.theme.accent.darker(0.3f), logoRect.getX(), logoRect.getBottom(), false);
    logoGradient.addColour(0.5f, lnf.theme.accent);
    
    g.setGradientFill(logoGradient);
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
    
    // Enhanced highlight for header (stronger)
    g.setColour(lnf.theme.accent.brighter(0.7f).withAlpha(0.9f));
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
    
    // Final white highlight for shine (stronger)
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.drawText("FIELD", logoRect, juce::Justification::centredLeft);
}

void MyPluginAudioProcessorEditor::performLayout()
{
    if (!layoutReady) return;

    // Delegate layout to LayoutManager
    if (layoutManager) {
        layoutManager->performLayout();
        return;
    }
}

void MyPluginAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (eventManager)
    {
        eventManager->handleMouseDown(e);
    }
}

void MyPluginAudioProcessorEditor::mouseDrag (const juce::MouseEvent& e)
{
    noteUserInteraction();
    if (eventManager)
    {
        eventManager->handleMouseDrag(e);
    }
}

void MyPluginAudioProcessorEditor::mouseUp (const juce::MouseEvent& e)
{
    if (eventManager)
    {
        eventManager->handleMouseUp(e);
    }
}

void MyPluginAudioProcessorEditor::resized()
{
    if (!layoutReady) return;
    performLayout();
}

void MyPluginAudioProcessorEditor::mouseMove (const juce::MouseEvent& e)
{
    if (eventManager)
    {
        eventManager->handleMouseMove(e);
    }
}

// Repaint waveform from UI thread at ~30 Hz
void MyPluginAudioProcessorEditor::timerCallback()
{
    if (eventManager)
    {
        eventManager->handleTimerCallback();
    }
    
    // Additional timer logic that needs to stay in PluginEditor
    // This will be implemented when we extract the remaining timer logic
}

void MyPluginAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    juce::ignoreUnused (g);
    // No extra overlay on top of children for now
}

// Move-only animation of Group 2 overlay; avoid reflow during slide
void MyPluginAudioProcessorEditor::updateGroup2OverlayDuringSlide()
{
    // Implementation moved to LayoutManager
}

void MyPluginAudioProcessorEditor::setScaleFactor (float newScale)
{
    scaleFactor = newScale;
    resized();
    repaint();
}

void MyPluginAudioProcessorEditor::sliderValueChanged (juce::Slider* s)
{
    if (eventManager)
    {
        eventManager->handleSliderValueChanged(s);
    }
    
    // Refresh muted visuals when any control changes
    updateMutedKnobVisuals();
}

void MyPluginAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (eventManager)
    {
        eventManager->handleComboBoxChanged(comboBox);
    }
}

void MyPluginAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (eventManager)
    {
        eventManager->handleButtonClicked(button);
    }
}

void MyPluginAudioProcessorEditor::applyGlobalCursorPolicy()
{
    // Implementation moved to LayoutManager
}

void MyPluginAudioProcessorEditor::updateMutedKnobVisuals()
{
    // Implementation moved to LayoutManager
}

void MyPluginAudioProcessorEditor::parameterChanged (const juce::String& id, float nv)
{
    // Implementation moved to EventManager
}

void MyPluginAudioProcessorEditor::updatePresetDisplay() { /* hook to PresetManager */ }

void MyPluginAudioProcessorEditor::syncXYPadWithParameters()
{
    // Implementation moved to LayoutManager
}

void MyPluginAudioProcessorEditor::saveCurrentState()
{
    std::map<juce::String, float> s;
    s["gain"]     = (float) gain.getValue();
    s["width"]    = (float) width.getValue();
    s["tilt"]     = (float) tilt.getValue();
    s["mono"]     = (float) monoHz.getValue();
    s["hp"]       = (float) hpHz.getValue();
    s["lp"]       = (float) lpHz.getValue();
    s["satDrive"] = (float) satDrive.getValue();
    s["satMix"]   = (float) satMix.getValue();
    s["air"]      = (float) air.getValue();
    s["bass"]     = (float) bass.getValue();
    s["scoop"]    = (float) scoop.getValue();
    s["pan"]      = (float) panKnob.getValue();
    if (isStateA) stateA = std::move (s); else stateB = std::move (s);
}

static void applyStateToSlider (juce::Slider& s, float v) { s.setValue (v, juce::sendNotificationSync); }

void MyPluginAudioProcessorEditor::loadState (bool loadStateA)
{
    auto& s = loadStateA ? stateA : stateB;
    if (auto it = s.find ("gain");     it != s.end()) applyStateToSlider (gain, it->second);
    if (auto it = s.find ("width");    it != s.end()) applyStateToSlider (width, it->second);
    if (auto it = s.find ("tilt");     it != s.end()) applyStateToSlider (tilt, it->second);
    if (auto it = s.find ("mono");     it != s.end()) applyStateToSlider (monoHz, it->second);
    if (auto it = s.find ("hp");       it != s.end()) applyStateToSlider (hpHz, it->second);
    if (auto it = s.find ("lp");       it != s.end()) applyStateToSlider (lpHz, it->second);
    if (auto it = s.find ("satDrive"); it != s.end()) applyStateToSlider (satDrive, it->second);
    if (auto it = s.find ("satMix");   it != s.end()) applyStateToSlider (satMix, it->second);
    if (auto it = s.find ("air");      it != s.end()) applyStateToSlider (air, it->second);
    if (auto it = s.find ("bass");     it != s.end()) applyStateToSlider (bass, it->second);
    if (auto it = s.find ("scoop");    it != s.end()) applyStateToSlider (scoop, it->second);
    if (auto it = s.find ("pan");      it != s.end()) applyStateToSlider (panKnob, it->second);
}

void MyPluginAudioProcessorEditor::toggleABState()
{
    isStateA = !isStateA;
    abButtonA.setToggleState (isStateA,  juce::dontSendNotification);
    abButtonB.setToggleState (!isStateA,  juce::dontSendNotification);
    loadState (isStateA);
}

void MyPluginAudioProcessorEditor::copyState (bool fromA) { clipboardState = fromA ? stateA : stateB; }

void MyPluginAudioProcessorEditor::pasteState (bool pasteToA)
{
    if (pasteToA) stateA = clipboardState; else stateB = clipboardState;
    loadState (pasteToA);
}

void MyPluginAudioProcessorEditor::layoutMeters(juce::Rectangle<int> metersArea, float s, float sv)
{
    // Layout the three meter components in the right strip
    if (metersArea.getWidth() <= 0 || metersArea.getHeight() <= 0) return;
    
    // Calculate meter dimensions
    const int meterWidth = juce::jlimit(Layout::dp(24, s), Layout::dp(56, s), 
                                       juce::roundToInt(metersArea.getWidth() * 0.75f));
    const int corrWidth = juce::jmax(Layout::dp(10, s), juce::roundToInt(meterWidth * 0.5f));
    const int interGap = juce::jmax(1, Layout::dp(Layout::GAP_S, s) / 2);
    const int outerPadX = juce::jmax(1, Layout::dp(Layout::GAP_S, s));
    const int outerPadY = Layout::dp(Layout::GAP, sv);
    
    // Calculate total width needed
    const int totalWidth = meterWidth * 2 + corrWidth + interGap * 2 + outerPadX * 2;
    const int actualWidth = juce::jlimit(Layout::dp(96, s), Layout::dp(240, s), totalWidth);
    
    // Center the meters in the available area
    auto centeredArea = metersArea.withWidth(actualWidth).withX(metersArea.getX() + (metersArea.getWidth() - actualWidth) / 2);
    
    // Position the three meters
    auto ioArea = centeredArea.removeFromLeft(meterWidth).reduced(outerPadX, outerPadY);
    auto lrArea = centeredArea.removeFromLeft(meterWidth).reduced(outerPadX, outerPadY);
    auto corrArea = centeredArea.removeFromLeft(corrWidth).reduced(outerPadX, outerPadY);
    
    // Set bounds for each meter
    ioMeters.setBounds(ioArea);
    lrMeters.setBounds(lrArea);
    corrMeter.setBounds(corrArea);
}

void MyPluginAudioProcessorEditor::initializeParameterAttachments()
{
    if (attachmentManager)
    {
        attachmentManager->attachAllParameters();
    }
}

void MyPluginAudioProcessorEditor::finalizeInitialization()
{
    // History removed
    // History group dividers removed
    
    // Re-enable resized() call since crash is happening after constructor
    resized();
}

// end
