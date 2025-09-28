#include "ToggleSwitch.h"
#include "../../Core/FieldLookAndFeel.h"

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

    // match editor theme
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    const auto accent = th.accent;

    // track
    g.setColour (th.sh);
    g.fillRoundedRectangle (b, rad);
    g.setColour (th.hl);
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
    g.setColour ((lf ? lf->theme.shadowDark : juce::Colours::black).withAlpha (0.4f));
    g.fillEllipse (k.translated (2.0f, 2.0f));

    // fill: stereo = accent blue/green, split = grey
    g.setColour (isOn ? (lf ? lf->theme.textMuted : juce::Colour (0xFF7A7D85)) : accent);
    g.fillEllipse (k);

    // rim + split marker
    g.setColour (lf ? lf->theme.textMuted : juce::Colour (0xFF9A9DA5));
    g.drawEllipse (k, 2.0f);
    if (isOn)
    {
        g.setColour (lf ? lf->theme.text : juce::Colour (0xFFB0B3B8));
        const float cx = k.getCentreX();
        g.drawLine (cx, k.getY() + 4.0f, cx, k.getBottom() - 4.0f, 1.5f);
    }

    if (sliderValue.isSmoothing()) repaint();
}
