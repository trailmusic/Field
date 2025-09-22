#include "ReverbCanvasComponent.h"
#include "../ReverbParamIDs.h"
#include "../../FieldLookAndFeel.h"

using namespace juce;

ReverbCanvasComponent::ReverbCanvasComponent(AudioProcessorValueTreeState& s,
                                             std::function<float()> erLevelNow,
                                             std::function<float()> tailLevelNow,
                                             std::function<float()> duckGrNow,
                                             std::function<float()> widthNow,
                                             std::function<double()> sampleRateNow)
: state(s), erNow(std::move(erLevelNow)), tailNow(std::move(tailLevelNow)),
  grNow(std::move(duckGrNow)), widthNowFn(std::move(widthNow)), srNow(std::move(sampleRateNow))
{
    setOpaque(false);
    startTimerHz(30);
}

void ReverbCanvasComponent::paint(Graphics& g)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    auto R = getLocalBounds().toFloat().reduced(6);
    drawBackground(g, R);
    drawTailHeatmapImage(g, R);
    drawER(g, R);
    drawWidthRotation(g, R);
    drawToneCurtainAndEQ(g, R);
    drawDucking(g, R);
    drawSpecials(g, R);

    g.setColour(th.text.withAlpha(0.20f));
    g.drawRoundedRectangle(R, 8.f, 1.2f);
}

void ReverbCanvasComponent::timerCallback()
{
    const float rt60 = getF2("reverb_rt60_s", ReverbIDs::decaySec, 2.4f);
    horizonSec = jlimit(0.6f, 8.0f, rt60 * 1.2f);

    const float rate = getF(ReverbIDs::modRateHz, 0.2f);
    phase = std::fmod(phase + (float)(2.0 * MathConstants<double>::pi) * rate / 30.f, (float) MathConstants<double>::twoPi);

    if (! getB(ReverbIDs::freeze, false))
        advanceHeatmapRow();

    repaint();
}

void ReverbCanvasComponent::drawBackground(Graphics& g, Rectangle<float> r)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    g.setColour(th.panel.withAlpha(0.45f));
    g.fillRoundedRectangle(r, 8.f);

    g.saveState();
    g.reduceClipRegion(r.toNearestInt());

    g.setColour(th.text.withAlpha(0.10f));
    for (float s : {0.5f, 1.f, 2.f, 4.f, 8.f})
    {
        if (s > horizonSec) break;
        const float y = jmap(s, 0.f, horizonSec, r.getY()+8.f, r.getBottom()-10.f);
        g.drawHorizontalLine((int) y, r.getX(), r.getRight());
    }
    for (float hz : {50.f,100.f,200.f,500.f,1000.f,2000.f,5000.f,10000.f})
    {
        const float x = r.getX() + log01FromHz(hz) * r.getWidth();
        g.drawVerticalLine((int) x, r.getY()+6.f, r.getBottom()-6.f);
    }
    g.restoreState();
}

void ReverbCanvasComponent::drawER(Graphics& g, Rectangle<float> r)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    const float preMs    = getF(ReverbIDs::preDelayMs, 0.f);
    const float densPct  = clamp01(getF(ReverbIDs::erDensityPct, 50.f) / 100.f);
    const float widthPct = clamp01(getF(ReverbIDs::erWidthPct, 50.f) / 100.f);
    const float lvlDb    = getF(ReverbIDs::erLevelDb, -12.f);
    const float toTail   = clamp01(getF(ReverbIDs::erToTailPct, 30.f) / 100.f);

    const int seed = (int) (preMs * 7) ^ (int) (densPct * 997) ^ (int) (widthPct * 1439);
    Random rng (seed);
    const int starCount = jlimit(8, 160, (int) juce::roundToInt(20 + densPct * 180));
    const float xSpread = widthPct * 0.45f;
    const float preY    = jmap(preMs, 0.f, 120.f, r.getY()+6.f, r.getY()+r.getHeight()*0.22f);

    g.saveState(); g.reduceClipRegion(r.toNearestInt());
    const float baseAlpha = jlimit(0.05f, 0.95f, Decibels::decibelsToGain(lvlDb + 12.f) * 0.9f);
    for (int i=0; i<starCount; ++i)
    {
        const float y = preY + rng.nextFloat() * r.getHeight()*0.08f;
        const float xMid = r.getCentreX();
        const float lr   = (rng.nextBool() ? 1.f : -1.f);
        const float px   = xMid + lr * (rng.nextFloat() - 0.5f) * r.getWidth() * xSpread;
        const float sz = 1.5f + rng.nextFloat()*2.5f;
        Colour c = th.text.withAlpha(baseAlpha * (0.7f + 0.3f * rng.nextFloat()));
        g.setColour(c);
        g.fillEllipse(px - sz*0.5f, y - sz*0.5f, sz, sz);
    }

    Path glue; const float y0 = preY + r.getHeight()*0.06f, y1 = r.getY() + r.getHeight()*0.32f;
    glue.startNewSubPath(r.getX()+6.f, y0);
    glue.quadraticTo(r.getCentreX(), y0 + (y1 - y0)*0.4f, r.getRight()-6.f, y1);
    g.setColour(th.hl.withAlpha(0.15f + 0.35f * toTail));
    g.strokePath(glue, PathStrokeType(juce::jmap(toTail, 1.0f, 4.0f)));
    g.restoreState();
}

