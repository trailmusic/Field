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
    int gridFlavor = 0; // 0=Straight, 1=Dotted, 2=Triplet
    double tempoBpm = 120.0; // host tempo (fallback)
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
    int duckSource = 0; // 0=In (Pre), 1=In (Post), 2=External (SC)
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
        
        // Initialize mode coloration filters neutral
        for (int c = 0; c < 2; ++c) {
            preEmph[c].reset();
            deEmph[c].reset();
            headBump[c].reset();
        }
        hissLP.reset();
        
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
        wowPhase = 0.0;
        flutterPhase = 0.0;
        wowInc = 0.0;
        flutterInc = 0.0;

        // Freeze and feedback smoothing
        freezeRamp.reset(sampleRate, 0.03);
        fbSmoothed.reset(sampleRate, 0.02);
        fbSmoothed.setCurrentAndTargetValue((float)feedbackGain);

        // Look-ahead buffer (initial sizing from default lookMs)
        lookLen = (int)std::ceil(lookMs * sr * 0.001);
        lookBuf.setSize(2, juce::jmax(1, lookLen + 1));
        lookW = 0;

        // Ducking envelope init
        detEnv = 0.0f; duckGL = 1.0f; duckGR = 1.0f;
    }

    void setParameters(const DelayParams& p) 
    {
        params = p;
        
        // Compute delay times
        if (p.sync) {
            // Convert musical division to beats (assuming timeDiv encoded as index into a fixed table)
            // Expectation: timeDiv is an index into the UI division list (same ordering as APVTS delay_time_div)
            const int idx = juce::jlimit (0, 26, p.timeDiv); // 27 entries in the division list
            static const double beatsTable[] = {
                1.0/6.0, 1.0/4.0, 1.0/3.0,  // 1/64T, 1/64, 1/64D
                1.0/3.0, 1.0/2.0, 2.0/3.0,  // 1/32T, 1/32, 1/32D
                1.0/1.5, 1.0/1.0, 2.0/1.5,  // 1/16T, 1/16, 1/16D
                2.0/3.0, 0.5,       1.0,    // 1/8T,  1/8,  1/8D
                4.0/3.0, 1.0,       2.0,    // 1/4T,  1/4,  1/4D
                8.0/3.0, 2.0,       4.0,    // 1/2T,  1/2,  1/2D
                4.0,     8.0/3.0,   16.0/3.0, // 1/1T,  1/1,  1/1D
                16.0/3.0, 8.0,      16.0     // 2/1T,  2/1,  2/1D
            };
            double beats = beatsTable[idx];
            // Apply S/D/T flavor multiplier
            double mul = (p.gridFlavor == 1) ? 1.5 : (p.gridFlavor == 2) ? (2.0/3.0) : 1.0;
            beats *= mul;
            const double sec = (60.0 / juce::jmax (1.0, p.tempoBpm)) * beats;
            currentDelaySamples = sec * sampleRate;
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
        
        // Cap and smooth feedback by mode
        {
            const float fbCap = (p.mode == 0 ? 0.98f : (p.mode == 1 ? 0.97f : 0.995f));
            const float fbTarget = juce::jlimit(0.0f, fbCap, (float)feedbackGain);
            fbSmoothed.setTargetValue(fbTarget);
        }
        
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
        
        // Mode coloration filters setup
        if (p.mode == 1) {
            auto pre  = juce::dsp::IIR::Coefficients<Sample>::makeHighShelf(sampleRate, 2000.0, (Sample)0.7, juce::Decibels::decibelsToGain((Sample)+6.0));
            auto de   = juce::dsp::IIR::Coefficients<Sample>::makeHighShelf(sampleRate, 2000.0, (Sample)0.7, juce::Decibels::decibelsToGain((Sample)-6.0));
            for (int c = 0; c < 2; ++c) { preEmph[c].coefficients = pre; deEmph[c].coefficients = de; }
        } else if (p.mode == 2) {
            auto bump = juce::dsp::IIR::Coefficients<Sample>::makePeakFilter(sampleRate, 80.0, (Sample)0.7, juce::Decibels::decibelsToGain((Sample)+2.5));
            for (int c = 0; c < 2; ++c) { headBump[c].coefficients = bump; }
            hissLP.coefficients = juce::dsp::IIR::Coefficients<Sample>::makeLowPass(sampleRate, 6000.0);
        }
        
        // Update ducker
        ducker.set((float)p.duckThresholdDb, (float)p.duckRatio, 
                   (float)p.duckAttackMs, (float)p.duckReleaseMs, (float)p.duckDepth);
        
        // Update LFO
        lfoInc = (Sample)(2.0 * juce::MathConstants<double>::pi * p.modRateHz / sampleRate);
        wowInc = (Sample)(2.0 * juce::MathConstants<double>::pi * juce::jlimit(0.2, 1.0, p.modRateHz * 0.9) / sampleRate);
        flutterInc = (Sample)(2.0 * juce::MathConstants<double>::pi * 6.0 / sampleRate);

        // Ducking coefficients
        duckThr   = juce::Decibels::decibelsToGain((float)p.duckThresholdDb);
        duckRatio = (float)p.duckRatio;
        duckDepth = juce::jlimit(0.0f, 1.0f, (float)p.duckDepth);
        duckAtk   = std::exp(-1.0f / (float)(sampleRate * p.duckAttackMs * 0.001));
        duckRel   = std::exp(-1.0f / (float)(sampleRate * p.duckReleaseMs * 0.001));

        // Look-ahead buffer resize from parameter
        const int newLook = (int)std::ceil(p.duckLookaheadMs * 0.001 * sampleRate);
        if (newLook != lookLen) {
            lookLen = newLook;
            lookBuf.setSize(2, juce::jmax(1, lookLen + 1), true, false, true);
            lookW = 0;
        }

        // Freeze ramp target
        freezeRamp.setTargetValue(p.freeze ? 1.0f : 0.0f);
    }

    void process(juce::dsp::AudioBlock<Sample> block, float scInL, float scInR) 
    {
        (void)scInR; // Suppress unused parameter warning
        if (!params.enabled) return;
        juce::ScopedNoDenormals noDenormals;

        auto num = (int)block.getNumSamples();
        for (int n = 0; n < num; ++n) {
            Sample inL = block.getSample(0, n);
            Sample inR = block.getNumChannels() > 1 ? block.getSample(1, n) : inL;

            // Freeze: gate input gradually and smooth feedback
            const float fz = freezeRamp.getNextValue();
            const Sample inGain = (Sample)(1.0f - fz);
            feedbackGain = (double)fbSmoothed.getNextValue();

            // Compute modulated delay samples for L/R
            Sample tBaseSamp = (Sample)currentDelaySamples;
            Sample tL = tBaseSamp * (Sample)(1.0 - spread);
            Sample tR = tBaseSamp * (Sample)(1.0 + spread);
            
            // Add LFO modulation (mode-aware)
            Sample depthSamp = (Sample)(params.modDepthMs * 0.001 * sampleRate);
            Sample modOffset = 0;
            if (params.mode == 0) {
                modOffset = std::sin(lfoPhase) * depthSamp;
                lfoPhase += lfoInc;
            } else if (params.mode == 1) {
                lfoPhase += lfoInc + (Sample)(1e-4 * ((double)std::rand() / (double)RAND_MAX - 0.5));
                modOffset = std::sin(lfoPhase) * depthSamp;
            } else {
                wowPhase += wowInc; flutterPhase += flutterInc;
                modOffset = (Sample)(0.9 * std::sin(wowPhase) + 0.1 * std::sin(flutterPhase)) * depthSamp;
            }
            tL += modOffset;
            tR += modOffset;
            
            // Add jitter
            if (params.jitterPct > 0.0) {
                Sample jitter = (Sample)((rand() / (Sample)RAND_MAX - 0.5) * 2.0 * params.jitterPct * 0.01 * tBaseSamp);
                tL += jitter;
                tR += jitter;
            }
            
            dl[0].setDelaySamples(tL);
            dl[1].setDelaySamples(tR);

            // Write inputs (freeze reduces new input)
            dl[0].push(inL * inGain); 
            dl[1].push(inR * inGain);

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
            
            // Mode coloration and saturation
            switch (params.mode)
            {
                case 0: // Digital
                {
                    loopL = std::tanh(loopL * (Sample)(1.0 + params.sat * 2.0));
                    loopR = std::tanh(loopR * (Sample)(1.0 + params.sat * 2.0));
                } break;
                case 1: // Analog (BBD-ish)
                {
                    loopL = preEmph[0].processSample(loopL);
                    loopR = preEmph[1].processSample(loopR);
                    auto comp = [&] (Sample x) {
                        const Sample k = (Sample)(1.5 + params.sat * 2.0);
                        return (Sample)std::tanh(k * x) * (Sample)0.85;
                    };
                    loopL = comp(loopL); loopR = comp(loopR);
                    loopL = deEmph[0].processSample(loopL);
                    loopR = deEmph[1].processSample(loopR);
                } break;
                case 2: // Tape
                {
                    loopL = headBump[0].processSample(loopL);
                    loopR = headBump[1].processSample(loopR);
                    auto softSat = [&] (Sample x) {
                        const Sample k = (Sample)(1.2 + params.sat * 2.5);
                        return (Sample)juce::dsp::FastMathApproximations::tanh((float)(k * x)) * (Sample)0.9;
                    };
                    loopL = softSat(loopL); loopR = softSat(loopR);
                    float hn = ((float)std::rand() / (float)RAND_MAX - 0.5f) * 2.0f * 1e-4f; // very low hiss
                    hn = hissLP.processSample(hn);
                    loopL += (Sample)hn; loopR += (Sample)(hn * 0.9f);
                } break;
            }
            
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

            // Accumulate feedback (smoothed feedback target already applied)
            fbStateL = dL + (Sample)feedbackGain * loopL;
            fbStateR = dR + (Sample)feedbackGain * loopR;

            Sample wetL = dL, wetR = dR;
            
            // Ducking (look-ahead, select sidechain source: 0=Input, 1=Wet, 2=Both)
            float sc = 0.0f;
            if (params.duckSource == 0) {
                sc = (float)(0.5 * std::abs((double)inL) + 0.5 * std::abs((double)inR));
            } else if (params.duckSource == 1) {
                sc = (float)(0.5 * std::abs((double)wetL) + 0.5 * std::abs((double)wetR));
            } else {
                float inSc  = (float)(0.5 * std::abs((double)inL) + 0.5 * std::abs((double)inR));
                float wetSc = (float)(0.5 * std::abs((double)wetL) + 0.5 * std::abs((double)wetR));
                sc = 0.5f * (inSc + wetSc);
            }
            // Envelope with attack/release
            detEnv = (sc > detEnv) ? (duckAtk * detEnv + (1.0f - duckAtk) * sc)
                                   : (duckRel * detEnv + (1.0f - duckRel) * sc);
            float over = juce::jmax(0.0f, detEnv - duckThr);
            float compG = 1.0f / (1.0f + over * (duckRatio - 1.0f));
            float targetG = (1.0f - duckDepth) + duckDepth * compG;
            const float coefL = (targetG < duckGL) ? (1.0f - duckAtk) : (1.0f - duckRel);
            const float coefR = (targetG < duckGR) ? (1.0f - duckAtk) : (1.0f - duckRel);
            duckGL += (targetG - duckGL) * coefL;
            duckGR += (targetG - duckGR) * coefR;
            
            // Look-ahead delay of wet before applying gain
            auto* wL = lookBuf.getWritePointer(0);
            auto* wR = lookBuf.getWritePointer(1);
            wL[lookW] = (float)wetL; wR[lookW] = (float)wetR;
            int r = lookW - lookLen; if (r < 0) r += lookBuf.getNumSamples();
            auto* rL = lookBuf.getReadPointer(0);
            auto* rR = lookBuf.getReadPointer(1);
            float wetLd = rL[r]; float wetRd = rR[r];
            if (++lookW >= lookBuf.getNumSamples()) lookW = 0;
            if (duckPost) {
                wetL = (Sample)(wetLd * duckGL);
                wetR = (Sample)(wetRd * duckGR);
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
    juce::dsp::IIR::Filter<Sample> preEmph[2], deEmph[2], headBump[2];
    juce::dsp::IIR::Filter<Sample> hissLP;
    
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
    Sample wowPhase = 0.0;
    Sample flutterPhase = 0.0;
    Sample wowInc = 0.0;
    Sample flutterInc = 0.0;
    
    // Diffusion
    double diffuseSizeMs = 18.0;
    Sample diffuseG = (Sample)0.7;
    double lookMs = 5.0;
    
    // Freeze/feedback smoothing and look-ahead buffer for ducking
    juce::LinearSmoothedValue<float> freezeRamp, fbSmoothed;
    juce::AudioBuffer<float> lookBuf; int lookW = 0; int lookLen = 0;
    
    // Ducking envelope/state
    float detEnv = 0.0f; 
    float duckAtk = 0.9f, duckRel = 0.9f, duckThr = 0.5f, duckRatio = 2.0f, duckDepth = 0.6f;
    float duckGL = 1.0f, duckGR = 1.0f;
    
    // Parameters
    DelayParams params;
};
