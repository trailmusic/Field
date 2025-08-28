#pragma once

#include <JuceHeader.h>

// ===============================
// Delay DSP Engine Components
// ===============================

// Cubic Lagrange interpolation for fractional delay
struct CubicLagrange 
{
    static inline void coeffs(double u, double& a0, double& a1, double& a2, double& a3) 
    {
        // 3rd-order Lagrange around x1
        const double um1 = u - 1.0, up1 = u + 1.0;
        a0 = -(u * um1 * (u - 2.0)) / 6.0;
        a1 = (up1 * um1 * (u - 2.0)) / 2.0;
        a2 = -(up1 * u * (u - 2.0)) / 2.0;
        a3 = (up1 * u * um1) / 6.0;
    }
};

// Delay line with cubic interpolation and dual-reader crossfade
template<typename T>
struct DelayLine 
{
    void prepare(double sampleRate, double maxSeconds) 
    {
        sr = sampleRate;
        const size_t N = (size_t)std::ceil(maxSeconds * sr) + 8; // guard
        buffer.assign(N, T{});
        write = 0; 
        size = N;
        active.read = 0; 
        target.read = 0; 
        xfadeSamples = 0; 
        xfadePos = 0;
    }

    inline void push(T x) 
    {
        buffer[write] = x;
        write = (write + 1) % size;
    }

    // time in samples; glitchless set
    void setDelaySamples(double delaySamp) 
    {
        delaySamp = juce::jlimit(1.0, (double)size - 4.0, delaySamp);
        const double newRead = wrap(write - delaySamp);
        const double delta = std::abs(newRead - active.read);
        if (delta > 32.0) { // big jump -> crossfade readers
            target.read = newRead;
            xfadeSamples = (int)std::round(0.02 * sr); // 20 ms
            xfadePos = 0;
        } else {
            active.read = newRead;
        }
    }

    inline T read() 
    {
        auto sampleAt = [&](double pos) -> T {
            int i1 = (int)std::floor(pos);
            double u = pos - (double)i1;
            int i0 = wrapi(i1 - 1), i2 = wrapi(i1 + 1), i3 = wrapi(i1 + 2);
            double a0, a1, a2, a3; 
            CubicLagrange::coeffs(u, a0, a1, a2, a3);
            return (T)(a0 * buffer[i0] + a1 * buffer[i1] + a2 * buffer[i2] + a3 * buffer[i3]);
        };
        
        if (xfadeSamples > 0) {
            double t = (double)xfadePos / (double)xfadeSamples;
            T a = sampleAt(active.read);
            T b = sampleAt(target.read);
            T y = (T)((1.0 - t) * a + t * b);
            stepReader(active); 
            stepReader(target);
            if (++xfadePos >= xfadeSamples) { 
                active = target; 
                xfadeSamples = 0; 
            }
            return y;
        } else {
            T y = sampleAt(active.read);
            stepReader(active);
            return y;
        }
    }

private:
    struct Reader { double read = 0.0; } active, target;
    std::vector<T> buffer;
    size_t size = 0; 
    size_t write = 0;
    int xfadeSamples = 0, xfadePos = 0;
    double sr = 48000.0;

    inline int wrapi(int i) const { 
        i %= (int)size; 
        return i < 0 ? i + (int)size : i; 
    }
    
    inline double wrap(double x) const {
        while (x < 0.0) x += (double)size;
        while (x >= (double)size) x -= (double)size;
        return x;
    }
    
    inline void stepReader(Reader& r) { 
        r.read = wrap(r.read + 1.0); 
    }
};

// All-pass diffuser for delay feedback loop
template<typename T>
struct Allpass 
{
    void prepare(double sampleRate, double ms) 
    {
        const int d = (int)std::max<int>(1, std::round(ms * 0.001 * sampleRate));
        buf.assign((size_t)d, T{}); 
        idx = 0;
    }
    
    void setG(T g_) { 
        g = juce::jlimit((T)-0.95f, (T)0.95f, g_); 
    }
    
    inline T process(T x) 
    {
        T y = -g * x + z;
        T in = x + g * y;
        z = pushpop(in);
        return y;
    }

private:
    std::vector<T> buf; 
    size_t idx = 0; 
    T z = 0, g = (T)0.7;
    
    inline T pushpop(T v) { 
        auto& s = buf[idx]; 
        T r = s; 
        s = v; 
        idx = (idx + 1) % buf.size(); 
        return r; 
    }
};

// Per-delay ducker with sidechain and lookahead
struct Ducker 
{
    void prepare(double sr, double lookaheadMs) 
    {
        sampleRate = sr;
        int la = (int)std::round(lookaheadMs * 0.001 * sr);
        ring.assign((size_t)std::max(1, la), 0.0f);
        w = 0; 
        env = 0.f; 
        g = 1.f;
    }
    
