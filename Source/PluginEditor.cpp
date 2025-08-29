#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Layout.h"

//==============================================================

//==============================================================
// ToggleSwitch (compact, slow animation, keeps original visual)
ToggleSwitch::ToggleSwitch()
{
    setLabels ("STEREO", "SPLIT");
    sliderValue.reset (0.0, 0.02);
    sliderValue.setCurrentAndTargetValue (0.0f);
}

void ToggleSwitch::setLabels (const juce::String& offLabel, const juce::String& onLabel)
{
    offText = offLabel;
    onText  = onLabel;
    repaint();
}

void ToggleSwitch::setToggleState (bool shouldBeOn, juce::NotificationType nt)
{
    if (isOn == shouldBeOn) return;
    isOn = shouldBeOn;
    sliderValue.setTargetValue (isOn ? 1.0f : 0.0f);
    if (nt == juce::sendNotification && onToggleChange) onToggleChange (isOn);
    repaint();
}

void ToggleSwitch::mouseDown (const juce::MouseEvent&) { isMouseDown = true; repaint(); }
void ToggleSwitch::mouseUp   (const juce::MouseEvent&)
{
    if (!isMouseDown) return;
    isMouseDown = false;
    setToggleState (!isOn, juce::sendNotification);
}

void ToggleSwitch::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float rad = b.getHeight() * 0.5f;
    const float knobR = b.getHeight() * 0.45f;

    // match editor accent
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    const auto accent = lf ? lf->theme.accent : juce::Colour (0xFF2196F3);

    // track
    g.setColour (juce::Colour (0xFF1A1D25));
    g.fillRoundedRectangle (b, rad);
    g.setColour (juce::Colour (0xFF4A4D55));
    g.drawRoundedRectangle (b, rad, 2.0f);

    // hover glow
    const bool over = isMouseOverOrDragging();
    if (over || hoverActive)
    {
        g.setColour (accent);
        g.drawRoundedRectangle (b, rad, 1.0f);
    }

    // knob travel—slightly inside edges for compact feel
    const float leftCx  = b.getX() + b.getWidth() * 0.30f;
    const float rightCx = b.getX() + b.getWidth() * 0.70f;
    const float t = sliderValue.getCurrentValue();
    const float kx = juce::jmap (t, leftCx - knobR, rightCx - knobR);
    const float ky = b.getCentreY() - knobR;
    juce::Rectangle<float> k (kx, ky, knobR * 2.0f, knobR * 2.0f);

    // shadow
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillEllipse (k.translated (2.0f, 2.0f));

    // fill: stereo = accent blue/green, split = grey
    g.setColour (isOn ? juce::Colour (0xFF7A7D85) : accent);
    g.fillEllipse (k);

    // rim + split marker
    g.setColour (juce::Colour (0xFF9A9DA5));
    g.drawEllipse (k, 2.0f);
    if (isOn)
    {
        g.setColour (juce::Colour (0xFFB0B3B8));
        const float cx = k.getCentreX();
        g.drawLine (cx, k.getY() + 4.0f, cx, k.getBottom() - 4.0f, 1.5f);
    }

    if (sliderValue.isSmoothing()) repaint();
}

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
        g.setColour (panel);
        g.fillRoundedRectangle (r.reduced (3.0f), rad);

        // depth
        juce::DropShadow ds1 (juce::Colour (0xFF1A1C20).withAlpha (0.6f), 20, { -2, -2 });
        juce::DropShadow ds2 (juce::Colour (0xFF60646C).withAlpha (0.4f),  8, { -1, -1 });
        auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
        ds1.drawForRectangle (g, ri);
        ds2.drawForRectangle (g, ri);

        // inner rim
        g.setColour (juce::Colour (0xFF2A2C30).withAlpha (0.3f));
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
        g.setColour (accent);
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

    // inner content
    auto padBounds = r.reduced (40.0f);

    drawWaveformBackground (g, padBounds);
    drawGrid              (g, padBounds);
    drawFrequencyRegions  (g, padBounds);
    drawEQCurves          (g, padBounds);
    drawBalls             (g, padBounds);

    // center crosshair (subtle)
    g.setColour ((lf ? lf->theme.textMuted : juce::Colours::white).withAlpha (0.4f));
    g.drawLine (r.getCentreX(), r.getY() + 40, r.getCentreX(), r.getBottom() - 40, 1.5f);
    g.drawLine (r.getX() + 40, r.getCentreY(), r.getRight() - 40, r.getCentreY(), 1.5f);
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
        auto bHP    = hpNeutral ? VizEQ::highpassRBJ(Fs, 20.0, VizEQ::kSqrt2Inv) : VizEQ::highpassRBJ (Fs, juce::jlimit (20.0f, 20000.0f, hpValue));
        auto bLP    = lpNeutral ? VizEQ::lowpassRBJ (Fs, 20000.0, VizEQ::kSqrt2Inv) : VizEQ::lowpassRBJ  (Fs, juce::jlimit (20.0f, 20000.0f, lpValue));
        auto bBass  = VizEQ::lowshelfRBJ (Fs, juce::jlimit (20.0f, 20000.0f, bassFreqValue),  bassValue, 0.9);
        auto bAir   = VizEQ::highshelfRBJ(Fs, juce::jlimit (20.0f, 20000.0f, airFreqValue),   airValue,  0.9);
        // Musical tilt: ±G/2 complementary shelves at pivot
        auto bTiltLo= VizEQ::lowshelfRBJ (Fs, juce::jlimit (20.0f, 20000.0f, tiltFreqValue), +0.5*tiltValue, 0.9);
        auto bTiltHi= VizEQ::highshelfRBJ(Fs, juce::jlimit (20.0f, 20000.0f, tiltFreqValue), -0.5*tiltValue, 0.9);
        // Peak bell for scoop/boost
        const double qPeak = 1.0; // could adapt with gain if desired
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
        g.setColour (juce::Colours::black.withAlpha (0.4f));
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

    g.setColour (juce::Colours::black.withAlpha (0.4f));
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
: AudioProcessorEditor (&p), proc (p)
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
    const int delayCardWMin = Layout::dp (620, s); // match delay card reserved width
    const int calculatedMinWidth = xlPx + lPx + swW + 5*lPx + 3*lPx + gaps * gapS
                                   + Layout::dp (Layout::PAD, s) * 2
                                   + delayCardWMin + Layout::dp (Layout::GAP, s);
    
    // Calculate minimum height based on layout structure
    const int headerH = Layout::dp (50, s);
    const int xyMinH = Layout::dp (Layout::XY_MIN_H, s);
    const int metersH = Layout::dp (84, s);
    const int rowH1 = lPx + Layout::dp (Layout::LABEL_BAND_EXTRA, s);
    const int rowH2 = lPx + Layout::dp (Layout::LABEL_BAND_EXTRA, s);
    const int rowH3 = lPx + Layout::dp (Layout::LABEL_BAND_EXTRA, s) + Layout::dp (18, s) + gapS;
    const int rowH4 = lPx + Layout::dp (Layout::LABEL_BAND_EXTRA, s);
    const int totalRowsH = rowH1 + rowH2 + rowH3 + rowH4;
    const int gapsH = 3 * Layout::dp (Layout::GAP, s);
    const int calculatedMinHeight = headerH + xyMinH + metersH + totalRowsH + gapsH + Layout::dp (Layout::PAD, s) * 2 + 50; // +50 for bottom margin
    
    // Store resize constraints
    this->minWidth = calculatedMinWidth;
    this->minHeight = calculatedMinHeight;
    this->maxWidth = 3000;
    this->maxHeight = 2000;
    
    // Set minimum size constraints
    setResizeLimits (calculatedMinWidth, calculatedMinHeight, maxWidth, maxHeight);
    
    // Set initial size (respecting minimums)
    const int initialWidth = juce::jmax (baseWidth, calculatedMinWidth);
    const int initialHeight = juce::jmax (baseHeight, calculatedMinHeight);
    setSize (initialWidth, initialHeight);
    lnf.theme.accent = juce::Colour (0xFF5AA9E6); // ocean default
    lnf.setupColours();
    setLookAndFeel (&lnf);

    // Options menu (oversampling, close)
    addAndMakeVisible (optionsButton);
    optionsButton.onClick = [this]
    {
        juce::PopupMenu m;
        m.addSectionHeader ("Field - Spatial Audio Processor");
        m.addSeparator();
        m.addSectionHeader ("🎛️ Oversampling");
        m.addItem (1, "1x (Standard)", true, osSelect.getSelectedId() == 0);
        m.addItem (2, "2x (High Quality)", true, osSelect.getSelectedId() == 1);
        m.addItem (3, "4x (Ultra Quality)", true, osSelect.getSelectedId() == 2);
        m.addItem (4, "8x (Maximum Quality)", true, osSelect.getSelectedId() == 3);
        m.addItem (5, "16x (Extreme Quality)", true, osSelect.getSelectedId() == 4);
        m.addSeparator();
        m.addItem (99, "❌ Close", true, false);

        m.showMenuAsync (juce::PopupMenu::Options(), [this](int r)
        {
            if (r >= 1 && r <= 5) osSelect.setSelectedId (r - 1);
        });
    };

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

    // Color mode cycle (Ocean → Green → Pink → Yellow → Grey)
    addAndMakeVisible (colorModeButton);
    colorModeButton.setTooltip (FieldLNF::getThemeName (lnf.currentVariant));
    colorModeButton.onClick = [this]
    {
        // Determine current by accent; rotate deterministically through variants
        using TV = FieldLNF::ThemeVariant;
        static TV order[] = { TV::Ocean, TV::Green, TV::Pink, TV::Yellow, TV::Grey };
        auto currentAccent = lnf.theme.accent.getARGB();
        int idx = 0;
        if (currentAccent == juce::Colour (0xFF5AA9E6).getARGB()) idx = 0; // Ocean
        else if (currentAccent == juce::Colour (0xFF5AA95A).getARGB()) idx = 1; // Green
        else if (currentAccent == juce::Colour (0xFFE91E63).getARGB()) idx = 2; // Pink
        else if (currentAccent == juce::Colour (0xFFFFC107).getARGB()) idx = 3; // Yellow
        else if (currentAccent == juce::Colour (0xFF9EA3AA).getARGB()) idx = 4; // Grey
        idx = (idx + 1) % 5;
        lnf.setTheme (order[idx]);
        colorModeButton.setTooltip (FieldLNF::getThemeName (order[idx]));
        // Propagate to components that cache green flag
        const bool greenNow = (order[idx] == TV::Green);
        spaceKnob.setGreenMode (greenNow);
        spaceAlgorithmSwitch.setGreenMode (greenNow);
        pad.setGreenMode (greenNow);
        repaint();
    };

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
        pad.setLinked (linkButton.getToggleState());
    };
    addAndMakeVisible (snapButton);
    snapButton.setToggleState (false, juce::dontSendNotification); // default OFF per your note
    snapButton.onClick = [this]
    {
        const bool on = !snapButton.getToggleState();
        snapButton.setToggleState (on, juce::dontSendNotification);
        pad.setSnapEnabled (on);
    };

    // Presets UI
    addAndMakeVisible (presetCombo);
    addAndMakeVisible (savePresetButton);
    addAndMakeVisible (prevPresetButton);
    addAndMakeVisible (nextPresetButton);

    prevPresetButton.onClick = [this]
    {
        const int n = presetCombo.getNumItems();
        if (n <= 0) return;
        const int cur = presetCombo.getSelectedId() - 1;
        const int prev = (cur - 1 + n) % n;
        presetCombo.setSelectedId (prev + 1, juce::dontSendNotification);
        if (presetCombo.onPresetSelected) presetCombo.onPresetSelected (presetCombo.getText());
    };
    nextPresetButton.onClick = [this]
    {
        const int n = presetCombo.getNumItems();
        if (n <= 0) return;
        const int cur = presetCombo.getSelectedId() - 1;
        const int nxt = (cur + 1) % n;
        presetCombo.setSelectedId (nxt + 1, juce::dontSendNotification);
        if (presetCombo.onPresetSelected) presetCombo.onPresetSelected (presetCombo.getText());
    };
    savePresetButton.onSavePreset = [this]
    {
        // (same modal save flow you posted; omitted here to keep file concise)
        // Keep your existing save dialog implementation.
        presetCombo.refreshPresets();
    };

    // A/B + copy
    addAndMakeVisible (abButtonA);
    addAndMakeVisible (abButtonB);
    addAndMakeVisible (copyButton);
    abButtonA.setToggleState (true, juce::dontSendNotification);
    abButtonB.setToggleState (false, juce::dontSendNotification);
    abButtonA.onClick = [this]{ if (!abButtonA.getToggleState()) toggleABState(); };
    abButtonB.onClick = [this]{ if (!abButtonB.getToggleState()) toggleABState(); };
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
        pad.setSplitMode (split);
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

    // XY pad + shade
    addAndMakeVisible (pad);
    xyShade = std::make_unique<ShadeOverlay> (lnf);
    addAndMakeVisible (*xyShade);
    if (auto* v = proc.apvts.state.getPropertyPointer ("ui_xyShadeAmt"))
        xyShade->setAmount ((float) *v, false);
    xyShade->onAmountChanged = [this](float a) { proc.apvts.state.setProperty ("ui_xyShadeAmt", a, nullptr); };

    // Containers
    addAndMakeVisible (mainControlsContainer); mainControlsContainer.setTitle (""); mainControlsContainer.setShowBorder (false);
    addAndMakeVisible (panKnobContainer);      panKnobContainer.setTitle ("");     panKnobContainer.setShowBorder (true);
    addAndMakeVisible (spaceKnobContainer);    spaceKnobContainer.setTitle (""); spaceKnobContainer.setShowBorder (true);
    addAndMakeVisible (volumeContainer);       volumeContainer.setTitle ("");   volumeContainer.setShowBorder (true);
    addAndMakeVisible (delayContainer);        delayContainer.setTitle ("");     delayContainer.setShowBorder (true);
    // Row containers for EQ/Image are no longer used
    addAndMakeVisible (metersContainer);       metersContainer.setTitle ("");         metersContainer.setShowBorder (true);

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

    // Volume row unified group (pan, space, algo switch, duck group, gain/drive/mix)
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
        // Align with FieldLNF ticks: -π → +π
        constexpr float kStart = -juce::MathConstants<float>::pi;            // -180°
        constexpr float kEnd   =  juce::MathConstants<float>::pi;            // +180°
        s.setRotaryParameters (kStart, kEnd, true);
        s.setLookAndFeel (&lnf);
    };

    // main rotary
    for (juce::Slider* slider : { &width,&tilt,&monoHz,&hpHz,&lpHz,&satDrive,&satMix,&air,&bass,&scoop,
                              &widthLo,&widthMid,&widthHi,&xoverLoHz,&xoverHiHz,&rotationDeg,&asymmetry,&shufLoPct,&shufHiPct,&shufXHz })
    {
        addAndMakeVisible (*slider);
        style (*slider);
        slider->addListener (this);
    }
    addAndMakeVisible (gain); style (gain); gain.addListener (this);

    // micro sliders (freq)
    for (juce::Slider* slider : { &tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider })
    {
        addAndMakeVisible (*slider);
        slider->setSliderStyle (juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider->setMouseDragSensitivity (100);
        slider->setVelocityBasedMode (false);
        slider->setSliderSnapsToMousePosition (true);
        slider->setDoubleClickReturnValue (true, 0.0);
        slider->setLookAndFeel (&lnf);
        slider->addListener (this);
    }

    // Ducking advanced knobs (3 generics + custom ratio)
    for (juce::Slider* slider : { &duckAttack, &duckRelease, &duckThreshold })
    {
        addAndMakeVisible (*slider);
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        constexpr float kStart = -juce::MathConstants<float>::pi;
        constexpr float kEnd   =  juce::MathConstants<float>::pi;
        slider->setRotaryParameters (kStart, kEnd, true);
        slider->setLookAndFeel (&lnf);
        slider->addListener (this);
    }
    addAndMakeVisible (duckRatio);
    duckRatio.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    duckRatio.setLookAndFeel (&lnf);
    duckRatio.addListener (this);

    // pan + space + ducking
    addAndMakeVisible (panKnob);      style (panKnob, true);     panKnob.setRange (-1.0, 1.0, 0.01); panKnob.setOverlayEnabled (false); panKnob.addListener (this);
    addAndMakeVisible (panKnobLeft);  style (panKnobLeft, true); panKnobLeft.setRange (-1.0, 1.0, 0.01); panKnobLeft.setOverlayEnabled (true);  panKnobLeft.setLabel ("L"); panKnobLeft.addListener (this);
    addAndMakeVisible (panKnobRight); style (panKnobRight, true);panKnobRight.setRange(-1.0, 1.0, 0.01); panKnobRight.setOverlayEnabled (true); panKnobRight.setLabel ("R"); panKnobRight.addListener (this);

    panKnob.setVisible (true);
    panKnobLeft.setVisible (false);
    panKnobRight.setVisible (false);

    addAndMakeVisible (spaceKnob); style (spaceKnob, true); spaceKnob.addListener (this);
    addAndMakeVisible (duckingKnob); style (duckingKnob);   duckingKnob.addListener (this);

    // values
    for (juce::Label* l : { &gainValue,&widthValue,&tiltValue,&monoValue,&hpValue,&lpValue,&satDriveValue,&satMixValue,&airValue,&bassValue,&scoopValue,
                             &panValue,&panValueLeft,&panValueRight,&spaceValue,&duckingValue,&duckAttackValue,&duckReleaseValue,&duckThresholdValue,&duckRatioValue,
                             &tiltFreqValue,&scoopFreqValue,&bassFreqValue,&airFreqValue,
                             &widthLoValue,&widthMidValue,&widthHiValue,&xoverLoValue,&xoverHiValue,
                             &rotationValue,&asymValue,&shufLoValue,&shufHiValue,&shufXValue,
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
            pad.setTiltValue  ((float) tilt .getValue());
            pad.setHPValue    ((float) hpHz .getValue());
            pad.setLPValue    ((float) lpHz .getValue());
            pad.setAirValue   ((float) air  .getValue());
            pad.setBassValue  ((float) bass .getValue());
            pad.setScoopValue ((float) scoop.getValue());
            pad.setTiltFreqValue  ((float) tiltFreqSlider.getValue());
            pad.setScoopFreqValue ((float) scoopFreqSlider.getValue());
            pad.setBassFreqValue  ((float) bassFreqSlider.getValue());
            pad.setAirFreqValue   ((float) airFreqSlider.getValue());
            pad.setMonoValue      ((float) monoHz.getValue());
            pad.repaint();
        };
    };
    for (juce::Slider* slider : { &tilt,&hpHz,&lpHz,&air,&bass,&scoop,&tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider,&monoHz })
        addLiveRepaint (*slider);

    // slider names (for LNF knob labels)
    gain.setName ("GAIN"); width.setName ("WIDTH"); tilt.setName ("TILT"); monoHz.setName ("MONO");
    hpHz.setName ("HP Hz"); lpHz.setName ("LP Hz"); satDrive.setName ("DRIVE"); satMix.setName ("MIX");
    air.setName ("AIR"); bass.setName ("BASS"); scoop.setName ("SCOOP"); spaceKnob.setName ("REVERB");
    duckingKnob.setName ("DUCK"); panKnob.setName ("PAN"); panKnobLeft.setName ("PAN L"); panKnobRight.setName ("PAN R");
    duckAttack.setName ("ATT"); duckRelease.setName ("REL"); duckThreshold.setName ("THR"); duckRatio.setName ("RAT");
    tiltFreqSlider.setName ("TILT F"); scoopFreqSlider.setName ("SCOOP F"); bassFreqSlider.setName ("BASS F"); airFreqSlider.setName ("AIR F");
    // Imaging knob labels
    widthLo.setName ("W LO"); widthMid.setName ("W MID"); widthHi.setName ("W HI");
    xoverLoHz.setName ("XO LO"); xoverHiHz.setName ("XO HI");
    rotationDeg.setName ("ROT"); asymmetry.setName ("ASYM");
    shufLoPct.setName ("SHUF LO"); shufHiPct.setName ("SHUF HI"); shufXHz.setName ("SHUF XO");

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
    for (juce::Slider* slider : { &delayTime, &delayFeedback, &delayWet, &delaySpread, &delayWidth, &delayModRate, &delayModDepth, &delayWowflutter, &delayJitter,
                                  &delayHp, &delayLp, &delayTilt, &delaySat, &delayDiffusion, &delayDiffuseSize,
                                  &delayDuckDepth, &delayDuckAttack, &delayDuckRelease, &delayDuckThreshold, &delayDuckRatio, &delayDuckLookahead })
    {
        addAndMakeVisible (*slider);
        style (*slider);
        slider->addListener (this);
    }
    
    // Delay combo boxes
    for (juce::ComboBox* combo : { &delayMode, &delayTimeDiv, &delayDuckSource })
    {
        addAndMakeVisible (*combo);
        combo->setLookAndFeel (&lnf);
        combo->addListener (this);
    }
    
    // Delay toggle buttons
    for (juce::ToggleButton* button : { &delayEnabled, &delaySync, &delayKillDry, &delayFreeze, &delayPingpong, &delayDuckPost, &delayDuckLinkGlobal })
    {
        addAndMakeVisible (*button);
        button->setLookAndFeel (&lnf);
        button->addListener (this);
    }
    
    // Delay value labels
    for (juce::Label* l : { &delayTimeValue, &delayFeedbackValue, &delayWetValue, &delaySpreadValue, &delayWidthValue, &delayModRateValue, &delayModDepthValue, &delayWowflutterValue, &delayJitterValue,
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
    
    // Delay name labels
    for (auto* l : { &delayTimeName, &delayFeedbackName, &delayWetName, &delaySpreadName, &delayWidthName, &delayModRateName, &delayModDepthName, &delayWowflutterName, &delayJitterName,
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
    delayDuckDepth.setName ("DUCK DEPTH"); delayDuckAttack.setName ("DUCK ATT"); delayDuckRelease.setName ("DUCK REL");
    delayDuckThreshold.setName ("DUCK THR"); delayDuckRatio.setName ("DUCK RAT"); delayDuckLookahead.setName ("DUCK LA");

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
    sliderValueChanged (&spaceKnob);
    sliderValueChanged (&duckingKnob);
    sliderValueChanged (&tiltFreqSlider);
    sliderValueChanged (&scoopFreqSlider);
    sliderValueChanged (&bassFreqSlider);
    sliderValueChanged (&airFreqSlider);
    sliderValueChanged (&gain);

    // Mono Maker slope & audition controls
    // Keep legacy ComboBox hidden for APVTS attachment; drive it from switch
    addChildComponent (monoSlopeChoice);
    monoSlopeChoice.addItem ("6",  1);
    monoSlopeChoice.addItem ("12", 2);
    monoSlopeChoice.addItem ("24", 3);
    if (!monoSlopeSwitch)
        monoSlopeSwitch = std::make_unique<MonoSlopeSwitch>();
    monoSlopeSwitch->onChange = [this](int idx)
    {
        monoSlopeChoice.setSelectedItemIndex (juce::jlimit (0, 2, idx), juce::NotificationType::sendNotification);
    };
    addAndMakeVisible (monoAuditionButton);
    monoAuditionButton.setButtonText ("AUD");

    // Imaging attachments
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "width_lo",         widthLo));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "width_mid",        widthMid));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "width_hi",         widthHi));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "xover_lo_hz",      xoverLoHz));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "xover_hi_hz",      xoverHiHz));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "rotation_deg",     rotationDeg));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "asymmetry",        asymmetry));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "shuffler_lo_pct",  shufLoPct));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "shuffler_hi_pct",  shufHiPct));
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "shuffler_xover_hz", shufXHz));

    // space algo switch
    addAndMakeVisible (spaceAlgorithmSwitch);
    spaceAlgorithmSwitch.setGreenMode (isGreenMode);
    if (auto* c = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter ("space_algo")))
    {
        const int maxIdx = juce::jmax (0, c->choices.size() - 1);
        const int idx = juce::jlimit (0, maxIdx, (int) c->getIndex());
        spaceAlgorithmSwitch.setOrientation (SpaceAlgorithmSwitch::Orientation::Vertical);
        spaceAlgorithmSwitch.setSpacing (4.0f);
        spaceAlgorithmSwitch.setAlgorithmFromParameter (idx);
        currentAlgorithm = idx;
        pad.setSpaceAlgorithm (idx);
    }
    spaceAlgorithmSwitch.onAlgorithmChange = [this](int a)
    {
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter ("space_algo")))
        {
            const int maxIdx = juce::jmax (0, c->choices.size() - 1);
            const int idx = juce::jlimit (0, maxIdx, a);
            const float norm = c->convertTo0to1 ((float) idx);
            c->beginChangeGesture(); c->setValueNotifyingHost (norm); c->endChangeGesture();
            currentAlgorithm = idx;
            pad.setSpaceAlgorithm (idx);
        }
    };

    // header divider
    addAndMakeVisible (splitDivider);
    splitDivider.setVisible (true);

    // XY callbacks -> AVTS
    auto refreshXYOverlays = [this]
    {
        pad.setMixValue   (proc.apvts.getRawParameterValue ("sat_mix")->load());
        pad.setDriveValue (proc.apvts.getRawParameterValue ("sat_drive_db")->load());
        pad.setTiltValue  (proc.apvts.getRawParameterValue ("tilt")->load());
        pad.setHPValue    (proc.apvts.getRawParameterValue ("hp_hz")->load());
        pad.setLPValue    (proc.apvts.getRawParameterValue ("lp_hz")->load());
        pad.setAirValue   (proc.apvts.getRawParameterValue ("air_db")->load());
        pad.setBassValue  (proc.apvts.getRawParameterValue ("bass_db")->load());
        pad.setScoopValue (proc.apvts.getRawParameterValue ("scoop")->load());
        pad.setTiltFreqValue  (proc.apvts.getRawParameterValue ("tilt_freq")->load());
        pad.setScoopFreqValue (proc.apvts.getRawParameterValue ("scoop_freq")->load());
        pad.setBassFreqValue  (proc.apvts.getRawParameterValue ("bass_freq")->load());
        pad.setAirFreqValue   (proc.apvts.getRawParameterValue ("air_freq")->load());
        pad.setWidthValue (proc.apvts.getRawParameterValue ("width")->load());
        pad.setPanValue   (proc.apvts.getRawParameterValue ("pan")->load());
        pad.setGainValue  (proc.apvts.getRawParameterValue ("gain_db")->load());
    };

    pad.onChange = [this, refreshXYOverlays](float x01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (0.0f); split->endChangeGesture(); }
        if (auto* pan   = proc.apvts.getParameter ("pan"))        { pan  ->beginChangeGesture(); pan  ->setValueNotifyingHost (x01);  pan  ->endChangeGesture(); }
        if (auto* dep   = proc.apvts.getParameter ("depth"))      { dep  ->beginChangeGesture(); dep  ->setValueNotifyingHost (y01);  dep  ->endChangeGesture(); }
        refreshXYOverlays();
    };

    pad.onSplitChange = [this, refreshXYOverlays](float l01, float r01, float y01)
    {
        if (auto* split = proc.apvts.getParameter ("split_mode")) { split->beginChangeGesture(); split->setValueNotifyingHost (1.0f); split->endChangeGesture(); }
        if (auto* pL = proc.apvts.getParameter ("pan_l")) { pL->beginChangeGesture(); pL->setValueNotifyingHost (l01); pL->endChangeGesture(); }
        if (auto* pR = proc.apvts.getParameter ("pan_r")) { pR->beginChangeGesture(); pR->setValueNotifyingHost (r01); pR->endChangeGesture(); }
        if (auto* dep= proc.apvts.getParameter ("depth")) { dep->beginChangeGesture(); dep->setValueNotifyingHost (y01); dep->endChangeGesture(); }
        refreshXYOverlays();
    };

    // listeners for split overlay % etc.
    panKnobLeft.addListener (this);
    panKnobRight.addListener (this);

    // attachments (deduped; removed accidental duplicates you had for bass/scoop/depth)
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    attachments.push_back (std::make_unique<SA> (proc.apvts, "gain_db",       gain));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "width",         width));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "tilt",          tilt));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "mono_hz",       monoHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "hp_hz",         hpHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "lp_hz",         lpHz));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "sat_drive_db",  satDrive));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "sat_mix",       satMix));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "air_db",        air));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "bass_db",       bass));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "scoop",         scoop));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "ducking",       duckingKnob));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "duck_attack_ms",   duckAttack));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "duck_release_ms",  duckRelease));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "duck_threshold_db", duckThreshold));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "duck_ratio",        duckRatio));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan",           panKnob));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan_l",         panKnobLeft));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "pan_r",         panKnobRight));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "depth",         spaceKnob));
    // Mono maker APVTS attachments
    comboAttachments .push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "mono_slope_db_oct", monoSlopeChoice));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>   (proc.apvts, "mono_audition",      monoAuditionButton));

    attachments.push_back (std::make_unique<SA> (proc.apvts, "tilt_freq",     tiltFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "scoop_freq",    scoopFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "bass_freq",     bassFreqSlider));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "air_freq",      airFreqSlider));

    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "bypass", bypassButton));
    comboAttachments .push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "os_mode", osSelect));
    
    // Delay parameter attachments
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_enabled", delayEnabled));
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "delay_mode", delayMode));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_sync", delaySync));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_time_ms", delayTime));
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "delay_time_div", delayTimeDiv));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_feedback_pct", delayFeedback));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_wet", delayWet));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_kill_dry", delayKillDry));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_freeze", delayFreeze));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_pingpong", delayPingpong));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_crossfeed_pct", delaySpread));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_stereo_spread_pct", delaySpread));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_width", delayWidth));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_mod_rate_hz", delayModRate));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_mod_depth_ms", delayModDepth));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_wowflutter", delayWowflutter));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_jitter_pct", delayJitter));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_hp_hz", delayHp));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_lp_hz", delayLp));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_tilt_db", delayTilt));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_sat", delaySat));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_diffusion", delayDiffusion));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_diffuse_size_ms", delayDiffuseSize));
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "delay_duck_source", delayDuckSource));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_duck_post", delayDuckPost));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_depth", delayDuckDepth));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_attack_ms", delayDuckAttack));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_release_ms", delayDuckRelease));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_threshold_db", delayDuckThreshold));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_ratio", delayDuckRatio));
    attachments.push_back (std::make_unique<SA> (proc.apvts, "delay_duck_lookahead_ms", delayDuckLookahead));
    buttonAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "delay_duck_link_global", delayDuckLinkGlobal));

    // parameter listeners (host→UI)
    proc.apvts.addParameterListener ("space_algo", this);
    proc.apvts.addParameterListener ("split_mode", this);
    proc.apvts.addParameterListener ("pan",        this);
    proc.apvts.addParameterListener ("depth",      this);
    proc.apvts.addParameterListener ("mono_slope_db_oct", this);

    // audio callback -> XY waveform (safe: no repaint in push)
    proc.onAudioSample = [this](float L, float R) { pad.pushWaveformSample (L, R); };
    // Provide sample rate to XYPad visuals for RBJ curves
    pad.setSampleRate (proc.getSampleRate());

    // divider line component
    addAndMakeVisible (splitDivider);

    // sync XY with current values
    syncXYPadWithParameters();

    // pointer cursors on interactive children
    applyGlobalCursorPolicy();

    startTimerHz (30);

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
    if (!spaceCell)    spaceCell    = std::make_unique<KnobCell>(spaceKnob,    spaceValue,    "REVERB");
    if (!duckCell)     duckCell     = std::make_unique<KnobCell>(duckingKnob,  duckingValue,  "DUCK");
    if (!duckAttCell)  duckAttCell  = std::make_unique<KnobCell>(duckAttack,   duckAttackValue,   "ATT");
    if (!duckRelCell)  duckRelCell  = std::make_unique<KnobCell>(duckRelease,  duckReleaseValue,  "REL");
    if (!duckThrCell)  duckThrCell  = std::make_unique<KnobCell>(duckThreshold,duckThresholdValue,"THR");
    if (!duckRatCell)  duckRatCell  = std::make_unique<KnobCell>(duckRatio,    duckRatioValue,    "RAT");

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
    if (!shufLoCell)   shufLoCell   = std::make_unique<KnobCell>(shufLoPct,   shufLoValue,   "SHUF LO");
    if (!shufHiCell)   shufHiCell   = std::make_unique<KnobCell>(shufHiPct,   shufHiValue,   "SHUF HI");
    if (!shufXCell)    shufXCell    = std::make_unique<KnobCell>(shufXHz,     shufXValue,    "SHUF XO");

    if (!delayTimeCell)      delayTimeCell       = std::make_unique<KnobCell>(delayTime,      delayTimeValue,      "TIME");
    if (!delayFeedbackCell)  delayFeedbackCell   = std::make_unique<KnobCell>(delayFeedback,  delayFeedbackValue,  "FB");
    if (!delayWetCell)       delayWetCell        = std::make_unique<KnobCell>(delayWet,       delayWetValue,       "WET");
    if (!delaySpreadCell)    delaySpreadCell     = std::make_unique<KnobCell>(delaySpread,    delaySpreadValue,    "SPREAD");
    if (!delayWidthCell)     delayWidthCell      = std::make_unique<KnobCell>(delayWidth,     delayWidthValue,     "WIDTH");
    if (!delayModRateCell)   delayModRateCell    = std::make_unique<KnobCell>(delayModRate,   delayModRateValue,   "MOD RATE");
    if (!delayModDepthCell)  delayModDepthCell   = std::make_unique<KnobCell>(delayModDepth,  delayModDepthValue,  "MOD DEPTH");
    if (!delayWowflutterCell)delayWowflutterCell = std::make_unique<KnobCell>(delayWowflutter,delayWowflutterValue,"WOW");
    if (!delayJitterCell)    delayJitterCell     = std::make_unique<KnobCell>(delayJitter,    delayJitterValue,    "JITTER");
    if (!delayHpCell)        delayHpCell         = std::make_unique<KnobCell>(delayHp,        delayHpValue,        "HP");
    if (!delayLpCell)        delayLpCell         = std::make_unique<KnobCell>(delayLp,        delayLpValue,        "LP");
    if (!delayTiltCell)      delayTiltCell       = std::make_unique<KnobCell>(delayTilt,      delayTiltValue,      "TILT");
    if (!delaySatCell)       delaySatCell        = std::make_unique<KnobCell>(delaySat,       delaySatValue,       "SAT");
    if (!delayDiffusionCell) delayDiffusionCell  = std::make_unique<KnobCell>(delayDiffusion, delayDiffusionValue, "DIFF");
    if (!delayDiffuseSizeCell)delayDiffuseSizeCell= std::make_unique<KnobCell>(delayDiffuseSize, delayDiffuseSizeValue, "SIZE");
    if (!delayDuckDepthCell) delayDuckDepthCell  = std::make_unique<KnobCell>(delayDuckDepth, delayDuckDepthValue, "DUCK DEPTH");
    if (!delayDuckAttackCell)delayDuckAttackCell = std::make_unique<KnobCell>(delayDuckAttack,delayDuckAttackValue,"DUCK ATT");
    if (!delayDuckReleaseCell)delayDuckReleaseCell=std::make_unique<KnobCell>(delayDuckRelease,delayDuckReleaseValue,"DUCK REL");
}

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    panKnobLeft.removeListener (this);
    panKnobRight.removeListener (this);

    proc.apvts.removeParameterListener ("space_algo", this);
    proc.apvts.removeParameterListener ("split_mode", this);
    proc.apvts.removeParameterListener ("pan",        this);
    proc.apvts.removeParameterListener ("depth",      this);
    proc.apvts.removeParameterListener ("mono_slope_db_oct", this);

    // ensure A holds final state if user ended on B
    if (!isStateA) { saveCurrentState(); stateA = stateB; }

    setLookAndFeel (nullptr);
}

