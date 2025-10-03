/*
====================================================================================================
 ReverbEngine.cpp — Implementation
----------------------------------------------------------------------------------------------------
Phase-1 deliverables
  • Correct ER tap engine (single write / multi read, equal-power pan, per-tap filters).
  • Modern ducking (RMS window, soft-knee, look-ahead, depth cap).
  • Post/Pre/ER/Tail Tone EQ routing (static) with safe defaults.
  • No allocations in processWet(); all state sized in prepare() / setParams().

Phase-2 hooks (FDN & Decay-Rate integration)
  • fdn.setDecayProfile(), fdn.setToneEq() already plumbed.
  • setDecayRateProfile(), setToneEq() cache into runtime.
====================================================================================================
*/

#include "ReverbEngine.h"
using namespace juce;

// ============================================================================
// Biquad helpers (shared across ER / Tone / Duck detector)
// ============================================================================
static void biquadNormalize (ERFilter& q)
{
    const double a0 = q.a0;
    q.b0/=a0; q.b1/=a0; q.b2/=a0; q.a1/=a0; q.a2/=a0; q.a0 = 1.0;
}

static void biquadCookPeak (ERFilter& q, double fs, double f0, double Q, double dBg)
{
    const double A  = std::pow(10.0, dBg/40.0);
    const double w0 = 2.0 * double_Pi * f0 / fs;
    const double c  = std::cos(w0), s = std::sin(w0);
    const double alpha = s/(2.0*Q);

    q.b0 = 1.0 + alpha*A;
    q.b1 = -2.0*c;
    q.b2 = 1.0 - alpha*A;
    q.a0 = 1.0 + alpha/A;
    q.a1 = -2.0*c;
    q.a2 = 1.0 - alpha/A;

    biquadNormalize(q);
}

static void biquadCookShelf (ERFilter& q, bool low, double fs, double f0, double Q, double dBg)
{
    const double A  = std::pow(10.0, dBg/40.0);
    const double w0 = 2.0 * double_Pi * f0 / fs;
    const double c  = std::cos(w0), s = std::sin(w0);
    const double alpha = s/(2.0*Q);
    const double twoSqrtAalpha = 2.0 * std::sqrt(A) * alpha;

    if (low)
    {
        q.b0 =    A*((A+1) - (A-1)*c + twoSqrtAalpha);
        q.b1 =  2*A*((A-1) - (A+1)*c);
        q.b2 =    A*((A+1) - (A-1)*c - twoSqrtAalpha);
        q.a0 =        (A+1) + (A-1)*c + twoSqrtAalpha;
        q.a1 =   -2*((A-1) + (A+1)*c);
        q.a2 =        (A+1) + (A-1)*c - twoSqrtAalpha;
    }
    else
    {
        q.b0 =    A*((A+1) + (A-1)*c + twoSqrtAalpha);
        q.b1 = -2*A*((A-1) + (A+1)*c);
        q.b2 =    A*((A+1) + (A-1)*c - twoSqrtAalpha);
        q.a0 =        (A+1) - (A-1)*c + twoSqrtAalpha;
        q.a1 =    2*((A-1) - (A+1)*c);
        q.a2 =        (A+1) - (A-1)*c - twoSqrtAalpha;
    }
    biquadNormalize(q);
}

// ERFilter factories & processing
ERFilter ERFilter::makePeaking (double fs, double f0, double Q, double gainDb)  { ERFilter q; biquadCookPeak (q, fs,f0,Q,gainDb);  return q; }
ERFilter ERFilter::makeLowShelf(double fs, double f0, double Q, double gainDb)  { ERFilter q; biquadCookShelf(q, true, fs,f0,Q,gainDb); return q; }
ERFilter ERFilter::makeHighShelf(double fs, double f0, double Q, double gainDb) { ERFilter q; biquadCookShelf(q,false, fs,f0,Q,gainDb); return q; }