void ReverbCanvasComponent::advanceHeatmapRow()
{
    const int H = heatmap.getHeight();
    const int W = heatmap.getWidth();
    const float dt = horizonSec / (float) H;
    tSeconds += dt;

    const float rt60Base = jlimit(0.2f, 20.f, getF2("reverb_rt60_s", ReverbIDs::decaySec, 2.4f));
    const float loX      = jlimit(0.3f, 2.0f, getF(ReverbIDs::dreqLowX, 1.f));
    const float midX     = jlimit(0.5f, 1.5f, getF(ReverbIDs::dreqMidX, 1.f));
    const float hiX      = jlimit(0.3f, 2.0f, getF(ReverbIDs::dreqHighX, 1.f));
    const float xLoHz    = jlimit(80.f, 800.f,     getF(ReverbIDs::dreqXoverLoHz, 250.f));
    const float xHiHz    = jlimit(1000.f, 10000.f, getF(ReverbIDs::dreqXoverHiHz, 4500.f));
    const float bloom01  = clamp01(getF(ReverbIDs::bloomPct, 35.f) / 100.f);
    const float modDepth = getF(ReverbIDs::modDepthCents, 0.f) / 50.f;
    const float tailLive = tailNow ? jlimit(0.f, 1.f, tailNow()) : 0.3f;

    const float gamma = juce::jmap(bloom01, 0.2f, 1.2f);
    const float alpha = juce::jmap(bloom01, 1.0f, 3.0f);

    for (int x = 0; x < W; ++x) heatmap.setPixelAt(x, writeRow, Colours::transparentBlack);

    Image::BitmapData bd (heatmap, Image::BitmapData::writeOnly);
    for (int x = 0; x < W; ++x)
    {
        const float f01 = (float) x / (float)(W - 1);
        const float hz  = hzFromLog01(f01);

        float mult = midX;
        if (hz < xLoHz)      mult = loX;
        else if (hz > xHiHz) mult = hiX;

        const float rt = jlimit(0.2f, 20.f, rt60Base * mult);
        float E = std::exp(-6.9078f * (tSeconds / rt));
        float bloomSkew = std::pow(1.f - std::exp(-alpha * tSeconds), gamma);
        E *= juce::jmap(bloom01, 1.f, 0.6f) * (0.4f + 0.6f * bloomSkew);
        const float shimmer = 1.f + 0.06f * modDepth * std::sin(phase + f01 * 7.0f);
        E *= shimmer;
        E *= (0.35f + 0.65f * tailLive);

        const uint8 a = (uint8) jlimit(0, 255, (int) std::round(jmin(1.f, E) * 255.f));
        bd.setPixelColour(x, writeRow, Colour::fromRGBA(255, 255, 255, a));
    }

    writeRow = (writeRow + 1) % heatmap.getHeight();
}