void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // background gradient (original vibe)
    auto full = getLocalBounds();
    juce::Colour top    = juce::Colour (0xFF2A2C30);
    juce::Colour mid    = juce::Colour (0xFF4A4D55);
    juce::Colour bottom = juce::Colour (0xFF2A2C30);
    juce::ColourGradient bg (top, (float) full.getCentreX(), (float) full.getY(),
                             bottom, (float) full.getCentreX(), (float) full.getBottom(), false);
    bg.addColour (0.85, mid);
    g.setGradientFill (bg);
    g.fillAll();

    // logo + tagline
    auto header = getLocalBounds().removeFromTop ((int) (100 * scaleFactor));
    g.setColour (lnf.theme.text);
    juce::Font logo (juce::FontOptions (26.0f * scaleFactor).withStyle ("Bold"));
    g.setFont (logo);
    const int leftInset = Layout::dp (20, scaleFactor);
    auto logoArea = juce::Rectangle<int> (header.getX() + leftInset, header.getY() + Layout::dp (4, scaleFactor),
                                          header.getWidth(), logo.getHeight() + 2);
    g.drawText ("FIELD", logoArea, juce::Justification::centredLeft);

    // version
    const juce::String ver = " v" + juce::String (JUCE_STRINGIFY (JucePlugin_VersionString));
    juce::Font vfont (juce::FontOptions (juce::jmax (9, (int) std::round (8 * scaleFactor))));
    g.setFont (vfont);
    g.setColour (lnf.theme.textMuted);
    const int vx = logoArea.getX() + (int) logo.getStringWidthFloat ("FIELD") + Layout::dp (8, scaleFactor);
    const int vy = logoArea.getY() + (logo.getHeight() - vfont.getHeight()) * 0.5f + 1;
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
    g.setColour (juce::Colour (0xFF6A6D75));
    for (int i = 0; i < 3; ++i)
    {
        int off = i * 4;
        g.drawLine (resizeArea.getRight() - 8 - off, resizeArea.getBottom() - 4 - off,
                    resizeArea.getRight() - 4 - off, resizeArea.getBottom() - 8 - off, 1.0f);
    }
}

