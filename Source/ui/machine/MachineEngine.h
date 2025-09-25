#pragma once
#include <JuceHeader.h>
#include "../../PluginProcessor.h"

struct ParamDelta {
    const char* id; float current{}, target{}, lo{}, hi{}; float weight{}; juce::String reason;
};

struct Proposal {
    juce::String id;           // "imaging", "tone", "space"
    juce::String title;        // card title
    juce::String summary;      // 1–2 lines
    std::vector<ParamDelta> params;           // concrete targets
    juce::NamedValueSet metrics;              // small viz payload for the card
};

class MachineEngine : private juce::Timer
{
public:
    enum State { Idle, Learning, Ready };

    ~MachineEngine() { stopTimer(); }
    
    MachineEngine (MyPluginAudioProcessor& procRef, juce::AudioProcessorValueTreeState& stateRef);

    void setSampleRate (double sr) { sampleRate = sr; }
    // Compatibility: UI may still push blocks; safe to ignore (engine pulls from vis buses)
    void push (const float*, const float*, int) {}

    void startLearn (bool usePre, double seconds = 60.0);
    void stopLearn (bool finalize = true);

    bool   isLearning() const { return state == Learning; }
    double getRemainingSeconds() const;
    State  getState() const { return state; }

    std::vector<Proposal> getProposals() const;

    void applyProposal (const Proposal& p, bool previewOnly);
    void applyProposal (const Proposal& p, float amount01, bool previewOnly);
    void applyComposite (const std::vector<Proposal>& active, float amount01, bool previewOnly);
    void revertPreview();

    // UI status reporting during learning
    juce::String getStatus() const;
    juce::String getAnalysisInfo() const;
    juce::StringArray getMessages() const;

    // optional helper (UI may want to toggle source)
    void setUsePre (bool b) { usePreBus = b; }
    // Context
    struct Context { int genre{2}; int venue{0}; int trackType{6}; };
    struct Targets {
        float corrLowMin{}, corrFullTarget{}, widthLoTarget{}, widthMidTarget{}, widthHiTarget{};
        float slopeTarget{}, lfRumbleMaxDb{}, hfFizzMaxDb{}, sibilanceMax{};
        float depthLo{}, depthHi{}; int spacePref0{}, spacePref1{}, spacePref2{};
        float duckDepthDefault{}, duckThrDb{}, duckRatio{}, duckAtkMs{}, duckRelMs{};
        float sideTiltDbOct{}, tiltPivotHz{}, widthMax{};
    };
    void setContext (int genreIndex, int venueIndex, int trackTypeIndex = 6) { ctx.genre = genreIndex; ctx.venue = venueIndex; ctx.trackType = trackTypeIndex; }

private:
    void timerCallback() override;
    void setStatus (const juce::String& s);

    // --- data sources
    MyPluginAudioProcessor& proc;
    juce::AudioProcessorValueTreeState& apvts;
    bool usePreBus = false;

    // --- time/learn state
    State  state = Idle;
    double tStart = 0.0, tDuration = 60.0;

    // --- running feature aggregates
    struct Agg {
        // loudness / energy
        double sumPowL=0, sumPowR=0, sumPow=0; int samples=0;
        // peak
        float peakL=0, peakR=0;
        // crest factor (per channel)
        double sumAbsL=0, sumAbsR=0;
        // stereo correlation (fullband)
        long double sLL=0, sRR=0, sLR=0;
        // banded analysis (log bands)
        int bands=0;
        std::vector<double> bandE_L, bandE_R, bandSLL, bandSRR, bandSLR; // accumulators
        std::vector<double> bandCount;
        // tonal regression (slope dB/oct)
        double regN=0, regSumX=0, regSumY=0, regSumXX=0, regSumXY=0;
        // transientness (spectral flux proxy)
        double fluxSum=0; int fluxFrames=0;
    } A;

    // fft / window (re-use Analyzer sizes: 2048 fft, 50% hop)
    int fftOrder = 11, fftSize = 1<<11, hop = (1<<11)/2;
    juce::dsp::FFT fft{fftOrder};
    std::unique_ptr<juce::dsp::WindowingFunction<float>> win;
    juce::HeapBlock<float> td;      // time
    juce::HeapBlock<float> fd;      // freq (2*fftSize) mono
    // Stereo analysis buffers (for band L/R power and cross)
    juce::HeapBlock<float> tdL, tdR;
    juce::HeapBlock<float> fdL, fdR; // freq L/R (2*fftSize each)
    std::vector<float> prevMag;     // for flux
    std::vector<std::pair<int,int>> bandBinRanges; // [k0,k1] per band
    double sampleRate = 48000.0;

    // proposals (final results)
    std::vector<Proposal> proposals;

    // preview baseline (to revert)
    std::vector<std::pair<const char*, float>> baselineParams;

    // helpers
    void prepareOnce();
    void pullAudioAndStep();
    void stftAndAccumulate (const float* L, const float* R, int n);
    void finalizeAndBuildProposals();
    void captureBaseline();
    void setParam (const char* id, float target, bool notifyHost=true);

    // status
    juce::String status;
    Context ctx;
    Targets tgt;
    juce::StringArray messageLog;
    juce::String lastLoggedStatus;
    mutable juce::CriticalSection msgLock;
};

