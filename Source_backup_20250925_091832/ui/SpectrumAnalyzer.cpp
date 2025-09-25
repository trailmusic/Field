#include "SpectrumAnalyzer.h"
#include <cmath>

static inline float clampf (float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Linear interpolate an array at fractional bin index
static inline float sampleFrac (const std::vector<float>& arr, double kf)
{
    const int N = (int) arr.size();
    if (N == 0) return 0.f;
    if (kf <= 0.0) return arr[0];
    if (kf >= (double) (N - 1)) return arr[(size_t) (N - 1)];
    const int k = (int) kf;
    const double t = kf - (double) k;
    return (float) ((1.0 - t) * (double) arr[(size_t) k] + t * (double) arr[(size_t) (k + 1)]);
}

// Soft triangular averaging whose width scales with how many bins fit in a pixel.
static inline float sampleFracSmoothBins (const std::vector<float>& arr, double kf, double binsPerPixel, double halfMin, double halfMax, double halfScale)
{
    const int N = (int) arr.size();
    if (N <= 0) return 0.f;

    const double half = juce::jlimit (halfMin, halfMax, halfScale * juce::jmax (1.0, binsPerPixel));
    const double a = juce::jlimit (0.0, (double) (N - 1), kf - half);
    const double b = juce::jlimit (0.0, (double) (N - 1), kf + half);
    const int ka = (int) std::floor (a);
    const int kb = (int) std::ceil  (b);

    double sumW = 0.0, sumV = 0.0;
    for (int k = ka; k <= kb; ++k)
    {
        const double t = 1.0 - std::abs ((double) k - kf) / half;
        const double w = half > 0.0 ? juce::jmax (0.0, t) : 1.0;
        const double v = (double) arr[(size_t) juce::jlimit (0, N - 1, k)];
        sumW += w; sumV += w * v;
    }
    if (sumW <= 0.0) return sampleFrac (arr, kf);
    return (float) (sumV / sumW);
}
// Light triangular averaging whose width shrinks with frequency:
// more smoothing at small k (low end), gentler up high.
static inline float sampleFracSmooth (const std::vector<float>& arr, double kf)
{
    const int N = (int) arr.size();
    if (N <= 0) return 0.f;

    // half-width ~0.9 bin near DC → ~0.4 bins near Nyquist
    const double half = juce::jlimit (0.35, 0.90,
                        juce::jmap (kf, 0.0, (double) juce::jmax (1, N),
                                         0.90, 0.40));

    const double a = juce::jlimit (0.0, (double) (N - 1), kf - half);
    const double b = juce::jlimit (0.0, (double) (N - 1), kf + half);
    const int ka = (int) std::floor (a);
    const int kb = (int) std::ceil  (b);

    double sumW = 0.0, sumV = 0.0;
    for (int k = ka; k <= kb; ++k)
    {
        // triangle weight peaked at kf
        const double t = 1.0 - std::abs ((double) k - kf) / half;
        const double w = half > 0.0 ? juce::jmax (0.0, t) : 1.0;
        const double v = (double) arr[(size_t) juce::jlimit (0, N - 1, k)];
        sumW += w; sumV += w * v;
    }
    if (sumW <= 0.0) return sampleFrac (arr, kf);
    return (float) (sumV / sumW);
}

SpectrumAnalyzer::SpectrumAnalyzer()
{
    setOpaque (false);
    setParams (params);
    setFreqRange (20.0, 20000.0);
    configurePost (params.fftOrder);

    fifoPost.setSize (1, 1 << 11);
    fifoPre .setSize (1, 1 << 11);
    preDelay.setSize (1, 1);
    timeDomain.allocate (fftSize, true);
    freqDomain.allocate (2 * fftSize, true);
    magDbPost.resize (fftSize / 2, params.minDb);
    smoothedDbPost.resize (fftSize / 2, params.minDb);
    peakDbPost.resize (fftSize / 2, params.minDb);
    magDbPre.resize (fftSize / 2, params.minDb);
    smoothedDbPre.resize (fftSize / 2, params.minDb);
    peakDbPre.resize (fftSize / 2, params.minDb);

    // Ensure smoothers exist for first activation before any param changes
    smoothersPost.assign (fftSize / 2, {});
    smoothersPre.assign  (fftSize / 2, {});

    startTimerHz (params.fps);
    acceptingAudio.store (true, std::memory_order_release);
}
void SpectrumAnalyzer::setSmoothingPreset (SmoothingPreset p)
{
    smoothingPreset = p;
    if (p == SmoothingPreset::Silky)
    {
        // Heavier LF averaging and gentler pixel-domain smoothing
        smoothHalfMin   = 0.80; smoothHalfMax = 4.00; smoothHalfScale = 0.90;
        aMainStart = 0.05f; aMainEnd = 0.18f;
        aPeakStart = 0.08f; aPeakEnd = 0.22f;
        strokeWidthMain = 1.2f; strokeWidthPeak = 0.9f;
        fillAlphaMul = 1.10f; strokeAlphaMul = 0.90f; peakAlphaMul = 0.85f;
    }
    else // Clean
    {
        // Tighter averaging and snappier pixel-domain smoothing
        smoothHalfMin   = 0.20; smoothHalfMax = 1.20; smoothHalfScale = 0.35;
        aMainStart = 0.18f; aMainEnd = 0.48f;
        aPeakStart = 0.24f; aPeakEnd = 0.60f;
        strokeWidthMain = 1.8f; strokeWidthPeak = 1.2f;
        fillAlphaMul = 0.90f; strokeAlphaMul = 1.05f; peakAlphaMul = 1.10f;
    }
    repaint();
}
void SpectrumAnalyzer::configurePost (int order)
{
    const juce::ScopedLock sl (cfgLock);
    postConfigured.store (false, std::memory_order_release);
    postFrameReady.store (false, std::memory_order_release);

    fftOrder = juce::jlimit (8, 15, order);
    fftSize  = 1 << fftOrder;
    hopSize  = fftSize / 2;

    fft = juce::dsp::FFT (fftOrder);
    window = std::make_unique<juce::dsp::WindowingFunction<float>> ((size_t) fftSize, juce::dsp::WindowingFunction<float>::hann, true);

    timeDomain.allocate (fftSize, true);
    freqDomain.allocate (2 * fftSize, true);
    magDbPost.assign (fftSize / 2, params.minDb);
    smoothedDbPost.assign (fftSize / 2, params.minDb);
    peakDbPost.assign (fftSize / 2, params.minDb);
    // Keep smoothers and pre lane consistent on any (re)configure
    smoothersPost.assign (fftSize / 2, {});
    smoothersPre.assign  (fftSize / 2, {});
    magDbPre.assign      (fftSize / 2, params.minDb);
    smoothedDbPre.assign (fftSize / 2, params.minDb);
    peakDbPre.assign     (fftSize / 2, params.minDb);

    fifoPost.setSize (1, fftSize);
    fifoPost.clear();
    fifoWritePost.store (0, std::memory_order_release);
    samplesAccumPost.store (0, std::memory_order_release);

    // Align pre FIFO as well to avoid mismatches during first paint
    fifoPre.setSize (1, fftSize);
    fifoPre.clear();
    fifoWritePre.store (0, std::memory_order_release);
    samplesAccumPre.store (0, std::memory_order_release);

    postConfigured.store (true, std::memory_order_release);
}

void SpectrumAnalyzer::resetPost()
{
    const juce::ScopedLock sl (cfgLock);
    postConfigured.store (false, std::memory_order_release);
    postFrameReady.store (false,  std::memory_order_release);
    fifoWritePost.store (0, std::memory_order_release);
    samplesAccumPost.store (0, std::memory_order_release);
    if (fifoPost.getNumSamples() > 0) fifoPost.clear();
}

void SpectrumAnalyzer::setSampleRate (double sr)
{
    sampleRate = (sr > 0 ? sr : 48000.0);
    const int W = getWidth();
    if (W > 0) rebuildPixelMap (W);
}

void SpectrumAnalyzer::setParams (const Params& p)
{
    params = p;

    if (fftOrder != params.fftOrder)
    {
        fftOrder = params.fftOrder;
        fftSize  = 1 << fftOrder;
        hopSize  = fftSize / 2;
        fft = juce::dsp::FFT (fftOrder);
        window = std::make_unique<juce::dsp::WindowingFunction<float>> ((size_t) fftSize, juce::dsp::WindowingFunction<float>::hann, true);

        pauseAudio();
        postFrameReady.store (false, std::memory_order_release);
        {
            const juce::SpinLock::ScopedLockType sl (dataLock);
            fifoPost.setSize (1, fftSize);
            fifoPost.clear();
            fifoWritePost.store (0);
            samplesAccumPost.store (0);
            fifoPre.setSize (1, fftSize);
            fifoPre.clear();
            fifoWritePre.store (0);
            samplesAccumPre.store (0);

            timeDomain.allocate (fftSize, true);
            freqDomain.allocate (2 * fftSize, true);
            magDbPost.assign (fftSize / 2, params.minDb);
            smoothedDbPost.assign (fftSize / 2, params.minDb);
            peakDbPost.assign (fftSize / 2, params.minDb);
            magDbPre.assign (fftSize / 2, params.minDb);
            smoothedDbPre.assign (fftSize / 2, params.minDb);
            peakDbPre.assign (fftSize / 2, params.minDb);
            smoothersPost.assign (fftSize / 2, {});
            smoothersPre.assign  (fftSize / 2, {});
        }
        resumeAudio();

        // compute window RMS scaling for dB calibration (single-sided)
        double wSumSq = 0.0;
        if (window)
        {
            // Recreate a temp window table to compute RMS (no direct getter in JUCE API)
            juce::HeapBlock<float> wt;
            wt.allocate ((size_t) fftSize, true);
            juce::dsp::WindowingFunction<float>::fillWindowingTables (wt.get(), (size_t) fftSize,
                                                                       juce::dsp::WindowingFunction<float>::hann, true);
            for (int i = 0; i < fftSize; ++i) { const float w = wt[i]; wSumSq += (double) w * (double) w; }
        }
        const float rmsWin = std::sqrt ((float) (wSumSq / (double) juce::jmax (1, fftSize)));
        windowScale = (rmsWin > 0.0f ? 1.0f / (fftSize * rmsWin) : 1.0f / (float) fftSize);
    }

    const float dt = 1.0f / juce::jmax (1, params.fps);
    const float tau = juce::jmax (1.0f, params.avgTimeMs) * 0.001f;
    alphaAvg = 1.0f - std::exp (-dt / tau);

    peakFallPerFrameDb = (params.peakFallDbPerSec / juce::jmax (1, params.fps));

    // asymmetric smoothing coefficients (fast attack ~50 ms, slow release ~250 ms)
    alphaFast = 1.0f - std::exp (-dt / 0.050f);
    alphaSlow = 1.0f - std::exp (-dt / 0.250f);

    // Rebuild pixel map if width known (FFT order or fps might have changed)
    const int W = getWidth();
    if (W > 0) rebuildPixelMap (W);
}

void SpectrumAnalyzer::setFreqRange (double lo, double hi)
{
    fMin = juce::jlimit (10.0, 2000.0, lo);
    fMax = juce::jlimit (4000.0, 48000.0, hi);
    if (fMax <= fMin + 1.0) fMax = fMin + 1.0;
    const int W = getWidth();
    if (W > 0) rebuildPixelMap (W);
}

void SpectrumAnalyzer::setPreDelaySamples (int n)
{
    preDelaySamples = juce::jmax (0, n);
    preDelay.setSize (1, juce::jmax (1, preDelaySamples));
    preDelay.clear();
    preDelayWrite = 0;
}

void SpectrumAnalyzer::pushBlock (const float* L, const float* R, int n)
{
    if (!acceptingAudio.load (std::memory_order_acquire)) return;
    if (!postConfigured.load (std::memory_order_acquire)) return;
    if (n <= 0 || fifoPost.getNumSamples() <= 0 || fftSize <= 0) return;

    auto* fifoData = fifoPost.getWritePointer (0);
    int w = fifoWritePost.load (std::memory_order_relaxed);

    for (int i = 0; i < n; ++i)
    {
        float s = params.monoSum
                    ? 0.5f * ((L ? L[i] : 0.0f) + (R ? R[i] : 0.0f))
                    : (L ? L[i] : 0.0f);

        fifoData[w] = s;
        w = (w + 1) & (fftSize - 1);
    }

    fifoWritePost.store (w, std::memory_order_release);

    int acc = samplesAccumPost.load (std::memory_order_relaxed);
    acc += n;
    while (acc >= hopSize)
    {
        acc -= hopSize;
        performFFTIfReadyPost();
    }
    samplesAccumPost.store (acc, std::memory_order_release);
    hasPost.store (true, std::memory_order_release);
}

void SpectrumAnalyzer::pushBlockPre (const float* L, const float* R, int n)
{
    if (!acceptingAudio.load (std::memory_order_acquire)) return;
    if (n <= 0 || fifoPre.getNumSamples() <= 0 || fftSize <= 0) return;

    const int D = juce::jmax (1, preDelaySamples);
    if (D == 1)
    {
        // Bypass delay entirely: write straight to PRE FIFO
        auto* fifoData = fifoPre.getWritePointer (0);
        int w = fifoWritePre.load (std::memory_order_relaxed);
        for (int i = 0; i < n; ++i)
        {
            const float sIn = params.monoSum
                ? 0.5f * ((L ? L[i] : 0.0f) + (R ? R[i] : 0.0f))
                : (L ? L[i] : 0.0f);
            fifoData[w] = sIn;
            w = (w + 1) & (fftSize - 1);
        }
        fifoWritePre.store (w, std::memory_order_release);

        int acc = samplesAccumPre.load (std::memory_order_relaxed);
        acc += n;
        while (acc >= hopSize) { acc -= hopSize; performFFTIfReadyPre(); }
        samplesAccumPre.store (acc, std::memory_order_release);
        hasPre.store (true, std::memory_order_release);
        return;
    }

    // write incoming to preDelay
    auto* d = preDelay.getWritePointer (0);
    for (int i = 0; i < n; ++i)
    {
        const float sIn = params.monoSum ? 0.5f * ((L ? L[i] : 0.0f) + (R ? R[i] : 0.0f)) : (L ? L[i] : 0.0f);
        d[preDelayWrite] = sIn;
        preDelayWrite = (preDelayWrite + 1) % D;
    }

    // read delayed samples and push into pre FIFO
    auto* fifoData = fifoPre.getWritePointer (0);
    int w = fifoWritePre.load (std::memory_order_relaxed);
    int readIdx = preDelayWrite; // current write is next to overwrite; oldest is here
    for (int i = 0; i < n; ++i)
    {
        float s = d[readIdx];
        readIdx = (readIdx + 1) % D;
        fifoData[w] = s;
        w = (w + 1) & (fftSize - 1);
    }
    fifoWritePre.store (w, std::memory_order_release);

    int acc = samplesAccumPre.load (std::memory_order_relaxed);
    acc += n;
    while (acc >= hopSize)
    {
        acc -= hopSize;
        performFFTIfReadyPre();
    }
    samplesAccumPre.store (acc, std::memory_order_release);
    hasPre.store (true, std::memory_order_release);
}

void SpectrumAnalyzer::performFFTIfReadyPost()
{
    // Strong preconditions to avoid any deref on half-configured state
    if (!postConfigured.load (std::memory_order_acquire)) return;
    const int N = fftSize;
    if (N <= 0) return;
    if (!(std::isfinite (sampleRate) && sampleRate > 1.0)) return;
    if (timeDomain.get() == nullptr || freqDomain.get() == nullptr) return;
    if ((int) magDbPost.size() != N/2 || (int) smoothedDbPost.size() != N/2 || (int) peakDbPost.size() != N/2) return;
    if ((int) smoothersPost.size() != N/2) return;
    if (fifoPost.getNumSamples() < N) return;

    auto* fifoData = fifoPost.getReadPointer (0);
    const int w = fifoWritePost.load (std::memory_order_acquire);
    if (fifoData == nullptr || w < 0 || w >= N) return;

    for (int i = 0; i < fftSize; ++i)
        timeDomain[i] = fifoData[(w + i) & (fftSize - 1)];

    if (window) window->multiplyWithWindowingTable (timeDomain.get(), fftSize);

    for (int i = 0; i < fftSize; ++i)
        freqDomain[i] = timeDomain[i];
    std::fill (freqDomain.get() + fftSize, freqDomain.get() + 2 * fftSize, 0.0f);

    fft.performRealOnlyForwardTransform (freqDomain.get());

    // const float norm = 1.0f / (float) fftSize; // unused
    for (int k = 0; k < fftSize / 2; ++k)
    {
        const float re = freqDomain[2 * k];
        const float im = freqDomain[2 * k + 1];
        float mag = std::sqrt (re*re + im*im) * windowScale;
        if (k > 0 && k < fftSize/2) mag *= 2.0f; // single-sided
        mag = juce::jmax (mag, 1.0e-12f);
        float dB = 20.0f * std::log10 (mag);

        if (params.slopeDbPerOct != 0.0f)
        {
            double f = (double) k * (sampleRate / (double) fftSize);
            if (f < 1.0) f = 1.0;
            const double oct = std::log2 (f / 1000.0);
            dB += (float) (params.slopeDbPerOct * oct);
        }

        dB = clampf (dB, params.minDb - 24.0f, params.maxDb + 12.0f);
        magDbPost[(size_t) k] = dB;
    }

    {
        const juce::SpinLock::ScopedLockType sl (dataLock);
        for (int k = 0; k < fftSize / 2; ++k)
        {
            const float in  = magDbPost[(size_t) k];
            auto& sm = smoothersPost[(size_t) k];
            sm.yFast += alphaFast * (in - sm.yFast);
            sm.ySlow += (in > sm.ySlow ? alphaFast : alphaSlow) * (in - sm.ySlow);
            float& out = smoothedDbPost[(size_t) k];
            out = juce::jmax (sm.yFast, sm.ySlow);

            if (params.drawPeaks)
            {
                float& pk = peakDbPost[(size_t) k];
                pk = juce::jmax (pk - peakFallPerFrameDb, in);
            }
        }
        // Track loudest smoothed bin this frame for auto-headroom
        float frameMax = params.minDb;
        for (int k = 0; k < fftSize / 2; ++k)
            frameMax = juce::jmax (frameMax, smoothedDbPost[(size_t) k]);
        frameMaxDbPost.store (frameMax, std::memory_order_release);
        postFrameReady.store (true, std::memory_order_release);
    }
}

void SpectrumAnalyzer::performFFTIfReadyPre()
{
    // Guards to prevent reading half-configured state
    if (!postConfigured.load (std::memory_order_acquire)) return;
    const int N = fftSize;
    if (N <= 0) return;
    if (!(std::isfinite (sampleRate) && sampleRate > 1.0)) return;
    if (timeDomain.get() == nullptr || freqDomain.get() == nullptr) return;
    if ((int) magDbPre.size() != N/2 || (int) smoothedDbPre.size() != N/2 || (int) peakDbPre.size() != N/2) return;
    if ((int) smoothersPre.size() != N/2) return;
    if (fifoPre.getNumSamples() < N) return;

    auto* fifoData = fifoPre.getReadPointer (0);
    const int w = fifoWritePre.load (std::memory_order_acquire);
    if (fifoData == nullptr || w < 0 || w >= N) return;

    for (int i = 0; i < fftSize; ++i)
        timeDomain[i] = fifoData[(w + i) & (fftSize - 1)];

    if (window) window->multiplyWithWindowingTable (timeDomain.get(), fftSize);

    for (int i = 0; i < fftSize; ++i)
        freqDomain[i] = timeDomain[i];
    std::fill (freqDomain.get() + fftSize, freqDomain.get() + 2 * fftSize, 0.0f);

    fft.performRealOnlyForwardTransform (freqDomain.get());

    // const float norm = 1.0f / (float) fftSize; // unused
    for (int k = 0; k < fftSize / 2; ++k)
    {
        const float re = freqDomain[2 * k];
        const float im = freqDomain[2 * k + 1];
        float mag = std::sqrt (re*re + im*im) * windowScale;
        if (k > 0 && k < fftSize/2) mag *= 2.0f;
        mag = juce::jmax (mag, 1.0e-12f);
        float dB = 20.0f * std::log10 (mag);

        if (params.slopeDbPerOct != 0.0f)
        {
            double f = (double) k * (sampleRate / (double) fftSize);
            if (f < 1.0) f = 1.0;
            const double oct = std::log2 (f / 1000.0);
            dB += (float) (params.slopeDbPerOct * oct);
        }

        dB = clampf (dB, params.minDb - 24.0f, params.maxDb + 12.0f);
        magDbPre[(size_t) k] = dB;
    }

    {
        const juce::SpinLock::ScopedLockType sl (dataLock);
        for (int k = 0; k < fftSize / 2; ++k)
        {
            const float in  = magDbPre[(size_t) k];
            auto& sm = smoothersPre[(size_t) k];
            sm.yFast += alphaFast * (in - sm.yFast);
            sm.ySlow += (in > sm.ySlow ? alphaFast : alphaSlow) * (in - sm.ySlow);
            float& out = smoothedDbPre[(size_t) k];
            out = juce::jmax (sm.yFast, sm.ySlow);

            if (params.drawPeaks)
            {
                float& pk = peakDbPre[(size_t) k];
                pk = juce::jmax (pk - peakFallPerFrameDb, in);
            }
        }
        preFrameReady.store (true, std::memory_order_release);
    }
}

void SpectrumAnalyzer::timerCallback()
{
    repaint();
}

void SpectrumAnalyzer::resized()
{
    const int W = getWidth();
    if (W > 0) rebuildPixelMap (W);
}

float SpectrumAnalyzer::freqToX (double hz, float left, float right) const
{
    hz = juce::jlimit (fMin, fMax, hz);
    const double t = (std::log10 (hz) - std::log10 (fMin)) / (std::log10 (fMax) - std::log10 (fMin));
    return left + (float) t * (right - left);
}

float SpectrumAnalyzer::dbToY (float dB, float top, float bottom) const
{
    float topDb = params.maxDb + displayHeadroomDb;
    if (autoHeadroom)
    {
        const float dTop = frameMaxDbPost.load (std::memory_order_acquire);
        const float targetTop = params.minDb + (dTop - params.minDb) / juce::jlimit (0.20f, 0.95f, targetFill);
        const float minTop = params.maxDb + minHeadroomDb;
        const float maxTop = params.maxDb + maxHeadroomDb;
        topDb = juce::jlimit (minTop, maxTop, targetTop);
    }

    dB = clampf (dB, params.minDb, topDb);
    const float denom = juce::jmax (0.001f, (topDb - params.minDb));
    const float t = (dB - params.minDb) / denom;
    return bottom - t * (bottom - top);
}

bool SpectrumAnalyzer::isFrameDrawable (juce::Rectangle<float> r) const
{
    if (r.getWidth() < 2.0f || r.getHeight() < 2.0f) return false;
    if (!postFrameReady.load (std::memory_order_acquire)) return false;
    if (!(std::isfinite (sampleRate) && sampleRate > 1.0)) return false;
    if (!(std::isfinite (fMin) && std::isfinite (fMax) && fMin > 0.0 && fMax > fMin)) return false;
    if (fftSize < 32) return false;
    const int n = (int) smoothedDbPost.size();
    if (n <= 8 || n != fftSize/2) return false;
    return true;
}

void SpectrumAnalyzer::drawGridOnly (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (gridColour);
    const double decades[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    if (drawGridVertical)
    {
        for (double f : decades)
        {
            if (f < fMin || f > fMax) continue;
            const float x = freqToX (f, r.getX(), r.getRight());
            g.drawVerticalLine ((int) std::round (x), r.getY(), r.getBottom());
        }
    }
    if (drawGridHorizontal)
    {
        const float majors[] = { -60, -48, -36, -24, -12, 0, +6 };
        for (float d : majors)
        {
            if (d < params.minDb || d > params.maxDb) continue;
            const float y = dbToY (d, r.getY(), r.getBottom());
            g.drawHorizontalLine ((int) std::round (y), r.getX(), r.getRight());
        }
    }
}

void SpectrumAnalyzer::renderPaths (juce::Graphics& g, juce::Rectangle<float> r)
{
    areaPathPost.clear(); linePathPost.clear(); peakPathPost.clear(); eqPath.clear();

    // If we have no data yet, avoid building curves on activation.
    if (!hasPost.load (std::memory_order_acquire))
        return;

    const int N = (int) smoothedDbPost.size();
    if (N <= 8 || r.getWidth() <= 1.0f || r.getHeight() <= 1.0f)
        return;

    const int W = juce::jmax (1, (int) r.getWidth());
    if ((int) pixelKf.size() != W) rebuildPixelMap (W);
    std::vector<float> yMain (W), yPeak (W), yEq (W);

    const double hzToK = (sampleRate > 0.0 ? (double) fftSize / sampleRate : 0.0);
    const double logFMin = std::log10 (fMin);
    const double logFMax = std::log10 (fMax);

    float prevDbMain = 0.0f, prevDbPeak = 0.0f; bool prevInit = false;
    for (int x = 0; x < W; ++x)
    {
        const double f0 = std::pow (10.0, logFMin + (logFMax - logFMin) * (double)  x      / W);
        const double f1 = std::pow (10.0, logFMin + (logFMax - logFMin) * (double) (x + 1) / W);
        const double fm = std::sqrt (f0 * f1);
        const double kf = juce::jlimit (0.0, (double) (N - 1), fm * hzToK);
        const double binsPerPixel = juce::jmax (1e-6, (f1 - f0) * hzToK);

        float dB   = sampleFracSmoothBins (smoothedDbPost, kf, binsPerPixel, smoothHalfMin, smoothHalfMax, smoothHalfScale);
        float dBPk = sampleFracSmoothBins (peakDbPost,     kf, binsPerPixel, smoothHalfMin, smoothHalfMax, smoothHalfScale);

        const float pos01 = (float) x / (float) juce::jmax (1, W - 1);
        const float aMain = juce::jmap (pos01, aMainStart, aMainEnd);
        const float aPeak = juce::jmap (pos01, aPeakStart, aPeakEnd);

        if (!prevInit) { prevDbMain = dB; prevDbPeak = dBPk; prevInit = true; }
        else           { prevDbMain += aMain * (dB   - prevDbMain);
                         prevDbPeak += aPeak * (dBPk - prevDbPeak); }

        yMain[(size_t) x] = dbToY (prevDbMain, r.getY(), r.getBottom());
        yPeak[(size_t) x] = dbToY (prevDbPeak, r.getY(), r.getBottom());

        if (eqOverlayFn)
            yEq[(size_t) x] = dbToY (eqOverlayFn (fm), r.getY(), r.getBottom());
    }

    areaPathPost.startNewSubPath (r.getX(), r.getBottom());
    for (int x = 0; x < W; ++x)
        areaPathPost.lineTo (r.getX() + (float) x + 0.5f, yMain[(size_t) x]);
    areaPathPost.lineTo (r.getRight(), r.getBottom());
    areaPathPost.closeSubPath();

    if (yMain.empty()) return;
    linePathPost.startNewSubPath (r.getX(), yMain[0]);
    for (int x = 1; x < W; ++x)
        linePathPost.lineTo (r.getX() + (float) x + 0.5f, yMain[(size_t) x]);

    if (params.drawPeaks)
    {
        if (yPeak.empty()) return;
        peakPathPost.startNewSubPath (r.getX(), yPeak[0]);
        for (int x = 1; x < W; ++x)
            peakPathPost.lineTo (r.getX() + (float) x + 0.5f, yPeak[(size_t) x]);
    }

    g.setColour (gridColour);
    const double decades[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    if (drawGridVertical)
    {
        for (double f : decades)
        {
            const float x = freqToX (f, r.getX(), r.getRight());
            g.drawVerticalLine ((int) std::round (x), r.getY(), r.getBottom());
        }
    }
    if (drawGridHorizontal)
    {
        const float dbMajors[] = { -60, -48, -36, -24, -12, 0, +6 };
        for (float d : dbMajors)
        {
            if (d < params.minDb || d > params.maxDb) continue;
            const float y = dbToY (d, r.getY(), r.getBottom());
            g.drawHorizontalLine ((int) std::round (y), r.getX(), r.getRight());
        }
    }

    // Draw PRE overlay first so POST sits visually on top
    if (hasPre.load (std::memory_order_acquire) && preFrameReady.load (std::memory_order_acquire))
    {
        const int Wpre = juce::jmax (1, (int) r.getWidth());
        juce::Path preArea, preLine, prePeak;
        preArea.startNewSubPath (r.getX(), r.getBottom());
        bool first = true;
        float prevDbMainPre = 0.0f, prevDbPeakPre = 0.0f; bool prevInitPre = false;
        for (int xPix = 0; xPix < Wpre; ++xPix)
        {
            const double f0 = std::pow (10.0, logFMin + (logFMax - logFMin) * (double)  xPix      / Wpre);
            const double f1 = std::pow (10.0, logFMin + (logFMax - logFMin) * (double) (xPix + 1) / Wpre);
            const double fm = std::sqrt (f0 * f1);
            const double kf = juce::jlimit (0.0, (double) ((int) smoothedDbPre.size() - 1), fm * hzToK);
            const double binsPerPixel = juce::jmax (1e-6, (f1 - f0) * hzToK);

            const float dB   = sampleFracSmoothBins (smoothedDbPre, kf, binsPerPixel, smoothHalfMin, smoothHalfMax, smoothHalfScale);
            const float dBPk = sampleFracSmoothBins (peakDbPre,     kf, binsPerPixel, smoothHalfMin, smoothHalfMax, smoothHalfScale);

            const float pos01 = (float) xPix / (float) juce::jmax (1, Wpre - 1);
            const float aMain = juce::jmap (pos01, aMainStart, aMainEnd);
            const float aPeak = juce::jmap (pos01, aPeakStart, aPeakEnd);

            float dBs = dB, dPs = dBPk;
            if (!prevInitPre) { prevDbMainPre = dBs; prevDbPeakPre = dPs; prevInitPre = true; }
            else              { prevDbMainPre += aMain * (dBs - prevDbMainPre);
                                prevDbPeakPre += aPeak * (dPs - prevDbPeakPre); }

            const float x = r.getX() + (float) xPix + 0.5f;
            const float yM = dbToY (prevDbMainPre, r.getY(), r.getBottom());
            const float yP = dbToY (prevDbPeakPre, r.getY(), r.getBottom());

            if (first)
            {
                preLine.startNewSubPath (x, yM);
                prePeak.startNewSubPath (x, yP);
                first = false;
            }
            preArea.lineTo (x, yM);
            preLine.lineTo (x, yM);
            prePeak.lineTo (x, yP);
        }
        preArea.lineTo (r.getRight(), r.getBottom());
        preArea.closeSubPath();

        g.setColour (preFillColour);   g.fillPath (preArea);
        g.setColour (preStrokeColour); g.strokePath (preLine, juce::PathStrokeType (1.2f));
        if (params.drawPeaks)
        {
            g.setColour (prePeakColour);
            g.strokePath (prePeak, juce::PathStrokeType (0.8f));
        }
    }

    // Draw POST on top
    g.setColour (fillColour.withAlpha (fillColour.getFloatAlpha() * fillAlphaMul));
    g.fillPath (areaPathPost);
    g.setColour (strokeColour.withAlpha (strokeColour.getFloatAlpha() * strokeAlphaMul));
    g.strokePath (linePathPost, juce::PathStrokeType (strokeWidthMain));
    if (params.drawPeaks)
    {
        g.setColour (peakColour.withAlpha (peakColour.getFloatAlpha() * peakAlphaMul));
        g.strokePath (peakPathPost, juce::PathStrokeType (strokeWidthPeak, juce::PathStrokeType::beveled, juce::PathStrokeType::rounded));
    }

    if (eqOverlayFn)
    {
        eqPath.startNewSubPath (r.getX(), yMain.empty() ? r.getBottom() : yMain[0]);
        for (int x = 1; x < W; ++x)
            eqPath.lineTo (r.getX() + (float) x + 0.5f, yMain[(size_t) x]);

        g.setColour (eqOverlayColour);
        g.strokePath (eqPath, juce::PathStrokeType (2.0f));
    }
}

void SpectrumAnalyzer::rebuildPixelMap (int W)
{
    if (W <= 0) { pixelKf.clear(); return; }
    pixelKf.resize (W);
    const double hzToK = (sampleRate > 0.0 ? (double) fftSize / sampleRate : 0.0);
    const double logFMin = std::log10 (juce::jmax (1.0, fMin));
    const double logFMax = std::log10 (juce::jmax (fMin + 1.0, fMax));
    for (int x = 0; x < W; ++x)
    {
        const double f0 = std::pow (10.0, logFMin + (logFMax - logFMin) * (double) x / (double) W);
        const double f1 = std::pow (10.0, logFMin + (logFMax - logFMin) * (double) (x + 1) / (double) W);
        const double fm = std::sqrt (f0 * f1);
        pixelKf[(size_t) x] = fm * hzToK;
    }
}

void SpectrumAnalyzer::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // Try-lock to avoid blocking audio writer; draw grid if busy or not ready
    if (b.getWidth() < 2.0f || b.getHeight() < 2.0f)
        return;
    if (!postFrameReady.load (std::memory_order_acquire))
    {
        drawGridOnly (g, b);
        return;
    }
    const juce::SpinLock::ScopedTryLockType tl (dataLock);
    if (!tl.isLocked())
    {
        drawGridOnly (g, b);
        return;
    }
    renderPaths (g, b);
}