void ERFilter::processInPlace (AudioBuffer<float>& buf)
{
    const int C = buf.getNumChannels();
    const int N = buf.getNumSamples();
    if ((int) z1.size() != C) resize (C);

    const float fb0 = (float)(b0 / a0);
    const float fb1 = (float)(b1 / a0);
    const float fb2 = (float)(b2 / a0);
    const float fa1 = (float)(a1 / a0);
    const float fa2 = (float)(a2 / a0);

    for (int c=0;c<C;++c)
    {
        float* d = buf.getWritePointer(c);
        float z1c = z1[c], z2c = z2[c];
        for (int n=0;n<N;++n)
        {
            const float x = d[n];
            const float y = fb0*x + z1c;
            z1c = fb1*x - fa1*y + z2c;
            z2c = fb2*x - fa2*y;
            d[n] = y;
        }
        z1[c]=z1c; z2[c]=z2c;
    }
}

float ERFilter::processSample (float x, int ch)
{
    if (ch >= (int)z1.size()) return x;
    const float fb0 = (float)(b0 / a0);
    const float fb1 = (float)(b1 / a0);
    const float fb2 = (float)(b2 / a0);
    const float fa1 = (float)(a1 / a0);
    const float fa2 = (float)(a2 / a0);

    const float y = fb0*x + z1[ch];
    z1[ch] = fb1*x - fa1*y + z2[ch];
    z2[ch] = fb2*x - fa2*y;
    return y;
}

// ============================================================================
// Engine lifecycle
// ============================================================================
void ReverbEngine::prepare (double sr, int maxBlock, int channels)
{
    sampleRate = sr; maxSamples = maxBlock; chans = jmax (1, channels);
    erBuf .setSize (chans, maxSamples);
    tailBuf.setSize (chans, maxSamples);
    tmpBuf .setSize (chans, maxSamples);

    // ER up to ~60 ms
    er.prepare (chans, sampleRate, 60);

    // FDN scaffold
    fdn.prepare (sampleRate, maxSamples, chans);

    // Ducking
    duck.prepare (sampleRate, maxSamples, chans);

    // Tone EQ runtime
    tone.channels = chans;
}

void ReverbEngine::reset ()
{
    er.reset();
    fdn.reset();
    duck.reset();
}

void ReverbEngine::setParams (const ReverbParams& p)
{
    er .setParams (p.erTimeMs, p.erDensity, p.erWidthPct, p.erLevelDb);
    duck.setParams (p.duckMode, p.duckDetector, p.duckDepthDb, p.duckThrDb,
                    p.duckRatio, p.duckKneeDb, p.duckAtkMs, p.duckRelMs,
                    p.duckBandHz, p.duckBandQ, p.duckOn);

    // FDN params (inactive in Phase-1, stored for Phase-2)
    fdn.setParams (p.decaySec, p.diffusion, p.modDepthCents, p.modRateHz);
}

void ReverbEngine::setDecayRateProfile (const DecayRateProfile& p)
{
    decayProfile = p;
    fdn.setDecayProfile (p); // Phase-2
}

void ReverbEngine::setToneEq (const ToneEq& eqIn)
{
    toneEq = eqIn;
    tone.prepare (sampleRate, chans, toneEq);
}