    void set(float thrDb, float ratio, float attMs, float relMs, float depth) 
    {
        thr = juce::Decibels::decibelsToGain(thrDb);
        R = juce::jmax(1.f, ratio);
        a = std::exp(-1.0f / (attMs * 0.001f * (float)sampleRate));
        r = std::exp(-1.0f / (relMs * 0.001f * (float)sampleRate));
        mix = juce::jlimit(0.f, 1.f, depth);
    }
    
    inline float process(float x, float sc) 
    {
        // lookahead read
        float scLa = pushpop(sc);
        float lev = std::sqrt(scLa * scLa); // peak-ish
        env = lev > env ? a * env + (1 - a) * lev
                       : r * env + (1 - r) * lev;
        float over = juce::jmax(0.f, env - thr);
        float comp = 1.0f / (1.0f + over * (R - 1.0f)); // soft-ish
        float gd = (1.0f - mix) + mix * comp; // depth blend
        return x * gd;
    }

private:
    std::vector<float> ring; 
    size_t w = 0; 
    double sampleRate = 48000.0;
    float env = 0, g = 1, thr = 0.5f, R = 2.f, a = 0.9f, r = 0.9f, mix = 0.6f;
    
    inline float pushpop(float v) { 
        float result = ring[w]; 
        ring[w] = v; 
        w = (w + 1) % ring.size(); 
        return result; 
    }
};

// ===============================
// Delay Parameters Structure
// ===============================

struct DelayParams 
{
    // Core
    bool enabled = false;
    int mode = 0; // 0=Digital, 1=Analog, 2=Tape
    bool sync = true;
    double timeMs = 350.0;
    int timeDiv = 4; // 1/4 note default
    double feedbackPct = 36.0;
    double wet = 0.25;
    bool killDry = false;
    bool freeze = false;
    
    // Stereo & Image
    bool pingpong = false;
    double crossfeedPct = 35.0;
    double stereoSpreadPct = 25.0;
    double width = 1.0;
    
    // Modulation
    double modRateHz = 0.35;
    double modDepthMs = 3.0;
    double wowflutter = 0.25;
    double jitterPct = 2.0;
    
    // Tone / Color
    double hpHz = 120.0;
    double lpHz = 12000.0;
    double tiltDb = 0.0;
    double sat = 0.2;
    double diffusion = 0.0;
    double diffuseSizeMs = 18.0;
    
    // Ducking
    int duckSource = 0; // 0=Input, 1=Wet, 2=Both
    bool duckPost = true;
    double duckDepth = 0.6;
    double duckAttackMs = 12.0;
    double duckReleaseMs = 220.0;
    double duckThresholdDb = -26.0;
    double duckRatio = 2.0;
    double duckLookaheadMs = 5.0;
    bool duckLinkGlobal = true;
};

// ===============================
// Main Delay Engine
// ===============================

template<typename Sample>
struct DelayEngine 
{
    void prepare(double sr, int maxCh = 2) 
    {
        (void)maxCh; // Suppress unused parameter warning
        sampleRate = sr;
        for (int c = 0; c < 2; ++c) { 
            dl[c].prepare(sr, 4.0); 
        }
        
        // Prepare all-pass diffuser chain
        for (auto& ap : apChain) { 
            ap.prepare(sr, diffuseSizeMs); 
            ap.setG(diffuseG); 
        }
        
        ducker.prepare(sr, lookMs);
        
        // Prepare filters
        hpFilter.prepare({ sr, 512, 2 });
        lpFilter.prepare({ sr, 512, 2 });
        hpFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        lpFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        
        // Initialize state
        fbStateL = 0.0f;
        fbStateR = 0.0f;
        currentDelaySamples = 4800; // ~100ms default
        spread = 0.25;
        width = 1.0;
        feedbackGain = 0.36;
        wet = 0.25;
        pingpong = false;
        killDry = false;
        duckPost = true;
        crossfeed = 0.35f;
        
        // LFO for modulation
        lfoPhase = 0.0;
        lfoInc = 0.0;
    }

    void setParameters(const DelayParams& p) 
    {
        params = p;
        
        // Compute delay times
        if (p.sync) {
            // TODO: Implement tempo sync
            currentDelaySamples = p.timeMs * 0.001 * sampleRate;
        } else {
            currentDelaySamples = p.timeMs * 0.001 * sampleRate;
        }
        
        // Update derived parameters
        spread = p.stereoSpreadPct * 0.01;
        width = p.width;
        feedbackGain = p.feedbackPct * 0.01;
        wet = p.wet;
        pingpong = p.pingpong;
        killDry = p.killDry;
        duckPost = p.duckPost;
        crossfeed = p.crossfeedPct * 0.01;
        
        // Update filters
        hpFilter.setCutoffFrequency((Sample)p.hpHz);
        lpFilter.setCutoffFrequency((Sample)p.lpHz);
        
        // Update diffuser
        diffuseSizeMs = p.diffuseSizeMs;
        diffuseG = (Sample)p.diffusion;
        for (auto& ap : apChain) {
            ap.prepare(sampleRate, diffuseSizeMs);
            ap.setG(diffuseG);
        }
        
        // Update ducker
        ducker.set((float)p.duckThresholdDb, (float)p.duckRatio, 
                   (float)p.duckAttackMs, (float)p.duckReleaseMs, (float)p.duckDepth);
        
        // Update LFO
        lfoInc = (Sample)(2.0 * juce::MathConstants<double>::pi * p.modRateHz / sampleRate);
    }