void ReverbCanvasComponent::drawTailHeatmapImage(Graphics& g, Rectangle<float> r)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    g.saveState(); g.reduceClipRegion(r.toNearestInt());
    const int W = heatmap.getWidth(), H = heatmap.getHeight();
    Image tinted (Image::ARGB, W, H, true);

    const float xLoHz  = jlimit(80.f, 800.f,     getF(ReverbIDs::dreqXoverLoHz, 250.f));
    const float xHiHz  = jlimit(1000.f, 10000.f, getF(ReverbIDs::dreqXoverHiHz, 4500.f));
    for (int y=0; y<H; ++y)
    {
        for (int x=0; x<W; ++x)
        {
            const Colour src = heatmap.getPixelAt(x, y);
            const float hz = hzFromLog01((float) x / (float)(W - 1));
            Colour tint = th.eq.tilt;
            if (hz < xLoHz)      tint = th.eq.bass;
            else if (hz > xHiHz) tint = th.eq.air;
            tinted.setPixelAt(x, y, tint.withAlpha(src.getAlpha() / 255.f * 0.9f));
        }
    }

    const float top = r.getY()+8.f, bottom = r.getBottom()-10.f, left = r.getX()+6.f, right = r.getRight()-6.f;
    const int h1 = H - writeRow;
    const Rectangle<float> dst1 (left, jmap(0.f, 0.f, horizonSec, top, bottom),
                                 right - left, jmap((float)h1 / (float)H * horizonSec, 0.f, horizonSec, 0.f, bottom-top));
    g.drawImageWithin(tinted, (int) dst1.getX(), (int) dst1.getY(), (int) dst1.getWidth(), (int) dst1.getHeight(), RectanglePlacement::stretchToFit, false);
    if (writeRow > 0)
    {
        const Rectangle<float> dst2 (left, jmap((float)h1 / (float)H * horizonSec, 0.f, horizonSec, top, bottom),
                                     right - left, jmap((float)writeRow / (float)H * horizonSec, 0.f, horizonSec, 0.f, bottom-top));
        g.drawImageWithin(tinted, (int) dst2.getX(), (int) dst2.getY(), (int) dst2.getWidth(), (int) dst2.getHeight(), RectanglePlacement::stretchToFit, false);
    }

    g.setColour(th.text.withAlpha(0.18f));
    for (float hz : { getF(ReverbIDs::dreqXoverLoHz, 250.f), getF(ReverbIDs::dreqXoverHiHz, 4500.f) })
    {
        const float x = left + log01FromHz(hz) * (right - left);
        g.drawVerticalLine((int) x, top, bottom);
    }

    g.restoreState();
}

void ReverbCanvasComponent::drawWidthRotation(Graphics& g, Rectangle<float> r)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    const float wStart = getF(ReverbIDs::widthStartPct, getF(ReverbIDs::widthPct, 100.f));
    const float wEnd   = getF(ReverbIDs::widthEndPct,   getF(ReverbIDs::widthPct, 100.f));
    const float rotS   = getF(ReverbIDs::rotStartDeg, 0.f);
    const float rotE   = getF(ReverbIDs::rotEndDeg,   0.f);

    const float top = r.getY()+8.f, bottom = r.getBottom()-10.f, left = r.getX()+6.f, right = r.getRight()-6.f;
    Path p; p.preallocateSpace(128);
    const int steps = 40;
    for (int i=0;i<steps;++i)
    {
        const float t01 = (float) i / (float) (steps-1);
        const float y   = jmap(t01, 0.f, 1.f, top, bottom);
        const float wCurve  = clamp01(0.5f + 0.5f * getF(ReverbIDs::widthEnvCurve, 0.f));
        const float rotCurve= clamp01(0.5f + 0.5f * getF(ReverbIDs::rotEnvCurve,   0.f));
        const float tw = std::pow(t01, 1.f + (wCurve * 1.5f - 0.75f));
        const float tr = std::pow(t01, 1.f + (rotCurve * 1.5f - 0.75f));
        const float widthPct = lerp(wStart, wEnd, tw);
        const float rotDeg   = lerp(rotS, rotE, tr);
        const float thickness = jmap(widthPct, 0.f, 120.f, 1.5f, 8.0f);
        const float tilt      = jmap(rotDeg,  -45.f, 45.f, -0.15f, 0.15f) * (right-left);
        const float xMid = (left + right) * 0.5f + tilt * (t01 - 0.5f);
        if (i==0) p.startNewSubPath(xMid, y); else p.lineTo(xMid, y);
        g.setColour(th.hl.withAlpha(0.10f));
        g.fillRect(xMid - thickness*0.5f, y-1.0f, thickness, 2.0f);
    }
    g.setColour(th.hl.withAlpha(0.55f));
    g.strokePath(p, PathStrokeType(1.6f));
}

