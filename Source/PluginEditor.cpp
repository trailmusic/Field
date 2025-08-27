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
    auto mouse = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition() - getScreenPosition().toFloat();
    if (b.contains (mouse) || hoverActive)
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

    if (showBorder)
    {
        auto border = r.reduced (3.0f);
        // hover halo
        auto mouse = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition() - getScreenPosition().toFloat();
        if (border.contains (mouse) || hoverActive)
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
    auto mouse  = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition() - getScreenPosition().toFloat();
    g.setColour (accent);
    g.drawRoundedRectangle (border, rad, 2.0f);
    if (border.contains (mouse) || hoverActive)
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

    // Draw mono cutoff line + stronger gradient shading to the left of the line
    if (monoHzValue > 20.0f)
    {
        const float minHz = 20.0f, maxHz = 20000.0f;
        const float t = (float) (std::log10 (juce::jlimit (minHz, maxHz, monoHzValue) / minHz) / std::log10 (maxHz / minHz));
        const float xMono = juce::jmap (t, 0.0f, 1.0f, b.getX(), b.getRight());
        // Stronger gradient: from line (high alpha) to far-left (0 alpha)
        juce::ColourGradient grad (hpLpCol.withAlpha (0.35f), xMono, b.getCentreY(), hpLpCol.withAlpha (0.00f), b.getX(), b.getCentreY(), true);
        g.setGradientFill (grad);
        g.fillRect (juce::Rectangle<float> (b.getX(), b.getY(), xMono - b.getX(), b.getHeight()));
        // Line on top
        g.setColour (hpLpCol.withAlpha (0.95f));
        g.drawLine (xMono, b.getY(), xMono, b.getBottom(), 1.2f);
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
    // Size + LNF
    setSize (baseWidth, baseHeight);
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
                    "Q: Why don’t knobs move when I resize?\n"
                    "A: Sizing happens in resized() only; layout is responsive via Layout::dp().\n\n"
                    "Q: How do I reset a control?\n"
                    "A: Double-click most knobs/sliders to reset to default.\n\n"
                    "Q: Where are presets saved?\n"
                    "A: In your user data folder under the plugin’s presets directory.\n\n"
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
    addAndMakeVisible (mainControlsContainer); mainControlsContainer.setTitle ("MAIN CONTROLS"); mainControlsContainer.setShowBorder (false);
    addAndMakeVisible (panKnobContainer);      panKnobContainer.setTitle ("PAN");     panKnobContainer.setShowBorder (true);
    addAndMakeVisible (spaceKnobContainer);    spaceKnobContainer.setTitle ("SPACE"); spaceKnobContainer.setShowBorder (true);
    addAndMakeVisible (volumeContainer);       volumeContainer.setTitle ("VOLUME");   volumeContainer.setShowBorder (true);
    addAndMakeVisible (eqContainer);           eqContainer.setTitle ("EQ");           eqContainer.setShowBorder (true);
    addAndMakeVisible (imageContainer);        imageContainer.setTitle ("IMAGE");     imageContainer.setShowBorder (true);
    addAndMakeVisible (metersContainer);       metersContainer.setTitle ("METERS");   metersContainer.setShowBorder (true);

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
    for (juce::Slider* s : { &width,&tilt,&monoHz,&hpHz,&lpHz,&satDrive,&satMix,&air,&bass,&scoop,
                              &widthLo,&widthMid,&widthHi,&xoverLoHz,&xoverHiHz,&rotationDeg,&asymmetry,&shufLoPct,&shufHiPct,&shufXHz })
    {
        addAndMakeVisible (*s);
        style (*s);
        s->addListener (this);
    }
    addAndMakeVisible (gain); style (gain); gain.addListener (this);

    // micro sliders (freq)
    for (juce::Slider* s : { &tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider })
    {
        addAndMakeVisible (*s);
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s->setMouseDragSensitivity (100);
        s->setVelocityBasedMode (false);
        s->setSliderSnapsToMousePosition (true);
        s->setDoubleClickReturnValue (true, 0.0);
        s->addListener (this);
    }

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
                             &panValue,&panValueLeft,&panValueRight,&spaceValue,&duckingValue,
                             &tiltFreqValue,&scoopFreqValue,&bassFreqValue,&airFreqValue,
                             &widthLoValue,&widthMidValue,&widthHiValue,&xoverLoValue,&xoverHiValue,
                             &rotationValue,&asymValue,&shufLoValue,&shufHiValue,&shufXValue })
    {
        addAndMakeVisible (*l);
        l->setJustificationType (juce::Justification::centred);
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
    for (juce::Slider* s : { &tilt,&hpHz,&lpHz,&air,&bass,&scoop,&tiltFreqSlider,&scoopFreqSlider,&bassFreqSlider,&airFreqSlider,&monoHz })
        addLiveRepaint (*s);

    // slider names (for LNF knob labels)
    gain.setName ("GAIN"); width.setName ("WIDTH"); tilt.setName ("TILT"); monoHz.setName ("MONO");
    hpHz.setName ("HP Hz"); lpHz.setName ("LP Hz"); satDrive.setName ("DRIVE"); satMix.setName ("MIX");
    air.setName ("AIR"); bass.setName ("BASS"); scoop.setName ("SCOOP"); spaceKnob.setName ("SPACE");
    duckingKnob.setName ("DUCK"); panKnob.setName ("PAN"); panKnobLeft.setName ("PAN L"); panKnobRight.setName ("PAN R");
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
    addAndMakeVisible (monoSlopeChoice);
    monoSlopeChoice.addItem ("6",  1);
    monoSlopeChoice.addItem ("12", 2);
    monoSlopeChoice.addItem ("24", 3);
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
        spaceAlgorithmSwitch.setAlgorithmFromParameter (idx);
    }
    spaceAlgorithmSwitch.onAlgorithmChange = [this](int a)
    {
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter ("space_algo")))
        {
            const int maxIdx = juce::jmax (0, c->choices.size() - 1);
            const int idx = juce::jlimit (0, maxIdx, a);
            const float norm = c->convertTo0to1 ((float) idx);
            c->beginChangeGesture(); c->setValueNotifyingHost (norm); c->endChangeGesture();
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

    // parameter listeners (host→UI)
    proc.apvts.addParameterListener ("space_algo", this);
    proc.apvts.addParameterListener ("split_mode", this);
    proc.apvts.addParameterListener ("pan",        this);
    proc.apvts.addParameterListener ("depth",      this);

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
}

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    panKnobLeft.removeListener (this);
    panKnobRight.removeListener (this);

    proc.apvts.removeParameterListener ("space_algo", this);
    proc.apvts.removeParameterListener ("split_mode", this);
    proc.apvts.removeParameterListener ("pan",        this);
    proc.apvts.removeParameterListener ("depth",      this);

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