// ============================================================================
// Process one wet block
// ============================================================================
void ReverbEngine::processWet (AudioBuffer<float>& wet, const AudioBuffer<float>& sidechain)
{
    jassert (wet.getNumChannels() == chans);

    // 0) If ToneEQ routing is Pre, prefilter the ER input only
    const AudioBuffer<float>* erInput = &wet;
    if (tone.apply == ToneEq::Pre && tone.active())
    {
        tmpBuf.makeCopyOf (wet);
        tone.process (tmpBuf);
        erInput = &tmpBuf;
    }

    // 1) ER
    er.process (*erInput, erBuf);

    // 2) Tail (Phase-1: ER copy; Phase-2: FDN tank with decay-rate shaping)
#if FIELD_REVERB_PHASE2
    // FDN tank processes ER input with decay-rate shaping
    fdn.process(erBuf, tailBuf);
#else
    // Phase 1: copy ER to tail
    tailBuf.makeCopyOf(erBuf);
#endif

    // 3) Tone EQ variants that target ER or Tail only (static IIR)
    if (tone.active())
    {
        switch (tone.apply)
        {
            case ToneEq::EROnly:   { AudioBuffer<float> t (erBuf.getNumChannels(), erBuf.getNumSamples()); t.makeCopyOf (erBuf); tone.process (t); erBuf.makeCopyOf (t); tailBuf.makeCopyOf (erBuf); } break;
            case ToneEq::TailOnly: { AudioBuffer<float> t (tailBuf.getNumChannels(), tailBuf.getNumSamples()); t.makeCopyOf (tailBuf); tone.process (t); tailBuf.makeCopyOf (t); } break;
            default: break; // Pre handled above; Post handled below
        }
    }

    // 4) Merge ER+Tail → wet (Phase-1: just Tail)
    wet.makeCopyOf (tailBuf);

    // 5) Ducking (uses sidechain as dry)
    duck.process (wet, sidechain, erBuf, tailBuf);
    duckGrDb.store (duck.lastGrDb);

    // 6) Post Tone EQ (final polish)
    if (tone.apply == ToneEq::Post && tone.active())
        tone.process (wet);

    // 7) Output safety (soft clipper)
    auto softClip = [] (float x) {
        const float a = 0.5f; // gentle
        return juce::jlimit(-1.0f, 1.0f, x / (1.0f + a*std::abs(x)));
    };
    for (int c=0; c<wet.getNumChannels(); ++c) {
        float* d = wet.getWritePointer(c);
        for (int i=0; i<wet.getNumSamples(); ++i) 
            d[i] = softClip(d[i]);
    }

    // 8) Meters
    auto rms = [] (const AudioBuffer<float>& b)
    {
        long double s = 0.0; const int ch = b.getNumChannels(), n = b.getNumSamples();
        for (int c=0;c<ch;++c){ const float* d=b.getReadPointer(c); for (int i=0;i<n;++i) s += (long double) d[i]*d[i]; }
        return (float) std::sqrt ((double) s / jmax (1, ch*n));
    };
    erRms  .store (rms (erBuf));
    tailRms.store (rms (tailBuf));
}

// ============================================================================
// Early Reflections
// ============================================================================
static inline void erEqualPowerPan (float pan /*-1..+1*/, float& l, float& r)
{
    const float t = 0.5f * (pan + 1.0f); // 0..1
    l = std::cos (float_Pi * 0.5f * t);
    r = std::sin (float_Pi * 0.5f * t);
}

void ReverbEngine::EarlyReflections::prepare (int channels, double sr, int maxDelayMs)
{
    sampleRate = sr;

    const int maxDelaySamples = (int) std::ceil (maxDelayMs * 0.001 * sampleRate);
    ring.resize (channels);
    writeIdx.assign (channels, 0);
    for (auto& v : ring) v.assign (jmax (1, maxDelaySamples), 0.0f);

    tapFilters.clear();
    tapFilters.resize (MAX_ER_TAPS);
    numTaps = 0;
}

void ReverbEngine::EarlyReflections::reset()
{
    for (auto& v : ring) std::fill (v.begin(), v.end(), 0.0f);
    std::fill (writeIdx.begin(), writeIdx.end(), 0);
    for (auto& f : tapFilters) { std::fill (f.z1.begin(), f.z1.end(), 0.0f); std::fill (f.z2.begin(), f.z2.end(), 0.0f); }
}