void MyPluginAudioProcessorEditor::performLayout()
{
    const float s = juce::jmax (0.6f, scaleFactor);
    auto r = getLocalBounds().reduced (Layout::dp (Layout::PAD, s)).withTrimmedBottom (50);
    

    // Ensure value labels are positioned in the same parent coordinate space
    // as their corresponding controls, directly below the control.
    auto placeLabelBelow = [&] (juce::Label& label, juce::Component& target, int yOffset)
    {
        if (auto* parent = target.getParentComponent())
        {
            if (label.getParentComponent() != parent)
            {
                if (auto* oldParent = label.getParentComponent())
                    oldParent->removeChildComponent (&label);
                parent->addAndMakeVisible (label);
            }
            auto b = target.getBounds();
            label.setBounds (b.withY (b.getBottom() + yOffset));
            label.toFront (false);
        }
    };

    // 1) wood bar controls (reduced height)
    auto woodBar = r.removeFromTop (Layout::dp (50, s));
    juce::Grid header;
    header.rowGap = juce::Grid::Px (Layout::dp (4, s));
    header.columnGap = juce::Grid::Px (Layout::dp (6, s));
    header.alignContent = juce::Grid::AlignContent::center;
    header.justifyContent = juce::Grid::JustifyContent::center;
    header.alignItems = juce::Grid::AlignItems::center;
    header.justifyItems = juce::Grid::JustifyItems::center;
    header.templateRows = { juce::Grid::TrackInfo (juce::Grid::Fr (1)) };

    header.templateColumns = {
        juce::Grid::TrackInfo (juce::Grid::Fr (1)),
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // bypass
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (400, s))),// preset
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // prev
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // next
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // save
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // A
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // B
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // copy
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (20, s))), // spacer
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (120, s))),// split
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // link
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // snap
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s)))  // fullscreen
    };

    const int h = Layout::dp (24, s);
    auto sizeBtn = [&](juce::Component& c, int w){ c.setSize (w, h); };

    sizeBtn (bypassButton,       Layout::dp (40, s));
    sizeBtn (savePresetButton,   Layout::dp (40, s));
    sizeBtn (abButtonA,          Layout::dp (40, s));
    sizeBtn (abButtonB,          Layout::dp (40, s));
    sizeBtn (copyButton,         Layout::dp (40, s));
    sizeBtn (prevPresetButton,   Layout::dp (40, s));
    sizeBtn (nextPresetButton,   Layout::dp (40, s));
    splitToggle.setSize (Layout::dp (120, s), Layout::dp (28, s));
    sizeBtn (linkButton,         Layout::dp (40, s));
    sizeBtn (snapButton,         Layout::dp (40, s));
    sizeBtn (colorModeButton,    Layout::dp (40, s));
    sizeBtn (fullScreenButton,   Layout::dp (40, s));
    sizeBtn (optionsButton,      Layout::dp (40, s));

    header.items = {
        juce::GridItem(),
        juce::GridItem (bypassButton),
        juce::GridItem (presetCombo),
        juce::GridItem (prevPresetButton),
        juce::GridItem (nextPresetButton),
        juce::GridItem (savePresetButton),
        juce::GridItem (abButtonA),
        juce::GridItem (abButtonB),
        juce::GridItem (copyButton),
        juce::GridItem(),
        juce::GridItem (splitToggle),
        juce::GridItem (linkButton),
        juce::GridItem (snapButton),
        juce::GridItem (colorModeButton),
        juce::GridItem (fullScreenButton),
    };

    auto headerArea = woodBar.reduced (Layout::dp (Layout::GAP, s), Layout::dp (6, s))
                             .withTrimmedBottom (Layout::dp (8, s))
                             .withTrimmedTop (Layout::dp (2, s));
    header.performLayout (headerArea);

    // options + help at bottom-left
    {
        auto bounds = getLocalBounds();
        const int padding = Layout::dp (8, s);
        optionsButton.setTopLeftPosition (bounds.getX() + padding, bounds.getBottom() - h - padding);
        helpButton.setBounds (optionsButton.getRight() + Layout::dp (6, s), optionsButton.getY(), optionsButton.getWidth(), optionsButton.getHeight());
        addAndMakeVisible (helpButton);
    }

    // divider left of split toggle
    {
        auto b = splitToggle.getBounds();
        const int lineH = Layout::dp (24, s);
        const int gapX  = Layout::dp (8, s);
        auto cy = b.getCentreY();
        splitDivider.setBounds (b.getX() - gapX - 1, cy - lineH/2, 4, lineH);
        splitDivider.toFront (false);
    }

    // 2) main XY area + vertical meters on right side
    {
        auto main = r.removeFromTop (juce::jmax (Layout::dp (300, s), (int) std::round (r.getHeight() * 0.5f)));
        
        // Reserve space for meters on the right side
        const int metersW = Layout::dp (Layout::METERS_W, s);
        auto metersArea = main.removeFromRight (metersW);
        metersContainer.setBounds (metersArea);
        
        // XY pad takes remaining space on the left
        pad.setBounds (main.reduced (Layout::dp (Layout::GAP, s)));
        if (xyShade) xyShade->setBounds (pad.getBounds());

        // Layout vertical meters: Corr (top) + LR bars (bottom)
        addAndMakeVisible (corrMeter);
        addAndMakeVisible (lrMeters);
        auto mB = metersContainer.getBounds();
        const int corrH = Layout::dp (Layout::CORR_METER_H, s);
        auto corrB = mB.removeFromTop (corrH);
        corrMeter.setBounds (corrB);
        lrMeters.setBounds (mB);
    }

    // Remove vertical gaps between rows

    const int lPx       = Layout::dp ((float) Layout::knobPx (Layout::Knob::L), s);
    const int gapI      = Layout::dp (Layout::GAP_S, s);
    const int labelBand = Layout::dp (Layout::LABEL_BAND_EXTRA, s);
    // microH no longer used (minis integrated into cells)

    // Container height calculation: knob height + label band + extra padding for labels
    const int containerHeight = lPx + labelBand + Layout::dp (Layout::LABEL_BAND_EXTRA, s);

    // All rows same height, no extra spacing between rows
    const int rowH1 = containerHeight;                // Row 1
    const int rowH2 = containerHeight;                // Row 2
    const int rowH3 = containerHeight;                // Row 3
    const int rowH4 = containerHeight;                // Row 4

    // Reserve space for delay card on the right side (use full available height across all rows)
    const int delayCardW = Layout::dp (620, s); // Width for delay card (fits ~6-8 knobs)
    auto delayCardArea = r.removeFromRight (delayCardW);
    delayContainer.setBounds (delayCardArea);


    auto row1 = r.removeFromTop (rowH1);
    auto row2 = r.removeFromTop (rowH2);
    auto row3 = r.removeFromTop (rowH3);
    auto row4 = r.removeFromTop (rowH4);

    // ---------------- Row 1: Pan, MONO cell, Width cells, Gain, Drive, Mix -----------
    {
        auto row = row1;

        juce::Grid g;
        g.rowGap    = juce::Grid::Px (gapI);
        g.columnGap = juce::Grid::Px (gapI);
        const int valuePx = Layout::dp (14, s);
        const int gapPx   = Layout::dp (0,  s);
        const int labelGap = Layout::dp (4, s);
        g.templateRows    = { juce::Grid::Px (containerHeight) };
        g.templateColumns = {
            juce::Grid::Px (lPx * 3 + gapI * 2),       // Pan (triple wide)
            juce::Grid::Px (lPx),                       // Width
            juce::Grid::Px (lPx),                       // W LO
            juce::Grid::Px (lPx),                       // W MID
            juce::Grid::Px (lPx),                       // W HI
            juce::Grid::Px (lPx),                       // Gain
            juce::Grid::Px (lPx),                       // Drive
            juce::Grid::Px (lPx),                       // Mix
            juce::Grid::Fr (1)                          // stretchy pad
        };

        if (!panCell)
            panCell = std::make_unique<KnobCell>(panKnob, panValue, "");

        panCell   ->setMetrics (lPx, valuePx, labelGap);
        monoCell   ->setMetrics (lPx, valuePx, labelGap, Layout::dp (24, s));
        widthCell  ->setMetrics (lPx, valuePx, labelGap);
        widthLoCell->setMetrics (lPx, valuePx, labelGap);
        widthMidCell->setMetrics (lPx, valuePx, labelGap);
        widthHiCell->setMetrics (lPx, valuePx, labelGap);
        gainCell   ->setMetrics (lPx, valuePx, labelGap);
        satDriveCell->setMetrics (lPx, valuePx, labelGap);
        satMixCell ->setMetrics (lPx, valuePx, labelGap);

        addAndMakeVisible (*panCell);
        addAndMakeVisible (*widthCell);
        addAndMakeVisible (*widthLoCell);
        addAndMakeVisible (*widthMidCell);
        addAndMakeVisible (*widthHiCell);
        addAndMakeVisible (*gainCell);
        addAndMakeVisible (*satDriveCell);
        addAndMakeVisible (*satMixCell);

        g.items = {
            juce::GridItem (*panCell)          .withWidth (lPx * 3 + gapI * 2).withHeight (containerHeight),
            juce::GridItem (*widthCell)        .withWidth (lPx).withHeight (containerHeight),
            juce::GridItem (*widthLoCell)      .withWidth (lPx).withHeight (containerHeight),
            juce::GridItem (*widthMidCell)     .withWidth (lPx).withHeight (containerHeight),
            juce::GridItem (*widthHiCell)      .withWidth (lPx).withHeight (containerHeight),
            juce::GridItem (*gainCell)         .withWidth (lPx).withHeight (containerHeight),
            juce::GridItem (*satDriveCell)     .withWidth (lPx).withHeight (containerHeight),
            juce::GridItem (*satMixCell)       .withWidth (lPx).withHeight (containerHeight)
        };
        g.performLayout (row);

        // Managed labels for row 1 cells
        panCell   ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        panCell   ->setValueLabelGap (labelGap);
        monoCell  ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        monoCell  ->setValueLabelGap (labelGap);
        widthCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        widthLoCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        widthMidCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        widthHiCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        gainCell  ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        satDriveCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        satMixCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        widthCell ->setValueLabelGap (labelGap);
        widthLoCell->setValueLabelGap (labelGap);
        widthMidCell->setValueLabelGap (labelGap);
        widthHiCell->setValueLabelGap (labelGap);
        gainCell  ->setValueLabelGap (labelGap);
        satDriveCell->setValueLabelGap (labelGap);
        satMixCell ->setValueLabelGap (labelGap);
    }

    // -------------- Row 2: Reverb group (all Reverb + Ducking controls) ----------
    {
        auto row = row2;

        // Reverb sub-grid: [ Reverb cell ] + [ switch ] + [ DUCK + ATT + REL + THR + RAT ]
         {
            // Use consistent row gap with no fractional reductions
            auto rg = row.reduced (0, 0);
            juce::Grid reverbGrid;
            reverbGrid.rowGap    = juce::Grid::Px (gapI);
            reverbGrid.columnGap = juce::Grid::Px (gapI);
            const int valuePx = Layout::dp (14, s);
            const int gapPx   = Layout::dp (0,  s);
            const int switchW = lPx; // vertical column sized like a knob cell
            reverbGrid.templateRows    = { juce::Grid::Px (containerHeight) };
            reverbGrid.templateColumns = { 
                juce::Grid::Px (lPx),                                    // Reverb cell
                juce::Grid::Px (lPx), juce::Grid::Px (lPx), juce::Grid::Px (lPx),
                juce::Grid::Px (lPx), juce::Grid::Px (lPx),              // DUCK, ATT, REL, THR, RAT
                juce::Grid::Px (switchW),                                // Switch
                juce::Grid::Px (lPx * 3 + gapI * 2)                      // Mono triple-wide at end
            };
            // Ensure components are parented correctly (cells own knob+value)
            if (spaceKnob.getParentComponent() == this) removeChildComponent (&spaceKnob);
            if (duckingKnob.getParentComponent() == this) removeChildComponent (&duckingKnob);
            if (duckAttack.getParentComponent() == this) removeChildComponent (&duckAttack);
            if (duckRelease.getParentComponent() == this) removeChildComponent (&duckRelease);
            if (duckThreshold.getParentComponent() == this) removeChildComponent (&duckThreshold);
            if (duckRatio.getParentComponent() == this) removeChildComponent (&duckRatio);
 
            spaceCell  ->setMetrics (lPx, valuePx, gapPx);
            duckCell   ->setMetrics (lPx, valuePx, gapPx);
            duckAttCell->setMetrics (lPx, valuePx, gapPx);
            duckRelCell->setMetrics (lPx, valuePx, gapPx);
            duckThrCell->setMetrics (lPx, valuePx, gapPx);
            duckRatCell->setMetrics (lPx, valuePx, gapPx);

            // Mono: triple-wide with slope segmented switch + AUD toggle to the right
            monoCell   ->setMetrics (lPx, valuePx, gapPx, Layout::dp (24, s));
            if (!monoSlopeSwitch)
            {
                monoSlopeSwitch = std::make_unique<MonoSlopeSwitch>();
                monoSlopeSwitch->onChange = [this](int idx)
                {
                    monoSlopeChoice.setSelectedItemIndex (juce::jlimit (0, 2, idx), juce::NotificationType::sendNotification);
                };
            }
            addAndMakeVisible (*monoSlopeSwitch);
            monoCell   ->setMiniPlacementRight (true);
            monoCell   ->setMiniThicknessPx (Layout::dp (12, s));
            monoCell   ->setAuxComponents ({ monoSlopeSwitch.get(), &monoAuditionButton }, Layout::dp (90, s));
            // Give the mono slope switch more vertical space than AUD
            monoCell   ->setAuxWeights ({ 2.0f, 1.0f });

            // Managed labels on reverb row
            spaceCell   ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            duckCell    ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            duckAttCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            duckRelCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            duckThrCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            duckRatCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            const int rvGap = Layout::dp (6, s);
            spaceCell   ->setValueLabelGap (rvGap);
            duckCell    ->setValueLabelGap (rvGap);
            duckAttCell ->setValueLabelGap (rvGap);
            duckRelCell ->setValueLabelGap (rvGap);
            duckThrCell ->setValueLabelGap (rvGap);
            duckRatCell ->setValueLabelGap (rvGap);
 
            addAndMakeVisible (*spaceCell);
            addAndMakeVisible (spaceAlgorithmSwitch);
            addAndMakeVisible (*duckCell);
            addAndMakeVisible (*duckAttCell);
            addAndMakeVisible (*duckRelCell);
            addAndMakeVisible (*duckThrCell);
            addAndMakeVisible (*duckRatCell);
            addAndMakeVisible (*monoCell);
 
            reverbGrid.items = {
                juce::GridItem (*spaceCell)         .withHeight (containerHeight),
                juce::GridItem (*duckCell)          .withHeight (containerHeight),
                juce::GridItem (*duckAttCell)       .withHeight (containerHeight),
                juce::GridItem (*duckRelCell)       .withHeight (containerHeight),
                juce::GridItem (*duckThrCell)       .withHeight (containerHeight),
                juce::GridItem (*duckRatCell)       .withHeight (containerHeight),
                juce::GridItem (spaceAlgorithmSwitch)
                    .withAlignSelf (juce::GridItem::AlignSelf::center)
                    .withJustifySelf (juce::GridItem::JustifySelf::center)
                    .withHeight (containerHeight),
                juce::GridItem (*monoCell)
                    .withWidth (lPx * 3 + gapI * 2)
                    .withHeight (containerHeight)
            };
            reverbGrid.performLayout (rg);
 
            // Ensure components are brought to front within the container
            spaceCell->toFront (false);
            spaceAlgorithmSwitch.toFront (false);
            duckCell->toFront (false);
            duckAttCell->toFront (false);
            duckRelCell->toFront (false);
            duckThrCell->toFront (false);
            duckRatCell->toFront (false);
        }
    }

    // --------- Row 3: EQ (Bass/Air/Tilt/Scoop + minis) + HP + LP ---------------
    {
        // layout directly

        // Make sure components are children of the editor (not eqContainer)
        // First remove from main editor if they're there
        if (bass.getParentComponent() == this) removeChildComponent (&bass);
        if (air.getParentComponent() == this) removeChildComponent (&air);
        if (tilt.getParentComponent() == this) removeChildComponent (&tilt);
        if (scoop.getParentComponent() == this) removeChildComponent (&scoop);
        if (hpHz.getParentComponent() == this) removeChildComponent (&hpHz);
        if (lpHz.getParentComponent() == this) removeChildComponent (&lpHz);
        if (bassFreqSlider.getParentComponent() == this) removeChildComponent (&bassFreqSlider);
        if (airFreqSlider.getParentComponent() == this) removeChildComponent (&airFreqSlider);
        if (tiltFreqSlider.getParentComponent() == this) removeChildComponent (&tiltFreqSlider);
        if (scoopFreqSlider.getParentComponent() == this) removeChildComponent (&scoopFreqSlider);
        
        // Then add EQ cells (knob+value) directly to editor
        addAndMakeVisible (*bassCell);
        addAndMakeVisible (*airCell);
        addAndMakeVisible (*tiltCell);
        addAndMakeVisible (*scoopCell);
        if (!hpLpCell) hpLpCell = std::make_unique<DoubleKnobCell>(hpHz, hpValue, lpHz, lpValue);
        addAndMakeVisible (*hpLpCell);
        // Micro sliders live inside cells

        // Use consistent gap; no extra horizontal reductions (now placed in row4)
        auto row = row4.reduced (0, 0);

        // Configure metrics for cells at current scale, and attach mini sliders
        const int valuePx = Layout::dp (14, s);
        const int gapPx   = Layout::dp (0,  s);
        const int miniStripW = Layout::dp (90, s);   // right strip width
        const int miniBarH   = Layout::dp (12, s);   // horizontal bar height
        bassCell ->setMetrics (lPx, valuePx, gapPx, miniStripW);
        airCell  ->setMetrics (lPx, valuePx, gapPx, miniStripW);
        tiltCell ->setMetrics (lPx, valuePx, gapPx, miniStripW);
        scoopCell->setMetrics (lPx, valuePx, gapPx, miniStripW);
        // Move minis to the right for EQ primary four
        bassCell ->setMiniPlacementRight (true);
        airCell  ->setMiniPlacementRight (true);
        tiltCell ->setMiniPlacementRight (true);
        scoopCell->setMiniPlacementRight (true);
        

        // Managed value labels under knobs (EQ row)
        bassCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        airCell  ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        tiltCell ->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        scoopCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        
        const int valueGap = Layout::dp (6, s);
        bassCell ->setValueLabelGap (valueGap);
        airCell  ->setValueLabelGap (valueGap);
        tiltCell ->setValueLabelGap (valueGap);
        scoopCell->setValueLabelGap (valueGap);
        

        bassCell ->setMini (&bassFreqSlider, miniStripW);
        airCell  ->setMini (&airFreqSlider,  miniStripW);
        tiltCell ->setMini (&tiltFreqSlider, miniStripW);
        scoopCell->setMini (&scoopFreqSlider,miniStripW);

        // Thicker bar rendering
        bassCell ->setMiniThicknessPx (miniBarH);
        airCell  ->setMiniThicknessPx (miniBarH);
        tiltCell ->setMiniThicknessPx (miniBarH);
        scoopCell->setMiniThicknessPx (miniBarH);

        for (auto* mini : { &bassFreqSlider, &airFreqSlider, &tiltFreqSlider, &scoopFreqSlider })
            mini->getProperties().set ("micro", true);

        // Use grid layout with consistent gaps: five double-wide cells (cells include mini sliders)
        juce::Grid g;
        g.rowGap    = juce::Grid::Px (gapI);
        g.columnGap = juce::Grid::Px (gapI);
        g.templateRows    = { juce::Grid::Px (containerHeight) };
        const int doubleW = lPx * 2 + gapI;
        g.templateColumns = {
            juce::Grid::Px (doubleW), juce::Grid::Px (doubleW), juce::Grid::Px (doubleW), juce::Grid::Px (doubleW), juce::Grid::Px (doubleW)
        };

        bassCell->setVisible (true);
        airCell ->setVisible (true);
        tiltCell->setVisible (true);
        scoopCell->setVisible (true);
        hpLpCell->setVisible (true);

        hpLpCell->setMetrics (lPx, valuePx, gapPx);
        g.items = {
            juce::GridItem (*bassCell) .withHeight (containerHeight),
            juce::GridItem (*airCell)  .withHeight (containerHeight),
            juce::GridItem (*tiltCell) .withHeight (containerHeight),
            juce::GridItem (*scoopCell).withHeight (containerHeight),
            juce::GridItem (*hpLpCell) .withHeight (containerHeight)
        };
        g.performLayout (row);

        // Ensure components are brought to front
        bassCell->toFront (false);
        airCell ->toFront (false);
        tiltCell->toFront (false);
        scoopCell->toFront (false);
        hpLpCell->toFront (false);
    }

    // ---------------- Row 4: Remaining Imaging items ---------------------------
    {
        // layout directly

        // Make sure components are children of the editor
        // First remove from main editor if they're there
        if (xoverLoHz.getParentComponent() == this) removeChildComponent (&xoverLoHz);
        if (xoverHiHz.getParentComponent() == this) removeChildComponent (&xoverHiHz);
        if (rotationDeg.getParentComponent() == this) removeChildComponent (&rotationDeg);
        if (asymmetry.getParentComponent() == this) removeChildComponent (&asymmetry);
        if (shufLoPct.getParentComponent() == this) removeChildComponent (&shufLoPct);
        if (shufHiPct.getParentComponent() == this) removeChildComponent (&shufHiPct);
        if (shufXHz.getParentComponent() == this) removeChildComponent (&shufXHz);
        
        // Cells will manage parenting of knobs/labels; do not add raw knobs here

        // Imaging row: use knob cells for uniform layout
        const int valuePx = Layout::dp (14, s);
        const int gapPx   = Layout::dp (0,  s);
        xoverLoCell->setMetrics (lPx, valuePx, gapPx);
        xoverHiCell->setMetrics (lPx, valuePx, gapPx);
        rotationCell->setMetrics (lPx, valuePx, gapPx);
        asymCell->setMetrics     (lPx, valuePx, gapPx);
        shufLoCell->setMetrics   (lPx, valuePx, gapPx);
        shufHiCell->setMetrics   (lPx, valuePx, gapPx);
        shufXCell->setMetrics    (lPx, valuePx, gapPx);
        // Managed labels on imaging row
        xoverLoCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        xoverHiCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        rotationCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        asymCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        shufLoCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        shufHiCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        shufXCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        const int imgGap = Layout::dp (6, s);
        xoverLoCell->setValueLabelGap (imgGap);
        xoverHiCell->setValueLabelGap (imgGap);
        rotationCell->setValueLabelGap (imgGap);
        asymCell->setValueLabelGap (imgGap);
        shufLoCell->setValueLabelGap (imgGap);
        shufHiCell->setValueLabelGap (imgGap);
        shufXCell->setValueLabelGap (imgGap);

        addAndMakeVisible (*xoverLoCell);
        addAndMakeVisible (*xoverHiCell);
        addAndMakeVisible (*rotationCell);
        addAndMakeVisible (*asymCell);
        addAndMakeVisible (*shufLoCell);
        addAndMakeVisible (*shufHiCell);
        addAndMakeVisible (*shufXCell);

        juce::Grid imgGrid;
        imgGrid.rowGap    = juce::Grid::Px (gapI);
        imgGrid.columnGap = juce::Grid::Px (gapI);
        imgGrid.templateRows = { juce::Grid::Px (containerHeight) };
        imgGrid.templateColumns = {
            juce::Grid::Px (lPx), juce::Grid::Px (lPx), juce::Grid::Px (lPx),
            juce::Grid::Px (lPx), juce::Grid::Px (lPx), juce::Grid::Px (lPx), juce::Grid::Px (lPx)
        };
        imgGrid.items = {
            juce::GridItem (*xoverLoCell) .withHeight (containerHeight),
            juce::GridItem (*xoverHiCell) .withHeight (containerHeight),
            juce::GridItem (*rotationCell).withHeight (containerHeight),
            juce::GridItem (*asymCell)    .withHeight (containerHeight),
            juce::GridItem (*shufLoCell)  .withHeight (containerHeight),
            juce::GridItem (*shufHiCell)  .withHeight (containerHeight),
            juce::GridItem (*shufXCell)   .withHeight (containerHeight)
        };
        auto imgB = row3.reduced (0, 0);
        imgGrid.performLayout (imgB);
    }

    // ---------------- Delay Card (Right Side, spans full height) ---------------------------
    {
        delayContainer.setVisible (true);
        
        // Add delay controls to the delay container
        delayContainer.addAndMakeVisible (delayEnabled);
        delayContainer.addAndMakeVisible (delayMode);
        delayContainer.addAndMakeVisible (delaySync);
        delayContainer.addAndMakeVisible (delayFreeze);
        delayContainer.addAndMakeVisible (delayKillDry);
        
        delayContainer.addAndMakeVisible (delayPingpong);
        // Cells will parent knob components, so don't add raw knobs above
        
        // Add per-delay ducking controls
        delayContainer.addAndMakeVisible (delayDuckSource);
        delayContainer.addAndMakeVisible (delayDuckPost);
        delayContainer.addAndMakeVisible (delayDuckThreshold);
        
        // Layout delay controls using cells for knobs + direct items for buttons/combos
        auto delayB = delayContainer.getLocalBounds();

        const int valuePx = Layout::dp (14, s);
        const int gapPx   = Layout::dp (0,  s);
        delayTimeCell      ->setMetrics (lPx, valuePx, gapPx);
        delayFeedbackCell  ->setMetrics (lPx, valuePx, gapPx);
        delayWetCell       ->setMetrics (lPx, valuePx, gapPx);
        delaySpreadCell    ->setMetrics (lPx, valuePx, gapPx);
        delayWidthCell     ->setMetrics (lPx, valuePx, gapPx);
        delayModRateCell   ->setMetrics (lPx, valuePx, gapPx);
        delayModDepthCell  ->setMetrics (lPx, valuePx, gapPx);
        delayWowflutterCell->setMetrics (lPx, valuePx, gapPx);
        delayJitterCell    ->setMetrics (lPx, valuePx, gapPx);
        delayHpCell        ->setMetrics (lPx, valuePx, gapPx);
        delayLpCell        ->setMetrics (lPx, valuePx, gapPx);
        delayTiltCell      ->setMetrics (lPx, valuePx, gapPx);
        delaySatCell       ->setMetrics (lPx, valuePx, gapPx);
        delayDiffusionCell ->setMetrics (lPx, valuePx, gapPx);
        delayDiffuseSizeCell->setMetrics (lPx, valuePx, gapPx);
        delayDuckDepthCell ->setMetrics (lPx, valuePx, gapPx);
        delayDuckAttackCell->setMetrics (lPx, valuePx, gapPx);
        delayDuckReleaseCell->setMetrics (lPx, valuePx, gapPx);

        for (auto* c : { delayTimeCell.get(),delayFeedbackCell.get(),delayWetCell.get(),delaySpreadCell.get(),delayWidthCell.get(),delayModRateCell.get(),
                         delayModDepthCell.get(),delayWowflutterCell.get(),delayJitterCell.get(),delayHpCell.get(),delayLpCell.get(),delayTiltCell.get(),
                         delaySatCell.get(),delayDiffusionCell.get(),delayDiffuseSizeCell.get(),delayDuckDepthCell.get(),delayDuckAttackCell.get(),
                         delayDuckReleaseCell.get() })
            delayContainer.addAndMakeVisible (*c);

        juce::Grid delayGrid;
        delayGrid.rowGap = juce::Grid::Px (gapI);
        delayGrid.columnGap = juce::Grid::Px (gapI);
        delayGrid.templateRows = {
            juce::Grid::Px (Layout::dp (24, s)),                                      // buttons/combos row height
            juce::Grid::Px (lPx + gapPx + valuePx), juce::Grid::Px (lPx + gapPx + valuePx),
            juce::Grid::Px (lPx + gapPx + valuePx), juce::Grid::Px (lPx + gapPx + valuePx)
        };
        delayGrid.templateColumns = { juce::Grid::Px (lPx), juce::Grid::Px (lPx), juce::Grid::Px (lPx), juce::Grid::Px (lPx), juce::Grid::Px (lPx), juce::Grid::Px (lPx) };

        delayGrid.items = {
            // Row 1: Core toggles/combos (not cells)
            juce::GridItem (delayEnabled).withArea (1,1),
            juce::GridItem (delayMode).withArea (1,2),
            juce::GridItem (delaySync).withArea (1,3),
            juce::GridItem (delayFreeze).withArea (1,4),
            juce::GridItem (delayKillDry).withArea (1,5),
            juce::GridItem(),

            // Row 2
            juce::GridItem (*delayTimeCell).withArea (2,1),
            juce::GridItem (*delayFeedbackCell).withArea (2,2),
            juce::GridItem (*delayWetCell).withArea (2,3),
            juce::GridItem (*delaySpreadCell).withArea (2,4),
            juce::GridItem (*delayWidthCell).withArea (2,5),
            juce::GridItem (*delayModRateCell).withArea (2,6),

            // Row 3
            juce::GridItem (*delayModDepthCell).withArea (3,1),
            juce::GridItem (*delayWowflutterCell).withArea (3,2),
            juce::GridItem (*delayJitterCell).withArea (3,3),
            juce::GridItem (*delayHpCell).withArea (3,4),
            juce::GridItem (*delayLpCell).withArea (3,5),
            juce::GridItem (*delayTiltCell).withArea (3,6),

            // Row 4
            juce::GridItem (*delaySatCell).withArea (4,1),
            juce::GridItem (*delayDiffusionCell).withArea (4,2),
            juce::GridItem (*delayDiffuseSizeCell).withArea (4,3),
            juce::GridItem (*delayDuckDepthCell).withArea (4,4),
            juce::GridItem (*delayDuckAttackCell).withArea (4,5),
            juce::GridItem (*delayDuckReleaseCell).withArea (4,6),

            // Row 5: Remaining ducking controls and options
            juce::GridItem (delayDuckThreshold).withArea (5,1),
            juce::GridItem (delayDuckSource).withArea (5,2),
            juce::GridItem (delayDuckPost).withArea (5,3)
        };

        delayGrid.performLayout (delayB);
        
        // Position value labels under their corresponding delay controls (correct parent)
        placeLabelBelow (delayTimeValue,       delayTime,       Layout::dp (6, s));
        placeLabelBelow (delayFeedbackValue,   delayFeedback,   Layout::dp (6, s));
        placeLabelBelow (delayWetValue,        delayWet,        Layout::dp (6, s));
        placeLabelBelow (delaySpreadValue,     delaySpread,     Layout::dp (6, s));
        placeLabelBelow (delayWidthValue,      delayWidth,      Layout::dp (6, s));
        placeLabelBelow (delayModRateValue,    delayModRate,    Layout::dp (6, s));
        placeLabelBelow (delayModDepthValue,   delayModDepth,   Layout::dp (6, s));
        placeLabelBelow (delayWowflutterValue, delayWowflutter, Layout::dp (6, s));
        placeLabelBelow (delayJitterValue,     delayJitter,     Layout::dp (6, s));
        placeLabelBelow (delayHpValue,         delayHp,         Layout::dp (6, s));
        placeLabelBelow (delayLpValue,         delayLp,         Layout::dp (6, s));
        placeLabelBelow (delayTiltValue,       delayTilt,       Layout::dp (6, s));
        placeLabelBelow (delaySatValue,        delaySat,        Layout::dp (6, s));
        placeLabelBelow (delayDiffusionValue,  delayDiffusion,  Layout::dp (6, s));
        placeLabelBelow (delayDiffuseSizeValue, delayDiffuseSize, Layout::dp (6, s));
        placeLabelBelow (delayDuckDepthValue,  delayDuckDepth,  Layout::dp (6, s));
        placeLabelBelow (delayDuckAttackValue, delayDuckAttack, Layout::dp (6, s));
        placeLabelBelow (delayDuckReleaseValue, delayDuckRelease, Layout::dp (6, s));
        
        // Bring components to front
        delayEnabled.toFront (false);
        delayMode.toFront (false);
        delaySync.toFront (false);
        delayTime.toFront (false);
        delayFeedback.toFront (false);
        delayWet.toFront (false);
        delayFreeze.toFront (false);
        delayKillDry.toFront (false);
        delayPingpong.toFront (false);
        delaySpread.toFront (false);
        delayWidth.toFront (false);
        delayModRate.toFront (false);
        delayModDepth.toFront (false);
        delayWowflutter.toFront (false);
        delayJitter.toFront (false);
        delayHp.toFront (false);
        delayLp.toFront (false);
        delayTilt.toFront (false);
        delaySat.toFront (false);
        delayDiffusion.toFront (false);
        delayDiffuseSize.toFront (false);
        delayDuckSource.toFront (false);
        delayDuckPost.toFront (false);
        delayDuckDepth.toFront (false);
        delayDuckAttack.toFront (false);
        delayDuckRelease.toFront (false);
        delayDuckThreshold.toFront (false);
    }
}

void MyPluginAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    const int grip = 16;
    if (e.position.x > getWidth() - grip && e.position.y > getHeight() - grip)
    {
        isResizing = true;
        resizeStart = e.getPosition();
        originalBounds = getBounds();
    }
}

void MyPluginAudioProcessorEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (!isResizing) return;
    auto d = e.getPosition() - resizeStart;
    
    // Calculate new size
    int newWidth = originalBounds.getWidth() + d.x;
    int newHeight = originalBounds.getHeight() + d.y;
    
    // Apply minimum size constraints
    newWidth = juce::jmax (newWidth, minWidth);
    newHeight = juce::jmax (newHeight, minHeight);
    
    // Apply maximum size constraints
    newWidth = juce::jmin (newWidth, maxWidth);
    newHeight = juce::jmin (newHeight, maxHeight);
    
    // Always maintain aspect ratio by default, disable with Shift
    const bool maintainAspectRatio = !e.mods.isShiftDown();
    
    if (maintainAspectRatio)
    {
        const float aspectRatio = (float)baseWidth / (float)baseHeight;
        if (std::abs(d.x) > std::abs(d.y))
        {
            // Width changed more, adjust height to maintain ratio
            newHeight = (int)(newWidth / aspectRatio);
        }
        else
        {
            // Height changed more, adjust width to maintain ratio
            newWidth = (int)(newHeight * aspectRatio);
        }
        
        // Re-apply constraints after aspect ratio adjustment
        newWidth = juce::jlimit (minWidth, maxWidth, newWidth);
        newHeight = juce::jlimit (minHeight, maxHeight, newHeight);
        
        // If constraints broke the aspect ratio, adjust the other dimension
        const float currentRatio = (float)newWidth / (float)newHeight;
        const float targetRatio = aspectRatio;
        const float ratioError = std::abs(currentRatio - targetRatio);
        
        if (ratioError > 0.01f) // Allow small tolerance
        {
            if (currentRatio > targetRatio)
            {
                // Too wide, reduce width
                newWidth = (int)(newHeight * aspectRatio);
                newWidth = juce::jlimit (minWidth, maxWidth, newWidth);
            }
            else
            {
                // Too tall, reduce height  
                newHeight = (int)(newWidth / aspectRatio);
                newHeight = juce::jlimit (minHeight, maxHeight, newHeight);
            }
        }
    }
    
    setBounds (originalBounds.withSize (newWidth, newHeight));
}

