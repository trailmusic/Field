#include "KnobCellWithAux.h"
#include "../../Core/FieldLookAndFeel.h"

KnobCellWithAux::KnobCellWithAux(juce::Slider& mainKnob,
                                 juce::Label& mainLabel,
                                 const std::vector<juce::Component*>& auxComponents,
                                 const std::vector<float>& auxWeights)
    : mainKnob(mainKnob), mainLabel(mainLabel), auxComponents(auxComponents), auxWeights(auxWeights)
{
    // Let the children handle interactions
    setWantsKeyboardFocus (false);
    setInterceptsMouseClicks (false, true);
}

void KnobCellWithAux::setMetrics (int knobPx, int valuePx, int gapPx)
{
    K = juce::jmax (16, knobPx);
    V = juce::jmax (0,  valuePx);
    G = juce::jmax (0,  gapPx);
    resized();
    repaint();
}

void KnobCellWithAux::setAuxHeight (int auxHeightPx)
{
    A = juce::jmax (0, auxHeightPx);
    resized();
    repaint();
}

void KnobCellWithAux::setAuxWeights (const std::vector<float>& weights)
{
    auxWeights = weights;
    resized();
    repaint();
}

void KnobCellWithAux::ensureChildren()
{
    // Ensure main knob and label are parented
    if (mainKnob.getParentComponent() != this) addAndMakeVisible (mainKnob);
    if (mainLabel.getParentComponent() != this) addAndMakeVisible (mainLabel);
    mainLabel.setInterceptsMouseClicks (false, false);

    // Ensure auxiliary components are parented
    for (auto* component : auxComponents)
    {
        if (component != nullptr && component->getParentComponent() != this)
            addAndMakeVisible (component);
    }
}

void KnobCellWithAux::layoutAuxComponents (juce::Rectangle<int> auxArea)
{
    if (auxComponents.empty()) return;

    const int count = (int) auxComponents.size();
    const int gapY = juce::jmax (6, G);
    const int totalG = gapY * juce::jmax (0, count - 1);
    const int H = juce::jmax (1, auxArea.getHeight() - totalG);

    // Calculate heights based on weights
    juce::Array<float> weights;
    if ((int) auxWeights.size() == count) {
        for (float v : auxWeights) weights.add (juce::jmax (0.0f, v));
    } else {
        for (int i = 0; i < count; ++i) weights.add (1.0f);
    }
    float sum = 0.0f; for (auto w : weights) sum += w; if (sum <= 0.0001f) sum = (float) count;

    juce::Array<int> heights; heights.resize (count);
    int acc = 0;
    for (int i = 0; i < count; ++i) { 
        int h = (int) std::round (H * (weights[i] / sum)); 
        heights.set (i, h); 
        acc += h; 
    }
    for (int d = 0; d < H - acc; ++d) heights.set (d % count, heights[d % count] + 1);

    // Layout components vertically
    juce::Rectangle<int> col = auxArea;
    col.setY (auxArea.getCentreY() - (H + totalG) / 2);
    col.setHeight (H + totalG);
    
    for (int i = 0; i < count; ++i)
    {
        auto* component = auxComponents[(size_t) i];
        auto rCell = col.removeFromTop (heights[i]).reduced (2, 2);
        if (component != nullptr)
            component->setBounds (rCell);
        if (i < count - 1) col.removeFromTop (gapY);
    }
}

void KnobCellWithAux::resized()
{
    ensureChildren();
    auto b = getLocalBounds().reduced (4);
    const int rimR = 6;

    // Split into left (knob) and right (aux) areas
    // The left area should behave exactly like a standard KnobCell
    auto leftArea = b.removeFromLeft ((b.getWidth() - G) * 2 / 3); // Knob gets 2/3 of space
    b.removeFromLeft (G); // Remove the gap between knob and aux areas
    auto rightArea = b;

    // Layout main knob and label in left area - exactly like standard KnobCell
    // Reserve space for the value label first (like standard KnobCell)
    if (V > 0)
        leftArea.removeFromBottom (V + G);

    // Fit the knob at the top-center with requested diameter K (like standard KnobCell)
    const int k = juce::jmin (K, juce::jmin (leftArea.getWidth(), leftArea.getHeight()));
    juce::Rectangle<int> knobBox (k, k);
    knobBox = knobBox.withCentre ({ leftArea.getCentreX(), leftArea.getY() + k / 2 });
    mainKnob.setBounds (knobBox);

    // Layout main label (like standard KnobCell)
    const int lh = (int) std::ceil (mainLabel.getFont().getHeight());
    juce::Rectangle<int> lb (knobBox.getX(), knobBox.getBottom() + G, knobBox.getWidth(), juce::jmax (V, lh));
    mainLabel.setBounds (lb);

    // Layout auxiliary components in right area
    if (A > 0 && !auxComponents.empty())
    {
        auto auxArea = rightArea.removeFromTop (A);
        layoutAuxComponents (auxArea);
    }
}

