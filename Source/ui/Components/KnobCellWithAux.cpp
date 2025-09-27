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
        
        // Debug: Metallic detection (removed console output for simplicity)
        
        if (metallic)
        {
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                const auto kind = metallicFromProps(getProperties());
                switch (kind)
                {
                    case MetallicKind::Reverb:  FieldLNF::paintMetal(g, rr, lf->theme.metal.reverb,  rad); break;
                    case MetallicKind::Delay:   FieldLNF::paintMetal(g, rr, lf->theme.metal.delay,   rad); break;
                    case MetallicKind::Band:    FieldLNF::paintMetal(g, rr, lf->theme.metal.band,    rad); break;
                    case MetallicKind::Phase:   
                    {
                        FieldLNF::PhaseMetal phaseMetalConfig {
                            lf->theme.metal.phase.top, lf->theme.metal.phase.bottom,
                            lf->theme.metal.phase.tint, lf->theme.metal.phase.tintAlpha,
                            juce::Colour (0xFF0A0C0F), 0.14f,  // bottom multiply
                            0.10f  // sheen alpha
                        };
                        FieldLNF::paintPhaseMetal(g, rr, phaseMetalConfig, rad);
                        break;
                    }
                    case MetallicKind::Motion:  FieldLNF::paintMetal(g, rr, lf->theme.metal.motion,  rad); break;
                    case MetallicKind::XY:      FieldLNF::paintMetal(g, rr, lf->theme.metal.xy,      rad); break;
                    case MetallicKind::Neutral: FieldLNF::paintMetal(g, rr, lf->theme.metal.neutral, rad); break;
                    default:                    FieldLNF::paintMetal(g, rr, lf->theme.metal.neutral, rad); break;
                }
            }
            else
            {
                // simple neutral fallback
                juce::ColourGradient grad (juce::Colour (0xFF9CA4AD), rr.getX(), rr.getY(),
                                           juce::Colour (0xFF6E747C), rr.getX(), rr.getBottom(), false);
                g.setGradientFill (grad);
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

    // Label background removed - use default styling to match KnobCell

    // XY controls use the main border system above - no additional border needed
}
