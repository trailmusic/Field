#include "MotionPanel.h"
#include "../ui/Layout.h"
using namespace UI;
namespace motion {

MotionPanel::MotionPanel(juce::AudioProcessorValueTreeState& s, juce::UndoManager*)
: state(s)
{
    // Only start timer for orb animation - no controls needed
    // Controls are handled by the 5x4 grid in Group 2 panel
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
    // Only position the orb - no controls to layout
    // Controls are handled by the 5x4 grid in Group 2 panel
    auto area = getLocalBounds().reduced(pad);
    int orbSize = juce::jmin(area.getWidth(), area.getHeight() - pad);
    orbBounds = area.withSizeKeepingCentre(orbSize, orbSize);
}

}