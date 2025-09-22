#include "DecayCurveComponent.h"
#include "../ReverbParamIDs.h"
#include "../../FieldLookAndFeel.h"

using namespace juce;

static inline float getParam (AudioProcessorValueTreeState& s, const String& id)
{
    if (auto* p = dynamic_cast<AudioParameterFloat*>(s.getParameter (id))) return p->get();
    return 1.0f;
}

void DecayCurveComponent::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (4);
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const Colour panel = lf ? lf->theme.panel : Colours::black;
    const Colour rim   = lf ? lf->theme.text.withAlpha (0.20f) : Colours::white.withAlpha (0.2f);
    g.setColour (panel.withAlpha (0.40f)); g.fillRoundedRectangle (r, 6.f);
    g.setColour (rim);                     g.drawRoundedRectangle (r, 6.f, 1.2f);

    auto mapX = [r](float hz){ float n=(std::log10 (hz)-std::log10 (20.f))/(std::log10 (20000.f)-std::log10 (20.f)); return r.getX()+n*r.getWidth(); };
    auto mapY = [r](float mult){ float t = jlimit (0.3f, 2.0f, mult); return jmap (t, 0.3f, 2.0f, r.getBottom()-12, r.getY()+8); };

    const float lowX  = getParam (state, lowParamId);
    const float midX  = getParam (state, midParamId);
    const float highX = getParam (state, highParamId);

    Path curve; curve.startNewSubPath (mapX (40.f), mapY (lowX));
    curve.quadraticTo (mapX (150.f),  mapY (lowX),  mapX (1500.f),  mapY (midX));
    curve.quadraticTo (mapX (10000.f),mapY (highX), mapX (18000.f), mapY (highX));
    g.setColour (lf ? lf->theme.hl : Colours::orange); g.strokePath (curve, PathStrokeType (2.f));

    auto drawHandle = [&] (float hz, float mult)
    {
        auto p = Point<float> (mapX (hz), mapY (mult));
        g.setColour ((lf ? lf->theme.hl : Colours::orange).withAlpha (0.85f));
        g.fillEllipse (p.x-4, p.y-4, 8, 8);
        g.setColour ((lf ? lf->theme.sh : Colours::black).withAlpha (0.6f));
        g.drawEllipse (p.x-4, p.y-4, 8, 8, 1.2f);
    };
    drawHandle (150.f,  lowX);
    drawHandle (1500.f, midX);
    drawHandle (10000.f,highX);
}

void DecayCurveComponent::mouseDown (const MouseEvent& e)
{
    auto r = getLocalBounds().toFloat().reduced (4);
    auto mapX = [r](float hz){ float n=(std::log10 (hz)-std::log10 (20.f))/(std::log10 (20000.f)-std::log10 (20.f)); return r.getX()+n*r.getWidth(); };
    const float xL = mapX (150.f), xM = mapX (1500.f), xH = mapX (10000.f);
    auto pick = [&](float x){ return std::abs (e.position.x - x); };
    float d0 = pick (xL), d1 = pick (xM), d2 = pick (xH);
    dragIdx = (d0<d1 && d0<d2) ? 0 : (d1<d2 ? 1 : 2);
}

void DecayCurveComponent::mouseDrag (const MouseEvent& e)
{
    auto setF = [&] (const String& id, float v)
    {
        if (auto* p = dynamic_cast<AudioParameterFloat*>(state.getParameter (id)))
        {
            p->beginChangeGesture(); p->setValueNotifyingHost (p->convertTo0to1 (v)); p->endChangeGesture();
        }
    };
    auto r = getLocalBounds().toFloat().reduced (4);
    float y = jlimit (r.getY()+8.0f, r.getBottom()-12.0f, e.position.y);
    float mult = jmap (y, r.getBottom()-12.0f, r.getY()+8.0f, 0.3f, 2.0f);
    if (dragIdx == 0) setF (lowParamId,  mult);
    if (dragIdx == 1) setF (midParamId,  mult);
    if (dragIdx == 2) setF (highParamId, mult);
}


