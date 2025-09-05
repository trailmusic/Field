#include "SpectrumAnalyzer.h"
#include <cmath>

static inline float clampf (float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

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

void SpectrumAnalyzer::setSampleRate (double sr) { sampleRate = (sr > 0 ? sr : 48000.0); }

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
}

void SpectrumAnalyzer::setFreqRange (double lo, double hi)
{
    fMin = juce::jlimit (10.0, 2000.0, lo);
    fMax = juce::jlimit (4000.0, 48000.0, hi);
    if (fMax <= fMin + 1.0) fMax = fMin + 1.0;
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
    if (n <= 0 || preDelay.getNumSamples() <= 0 || fifoPre.getNumSamples() <= 0 || fftSize <= 0) return;

    // write incoming to preDelay
    auto* d = preDelay.getWritePointer (0);
    const int D = juce::jmax (1, preDelaySamples);
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

    const float norm = 1.0f / (float) fftSize;
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
        postFrameReady.store (true, std::memory_order_release);
    }
}

void SpectrumAnalyzer::performFFTIfReadyPre()
{
    // Guards to prevent reading half-configured state
    const int N = fftSize;
    if (N <= 0) return;
    if (!(std::isfinite (sampleRate) && sampleRate > 1.0)) return;
    if (timeDomain.get() == nullptr || freqDomain.get() == nullptr) return;
    if (fifoPre.getNumSamples() < N) return;
    if ((int) magDbPre.size() != N/2 || (int) smoothedDbPre.size() != N/2 || (int) peakDbPre.size() != N/2) return;
    if ((int) smoothersPre.size() != N/2) return;

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

    const float norm = 1.0f / (float) fftSize;
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

float SpectrumAnalyzer::freqToX (double hz, float left, float right) const
{
    hz = juce::jlimit (fMin, fMax, hz);
    const double t = (std::log10 (hz) - std::log10 (fMin)) / (std::log10 (fMax) - std::log10 (fMin));
    return left + (float) t * (right - left);
}

float SpectrumAnalyzer::dbToY (float dB, float top, float bottom) const
{
    dB = clampf (dB, params.minDb, params.maxDb);
    const float t = (dB - params.minDb) / (params.maxDb - params.minDb);
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
    for (double f : decades)
    {
        if (f < fMin || f > fMax) continue;
        const float x = freqToX (f, r.getX(), r.getRight());
        g.drawVerticalLine ((int) std::round (x), r.getY(), r.getBottom());
    }
    const float majors[] = { -60, -48, -36, -24, -12, 0, +6 };
    for (float d : majors)
    {
        if (d < params.minDb || d > params.maxDb) continue;
        const float y = dbToY (d, r.getY(), r.getBottom());
        g.drawHorizontalLine ((int) std::round (y), r.getX(), r.getRight());
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
    std::vector<float> yMain (W), yPeak (W), yEq (W);

    for (int xPix = 0; xPix < W; ++xPix)
    {
        const double f0 = std::pow (10.0, std::log10 (fMin) + (std::log10 (fMax) - std::log10 (fMin)) * (double) (xPix     ) / W);
        const double f1 = std::pow (10.0, std::log10 (fMin) + (std::log10 (fMax) - std::log10 (fMin)) * (double) (xPix + 1) / W);

        const int k0 = juce::jlimit (0, N - 1, (int) std::floor ((f0 * fftSize) / sampleRate));
        const int k1 = juce::jlimit (k0, N - 1, (int) std::ceil  ((f1 * fftSize) / sampleRate));

        float acc = params.minDb, accPk = params.minDb;
        for (int k = k0; k <= k1; ++k)
        {
            acc   = juce::jmax (acc,   smoothedDbPost[(size_t) k]);
            accPk = juce::jmax (accPk, peakDbPost[(size_t) k]);
        }

        yMain[(size_t) xPix] = dbToY (acc,   r.getY(), r.getBottom());
        yPeak[(size_t) xPix] = dbToY (accPk, r.getY(), r.getBottom());

        if (eqOverlayFn)
        {
            const double fm = std::sqrt (f0 * f1);
            float eqdB = eqOverlayFn (fm);
            yEq[(size_t) xPix] = dbToY (eqdB, r.getY(), r.getBottom());
        }
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
    for (double f : decades)
    {
        const float x = freqToX (f, r.getX(), r.getRight());
        g.drawVerticalLine ((int) std::round (x), r.getY(), r.getBottom());
    }
    const float dbMajors[] = { -60, -48, -36, -24, -12, 0, +6 };
    for (float d : dbMajors)
    {
        if (d < params.minDb || d > params.maxDb) continue;
        const float y = dbToY (d, r.getY(), r.getBottom());
        g.drawHorizontalLine ((int) std::round (y), r.getX(), r.getRight());
    }

    g.setColour (fillColour);   g.fillPath (areaPathPost);
    g.setColour (strokeColour); g.strokePath (linePathPost, juce::PathStrokeType (1.5f));

    if (params.drawPeaks)
    {
        g.setColour (peakColour);
        g.strokePath (peakPathPost, juce::PathStrokeType (1.0f, juce::PathStrokeType::beveled, juce::PathStrokeType::rounded));

        // Draw pre overlay if available
        if (hasPre.load (std::memory_order_acquire) && preFrameReady.load (std::memory_order_acquire))
        {
            const int W = juce::jmax (1, (int) r.getWidth());
            juce::Path preArea, preLine, prePeak;
            preArea.startNewSubPath (r.getX(), r.getBottom());
            preLine.startNewSubPath (r.getX(), yMain[0]);
            prePeak.startNewSubPath (r.getX(), yPeak[0]);
            for (int xPix = 0; xPix < W; ++xPix)
            {
                const double f0 = std::pow (10.0, std::log10 (fMin) + (std::log10 (fMax) - std::log10 (fMin)) * (double) (xPix     ) / W);
                const double f1 = std::pow (10.0, std::log10 (fMin) + (std::log10 (fMax) - std::log10 (fMin)) * (double) (xPix + 1) / W);
                const int k0 = juce::jlimit (0, (int) smoothedDbPre.size() - 1, (int) std::floor ((f0 * fftSize) / sampleRate));
                const int k1 = juce::jlimit (k0, (int) smoothedDbPre.size() - 1,  (int) std::ceil  ((f1 * fftSize) / sampleRate));
                float acc = -1.0e9f, accPk = -1.0e9f;
                for (int k = k0; k <= k1; ++k)
                {
                    acc   = juce::jmax (acc,   smoothedDbPre[(size_t) k]);
                    accPk = juce::jmax (accPk, peakDbPre[(size_t) k]);
                }
                const float yM = dbToY (acc,   r.getY(), r.getBottom());
                const float yP = dbToY (accPk, r.getY(), r.getBottom());
                preArea.lineTo (r.getX() + (float) xPix + 0.5f, yM);
                preLine.lineTo (r.getX() + (float) xPix + 0.5f, yM);
                prePeak.lineTo (r.getX() + (float) xPix + 0.5f, yP);
            }
            preArea.lineTo (r.getRight(), r.getBottom());
            preArea.closeSubPath();

            g.setColour (preFillColour);   g.fillPath (preArea);
            g.setColour (preStrokeColour); g.strokePath (preLine, juce::PathStrokeType (1.2f));
            g.setColour (prePeakColour);   g.strokePath (prePeak, juce::PathStrokeType (0.8f));
        }
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


