
#include "MotionPanel.h"
#include "../ui/Layout.h"
using namespace UI;
namespace motion {
static void styleKnob(juce::Slider& s) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
}
static void styleMini(juce::Slider& s) {
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    s.setRange(0.0, 1.0, 0.001);
}
MotionPanel::MotionPanel(juce::AudioProcessorValueTreeState& s, juce::UndoManager*)
: state(s)
{
    pannerSelect.reset(new juce::ComboBox("Panner")); pannerSelect->addItemList(motion::choiceListPanner(), 1);
    path.reset(new juce::ComboBox("Path"));           path->addItemList(motion::choiceListPath(), 1);
    mode.reset(new juce::ComboBox("Mode"));           mode->addItemList(motion::choiceListMode(), 1);
    quant.reset(new juce::ComboBox("Quant"));         quant->addItemList(motion::choiceListQuant(), 1);
    rate.reset(new juce::Slider("Rate"));     styleKnob(*rate);
    depth.reset(new juce::Slider("Depth"));   styleKnob(*depth);
    phase.reset(new juce::Slider("Phase"));   styleKnob(*phase);
    spread.reset(new juce::Slider("Spread")); styleKnob(*spread);
    elevBias.reset(new juce::Slider("Elev")); styleKnob(*elevBias);
    bounce.reset(new juce::Slider("Bounce")); styleKnob(*bounce);
    jitter.reset(new juce::Slider("Jitter")); styleKnob(*jitter);
    holdMs.reset(new juce::Slider("Hold"));   styleMini(*holdMs);
    sens.reset(new juce::Slider("Sens"));     styleMini(*sens);
    offsetDeg.reset(new juce::Slider("Offset")); styleKnob(*offsetDeg);
    frontBias.reset(new juce::Slider("Front")); styleKnob(*frontBias);
    doppler.reset(new juce::Slider("Doppler")); styleKnob(*doppler);
    motionSend.reset(new juce::Slider("MotionSend")); styleKnob(*motionSend);
    bassFloor.reset(new juce::Slider("BassFloor")); styleKnob(*bassFloor);
    retrig.reset(new juce::ToggleButton("Retrig"));
    anchor.reset(new juce::ToggleButton("Anchor"));
    headphoneSafe.reset(new juce::ToggleButton("HeadphoneSafe"));
    aPanner.reset(new Box(state, id::panner_select, *pannerSelect));
    aPath.reset(new Box(state, id::path, *path));
    aMode.reset(new Box(state, id::mode, *mode));
    aQuant.reset(new Box(state, id::quantize_div, *quant));
    aRate.reset(new Att(state, id::rate_hz, *rate));
    aDepth.reset(new Att(state, id::depth_pct, *depth));
    aPhase.reset(new Att(state, id::phase_deg, *phase));
    aSpread.reset(new Att(state, id::spread_pct, *spread));
    aElevBias.reset(new Att(state, id::elev_bias, *elevBias));
    aBounce.reset(new Att(state, id::shape_bounce, *bounce));
    aJitter.reset(new Att(state, id::jitter_amt, *jitter));
    aHold.reset(new Att(state, id::hold_ms, *holdMs));
    aSens.reset(new Att(state, id::sens, *sens));
    aOffset.reset(new Att(state, id::offset_deg, *offsetDeg));
    aFront.reset(new Att(state, id::front_bias, *frontBias));
    aDoppler.reset(new Att(state, id::doppler_amt, *doppler));
    aMotionSend.reset(new Att(state, id::motion_send, *motionSend));
    aBassFloor.reset(new Att(state, id::bass_floor_hz, *bassFloor));
    aRetrig.reset(new Btn(state, id::retrig, *retrig));
    aAnchor.reset(new Btn(state, id::anchor_enable, *anchor));
    aHeadSafe.reset(new Btn(state, id::headphone_safe, *headphoneSafe));
    for (auto* c : {
        (juce::Component*)pannerSelect.get(), (juce::Component*)path.get(), (juce::Component*)rate.get(), (juce::Component*)depth.get(), (juce::Component*)phase.get(),
        (juce::Component*)spread.get(), (juce::Component*)elevBias.get(), (juce::Component*)bounce.get(), (juce::Component*)jitter.get(), (juce::Component*)quant.get(),
        (juce::Component*)mode.get(), (juce::Component*)retrig.get(), (juce::Component*)holdMs.get(), (juce::Component*)sens.get(), (juce::Component*)offsetDeg.get(),
        (juce::Component*)frontBias.get(), (juce::Component*)doppler.get(), (juce::Component*)motionSend.get(), (juce::Component*)anchor.get(), (juce::Component*)bassFloor.get(), (juce::Component*)headphoneSafe.get()
    }) addAndMakeVisible(c);
    startTimerHz(60);
}
void MotionPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121317));
    auto o = orbBounds.toFloat();
    g.setColour(juce::Colours::white.withAlpha(0.07f));
    g.fillEllipse(o);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    for (int i=1;i<6;++i) {
        float t = i/6.0f; auto r = o.reduced(o.getWidth()*t*0.12f); g.drawEllipse(r, 1.0f);
    }
    demoTheta += 0.02f; float th = demoTheta;
    float cx = o.getCentreX(), cy = o.getCentreY(), rad = o.getWidth()*0.35f;
    float x = cx + std::cos(th) * rad; float y = cy + std::sin(th) * rad * 0.6f;
    g.setColour(juce::Colours::aqua.withAlpha(0.8f)); g.fillEllipse(x-4, y-4, 8, 8);
    g.setColour(juce::Colours::white.withAlpha(0.6f)); g.setFont(14.0f);
    g.drawFittedText("Field • Motion", orbBounds.removeFromTop(24), juce::Justification::centredTop, 1);
}
void MotionPanel::resized()
{
    auto area = getLocalBounds().reduced(pad);
    int orbSize = juce::jmin(area.getWidth() * 2 / 5, area.getHeight() - pad);
    orbBounds = area.removeFromLeft(orbSize).withSizeKeepingCentre(orbSize, orbSize);
    auto ctl = area.reduced(pad);
    int rows = 4; int rowHeight = juce::jmax(rowH, ctl.getHeight() / rows);
    auto r1 = ctl.removeFromTop(rowHeight);
    auto r2 = ctl.removeFromTop(rowHeight);
    auto r3 = ctl.removeFromTop(rowHeight);
    auto r4 = ctl.removeFromTop(rowHeight);
    UI::placeRow(r1, { pannerSelect.get(), path.get(), rate.get(), depth.get(), phase.get() }, gap);
    UI::placeRow(r2, { spread.get(), elevBias.get(), bounce.get(), jitter.get(), quant.get() }, gap);
    UI::placeRow(r3, { mode.get(), retrig.get(), holdMs.get(), sens.get(), offsetDeg.get() }, gap);
    UI::placeRow(r4, { frontBias.get(), doppler.get(), motionSend.get(), anchor.get(), bassFloor.get() }, gap);
    auto topRight = getLocalBounds().reduced(pad).removeFromTop(24).removeFromRight(160);
    headphoneSafe->setBounds(topRight);
}
}
