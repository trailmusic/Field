#pragma once

#include <JuceHeader.h>
#include "../Panes/XYControlsPane.h"
#include "../Controls/ControlGridMetrics.h"
#include "../Components/XYPad.h"

class MyPluginAudioProcessor;

// Composite XY tab: XY visuals (XYPad) + 2x16 controls
class XYTab : public juce::Component
{
public:
    XYTab (MyPluginAudioProcessor& p)
        : proc (p)
    {
        // Create XYPad as internal component
        xyPad = std::make_unique<XYPad>();
        this->juce::Component::addAndMakeVisible (xyPad.get());
        
        // Create controls
        controls = std::make_unique<XYControlsPane>(p.apvts);
        this->juce::Component::addAndMakeVisible (controls.get());
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto m = ControlGridMetrics::compute (r.getWidth(), r.getHeight());
        if (controls) { controls->setCellMetrics (m.knobPx, m.valuePx, m.labelGapPx, m.colW); controls->setRowHeightPx (m.rowH); }
        auto controlsArea = r.removeFromBottom (m.controlsH);
        if (xyPad) xyPad->setBounds (r);
        if (controls) controls->juce::Component::setBounds (controlsArea);
    }

    // Forward XYPad methods
    XYPad* getXYPad() { return xyPad.get(); }
    void pushWaveformSample (double L, double R) { if (xyPad) xyPad->pushWaveformSample(L, R); }
    void setSampleRate (double sr) { if (xyPad) xyPad->setSampleRate(sr); }
    
    // Forward XYPad control methods
    void setGreenMode(bool green) { if (xyPad) xyPad->setGreenMode(green); }
    void setLinked(bool linked) { if (xyPad) xyPad->setLinked(linked); }
    void setSnapEnabled(bool enabled) { if (xyPad) xyPad->setSnapEnabled(enabled); }
    void setSplitMode(bool split) { if (xyPad) xyPad->setSplitMode(split); }
    void setTiltValue(float value) { if (xyPad) xyPad->setTiltValue(value); }
    void setHPValue(float value) { if (xyPad) xyPad->setHPValue(value); }
    void setLPValue(float value) { if (xyPad) xyPad->setLPValue(value); }
    void setAirValue(float value) { if (xyPad) xyPad->setAirValue(value); }
    void setBassValue(float value) { if (xyPad) xyPad->setBassValue(value); }
    void setScoopValue(float value) { if (xyPad) xyPad->setScoopValue(value); }
    void setTiltFreqValue(float value) { if (xyPad) xyPad->setTiltFreqValue(value); }
    void setScoopFreqValue(float value) { if (xyPad) xyPad->setScoopFreqValue(value); }
    void setBassFreqValue(float value) { if (xyPad) xyPad->setBassFreqValue(value); }
    void setAirFreqValue(float value) { if (xyPad) xyPad->setAirFreqValue(value); }
    void setMonoValue(float value) { if (xyPad) xyPad->setMonoValue(value); }
    
    // Additional methods needed by PluginEditor
    void setMixValue(float value) { if (xyPad) xyPad->setMixValue(value); }
    void setDriveValue(float value) { if (xyPad) xyPad->setDriveValue(value); }
    void setWidthValue(float value) { if (xyPad) xyPad->setWidthValue(value); }
    void setPanValue(float value) { if (xyPad) xyPad->setPanValue(value); }
    void setGainValue(float value) { if (xyPad) xyPad->setGainValue(value); }
    void setSpaceValue(float value) { if (xyPad) xyPad->setSpaceValue(value); }
    void setMonoSlopeDbPerOct(int slope) { if (xyPad) xyPad->setMonoSlopeDbPerOct(slope); }
    void setSpaceAlgorithm(int algorithm) { if (xyPad) xyPad->setSpaceAlgorithm(algorithm); }
    
    // Callback properties
    std::function<void(float, float)> onChange;
    std::function<void(float, float, float)> onSplitChange;
    
    // Public getter for graphics container
    XYPad* getXYPad() const { return xyPad.get(); }

private:
    MyPluginAudioProcessor& proc;
    std::unique_ptr<XYPad> xyPad;
    std::unique_ptr<XYControlsPane> controls;
};