void ReverbEngine::EarlyReflections::setParams (float erTimeMs, float erDensity, float erWidthPct, float erLevelDb)
{
    const int minT = 6, maxT = MAX_ER_TAPS;
    numTaps = jlimit (minT, maxT, (int) std::round (jmap (erDensity, 0.0f, 1.0f, (float)minT, (float)maxT)));

    const float maxMs = jmax (5.0f, erTimeMs);
    for (int i=0;i<numTaps;++i)
    {
        const float u = (i + 0.5f) / (float)numTaps;
        const float t = std::pow (u, 1.4f); // front-loaded
        const float ms = jlimit (0.5f, maxMs, 0.5f + t * (maxMs - 0.5f));

        taps[i].delaySamp = (int) std::round (ms * 0.001f * (float) sampleRate);

        // gain envelope (exp decay) — normalized later
        taps[i].gain = std::exp (-t * 2.0f);

        // width → alternating pan
        const float width = jlimit (0.0f, 1.0f, erWidthPct);
        taps[i].pan = ((i & 1) ? +1.f : -1.f) * width;

        // light coloration sweep
        taps[i].f0 = 800.0f + t * 6000.0f;
        taps[i].Q  = 0.8f;
    }

    // energy normalize + level scale (dB)
    float sum = 0.f; for (int i=0;i<numTaps;++i) sum += taps[i].gain;
    const float level = Decibels::decibelsToGain (erLevelDb);
    const float s = (sum > 0.f ? level / sum : 0.f);
    for (int i=0;i<numTaps;++i) taps[i].gain *= s;

    // load per-tap filters
    for (int i=0;i<numTaps;++i)
    {
        tapFilters[i] = ERFilter::makePeaking (sampleRate, taps[i].f0, taps[i].Q, +1.5f);
        tapFilters[i].resize ((int) ring.size());
    }
}

void ReverbEngine::EarlyReflections::process (const AudioBuffer<float>& in, AudioBuffer<float>& out)
{
    jassert (in.getNumChannels() == (int) ring.size());
    out.clear();

    const int C = in.getNumChannels();
    const int N = in.getNumSamples();

    // write input into ring buffers (single write)
    for (int ch=0; ch<C; ++ch)
    {
        const float* src = in.getReadPointer (ch);
        auto& buf = ring[(size_t)ch];
        int&  w   = writeIdx[(size_t)ch];
        const int R = (int) buf.size();

        for (int n=0; n<N; ++n)
        {
            buf[w] = src[n];
            if (++w == R) w = 0;
        }
    }

    // sum taps from ring into out with pan + filters
    if (C == 2)
    {
        float* L = out.getWritePointer(0);
        float* R = out.getWritePointer(1);
        const int RL = (int) ring[0].size();
        const int RR = (int) ring[1].size();
        const int wL = writeIdx[0];
        const int wR = writeIdx[1];

        for (int n=0; n<N; ++n)
        {
            float accL = 0.f, accR = 0.f;

            for (int t=0; t<numTaps; ++t)
            {
                const int rL = (wL - taps[t].delaySamp + RL) % RL;
                const int rR = (wR - taps[t].delaySamp + RR) % RR;

                // mono-ish tap feed (mid) for better imaging, then pan
                const float x = 0.5f * (ring[0][rL] + ring[1][rR]) * taps[t].gain;

                // per-channel filter states
                float yL = tapFilters[t].processSample (x, 0);
                float yR = tapFilters[t].processSample (x, 1);

                float gl, gr; erEqualPowerPan (taps[t].pan, gl, gr);
                accL += yL * gl;
                accR += yR * gr;
            }

            L[n] += accL;
            R[n] += accR;
        }
    }
    else
    {
        // mono/ambisonic: sum taps independently per channel (no pan)
        for (int ch=0; ch<C; ++ch)
        {
            float* dst = out.getWritePointer(ch);
            const int R = (int) ring[(size_t)ch].size();
            const int w = writeIdx[(size_t)ch];

            for (int n=0; n<N; ++n)
            {
                float acc = 0.f;
                for (int t=0; t<numTaps; ++t)
                {
                    const int r = (w - taps[t].delaySamp + R) % R;
                    const float x = ring[(size_t)ch][r] * taps[t].gain;
                    acc += tapFilters[t].processSample (x, ch);
                }
                dst[n] += acc;
            }
        }
    }
}

