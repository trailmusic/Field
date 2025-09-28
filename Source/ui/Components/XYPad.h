#pragma once

#include <JuceHeader.h>

class XYPad : public juce::Component, public juce::Timer
{
public:
    ~XYPad() override { stopTimer(); }
    
    std::function<void (float x01, float y01)> onChange; // x=pan, y=depth
    std::function<void (float leftX01, float rightX01, float y01)> onSplitChange; // for split mode
    std::function<void (int ballIndex, float x01, float y01)> onBallChange; // individual ball control
    
    void setPoint01 (float x, float y) { pt = { juce::jlimit(0.f,1.f,x), juce::jlimit(0.f,1.f,y) }; repaint(); }
    void setSplitPoints (float leftX, float rightX, float y) { leftPt = leftX; rightPt = rightX; pt.second = y; repaint(); }
    void setBallPosition (int ballIndex, float x, float y);
    std::pair<float,float> getPoint01() const { return pt; }
    std::pair<float,float> getSplitPoints() const { return {leftPt, rightPt}; }
    std::pair<float,float> getBallPosition (int ballIndex) const;
    
    void setSplitMode (bool split) { isSplitMode = split; if (split && isLinked) { leftPt = 0.5f; rightPt = 0.5f; } repaint(); }
    bool getSplitMode() const { return isSplitMode; }
    
    void setLinked (bool linked) { isLinked = linked; if (linked && isSplitMode) { leftPt = 0.5f; rightPt = 0.5f; } repaint(); }
    bool getLinked() const { return isLinked; }
    
    // Visual parameter setters
    void setGainValue (float gainDb) { gainValue = gainDb; repaint(); }
    void setWidthValue (float widthPercent) { widthValue = widthPercent; repaint(); }
    void setTiltValue (float tiltDegrees) { tiltValue = tiltDegrees; repaint(); }
    void setMixValue (float mix01) { mixValue = mix01; repaint(); }
    void setDriveValue (float driveDb) { driveValue = driveDb; repaint(); }
    void setAirValue (float airDb) { airValue = airDb; repaint(); }
    void setBassValue (float bassDb) { bassValue = bassDb; repaint(); }
    void setScoopValue (float scoopDb) { scoopValue = scoopDb; repaint(); }
    void setHPValue (float hpHz) { hpValue = hpHz; repaint(); }
    void setLPValue (float lpHz) { lpValue = lpHz; repaint(); }
    void setPanValue (float pan) { panValue = pan; repaint(); }
    void setMonoValue (float monoHz) { monoHzValue = monoHz; repaint(); }
    void setMonoSlopeDbPerOct (int slope) { monoSlopeDbPerOct = slope; repaint(); }
    void setSpaceValue (float depth) { spaceValue = depth; repaint(); }
    void setSpaceAlgorithm (int algorithm) { spaceAlgorithm = algorithm; repaint(); }
    void setGreenMode (bool enabled) { isGreenMode = enabled; repaint(); }
    
    // Frequency controls for EQ viz
    void setTiltFreqValue (float f) { tiltFreqValue = f; repaint(); }
    void setScoopFreqValue (float f) { scoopFreqValue = f; repaint(); }
    void setBassFreqValue (float f) { bassFreqValue = f; repaint(); }
    void setAirFreqValue (float f) { airFreqValue = f; repaint(); }
    
    // EQ S/Q shaping and links for visualization
    void setShelfShapeS (float s) { shelfShapeS = juce::jlimit(0.25f, 1.50f, s); repaint(); }
    void setQLink (bool on) { qLink = on; repaint(); }
    void setFilterQ (float q) { filterQGlobal = juce::jlimit(0.5f, 1.2f, q); repaint(); }
    void setHPQ (float q) { hpQ = juce::jlimit(0.5f, 1.2f, q); if (!qLink) repaint(); }
    void setLPQ (float q) { lpQ = juce::jlimit(0.5f, 1.2f, q); if (!qLink) repaint(); }
    void setTiltUseS (bool on) { tiltUsesShelfS = on; repaint(); }