void MyPluginAudioProcessorEditor::resized()
{
    const float s = juce::jmax (0.6f, scaleFactor);
    auto r = getLocalBounds().reduced (Layout::dp (Layout::PAD, s)).withTrimmedBottom (50);

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
        juce::Grid::TrackInfo (juce::Grid::Px (Layout::dp (40, s))), // color
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
        const int pad = Layout::dp (8, s);
        optionsButton.setTopLeftPosition (bounds.getX() + pad, bounds.getBottom() - h - pad);
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

    // 2) main XY area
    {
        auto main = r.removeFromTop (juce::jmax (Layout::dp (300, s), (int) std::round (r.getHeight() * 0.5f)));
        // Split main horizontally to add meters at right
        auto metersW = Layout::dp (160, s);
        auto mainPad = main.withTrimmedRight (metersW);
        auto mainMeters = main.removeFromRight (metersW);
        pad.setBounds (mainPad.reduced (Layout::dp (Layout::GAP, s)));
        if (xyShade) xyShade->setBounds (pad.getBounds());
        metersContainer.setBounds (mainMeters.reduced (Layout::dp (Layout::GAP, s)));
        // Layout meters: correlation + simple LR peak bars
        addAndMakeVisible (corrMeter);
        auto mB = metersContainer.getBounds().reduced (Layout::dp (Layout::GAP, s));
        corrMeter.setBounds (mB.removeFromTop (Layout::dp (140, s)));
    }

    r.removeFromTop (Layout::dp (Layout::GAP, s));

    // 3) bottom rows – Volume row then EQ row (keeps original spacing)
    {
        auto bottom = r;

        const int knobTop    = Layout::dp ((float) Layout::knobPx (Layout::Knob::M), s);
        const int knobBottom = Layout::dp ((float) Layout::knobPx (Layout::Knob::M), s) + Layout::dp (Layout::LABEL_BAND_EXTRA, s);
        const int gap        = Layout::dp (Layout::GAP, s);
        const int eqGap      = juce::jmax (1, gap / 3);

        juce::Grid vol;
        vol.rowGap     = juce::Grid::Px (gap);
        vol.columnGap  = juce::Grid::Px (gap);
        vol.templateRows    = { juce::Grid::TrackInfo (juce::Grid::Fr (1)) };
        vol.templateColumns = { juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)) };

        juce::Grid eq;
        eq.rowGap     = juce::Grid::Px (eqGap);
        eq.columnGap  = juce::Grid::Px (eqGap);
        eq.templateRows    = { juce::Grid::TrackInfo (juce::Grid::Fr (1)) };
        eq.templateColumns = { juce::Grid::TrackInfo (juce::Grid::Fr (1)), // bass
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // bass micro
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // air
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // air micro
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // tilt
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // tilt micro
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // scoop
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // scoop micro
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // hp
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)), // lp
                               juce::Grid::TrackInfo (juce::Grid::Fr (1)) };// mono

        auto volArea   = bottom.removeFromTop (knobTop + Layout::dp (70, s));
        bottom.removeFromTop (Layout::dp (28, s)); // spacer
        // Split remaining area into EQ row and IMAGE row
        auto eqArea    = bottom.removeFromTop (knobBottom + Layout::dp (70, s));
        bottom.removeFromTop (Layout::dp (28, s)); // spacer between rows
        auto imgArea   = bottom;

        auto setTop    = [&](juce::Component& c){ c.setBounds (0, 0, knobTop,    knobTop); };
        auto setBottom = [&](juce::Component& c){ c.setBounds (0, 0, knobBottom, knobBottom); };

        panKnob.setBounds (0, 0, Layout::dp ((float) Layout::knobPx (Layout::Knob::XL), s), Layout::dp ((float) Layout::knobPx (Layout::Knob::XL), s));
        setTop (spaceKnob);
        spaceAlgorithmSwitch.setBounds (0, 0, Layout::dp (Layout::ALGO_SWITCH_W, s), knobTop);
        setTop (gain); setTop (satDrive); setTop (satMix); setTop (width); setTop (duckingKnob);

        setBottom (bass); setBottom (air); setBottom (tilt); setBottom (scoop);
        setBottom (hpHz); setBottom (lpHz); setBottom (monoHz);
        // imaging knobs size
        setBottom (widthLo); setBottom (widthMid); setBottom (widthHi);
        setBottom (xoverLoHz); setBottom (xoverHiHz);
        setBottom (rotationDeg); setBottom (asymmetry);
        setBottom (shufLoPct); setBottom (shufHiPct); setBottom (shufXHz);

        const bool split = pad.getSplitMode();
        if (split)
            vol.items = {
                juce::GridItem (panSplitContainer),
                juce::GridItem (spaceKnob),
                juce::GridItem (spaceAlgorithmSwitch),
                juce::GridItem (duckingKnob),
                juce::GridItem (gain),
                juce::GridItem (satDrive),
                juce::GridItem (width),
                juce::GridItem (satMix),
            };
        else
            vol.items = {
                juce::GridItem (panKnob),
                juce::GridItem (spaceKnob),
                juce::GridItem (spaceAlgorithmSwitch),
                juce::GridItem (duckingKnob),
                juce::GridItem (gain),
                juce::GridItem (satDrive),
                juce::GridItem (width),
                juce::GridItem (satMix),
            };

        eq.items = {
            juce::GridItem (bass),
            juce::GridItem (bassFreqSlider)
                .withWidth  (Layout::dp (Layout::MICRO_W, s))
                .withHeight (Layout::dp (Layout::MICRO_H, s))
                .withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (air),
            juce::GridItem (airFreqSlider )
                .withWidth  (Layout::dp (Layout::MICRO_W, s))
                .withHeight (Layout::dp (Layout::MICRO_H, s))
                .withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (tilt),
            juce::GridItem (tiltFreqSlider)
                .withWidth  (Layout::dp (Layout::MICRO_W, s))
                .withHeight (Layout::dp (Layout::MICRO_H, s))
                .withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (scoop),
            juce::GridItem (scoopFreqSlider)
                .withWidth  (Layout::dp (Layout::MICRO_W, s))
                .withHeight (Layout::dp (Layout::MICRO_H, s))
                .withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (hpHz),
            juce::GridItem (lpHz),
            juce::GridItem (monoHz),
        };

        vol.performLayout (volArea);
        addAndMakeVisible (rowDivVol);
        rowDivVol.setBounds (volArea.withY (volArea.getBottom()).withHeight (2));
        eq.performLayout  (eqArea);
        // Place thin vertical dividers between LP|Mono and ScoopF|HP
        addAndMakeVisible (eqDivLpMono);
        addAndMakeVisible (eqDivScoopHp);
        // Divider between LP and Mono
        {
            auto lpB   = lpHz.getBounds();
            auto monoB = monoHz.getBounds();
            auto x = (lpB.getRight() + monoB.getX()) / 2;
            eqDivLpMono.setBounds (x, eqArea.getY(), 1, eqArea.getHeight());
        }
        // Divider between Scoop mini slider and HP
        {
            auto scMiniB = scoopFreqSlider.getBounds();
            auto hpB     = hpHz.getBounds();
            auto x = (scMiniB.getRight() + hpB.getX()) / 2;
            eqDivScoopHp.setBounds (x, eqArea.getY(), 1, eqArea.getHeight());
        }
        addAndMakeVisible (rowDivEQ);
        rowDivEQ.setBounds (eqArea.withY (eqArea.getBottom()).withHeight (2));

        // IMAGE row layout
        juce::Grid img;
        img.rowGap     = juce::Grid::Px (eqGap);
        img.columnGap  = juce::Grid::Px (eqGap);
        img.templateRows    = { juce::Grid::TrackInfo (juce::Grid::Fr (1)) };
        img.templateColumns = { juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)),
                                juce::Grid::TrackInfo (juce::Grid::Fr (1)) };

        const int imgKnob = knobBottom;
        img.items = {
            juce::GridItem (widthLo)   .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (widthMid)  .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (widthHi)   .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (xoverLoHz) .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (xoverHiHz) .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (rotationDeg).withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (asymmetry) .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (shufLoPct) .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (shufHiPct) .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
            juce::GridItem (shufXHz)   .withWidth (imgKnob).withHeight (imgKnob).withAlignSelf (juce::GridItem::AlignSelf::center),
        };

        img.performLayout (imgArea);

        // Place Mono slope and audition next to Mono knob
        {
            auto monoCell = monoHz.getBounds();
            auto rightHalf = monoCell.removeFromRight (juce::jmax (monoCell.getWidth() / 2, Layout::dp (120, s)));
            auto topRow    = rightHalf.removeFromTop (rightHalf.getHeight() / 2);
            monoSlopeChoice.setBounds (topRow.reduced (Layout::dp (4, s)));
            monoAuditionButton.setBounds (rightHalf.reduced (Layout::dp (4, s)));
            // Labels above them (subtle)
            monoSlopeName.setText ("SLOPE", juce::dontSendNotification);
            monoAudName  .setText ("AUD",   juce::dontSendNotification);
            addAndMakeVisible (monoSlopeName);
            addAndMakeVisible (monoAudName);
            monoSlopeName.setBounds (monoSlopeChoice.getBounds().translated (0, -Layout::dp (14, s)).withHeight (Layout::dp (12, s)));
            monoAudName  .setBounds (monoAuditionButton.getBounds().translated (0, -Layout::dp (14, s)).withHeight (Layout::dp (12, s)));
            monoSlopeName.setJustificationType (juce::Justification::centred);
            monoAudName  .setJustificationType (juce::Justification::centred);
            monoSlopeName.setColour (juce::Label::textColourId, lnf.theme.textMuted);
            monoAudName  .setColour (juce::Label::textColourId, lnf.theme.textMuted);
        }

        // Imaging name labels suppressed to avoid duplication with values

        // Place value labels under corresponding controls (centered, 40px width)
        auto placeBelow = [&] (juce::Component& comp, juce::Label& lab)
        {
            auto b = comp.getBounds();
            const int w = Layout::dp (40, s);
            lab.setBounds (b.getCentreX() - w / 2, b.getBottom() + Layout::dp (2, s), w, Layout::dp (16, s));
        };

        // Volume row
        placeBelow (gain,        gainValue);
        placeBelow (satDrive,    satDriveValue);
        placeBelow (satMix,      satMixValue);
        placeBelow (width,       widthValue);
        placeBelow (duckingKnob, duckingValue);
        placeBelow (spaceKnob,   spaceValue);

        if (pad.getSplitMode())
        {
            placeBelow (panKnobLeft,  panValueLeft);
            placeBelow (panKnobRight, panValueRight);
        }
        else
        {
            placeBelow (panKnob, panValue);
        }

        // EQ row
        placeBelow (bass,  bassValue);
        placeBelow (air,   airValue);
        placeBelow (tilt,  tiltValue);
        placeBelow (scoop, scoopValue);
        placeBelow (hpHz,  hpValue);
        placeBelow (lpHz,  lpValue);
        placeBelow (monoHz, monoValue);

        // IMAGE row values
        placeBelow (widthLo,   widthLoValue);
        placeBelow (widthMid,  widthMidValue);
        placeBelow (widthHi,   widthHiValue);
        placeBelow (xoverLoHz, xoverLoValue);
        placeBelow (xoverHiHz, xoverHiValue);
        placeBelow (rotationDeg, rotationValue);
        placeBelow (asymmetry, asymValue);
        placeBelow (shufLoPct, shufLoValue);
        placeBelow (shufHiPct, shufHiValue);
        placeBelow (shufXHz,   shufXValue);

        // Micro sliders labels inline below their sliders
        placeBelow (bassFreqSlider,  bassFreqValue);
        placeBelow (airFreqSlider,   airFreqValue);
        placeBelow (tiltFreqSlider,  tiltFreqValue);
        placeBelow (scoopFreqSlider, scoopFreqValue);

        // anchor split sub-knobs in the same grid cell as stereo pan
        if (split)
        {
            auto cell = panSplitContainer.getBounds();
            auto inner = cell.reduced (Layout::dp (10, s));
            auto half  = inner.removeFromLeft (inner.getWidth() / 2);
            panKnobLeft .setBounds (half.reduced  (Layout::dp (6, s)));
            panKnobRight.setBounds (inner.reduced (Layout::dp (6, s)));
            panSplitContainer.setBounds (cell);
        }
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
    setBounds (originalBounds.withSize (juce::jmax (500, originalBounds.getWidth()  + d.x),
                                        juce::jmax (350, originalBounds.getHeight() + d.y)));
}

void MyPluginAudioProcessorEditor::mouseUp (const juce::MouseEvent&) { isResizing = false; }

// Repaint waveform from UI thread at ~30 Hz
void MyPluginAudioProcessorEditor::timerCallback()
{
    pad.repaint();
}

void MyPluginAudioProcessorEditor::styleSlider (juce::Slider& s) { /* handled inline in ctor */ }
void MyPluginAudioProcessorEditor::styleMainSlider (juce::Slider& s) { /* handled inline in ctor */ }

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
    else if (s == &spaceKnob)   set (spaceValue, juce::String (spaceKnob.getValue(), 2));
    else if (s == &duckingKnob) set (duckingValue, juce::String (duckingKnob.getValue(), 1) + "%");

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
    pad.setAirValue   ((float) air  .getValue());
    pad.setBassValue  ((float) bass .getValue());
    pad.setScoopValue ((float) scoop.getValue());
    pad.setGainValue  ((float) gain .getValue());
}

// end