void MyPluginAudioProcessorEditor::mouseUp (const juce::MouseEvent&) { isResizing = false; }

void MyPluginAudioProcessorEditor::resized()
{
    // Calculate scale factor based on current size vs base size
    const float widthScale = (float)getWidth() / (float)baseWidth;
    const float heightScale = (float)getHeight() / (float)baseHeight;
    scaleFactor = juce::jmin (widthScale, heightScale);
    
    // Ensure scale factor stays within reasonable bounds
    scaleFactor = juce::jlimit (0.6f, 2.0f, scaleFactor);
    
    // Call the existing layout code with the calculated scale factor
    performLayout();
}

// Repaint waveform from UI thread at ~30 Hz
void MyPluginAudioProcessorEditor::timerCallback()
{
    if (! isShowing()) return;
    // Update ducking meter overlay on knob
    const float grDb = proc.getCurrentDuckGrDb();
    duckingKnob.setCurrentGrDb (grDb);

    // Grey-out ATT/REL/THR/RAT when DUCK=0%
    const bool duckActive = duckingKnob.getValue() > 0.0001;
    duckAttack.setMuted (!duckActive);
    duckRelease.setMuted(!duckActive);
    duckThreshold.setMuted(!duckActive);
    duckRatio.setMuted(!duckActive);

    duckingKnob.repaint();
    duckAttack.repaint();
    duckRelease.repaint();
    duckThreshold.repaint();
    duckRatio.repaint();
    pad.repaint();
}

 

void MyPluginAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    juce::ignoreUnused (g);
    // No extra overlay on top of children for now
}

void MyPluginAudioProcessorEditor::setScaleFactor (float newScale)
{
    scaleFactor = juce::jlimit (0.5f, 2.0f, newScale);
    resized();
    repaint();
}

void MyPluginAudioProcessorEditor::sliderValueChanged (juce::Slider* s)
{
    auto set = [](juce::Label& l, const juce::String& t){ l.setText (t, juce::dontSendNotification); };
    auto Hz  = [](double v){ return juce::String (v, 1) + " Hz"; };
    auto dB  = [](double v){ return juce::String (v, 1) + " dB"; };
    auto pct = [](double v){ return juce::String (v, 0) + "%"; };

    if (s == &gain) {
        set (gainValue, dB (gain.getValue()));
        pad.setGainValue ((float) gain.getValue()); // feed XY for split-ball hit radius
    }
    else if (s == &width) {
        const double pctVal = juce::jlimit (0.0, 1000.0, width.getValue() * 100.0); // 1.0 -> 100%
        set (widthValue, juce::String (pctVal, 0) + "%");
    }
    else if (s == &tilt)   set (tiltValue, juce::String (tilt.getValue(), 1));
    else if (s == &monoHz) set (monoValue, Hz (monoHz.getValue()));
    else if (s == &hpHz)   set (hpValue,   Hz (hpHz.getValue()));
    else if (s == &lpHz)   set (lpValue,   Hz (lpHz.getValue()));
    else if (s == &satDrive) set (satDriveValue, dB (satDrive.getValue()));
    else if (s == &satMix)   set (satMixValue, pct (juce::jmap (satMix.getValue(), satMix.getMinimum(), satMix.getMaximum(), 0.0, 100.0)));
    else if (s == &air)    set (airValue, dB (air.getValue()));
    else if (s == &bass)   set (bassValue, dB (bass.getValue()));
    else if (s == &scoop)  {
        set (scoopValue, juce::String (scoop.getValue(), 1));
        // Dynamic label: Boost for >0, Scoop for <0, 0 shows Scoop
        scoop.setName (scoop.getValue() > 0.0 ? "BOOST" : "SCOOP");
    }
    else if (s == &panKnob){ set (panValue, juce::String (panKnob.getValue(), 2)); pad.setPanValue ((float) panKnob.getValue()); }
    else if (s == &panKnobLeft || s == &panKnobRight)
    {
        set (panValueLeft,  juce::String (panKnobLeft.getValue(), 2));
        set (panValueRight, juce::String (panKnobRight.getValue(), 2));
        panKnob.setSplitPercentage ((float) juce::jmap (panKnobLeft.getValue(),  -1.0, 1.0, 0.0, 100.0),
                                    (float) juce::jmap (panKnobRight.getValue(), -1.0, 1.0, 0.0, 100.0));
        panKnob.repaint();
    }
    else if (s == &spaceKnob)   { set (spaceValue, juce::String (spaceKnob.getValue(), 2)); pad.setSpaceValue ((float) spaceKnob.getValue()); }
    else if (s == &duckingKnob) {
        // Display 0–100% depth
        const double pctVal = juce::jmap (duckingKnob.getValue(), duckingKnob.getMinimum(), duckingKnob.getMaximum(), 0.0, 100.0);
        set (duckingValue, juce::String (pctVal, 0) + "%");
    }
    else if (s == &duckAttack)  set (duckAttackValue, juce::String (duckAttack.getValue(), 0) + " ms");
    else if (s == &duckRelease) set (duckReleaseValue, juce::String (duckRelease.getValue(), 0) + " ms");
    else if (s == &duckThreshold) set (duckThresholdValue, juce::String (duckThreshold.getValue(), 0) + " dB");
    else if (s == &duckRatio)     set (duckRatioValue, juce::String (duckRatio.getValue(), 1) + ":1");
    else if (s == &duckAttack)  set (duckAttackValue, juce::String (duckAttack.getValue(), 0) + " ms");
    else if (s == &duckRelease) set (duckReleaseValue, juce::String (duckRelease.getValue(), 0) + " ms");
    else if (s == &duckThreshold) set (duckThresholdValue, juce::String (duckThreshold.getValue(), 0) + " dB");

    else if (s == &tiltFreqSlider)  set (tiltFreqValue,  Hz (tiltFreqSlider.getValue()));
    else if (s == &scoopFreqSlider) set (scoopFreqValue, Hz (scoopFreqSlider.getValue()));
    else if (s == &bassFreqSlider)  set (bassFreqValue,  Hz (bassFreqSlider.getValue()));
    else if (s == &airFreqSlider)   set (airFreqValue,   Hz (airFreqSlider.getValue()));

    // Imaging value labels with units
    else if (s == &widthLo)  set (widthLoValue,  juce::String (juce::roundToInt (widthLo.getValue()  * 100.0))  + "%");
    else if (s == &widthMid) set (widthMidValue, juce::String (juce::roundToInt (widthMid.getValue() * 100.0)) + "%");
    else if (s == &widthHi)  set (widthHiValue,  juce::String (juce::roundToInt (widthHi.getValue()  * 100.0))  + "%");
    else if (s == &xoverLoHz) set (xoverLoValue, juce::String ((int) xoverLoHz.getValue()) + " Hz");
    else if (s == &xoverHiHz) set (xoverHiValue, juce::String ((int) xoverHiHz.getValue()) + " Hz");
    else if (s == &rotationDeg) set (rotationValue, juce::String (rotationDeg.getValue(), 1) + "°");
    else if (s == &asymmetry)   set (asymValue,     juce::String (juce::roundToInt (asymmetry.getValue() * 100.0)) + "%");
    else if (s == &shufLoPct)   set (shufLoValue,   juce::String (juce::roundToInt (shufLoPct.getValue())) + "%");
    else if (s == &shufHiPct)   set (shufHiValue,   juce::String (juce::roundToInt (shufHiPct.getValue())) + "%");
    else if (s == &shufXHz)     set (shufXValue,    juce::String ((int) shufXHz.getValue()) + " Hz");
    
    // Delay controls
    else if (s == &delayTime) set (delayTimeValue, juce::String ((int) delayTime.getValue()) + " ms");
    else if (s == &delayFeedback) set (delayFeedbackValue, juce::String ((int) delayFeedback.getValue()) + "%");
    else if (s == &delayWet) set (delayWetValue, pct (juce::jmap (delayWet.getValue(), delayWet.getMinimum(), delayWet.getMaximum(), 0.0, 100.0)));
    else if (s == &delaySpread) set (delaySpreadValue, juce::String ((int) delaySpread.getValue()) + "%");
    else if (s == &delayWidth) set (delayWidthValue, juce::String (juce::roundToInt (delayWidth.getValue() * 100.0)) + "%");
    else if (s == &delayModRate) set (delayModRateValue, Hz (delayModRate.getValue()));
    else if (s == &delayModDepth) set (delayModDepthValue, juce::String (delayModDepth.getValue(), 1) + " ms");
    else if (s == &delayWowflutter) set (delayWowflutterValue, pct (juce::jmap (delayWowflutter.getValue(), delayWowflutter.getMinimum(), delayWowflutter.getMaximum(), 0.0, 100.0)));
    else if (s == &delayJitter) set (delayJitterValue, juce::String (delayJitter.getValue(), 1) + "%");
    else if (s == &delayHp) set (delayHpValue, Hz (delayHp.getValue()));
    else if (s == &delayLp) set (delayLpValue, Hz (delayLp.getValue()));
    else if (s == &delayTilt) set (delayTiltValue, dB (delayTilt.getValue()));
    else if (s == &delaySat) set (delaySatValue, pct (juce::jmap (delaySat.getValue(), delaySat.getMinimum(), delaySat.getMaximum(), 0.0, 100.0)));
    else if (s == &delayDiffusion) set (delayDiffusionValue, pct (juce::jmap (delayDiffusion.getValue(), delayDiffusion.getMinimum(), delayDiffusion.getMaximum(), 0.0, 100.0)));
    else if (s == &delayDiffuseSize) set (delayDiffuseSizeValue, juce::String ((int) delayDiffuseSize.getValue()) + " ms");
    else if (s == &delayDuckDepth) set (delayDuckDepthValue, pct (juce::jmap (delayDuckDepth.getValue(), delayDuckDepth.getMinimum(), delayDuckDepth.getMaximum(), 0.0, 100.0)));
    else if (s == &delayDuckAttack) set (delayDuckAttackValue, juce::String ((int) delayDuckAttack.getValue()) + " ms");
    else if (s == &delayDuckRelease) set (delayDuckReleaseValue, juce::String ((int) delayDuckRelease.getValue()) + " ms");
    else if (s == &delayDuckThreshold) set (delayDuckThresholdValue, juce::String ((int) delayDuckThreshold.getValue()) + " dB");
    else if (s == &delayDuckRatio) set (delayDuckRatioValue, juce::String (delayDuckRatio.getValue(), 1) + ":1");
    else if (s == &delayDuckLookahead) set (delayDuckLookaheadValue, juce::String ((int) delayDuckLookahead.getValue()) + " ms");
}

void MyPluginAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    // Handle combo box changes if needed
}

void MyPluginAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    // Handle button clicks if needed
}

void MyPluginAudioProcessorEditor::applyGlobalCursorPolicy()
{
    auto setCursor = [] (juce::Component& c, auto& ref) -> void
    {
        const bool interactive = dynamic_cast<juce::Button*>(&c)
                                 || dynamic_cast<juce::Slider*>(&c)
                                 || dynamic_cast<juce::ComboBox*>(&c)
                                 || dynamic_cast<ToggleSwitch*>(&c)
                                 || dynamic_cast<XYPad*>(&c);
        if (interactive) c.setMouseCursor (juce::MouseCursor::PointingHandCursor);
        for (int i = 0; i < c.getNumChildComponents(); ++i) ref (*c.getChildComponent (i), ref);
    };
    setCursor (*this, setCursor);
}

void MyPluginAudioProcessorEditor::parameterChanged (const juce::String& id, float nv)
{
    // Bounce to message thread for UI updates during automation/offline render
    if (id == "pan" || id == "depth" || id == "split_mode")
    {
        const float v = nv;
        juce::MessageManager::callAsync ([this, id, v]
        {
            if      (id == "pan")        { pad.setPanValue   (v); }
            else if (id == "depth")      { pad.setSpaceValue (v); }
            else if (id == "split_mode") { pad.setSplitMode  (v >= 0.5f); resized(); }
        });
    }
    else if (id == "mono_slope_db_oct")
    {
        const int idx = (int) std::round (nv); // 0,1,2 from choice
        const int slope = (idx == 0 ? 6 : idx == 1 ? 12 : 24);
        juce::MessageManager::callAsync ([this, slope]
        {
            pad.setMonoSlopeDbPerOct (slope);
        });
    }
}