    // Imaging/shuffler overlays state
    void setXoverLoHz (float hz) { xoverLoHz = juce::jlimit(40.0f, 400.0f, hz); repaint(); }
    void setXoverHiHz (float hz) { xoverHiHz = juce::jlimit(800.0f, 6000.0f, hz); if (xoverHiHz <= xoverLoHz) xoverHiHz = juce::jlimit(xoverLoHz + 10.0f, 6000.0f, xoverHiHz); repaint(); }
    void setRotationDeg (float d) { rotationDeg = juce::jlimit(-45.0f, 45.0f, d); repaint(); }
    void setAsymmetry (float a) { asym = juce::jlimit(-1.0f, 1.0f, a); repaint(); }
    
    void pushWaveformSample (double sampleL, double sampleR);
    void setSampleRate (double fs) { vizSampleRate = fs > 0.0 ? fs : 48000.0; }

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override { drag(e); }
    void mouseDrag (const juce::MouseEvent& e) override { drag(e); }
    void mouseUp (const juce::MouseEvent&) override { activeBall = 0; }
    void mouseEnter (const juce::MouseEvent&) override { hoverActive = true; stopTimer(); repaint(); }
    void mouseExit (const juce::MouseEvent&) override { startTimer(hoverOffDelayMs); }
    void timerCallback() override { hoverActive = false; stopTimer(); repaint(); }
    
    void setSnapEnabled (bool shouldSnap) { snapEnabled = shouldSnap; }
    bool getSnapEnabled () const { return snapEnabled; }

private:
    std::pair<float,float> pt { 0.5f, 0.2f };
    float leftPt = 0.5f, rightPt = 0.5f;
    bool isSplitMode = false;
    bool isLinked = true;
    int activeBall = 0;
    bool snapEnabled = false;
    bool hoverActive = false;
    const int hoverOffDelayMs = 160;

    // Visual state mirrors of params
    float gainValue = 0.0f;
    float widthValue = 50.0f;
    float tiltValue = 0.0f;
    float mixValue = 0.5f;
    float driveValue = 0.0f;
    float airValue = 0.0f;
    float bassValue = 0.0f;
    float scoopValue = 0.0f;
    float hpValue = 20.0f;
    float lpValue = 20000.0f;
    float panValue = 0.0f;
    float monoHzValue = 0.0f;
    int monoSlopeDbPerOct = 12;
    float spaceValue = 0.0f;
    int spaceAlgorithm = 0;
    bool isGreenMode = false;
    
    // EQ frequency positions
    float tiltFreqValue = 500.0f;
    float scoopFreqValue = 800.0f;
    float bassFreqValue = 150.0f;
    float airFreqValue = 8000.0f;
    
    // EQ S/Q state
    float shelfShapeS = 0.90f;
    bool qLink = true;
    float filterQGlobal = 0.7071f;
    float hpQ = 0.7071f;
    float lpQ = 0.7071f;
    bool tiltUsesShelfS = true;

    // Imaging/shuffler overlay state
    float xoverLoHz = 150.0f;
    float xoverHiHz = 2000.0f;
    float rotationDeg = 0.0f;
    float asym = 0.0f;
    
    // Waveform buffer
    static constexpr int waveformBufferSize = 512;
    std::array<double, waveformBufferSize> waveformL{}, waveformR{};
    int waveformWriteIndex = 0;
    bool hasWaveformData = false;
    double vizSampleRate = 48000.0;
    
    // internals
    void drag (const juce::MouseEvent& e);
    void drawGrid (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawBalls (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawWaveformBackground (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawEQCurves (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawImagingOverlays (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawParameterLabels (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawSnapGrid (juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawHoverEffects (juce::Graphics& g, juce::Rectangle<float> bounds);
};