void KnobCellWithAux::paint (juce::Graphics& g)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto panel = lf ? lf->theme.panel : juce::Colour (0xFF3A3D45);
    auto sh    = lf ? lf->theme.sh    : juce::Colour (0xFF2A2C30);
    auto acc2  = lf ? lf->theme.accentSecondary : juce::Colour (0xFF202226);

    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;

    // Panel fill (optional) with metallic mode - same as KnobCell
    const bool metallic = (bool) getProperties().getWithDefault ("metallic", false);
    if (showPanel)
    {
        auto rr = r.reduced (3.0f);
        if (metallic)
        {
            // Brushed-metal gradient with per-system tinting
            const bool motionGreen   = (bool) getProperties().getWithDefault ("motionPurpleBorder", (bool) getProperties().getWithDefault ("motionGreenBorder", false));
            const bool reverbMetal   = (bool) getProperties().getWithDefault ("reverbMetallic", false);
            const bool delayMetal    = (bool) getProperties().getWithDefault ("delayMetallic", false);
            const bool bandMetal     = (bool) getProperties().getWithDefault ("bandMetallic",  false);

            juce::Colour top, bot;
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                if (reverbMetal)
                {
                    // Burnt orange metallic
                    top = juce::Colour (0xFFB1592A);
                    bot = juce::Colour (0xFF7F2D1C);
                }
                else if (delayMetal)
                {
                    // Light yellowish-green metallic
                    top = juce::Colour (0xFFBFD86A);
                    bot = juce::Colour (0xFF88A845);
                }
                else if (bandMetal)
                {
                    // Band pane metallic blue
                    top = juce::Colour (0xFF6AA0D8);
                    bot = juce::Colour (0xFF3A6EA8);
                }
                else if (motionGreen)
                {
                    top = lf->theme.motionPanelTop;
                    bot = lf->theme.motionPanelBot;
                }
                else
                {
                    // Neutral steel fallback
                    top = juce::Colour (0xFF9AA0A7);
                    bot = juce::Colour (0xFF7F858D);
                }
            }
            else
            {
                if (reverbMetal)
                {
                    top = juce::Colour (0xFFB1592A);
                    bot = juce::Colour (0xFF7F2D1C);
                }
                else if (delayMetal)
                {
                    top = juce::Colour (0xFFBFD86A);
                    bot = juce::Colour (0xFF88A845);
                }
                else if (bandMetal)
                {
                    top = juce::Colour (0xFF6AA0D8);
                    bot = juce::Colour (0xFF3A6EA8);
                }
                else if (motionGreen)
                {
                    top = juce::Colour (0xFF7B81C1);
                    bot = juce::Colour (0xFF555A99);
                }
                else
                {
                    top = juce::Colour (0xFF9AA0A7);
                    bot = juce::Colour (0xFF7F858D);
                }
            }
            juce::ColourGradient grad (top, rr.getX(), rr.getY(), bot, rr.getX(), rr.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (rr, rad);

            // Subtle horizontal brushing lines (slightly denser)
            g.setColour (juce::Colours::white.withAlpha (0.045f));
            const int step = 1;
            for (int y = (int) rr.getY() + step; y < rr.getBottom(); y += step)
                g.fillRect (juce::Rectangle<int> ((int) rr.getX() + 4, y, (int) rr.getWidth() - 8, 1));

            // Static metallic texture (no randomization for performance)
            {
                g.setColour (juce::Colours::black.withAlpha (0.040f));
                const int noiseRows = juce::jmax (1, (int) rr.getHeight() / 4);
                for (int i = 0; i < noiseRows; ++i)
                {
                    const int y = (int) rr.getY() + 2 + i * 4;
                    const int w = juce::jmax (8, (int) rr.getWidth() - 12);
                    const int x = (int) rr.getX() + 6;
                    g.fillRect (juce::Rectangle<int> (x, y, w, 1));
                }
            }

            // Static diagonal micro-scratches (no randomization for performance)
            {
                const int scratches = juce::jmax (6, (int) rr.getWidth() / 22);
                g.setColour (juce::Colours::white.withAlpha (0.035f));
                for (int i = 0; i < scratches; ++i)
                {
                    float sx = rr.getX() + 6 + std::fmod (i * 3.7f, rr.getWidth() - 12);
                    float sy = rr.getY() + 6 + std::fmod (i * 2.3f, rr.getHeight() - 12);
                    float len = 14.0f;
                    float dx = len * 0.86f; // cos(~40deg)
                    float dy = len * 0.50f; // sin(~30deg)
                    g.drawLine (sx, sy, sx + dx, sy + dy, 1.0f);
                }
                g.setColour (juce::Colours::black.withAlpha (0.025f));
                for (int i = 0; i < scratches; ++i)
                {
                    float sx = rr.getX() + 6 + std::fmod (i * 4.1f, rr.getWidth() - 12);
                    float sy = rr.getY() + 6 + std::fmod (i * 3.1f, rr.getHeight() - 12);
                    float len = 11.0f;
                    float dx = len * -0.80f;
                    float dy = len * 0.58f;
                    g.drawLine (sx, sy, sx + dx, sy + dy, 1.0f);
                }
            }

            // Vignette to reduce perceived brightness near edges (slightly stronger for Motion)
            {
                const float edgeAlpha = motionGreen ? 0.22f : 0.16f;
                juce::ColourGradient vg (juce::Colours::transparentBlack, rr.getCentreX(), rr.getCentreY(),
                                         juce::Colours::black.withAlpha (edgeAlpha), rr.getCentreX(), rr.getCentreY() - rr.getHeight() * 0.6f, true);
                g.setGradientFill (vg);
                g.fillRoundedRectangle (rr, rad);
            }
        }
        else
        {
            // Standard panel background
            g.setColour (panel);
            g.fillRoundedRectangle (rr, rad);

            // Depth (soft) – match KnobCell
            {
                juce::DropShadow ds1 ((lf ? lf->theme.shadowDark  : juce::Colours::black).withAlpha (0.35f), 12, { -1, -1 });
                juce::DropShadow ds2 ((lf ? lf->theme.shadowLight : juce::Colours::grey ).withAlpha (0.25f),  6, { -1, -1 });
                auto ri = rr.getSmallestIntegerContainer();
                ds1.drawForRectangle (g, ri);
                ds2.drawForRectangle (g, ri);
            }

            g.setColour (sh.withAlpha (0.18f));
            g.drawRoundedRectangle (rr, rad - 1.0f, 0.8f);
        }
    }

    if (showBorder)
    {
        auto border = r.reduced (2.0f);
        g.setColour (acc2);
        g.drawRoundedRectangle (border, rad, 1.5f);
    }

    // Draw recessed badge for main label
    auto labelBounds = mainLabel.getBounds().toFloat();
    if (!labelBounds.isEmpty())
    {
        auto badge = labelBounds.reduced (2.0f);
        g.setColour (sh.withAlpha (0.15f));
        g.fillRoundedRectangle (badge, 3.0f);
        g.setColour (sh.withAlpha (0.25f));
        g.drawRoundedRectangle (badge, 3.0f, 0.5f);
    }

    // Standard border treatment for XY controls (reduced brightness)
    const bool isXYControl = (bool) getProperties().getWithDefault ("centerStyle", false);
    if (isXYControl)
    {
        auto accent = lf ? lf->theme.accent : juce::Colours::cyan;
        g.setColour (accent.withAlpha (0.3f));
        g.drawRoundedRectangle (r, rad, 1.0f);
    }
}