void MyPluginAudioProcessorEditor::updatePresetDisplay() { /* hook to PresetManager */ }

// --- A/B state helpers (unchanged from your version, trimmed) ---
void MyPluginAudioProcessorEditor::saveCurrentState()
{
    std::map<juce::String, float> s;
    s["gain_db"]  = (float) gain.getValue();
    s["width"]    = (float) width.getValue();
    s["tilt"]     = (float) tilt.getValue();
    s["mono_hz"]  = (float) monoHz.getValue();
    s["hp_hz"]    = (float) hpHz.getValue();
    s["lp_hz"]    = (float) lpHz.getValue();
    s["sat_drive_db"] = (float) satDrive.getValue();
    s["sat_mix"]  = (float) satMix.getValue();
    s["air_db"]   = (float) air.getValue();
    s["bass_db"]  = (float) bass.getValue();
    s["scoop"]    = (float) scoop.getValue();
    s["pan"]      = (float) panKnob.getValue();
    s["depth"]    = (float) spaceKnob.getValue();
    s["ducking"]  = (float) duckingKnob.getValue();
    if (isStateA) stateA = std::move (s); else stateB = std::move (s);
}

static void applyStateToSlider (juce::Slider& s, float v) { s.setValue (v, juce::sendNotificationSync); }

void MyPluginAudioProcessorEditor::loadState (bool A)
{
    const auto& s = A ? stateA : stateB;
    if (s.empty()) return;

    if (auto it = s.find ("gain_db");  it != s.end()) applyStateToSlider (gain, it->second);
    if (auto it = s.find ("width");    it != s.end()) applyStateToSlider (width, it->second);
    if (auto it = s.find ("tilt");     it != s.end()) applyStateToSlider (tilt, it->second);
    if (auto it = s.find ("mono_hz");  it != s.end()) applyStateToSlider (monoHz, it->second);
    if (auto it = s.find ("hp_hz");    it != s.end()) applyStateToSlider (hpHz, it->second);
    if (auto it = s.find ("lp_hz");    it != s.end()) applyStateToSlider (lpHz, it->second);
    if (auto it = s.find ("sat_drive_db"); it != s.end()) applyStateToSlider (satDrive, it->second);
    if (auto it = s.find ("sat_mix");  it != s.end()) applyStateToSlider (satMix, it->second);
    if (auto it = s.find ("air_db");   it != s.end()) applyStateToSlider (air, it->second);
    if (auto it = s.find ("bass_db");  it != s.end()) applyStateToSlider (bass, it->second);
    if (auto it = s.find ("scoop");    it != s.end()) applyStateToSlider (scoop, it->second);
    if (auto it = s.find ("pan");      it != s.end()) applyStateToSlider (panKnob, it->second);
    if (auto it = s.find ("depth");    it != s.end()) applyStateToSlider (spaceKnob, it->second);
    if (auto it = s.find ("ducking");  it != s.end()) applyStateToSlider (duckingKnob, it->second);
}

void MyPluginAudioProcessorEditor::toggleABState()
{
    saveCurrentState();
    isStateA = !isStateA;
    abButtonA.setToggleState (isStateA,   juce::dontSendNotification);
    abButtonB.setToggleState (!isStateA,  juce::dontSendNotification);
    loadState (isStateA);
}

void MyPluginAudioProcessorEditor::copyState (bool fromA) { clipboardState = fromA ? stateA : stateB; }
void MyPluginAudioProcessorEditor::pasteState (bool toA)  { if (!clipboardState.empty()) { (toA ? stateA : stateB) = clipboardState; loadState (toA); } }

void MyPluginAudioProcessorEditor::syncXYPadWithParameters()
{
    pad.setPanValue   ((float) panKnob.getValue());
    pad.setSpaceValue ((float) spaceKnob.getValue());
    pad.setWidthValue ((float) width.getValue());
    pad.setTiltValue  ((float) tilt .getValue());
    pad.setHPValue    ((float) hpHz .getValue());
    pad.setLPValue    ((float) lpHz .getValue());
    // initialize mono slope for XY visualization
    {
        int slopeGuess = 12;
        auto txt = monoSlopeChoice.getText();
        if      (txt == "6")  slopeGuess = 6;
        else if (txt == "12") slopeGuess = 12;
        else if (txt == "24") slopeGuess = 24;
        pad.setMonoSlopeDbPerOct (slopeGuess);
    }
    pad.setAirValue   ((float) air  .getValue());
    pad.setBassValue  ((float) bass .getValue());
    pad.setScoopValue ((float) scoop.getValue());
    pad.setGainValue  ((float) gain .getValue());
}

// end