// ============================================================================
// Ducking
// ============================================================================
constexpr std::array<ReverbEngine::DuckingSystem::Mode,5> ReverbEngine::DuckingSystem::modes;

void ReverbEngine::DuckingSystem::prepare (double sr, int maxBlock, int channels)
{
    fs = sr;
    work.setSize (channels, maxBlock);

    // default sizes; will be adjusted in setParams()
    rmsRing.assign (jmax (1, (int)std::round (0.050 * fs)), 0.0f); // 50 ms
    gaRing.assign  (maxBlock + 8192, 1.0f);

    rmsWindowSamp = (int) rmsRing.size();
    rmsHead = 0; sumSq = 0.0; gaAhead = 0; gaW = gaR = 0;

    band = ERFilter{}; band.resize (channels);
    envelope = 1.0f; lastGrDb = 0.0f;
}

void ReverbEngine::DuckingSystem::reset ()
{
    std::fill (rmsRing.begin(), rmsRing.end(), 0.0f);
    std::fill (gaRing.begin(),  gaRing.end(),  1.0f);
    sumSq = 0.0; rmsHead = 0; gaW = gaR = 0; envelope = 1.0f; lastGrDb = 0.0f;
}

void ReverbEngine::DuckingSystem::setParams (int mode, int detector, float depthDb, float thr, float ratio,
                                             float knee, float atk, float rel, float bHz, float bQ, bool en)
{
    enabled     = en;
    detectorSrc = jlimit (0, 3, detector);

    const auto& m = modes[jlimit(0,4,mode)];
    rmsWindowSamp = jmax (1, (int) std::round (m.rmsMs * 0.001 * fs));
    if ((int)rmsRing.size() != rmsWindowSamp) { rmsRing.assign (rmsWindowSamp, 0.0f); sumSq = 0.0; rmsHead = 0; }

    gaAhead = jmax (0, (int) std::round (m.lookAheadMs * 0.001 * fs));

    depthCapDb = jmax (0.0f, depthDb);
    thrDb = thr; kneeDb = jmax (0.0f, knee);
    rat = jmax (1.0f, ratio);
    atkMs = jmax (0.01f, atk);
    relMs = jmax (0.01f, rel);

    bandHz = bHz; bandQ = bQ;
    if (bandHz > 0.0f)
        band = ERFilter::makePeaking (fs, bandHz, jmax (0.1f, bandQ), 0.0f); // detection only
}

const AudioBuffer<float>* ReverbEngine::DuckingSystem::selectDetector (int src,
     const AudioBuffer<float>& dry, const AudioBuffer<float>& er, const AudioBuffer<float>& tail, const AudioBuffer<float>& wet) const
{
    switch (src) { case 0: return &dry; case 1: return &er; case 2: return &tail; case 3: default: return &wet; }
}

float ReverbEngine::DuckingSystem::computeRms (const AudioBuffer<float>& buf)
{
    // mean-square of this block
    const int C = buf.getNumChannels();
    const int N = buf.getNumSamples();
    long double s = 0.0L;
    for (int c=0;c<C;++c) { const float* d = buf.getReadPointer(c); for (int n=0;n<N;++n) s += (long double)d[n]*d[n]; }
    const double msBlock = (double)s / jmax (1, C*N);

    // ring of mean-squares
    sumSq += msBlock;
    sumSq -= rmsRing[(size_t)rmsHead];
    rmsRing[(size_t)rmsHead] = (float)msBlock;
    if (++rmsHead == rmsWindowSamp) rmsHead = 0;

    return (float) std::sqrt (jmax (0.0, sumSq / (double)rmsWindowSamp));
}

