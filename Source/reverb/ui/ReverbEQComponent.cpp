#include "ReverbEQComponent.h"
#include "../ReverbParamIDs.h"
#include "../../Core/FieldLookAndFeel.h"

using namespace juce;

static inline float getF (AudioProcessorValueTreeState& s, const String& id)
{ if (auto* p = dynamic_cast<AudioParameterFloat*>(s.getParameter (id))) return p->get(); return 0.f; }

void ReverbEQComponent::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (4);
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF defaultLnf;
    const auto& theme = lf ? lf->theme : defaultLnf.theme;
    const Colour panel = theme.panel;
    const Colour rim   = theme.text.withAlpha (0.20f);
    g.setColour (panel.withAlpha (0.40f)); g.fillRoundedRectangle (r, 6.f);
    g.setColour (rim);                     g.drawRoundedRectangle (r, 6.f, 1.2f);

    auto mapX = [r](float hz){ float n=(std::log10 (hz)-std::log10 (20.f))/(std::log10 (20000.f)-std::log10 (20.f)); return r.getX()+n*r.getWidth(); };
    auto mapY = [r](float dB){ return jmap (dB, 18.f, -18.f, r.getY()+8, r.getBottom()-8); };

    Path curve; curve.preallocateSpace (jmax (32, (int) r.getWidth()));
    const int steps = jmax (10, (int) r.getWidth());
    for (int i=0;i<steps;++i)
    {
        float n=(float)i/(float)(steps-1);
        float hz = std::pow (10.0f, jmap (n, std::log10 (20.f), std::log10 (20000.f)));
        // crude magnitude: sum the three band gains at that frequency (good enough for preview)
        float dB = 0.f;
        // low shelf acts wide; mid acts as point gain; high shelf acts wide
        dB += getF (state, ReverbIDs::eqLowGainDb);
        dB += getF (state, ReverbIDs::eqMidGainDb);
        dB += getF (state, ReverbIDs::eqHighGainDb);
        float x = r.getX() + n*r.getWidth(); float y = mapY (jlimit (-18.f, 18.f, dB));
        if (i==0) curve.startNewSubPath (x,y); else curve.lineTo (x,y);
        ignoreUnused (hz);
    }
    g.setColour (theme.accent); g.strokePath (curve, PathStrokeType (2.f));

    auto drawNode = [&](float hz, float dB, Colour c){ float x=mapX (hz), y=mapY (dB); g.setColour (c); g.fillEllipse (x-5,y-5,10,10); g.setColour (theme.sh.withAlpha (0.6f)); g.drawEllipse (x-5,y-5,10,10,1.2f); };
    drawNode (getF(state, ReverbIDs::eqLowFreqHz),  getF(state, ReverbIDs::eqLowGainDb),  theme.eq.bass);
    drawNode (getF(state, ReverbIDs::eqMidFreqHz),  getF(state, ReverbIDs::eqMidGainDb),  theme.eq.tilt);
    drawNode (getF(state, ReverbIDs::eqHighFreqHz), getF(state, ReverbIDs::eqHighGainDb), theme.eq.air);
}

void ReverbEQComponent::mouseDown (const MouseEvent& e)
{
    auto r = getLocalBounds().toFloat().reduced (4);
    auto mapX = [r](float hz){ float n=(std::log10 (hz)-std::log10 (20.f))/(std::log10 (20000.f)-std::log10 (20.f)); return r.getX()+n*r.getWidth(); };
    float lf = getF (state, ReverbIDs::eqLowFreqHz);
    float mf = getF (state, ReverbIDs::eqMidFreqHz);
    float hf = getF (state, ReverbIDs::eqHighFreqHz);
    float d0 = std::abs (e.x - mapX (lf)); float d1 = std::abs (e.x - mapX (mf)); float d2 = std::abs (e.x - mapX (hf));
    dragBand = (d0<d1 && d0<d2) ? 0 : (d1<d2 ? 1 : 2);
}

void ReverbEQComponent::mouseDrag (const MouseEvent& e)
{
    auto r = getLocalBounds().toFloat().reduced (4);
    auto setF = [&] (const String& id, float v)
    {
        if (auto* p = dynamic_cast<AudioParameterFloat*>(state.getParameter (id)))
        { p->beginChangeGesture(); p->setValueNotifyingHost (p->convertTo0to1 (v)); p->endChangeGesture(); }
    };
    float n = jlimit (0.f, 1.f, (e.x - r.getX()) / r.getWidth());
    float hz = std::pow (10.0f, jmap (n, std::log10 (20.f), std::log10 (20000.f)));
    float dB = jmap ((float) e.y, r.getBottom()-8, r.getY()+8, -18.f, 18.f);
    if (dragBand == 0) { setF (ReverbIDs::eqLowFreqHz,  hz); setF (ReverbIDs::eqLowGainDb,  dB); }
    if (dragBand == 1) { setF (ReverbIDs::eqMidFreqHz,  hz); setF (ReverbIDs::eqMidGainDb,  dB); }
    if (dragBand == 2) { setF (ReverbIDs::eqHighFreqHz, hz); setF (ReverbIDs::eqHighGainDb, dB); }
}


