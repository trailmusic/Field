#include "KnobCellWithAux.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/FieldMetallic.h"

KnobCellWithAux::KnobCellWithAux(juce::Slider& mainKnob,
                                 juce::Label& mainLabel,
                                 const std::vector<juce::Component*>& auxComponents,
                                 const std::vector<float>& auxWeights)
    : mainKnob(mainKnob), mainLabel(mainLabel), auxComponents(auxComponents), auxWeights(auxWeights)
{
    // Let the children handle interactions
    setWantsKeyboardFocus (false);
    setInterceptsMouseClicks (false, true);
    
    // Enable hover effects and pointer cursor
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    
    // Add double-click handler for flash effect
    mainKnob.addMouseListener(this, false);
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

    // Split into left (knob) and right (aux) areas - 50/50 with compensated middle gap
    auto leftArea = b.removeFromLeft ((b.getWidth() - G) / 2); // Knob gets 50% of space
    b.removeFromLeft (G); // Remove the gap between knob and aux areas
    auto rightArea = b;

    // Layout main knob and label in left area - exactly like standard KnobCell
    // Reserve space for the value label first (like standard KnobCell)
    if (V > 0)
        leftArea.removeFromBottom (V + G);

    // Fit the knob at the top-center with requested diameter K (like standard KnobCell)
    // Position the knob as if it's in a standard KnobCell that takes up the full width
    const int k = juce::jmin (K, juce::jmin (leftArea.getWidth(), leftArea.getHeight()));
    juce::Rectangle<int> knobBox (k, k);
    // Center horizontally in the full width (like standard KnobCell), position vertically from top
    auto fullWidth = getLocalBounds().reduced(4);
    // Compensate for the 50/50 split: knob should be centered in the left half of the full width
    auto leftHalfCenter = fullWidth.getX() + (fullWidth.getWidth() / 4); // Center of left 50%
    knobBox = knobBox.withCentre ({ leftHalfCenter, leftArea.getY() + k / 2 });
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
    const auto metallicKind = metallicFromProps (getProperties());
    const bool metallic = (metallicKind != MetallicKind::None);
    if (showPanel)
    {
        auto rr = r.reduced (3.0f);
        
        
        if (metallic)
        {
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                switch (metallicKind)
                {
                    case MetallicKind::Reverb:  MetallicRenderer::paintMetal(g, rr, lf->theme.metal.reverb,  rad); break;
                    case MetallicKind::Delay:   MetallicRenderer::paintMetal(g, rr, lf->theme.metal.delay,   rad); break;
                    case MetallicKind::Band:    MetallicRenderer::paintMetal(g, rr, lf->theme.metal.band,    rad); break;
                    case MetallicKind::Phase:   MetallicRenderer::paintMetal(g, rr, lf->theme.metal.phase, rad); break;
                    case MetallicKind::Motion:  MetallicRenderer::paintMetal(g, rr, lf->theme.metal.motion,  rad); break;
                    case MetallicKind::XY:      MetallicRenderer::paintMetal(g, rr, lf->theme.metal.xy,      rad); break;
                    case MetallicKind::Neutral: MetallicRenderer::paintMetal(g, rr, lf->theme.metal.neutral, rad); break;
                    default:                    MetallicRenderer::paintMetal(g, rr, lf->theme.metal.neutral, rad); break;
                }
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

    // Draw gradient background BEFORE slider rendering to avoid covering labels
    if (mainKnob.isVisible())
    {
        auto knobBounds = mainKnob.getBounds().toFloat();
        auto centre = knobBounds.getCentre();
        auto radius = juce::jmin(knobBounds.getWidth(), knobBounds.getHeight()) * 0.5f;
        auto trackRadius = radius * 0.80f;
        
        // Add top-down gradient background for the knob (full knob area)
        auto gradientRadius = trackRadius + 4.0f;  // Full knob area including track
        auto gradientBounds = juce::Rectangle<float>(centre.x - gradientRadius, centre.y - gradientRadius, 
                                                    gradientRadius * 2, gradientRadius * 2);
        
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            juce::ColourGradient knobGradient(lf->theme.meters.panelDark.brighter(0.2f), gradientBounds.getX(), gradientBounds.getY(),
                                             lf->theme.meters.panelDark.darker(0.1f), gradientBounds.getX(), gradientBounds.getBottom(), false);
            g.setGradientFill(knobGradient);
            g.fillEllipse(gradientBounds);
        }
    }

    // Render the slider with our custom LookAndFeel to get tick marks
    if (mainKnob.isVisible())
    {
        auto knobBounds = mainKnob.getBounds().toFloat();
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            const double minV = mainKnob.getMinimum();
            const double maxV = mainKnob.getMaximum();
            const float pos01 = (maxV > minV) ? (float) ((mainKnob.getValue() - minV) / (maxV - minV)) : 0.0f;
            
            lf->drawRotarySlider(g, knobBounds.getX(), knobBounds.getY(), knobBounds.getWidth(), knobBounds.getHeight(),
                                 pos01,
                                 juce::MathConstants<float>::pi,
                                 juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                                 mainKnob);
        }
    }

    // Draw caption/name above knob using LNF helper if available (AFTER slider to be on top)
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
    {
        juce::String caption;
        if (getProperties().contains ("caption"))
            caption = getProperties()["caption"].toString();
        if (caption.isNotEmpty())
        {
        // Use knob bounds for proper centering, but create a smaller area for the label
        auto knobBounds = mainKnob.getBounds().toFloat();
        auto knobCenter = knobBounds.getCentre();
        auto labelSize = juce::jmin(knobBounds.getWidth(), knobBounds.getHeight()) * 0.6f; // 60% of knob size
        auto r = juce::Rectangle<float>(knobCenter.x - labelSize/2, knobCenter.y - labelSize/2, labelSize, labelSize);
        lf->drawKnobLabel (g, r, caption);
        }
    }

    // Recessed background badge behind value label text (same as KnobCell)
    if (mainLabel.isShowing())
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto lb = mainLabel.getBounds().toFloat();
        auto f  = mainLabel.getFont();
        const juce::String txt = mainLabel.getText();

        const float th = std::ceil (f.getHeight());
        const float tw = f.getStringWidthFloat (txt);

        // Slightly larger than text bounds
        const float padX = 4.0f;
        const float padY = 2.0f;

        const float x = lb.getCentreX() - tw * 0.5f - padX;
        const float y = lb.getY() + (lb.getHeight() - th) * 0.5f - padY * 0.5f;
        juce::Rectangle<float> badge (x, y, tw + padX * 2.0f, th + padY);

        const float cr = 4.0f;

        juce::Colour base = lf ? lf->theme.panel : juce::Colour (0xFF2A2C30);
        juce::Colour top  = base.darker (0.70f);  // darker overall
        juce::Colour bot  = base.darker (0.38f);

        juce::ColourGradient grad (top, badge.getX(), badge.getY(),
                                   bot, badge.getX(), badge.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (badge, cr);

        // Stronger inner shadow using multiple inset strokes
        for (int i = 0; i < 3; ++i)
        {
            const float inset = 0.8f + i * 0.8f;
            const float alpha = 0.28f - i * 0.06f;
            g.setColour (juce::Colours::black.withAlpha (alpha));
            g.drawRoundedRectangle (badge.reduced (inset), juce::jmax (0.0f, cr - inset * 0.6f), 1.2f);
        }

        // Top inner highlight and bottom inner shadow lines for accent
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawLine (badge.getX() + 1.0f, badge.getY() + 1.0f,
                    badge.getRight() - 1.0f, badge.getY() + 1.0f, 1.0f);

        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.drawLine (badge.getX() + 1.0f, badge.getBottom() - 1.0f,
                    badge.getRight() - 1.0f, badge.getBottom() - 1.0f, 1.0f);
    }

    // XY controls use the main border system above - no additional border needed
}

void KnobCellWithAux::mouseDoubleClick (const juce::MouseEvent& event)
{
    // Trigger flash effect on double-click
    mainKnob.getProperties().set("flash", true);
    repaint();
    
    // Reset flash after animation
    juce::MessageManager::callAsync([this]()
    {
        mainKnob.getProperties().set("flash", false);
        repaint();
    });
}