void ReverbCanvasComponent::drawToneCurtainAndEQ(Graphics& g, Rectangle<float> r)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    const float hp = getF(ReverbIDs::hpfHz, 20.f);
    const float lp = getF(ReverbIDs::lpfHz, 20000.f);
    const float tiltDb = getF(ReverbIDs::tiltDb, 0.f);

    const float left = r.getX()+6.f, right = r.getRight()-6.f, top = r.getY()+8.f, bottom = r.getBottom()-10.f;

    g.setColour(th.text.withAlpha(0.10f));
    if (hp > 22.f)
    {
        const float x = left + log01FromHz(hp) * (right-left);
        g.fillRect(Rectangle<float>(left, top, x-left, bottom-top));
    }
    if (lp < 19000.f)
    {
        const float x = left + log01FromHz(lp) * (right-left);
        g.fillRect(Rectangle<float>(x, top, right-x, bottom-top));
    }

    if (std::abs(tiltDb) > 0.05f)
    {
        Colour a = th.eq.air.withAlpha(0.08f), b = th.eq.bass.withAlpha(0.08f);
        if (tiltDb < 0) std::swap(a,b);
        Graphics::ScopedSaveState sss (g);
        g.setGradientFill(ColourGradient(a, right, r.getCentreY(), b, left, r.getCentreY(), false));
        g.fillRoundedRectangle(r.reduced(6.f), 7.f);
    }

    if (getB(ReverbIDs::eqOn, false))
    {
        const double fs = srNow ? srNow() : 48000.0;
        const double lf = getF(ReverbIDs::eqLowFreqHz, 150.f);
        const double lg = getF(ReverbIDs::eqLowGainDb, 0.f);
        const double lq = jmax(0.1, (double)getF(ReverbIDs::eqLowQ, 0.7f));
        const double mf = getF(ReverbIDs::eqMidFreqHz, 1000.f);
        const double mg = getF(ReverbIDs::eqMidGainDb, 0.f);
        const double mq = jmax(0.1, (double)getF(ReverbIDs::eqMidQ, 1.0f));
        const double hf = getF(ReverbIDs::eqHighFreqHz, 8000.f);
        const double hg = getF(ReverbIDs::eqHighGainDb, 0.f);
        const double hq = jmax(0.1, (double)getF(ReverbIDs::eqHighQ, 0.7f));
        const float  mix = clamp01(getF(ReverbIDs::postEqMixPct, 100.f)/100.f);

        double b0,b1,b2,a0,a1,a2;
        Path pre, post;
        const int N = jmax(64, (int) r.getWidth());
        for (int i=0;i<N;++i)
        {
            const float n = (float) i / (float)(N-1);
            const float hz = hzFromLog01(n);
            const double w = 2.0 * MathConstants<double>::pi * (double) hz / fs;
            const float yPre = jmap(0.f, 18.f, -18.f, r.getY()+8.f, r.getBottom()-10.f);

            RBJ::lowShelf(fs, lf, lq, lg, b0,b1,b2,a0,a1,a2);  double H1 = RBJ::magAt(b0,b1,b2,a0,a1,a2, w);
            RBJ::peaking (fs, mf, mq, mg, b0,b1,b2,a0,a1,a2);  double H2 = RBJ::magAt(b0,b1,b2,a0,a1,a2, w);
            RBJ::highShelf(fs, hf, hq, hg, b0,b1,b2,a0,a1,a2); double H3 = RBJ::magAt(b0,b1,b2,a0,a1,a2, w);

            const float dB = (float) Decibels::gainToDecibels(H1*H2*H3, -200.0);
            const float x  = jmap(n, 0.f, 1.f, r.getX()+6.f, r.getRight()-6.f);
            const float y  = jmap(dB, 18.f, -18.f, r.getY()+8.f, r.getBottom()-10.f);
            if (i==0) { pre.startNewSubPath(x, yPre); post.startNewSubPath(x, y); }
            else      { pre.lineTo(x, yPre);         post.lineTo(x, y); }
        }
        g.setColour(th.text.withAlpha(0.15f * (1.f - mix)));
        g.strokePath(pre, PathStrokeType(1.0f));
        g.setColour(th.accent.withAlpha(0.65f * mix));
        g.strokePath(post, PathStrokeType(2.0f));
    }
}