    void process(juce::dsp::AudioBlock<Sample> block, float scInL, float scInR) 
    {
        (void)scInR; // Suppress unused parameter warning
        if (!params.enabled) return;
        
        auto num = (int)block.getNumSamples();
        for (int n = 0; n < num; ++n) {
            Sample inL = block.getSample(0, n);
            Sample inR = block.getNumChannels() > 1 ? block.getSample(1, n) : inL;

            // Sidechain (choose input/wet/both outside and pass here)
            Sample sc = scInL; // TODO: implement proper sidechain mixing

            // Compute modulated delay samples for L/R
            Sample tBaseSamp = (Sample)currentDelaySamples;
            Sample tL = tBaseSamp * (Sample)(1.0 - spread);
            Sample tR = tBaseSamp * (Sample)(1.0 + spread);
            
            // Add LFO modulation
            Sample modOffset = std::sin(lfoPhase) * (Sample)(params.modDepthMs * 0.001 * sampleRate);
            tL += modOffset;
            tR += modOffset;
            lfoPhase += lfoInc;
            
            // Add jitter
            if (params.jitterPct > 0.0) {
                Sample jitter = (Sample)((rand() / (Sample)RAND_MAX - 0.5) * 2.0 * params.jitterPct * 0.01 * tBaseSamp);
                tL += jitter;
                tR += jitter;
            }
            
            dl[0].setDelaySamples(tL);
            dl[1].setDelaySamples(tR);

            // Write inputs
            dl[0].push(inL); 
            dl[1].push(inR);

            Sample dL = dl[0].read();
            Sample dR = dl[1].read();

            // Feedback tap (last output cached)
            Sample fbL = fbStateL, fbR = fbStateR;

            // Crossfeed/pingpong
            Sample xf = (Sample)crossfeed;
            Sample ppL = pingpong ? fbR : fbL;
            Sample ppR = pingpong ? fbL : fbR;

            // Loop tone processing
            Sample loopL = ppL;
            Sample loopR = ppR;
            
            // Apply filters
            loopL = hpFilter.processSample(0, loopL);
            loopL = lpFilter.processSample(0, loopL);
            loopR = hpFilter.processSample(1, loopR);
            loopR = lpFilter.processSample(1, loopR);
            
            // Apply saturation (simple soft clip)
            loopL = std::tanh(loopL * (Sample)(1.0 + params.sat * 2.0));
            loopR = std::tanh(loopR * (Sample)(1.0 + params.sat * 2.0));
            
            // Apply diffusion
            if (params.diffusion > 0.0) {
                for (auto& ap : apChain) {
                    loopL = ap.process(loopL);
                    loopR = ap.process(loopR);
                }
            }
            
            // Crossfeed
            Sample tempL = loopL;
            loopL += xf * loopR;
            loopR += xf * tempL;

            // Accumulate feedback
            fbStateL = dL + (Sample)feedbackGain * loopL;
            fbStateR = dR + (Sample)feedbackGain * loopR;

            Sample wetL = dL, wetR = dR;
            
            // Ducking (post or pre)
            if (duckPost) { 
                wetL = ducker.process((float)wetL, (float)sc); 
                wetR = ducker.process((float)wetR, (float)sc); 
            }

            // Width processing
            Sample M = (Sample)0.5 * (wetL + wetR);
            Sample S = (Sample)0.5 * (wetL - wetR);
            S *= (Sample)width;
            wetL = M + S; 
            wetR = M - S;

            // Output mix
            Sample outL = killDry ? (Sample)0.0 : inL;
            Sample outR = killDry ? (Sample)0.0 : inR;
            outL += (Sample)wet * wetL; 
            outR += (Sample)wet * wetR;

            block.setSample(0, n, outL);
            if (block.getNumChannels() > 1) 
                block.setSample(1, n, outR);
        }
    }

private:
    DelayLine<Sample> dl[2];
    std::array<Allpass<Sample>, 4> apChain; // 4 all-pass diffusers
    Ducker ducker;
    
    // Filters
    juce::dsp::StateVariableTPTFilter<Sample> hpFilter, lpFilter;
    
    // State
    double sampleRate = 48000.0;
    double currentDelaySamples = 4800;
    double spread = 0.25;
    double width = 1.0;
    double feedbackGain = 0.36;
    double wet = 0.25;
    bool pingpong = false;
    bool killDry = false;
    bool duckPost = true;
    float crossfeed = 0.35f;
    Sample fbStateL = 0, fbStateR = 0;
    
    // Modulation
    Sample lfoPhase = 0.0;
    Sample lfoInc = 0.0;
    
    // Diffusion
    double diffuseSizeMs = 18.0;
    Sample diffuseG = (Sample)0.7;
    double lookMs = 5.0;
    
    // Parameters
    DelayParams params;
};