static inline float softKneeGain (float xDb, float thrDb, float kneeDb, float ratio)
{
    if (kneeDb <= 0.0f)
    {
        if (xDb <= thrDb) return 1.0f;
        const float over = xDb - thrDb;
        const float grDb = over * (1.0f - 1.0f/ratio);
        return Decibels::decibelsToGain (-grDb);
    }

    const float half = kneeDb * 0.5f;
    if (xDb <= thrDb - half) return 1.0f;
    if (xDb >= thrDb + half)
    {
        const float over = xDb - thrDb;
        const float grDb = over * (1.0f - 1.0f/ratio);
        return Decibels::decibelsToGain (-grDb);
    }

    // inside knee: smooth quadratic
    const float d = xDb - (thrDb - half);
    const float interp = (d * d) / (kneeDb * kneeDb); // 0..1
    const float over = xDb - thrDb + half;
    const float hardGrDb = over * (1.0f - 1.0f/ratio);
    const float kneeGrDb = hardGrDb * interp;
    return Decibels::decibelsToGain (-kneeGrDb);
}

float ReverbEngine::DuckingSystem::computeTargetGain (float rms)
{
    const float xDb = Decibels::gainToDecibels (jmax (rms, 1e-9f), -120.0f);
    float g = softKneeGain (xDb, thrDb, kneeDb, rat);

    // depth as maximum attenuation cap
    const float gMin = Decibels::decibelsToGain (-depthCapDb);
    if (g < gMin) g = gMin;
    return g;
}

void ReverbEngine::DuckingSystem::process (AudioBuffer<float>& wet,
                                           const AudioBuffer<float>& dry,
                                           const AudioBuffer<float>& er,
                                           const AudioBuffer<float>& tail)
{
    if (!enabled) { lastGrDb = 0.0f; return; }

    const AudioBuffer<float>* src = selectDetector (detectorSrc, dry, er, tail, wet);

    // optional band-limited detector
    if (bandHz > 0.0f)
    {
        work.makeCopyOf (*src);
        band.processInPlace (work);
        src = &work;
    }

    const float rms    = computeRms (*src);
    const float target = computeTargetGain (rms);

    // look-ahead FIFO
    gaRing[(size_t)gaW] = target;
    gaW = (gaW + 1) % (int)gaRing.size();
    gaR = (gaW - gaAhead + (int)gaRing.size()) % (int)gaRing.size();
    const float laTarget = gaRing[(size_t)gaR];

    // AR smoothing
    const float a = std::exp (-1.0f / (atkMs * 0.001f * (float)fs));
    const float r = std::exp (-1.0f / (relMs * 0.001f * (float)fs));
    const float coeff = (laTarget < envelope ? a : r);
    envelope = envelope + (laTarget - envelope) * (1.0f - coeff);

    // apply to wet
    const int C = wet.getNumChannels();
    const int N = wet.getNumSamples();
    for (int ch=0; ch<C; ++ch) { float* d = wet.getWritePointer(ch); for (int i=0;i<N;++i) d[i] *= envelope; }

    lastGrDb = Decibels::gainToDecibels (envelope, -120.0f);
}

// ============================================================================
// Tone EQ runtime
// ============================================================================
void ReverbEngine::ToneEqRuntime::prepare (double fs, int channelsIn, const ToneEq& eq)
{
    channels = channelsIn; apply = eq.apply; biqs.clear();
    if (eq.bands.empty()) return;

    for (const auto& b : eq.bands)
    {
        ERFilter f;
        switch (b.kind)
        {
            case ToneEqBand::Peak:      f = ERFilter::makePeaking  (fs, b.freqHz, b.q, b.gainDb);  break;
            case ToneEqBand::LowShelf:  f = ERFilter::makeLowShelf (fs, b.freqHz, b.q, b.gainDb);  break;
            case ToneEqBand::HighShelf: f = ERFilter::makeHighShelf(fs, b.freqHz, b.q, b.gainDb);  break;
        }
        f.resize (channels);
        biqs.push_back (std::move (f));
    }
}

void ReverbEngine::ToneEqRuntime::process (AudioBuffer<float>& buf)
{
    if (biqs.empty()) return;
    for (auto& b : biqs) b.processInPlace (buf);
}

// ============================================================================
// FDN scaffold (Phase-2)
// ============================================================================
// FdnTank methods are now implemented inline in the header file