void ReverbCanvasComponent::drawDucking(Graphics& g, Rectangle<float> r)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    const float GR = grNow ? jmax(0.f, grNow()) : 0.f;
    if (GR <= 0.01f) return;
    const float left = r.getX()+6.f, right = r.getRight()-6.f, top = r.getY()+8.f;
    const float widthPx = jmap(clamp01(GR / jmax(1.f, getF(ReverbIDs::duckDepthDb, 12.f))), 0.f, 1.f, 0.f, (right-left) * 0.35f);
    g.setColour(th.text.withAlpha(0.60f));
    g.drawText("Ducking", Rectangle<int>((int) left, (int) top-2, 80, 14), Justification::left, false);
    g.setColour(th.hl); g.fillRoundedRectangle(left, top+14.f, widthPx, 8.f, 4.f);
    g.setColour(th.text.withAlpha(0.90f));
    g.drawText(String(GR, 1) + " dB GR", Rectangle<int>((int) (left+widthPx+8.f), (int) (top+10.f), 90, 16), Justification::left, false);
}

void ReverbCanvasComponent::drawSpecials(Graphics& g, Rectangle<float> r)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    if (getB(ReverbIDs::freeze, false))
    {
        g.setColour(th.accent.withAlpha(0.10f));
        g.drawRoundedRectangle(r.reduced(10.f), 10.f, 2.0f);
        static float ang = 0.f; ang = std::fmod(ang + 0.01f, MathConstants<float>::twoPi);
        const float cx = r.getCentreX(), cy = r.getCentreY();
        const float radx = r.getWidth() * 0.42f, rady = r.getHeight() * 0.30f;
        const float px = cx + std::cos(ang) * radx, py = cy + std::sin(ang) * rady;
        g.fillEllipse(px-3.f, py-3.f, 6.f, 6.f);
    }

    const float dist = clamp01(getF(ReverbIDs::distancePct, 35.f)/100.f);
    if (dist > 0.02f)
    {
        Colour aC = th.eq.air.withAlpha(0.04f * dist);
        Graphics::ScopedSaveState sss (g);
        g.setGradientFill(ColourGradient(aC, r.getRight(), r.getCentreY(), Colours::transparentBlack, r.getX(), r.getCentreY(), false));
        g.fillRoundedRectangle(r.reduced(6.f), 7.f);
    }

    const float shim = clamp01(getF(ReverbIDs::shimmerAmtPct, 0.f)/100.f);
    if (shim > 0.01f)
    {
        const float left = r.getX()+6.f, right = r.getRight()-6.f, top = r.getY()+8.f, bottom = r.getBottom()-10.f;
        Path ridge; const int N = 64;
        for (int i=0;i<N;++i)
        {
            const float n = (float) i / (float) (N-1);
            const float hz = hzFromLog01(n);
            const float hz2 = jmin(20000.f, hz * 2.f);
            const float x  = left + log01FromHz(hz2) * (right-left);
            const float y  = jmap(0.66f, 0.f, 1.f, top, bottom);
            if (i==0) ridge.startNewSubPath(x, y); else ridge.lineTo(x, y);
        }
        g.setColour(th.eq.air.withAlpha(0.25f * shim));
        g.strokePath(ridge, PathStrokeType(1.6f));
    }

    const float gate = clamp01(getF(ReverbIDs::gateAmtPct, 0.f)/100.f);
    if (gate > 0.01f)
    {
        g.setColour(Colours::black.withAlpha(0.05f * gate));
        auto rr = r.reduced(6.f);
        for (float y = rr.getY(); y < rr.getBottom(); y += 16.f)
            g.fillRect(rr.getX(), y, rr.getWidth(), 8.f);
    }
}

// === RBJ ===
void ReverbCanvasComponent::RBJ::lowShelf(double fs, double f0, double Q, double gainDb, double& b0,double& b1,double& b2,double& a0,double& a1,double& a2)
{
    const double A = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * MathConstants<double>::pi * f0 / fs;
    const double cosw0 = std::cos(w0), sinw0 = std::sin(w0);
    const double alpha = sinw0 / (2.0 * Q);
    const double sqrtA = std::sqrt(A);
    b0 =    A*( (A+1) - (A-1)*cosw0 + 2*sqrtA*alpha );
    b1 =  2*A*( (A-1) - (A+1)*cosw0 );
    b2 =    A*( (A+1) - (A-1)*cosw0 - 2*sqrtA*alpha );
    a0 =        (A+1) + (A-1)*cosw0 + 2*sqrtA*alpha;
    a1 =   -2*( (A-1) + (A+1)*cosw0 );
    a2 =        (A+1) + (A-1)*cosw0 - 2*sqrtA*alpha;
}
void ReverbCanvasComponent::RBJ::peaking(double fs, double f0, double Q, double gainDb, double& b0,double& b1,double& b2,double& a0,double& a1,double& a2)
{
    const double A = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * MathConstants<double>::pi * f0 / fs;
    const double cosw0 = std::cos(w0), sinw0 = std::sin(w0);
    const double alpha = sinw0 / (2.0 * Q);
    b0 = 1 + alpha*A; b1 = -2*cosw0; b2 = 1 - alpha*A;
    a0 = 1 + alpha/A; a1 = -2*cosw0; a2 = 1 - alpha/A;
}
void ReverbCanvasComponent::RBJ::highShelf(double fs, double f0, double Q, double gainDb, double& b0,double& b1,double& b2,double& a0,double& a1,double& a2)
{
    const double A = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * MathConstants<double>::pi * f0 / fs;
    const double cosw0 = std::cos(w0), sinw0 = std::sin(w0);
    const double alpha = sinw0 / (2.0 * Q);
    const double sqrtA = std::sqrt(A);
    b0 =    A*( (A+1) + (A-1)*cosw0 + 2*sqrtA*alpha );
    b1 = -2*A*( (A-1) + (A+1)*cosw0 );
    b2 =    A*( (A+1) + (A-1)*cosw0 - 2*sqrtA*alpha );
    a0 =        (A+1) - (A-1)*cosw0 + 2*sqrtA*alpha;
    a1 =    2*( (A-1) - (A+1)*cosw0 );
    a2 =        (A+1) - (A-1)*cosw0 - 2*sqrtA*alpha;
}
double ReverbCanvasComponent::RBJ::magAt(double b0,double b1,double b2,double a0,double a1,double a2, double omega)
{
    const std::complex<double> ejw (std::cos(omega), std::sin(omega));
    const std::complex<double> ejw2 = ejw * ejw;
    const std::complex<double> num = b0 + b1/ejw + b2/ejw2;
    const std::complex<double> den = a0 + a1/ejw + a2/ejw2;
    return std::abs(num / den);
}

float ReverbCanvasComponent::getF(const String& id, float fb) const
{
    if (auto* p = dynamic_cast<AudioParameterFloat*>(state.getParameter(id))) return p->get();
    return fb;
}
float ReverbCanvasComponent::getF2(const String& idA, const String& idB, float fb) const
{
    if (auto* p = dynamic_cast<AudioParameterFloat*>(state.getParameter(idA))) return p->get();
    if (auto* q = dynamic_cast<AudioParameterFloat*>(state.getParameter(idB))) return q->get();
    return fb;
}
int ReverbCanvasComponent::getI(const String& id, int fb) const
{
    if (auto* p = dynamic_cast<AudioParameterChoice*>(state.getParameter(id))) return p->getIndex();
    if (auto* q = dynamic_cast<AudioParameterInt*>(state.getParameter(id)))   return q->get();
    return fb;
}
bool ReverbCanvasComponent::getB(const String& id, bool fb) const
{
    if (auto* p = dynamic_cast<AudioParameterBool*>(state.getParameter(id))) return p->get();
    return fb;
}


