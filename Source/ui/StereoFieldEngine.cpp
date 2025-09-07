#include "StereoFieldEngine.h"
#include <cmath>

namespace {
static inline double geoMean (double a, double b) { return std::sqrt (a * b); }
static inline void accumEdgeWeighted (double& Sxx, double& Syy, double& Sxy,
                                      const float* F1, const float* F2, int k, double w)
{
    const double re1 = (double) F1[2 * k], im1 = (double) F1[2 * k + 1];
    const double re2 = (double) F2[2 * k], im2 = (double) F2[2 * k + 1];
    const double m1  = re1 * re1 + im1 * im1;
    const double m2  = re2 * re2 + im2 * im2;
    const double cr  = re1 * re2 + im1 * im2; // Re{X1 conj X2}
    Sxx += w * m1; Syy += w * m2; Sxy += w * cr;
}
}

void StereoFieldEngine::prepare (double sampleRate, const Settings& s)
{
    sr = (sampleRate > 0.0 ? sampleRate : 48000.0);
    setSettings (s);
    rebuildFFT();
    ensureFrames();
    rebuildBands();
    rebuildImages();

    // simple perceptual diverging LUT: blue -> gray -> orange
    lutImg = juce::Image (juce::Image::RGB, 256, 1, false);
    for (int i = 0; i < 256; ++i)
    {
        float t = i / 255.0f; // 0..1 maps to r -1..+1 later
        auto blend = [](juce::Colour a, juce::Colour b, float x){ return a.interpolatedWith (b, juce::jlimit (0.f,1.f,x)); };
        juce::Colour cold (0xFF1B74D6); // blue
        juce::Colour mid  (0xFF8A8F98); // neutral gray
        juce::Colour hot  (0xFFE07A1F); // orange
        juce::Colour c = (t < 0.5f) ? blend (cold, mid, t * 2.0f)
                                    : blend (mid,  hot, (t - 0.5f) * 2.0f);
        lutImg.setPixelAt (i, 0, c);
    }
}

void StereoFieldEngine::setSettings (const Settings& s)
{
    settings = s;
    fftOrder = juce::jlimit (8, 13, s.fftOrder);
    fftSize  = 1 << fftOrder;
    hopSize  = juce::jmax (1, fftSize / juce::jmax (1, s.hopDiv));
}

void StereoFieldEngine::reset()
{
    afifoPost.reset(); afifoPre.reset();
    writeX = 0; frozen = false;
    for (auto& b : bands) { b.rSmooth = 0.f; b.gateOpen = false; }
}

void StereoFieldEngine::setHistoryWidth (int px)
{
    settings.historyWidthPx = juce::jlimit (64, 4096, px);
    rebuildImages();
}

void StereoFieldEngine::rebuildFFT()
{
    fft = juce::dsp::FFT (fftOrder);
    window = std::make_unique<juce::dsp::WindowingFunction<float>> (
        (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann, true);
    fdL.allocate (2 * fftSize, true);
    fdR.allocate (2 * fftSize, true);
}

void StereoFieldEngine::ensureFrames()
{
    frameL.allocate (fftSize, true);
    frameR.allocate (fftSize, true);
    tdBuf.allocate (fftSize, true);
}

void StereoFieldEngine::rebuildBands()
{
    bands.clear();
    const double fLo = juce::jlimit (20.0, sr * 0.45, settings.fMin);
    const double fHi = juce::jlimit (fLo * 1.1, sr * 0.49, settings.fMax);
    const double decades = std::log10 (fHi / fLo);
    const int nBands = juce::jmax (4, (int) std::round (decades * settings.bandsPerDecade));

    bands.reserve ((size_t) nBands);
    const double r = std::pow (10.0, 1.0 / (double) settings.bandsPerDecade); // per step
    double fStart = fLo;
    for (int i = 0; i < nBands; ++i)
    {
        double fEnd = juce::jmin (fHi, fStart * r);
        Band b{};
        b.fLo = fStart; b.fHi = fEnd; b.fC = geoMean (fStart, fEnd);

        // fractional-bin edges
        double k0f = hzToK (b.fLo, sr, fftSize);
        double k1f = hzToK (b.fHi, sr, fftSize);
        b.k0 = (int) std::floor (k0f);
        b.k1 = (int) std::floor (k1f);
        b.w0 = 1.0 - (k0f - (double) b.k0); // partial weight on lower edge bin
        b.w1 = (k1f - (double) b.k1);       // partial on upper edge bin
        b.rSmooth = 0.f; b.gateOpen = false;

        bands.push_back (b);
        fStart = fEnd;
        if (fStart >= fHi) break;
    }
}

void StereoFieldEngine::rebuildImages()
{
    const int W = juce::jmax (64, settings.historyWidthPx);
    const int H = juce::jmax (1, (int) bands.size());
    imgPost = juce::Image (juce::Image::RGB, W, H, false);
    imgPost.clear (imgPost.getBounds(), juce::Colours::black);
    imgPre  = juce::Image (juce::Image::RGB, W, H, true);
    imgPre.clear  (imgPre.getBounds(), juce::Colours::transparentBlack);
    writeX = 0;
    widthPost.assign ((size_t) H, 0.0f);
}

void StereoFieldEngine::pushSamplesImpl (juce::AudioBuffer<float>& dstBuf, juce::AbstractFifo& a,
                                         const float* L, const float* R, int n)
{
    if (n <= 0) return;
    int s1, n1, s2, n2;
    a.prepareToWrite (n, s1, n1, s2, n2);
    if (n1 > 0)
    {
        if (L) std::memcpy (dstBuf.getWritePointer(0) + s1, L, sizeof(float) * (size_t) n1);
        else   std::memset (dstBuf.getWritePointer(0) + s1, 0, sizeof(float) * (size_t) n1);
        if (R) std::memcpy (dstBuf.getWritePointer(1) + s1, R, sizeof(float) * (size_t) n1);
        else   std::memcpy (dstBuf.getWritePointer(1) + s1, dstBuf.getWritePointer(0) + s1, sizeof(float) * (size_t) n1);
    }
    if (n2 > 0)
    {
        if (L) std::memcpy (dstBuf.getWritePointer(0) + s2, L + n1, sizeof(float) * (size_t) n2);
        else   std::memset (dstBuf.getWritePointer(0) + s2, 0, sizeof(float) * (size_t) n2);
        if (R) std::memcpy (dstBuf.getWritePointer(1) + s2, R + n1, sizeof(float) * (size_t) n2);
        else   std::memcpy (dstBuf.getWritePointer(1) + s2, dstBuf.getWritePointer(0) + s2, sizeof(float) * (size_t) n2);
    }
    a.finishedWrite (n1 + n2);
}

void StereoFieldEngine::pushBlock (const float* L, const float* R, int n, bool isPre)
{
    if (isPre) pushSamplesImpl (fifoPre,  afifoPre,  L, R, n);
    else       pushSamplesImpl (fifoPost, afifoPost, L, R, n);
}

bool StereoFieldEngine::popHop (juce::AudioBuffer<float>& src, juce::AbstractFifo& a)
{
    int s1, n1, s2, n2;
    a.prepareToRead (hopSize, s1, n1, s2, n2);
    const int total = n1 + n2;
    if (total < hopSize) { a.finishedRead (0); return false; }

    // roll previous frame and append new
    const int keep = fftSize - hopSize;
    if (keep > 0)
    {
        std::memmove (frameL.getData(), frameL.getData() + hopSize, sizeof(float) * (size_t) keep);
        std::memmove (frameR.getData(), frameR.getData() + hopSize, sizeof(float) * (size_t) keep);
    }
    auto* sL = src.getReadPointer (0);
    auto* sR = src.getReadPointer (1);
    if (n1 > 0)
    {
        std::memcpy (frameL.getData() + keep, sL + s1, sizeof(float) * (size_t) n1);
        std::memcpy (frameR.getData() + keep, sR + s1, sizeof(float) * (size_t) n1);
    }
    if (n2 > 0)
    {
        std::memcpy (frameL.getData() + keep + n1, sL + s2, sizeof(float) * (size_t) n2);
        std::memcpy (frameR.getData() + keep + n1, sR + s2, sizeof(float) * (size_t) n2);
    }
    a.finishedRead (hopSize);
    return true;
}

void StereoFieldEngine::stft (const float* in, float* td, float* fd)
{
    // copy + window
    std::memcpy (td, in, sizeof(float) * (size_t) fftSize);
    if (window) window->multiplyWithWindowingTable (td, (size_t) fftSize);
    std::memset (fd, 0, sizeof(float) * (size_t) (2 * fftSize));
    for (int i = 0; i < fftSize; ++i) fd[2 * i] = td[i]; // real part
    fft.performRealOnlyForwardTransform (fd);
}

juce::Colour StereoFieldEngine::mapColour (float r, const juce::Image& lut)
{
    const float dead = 0.04f;
    if (std::abs (r) < dead) r = 0.0f;
    float t = juce::jlimit (0.0f, 1.0f, 0.5f * (r + 1.0f));
    const int i = (int) juce::jlimit (0, 255, (int) std::floor (t * 255.0f));
    return lut.getPixelAt (i, 0);
}

bool StereoFieldEngine::computeRColumn (juce::AudioBuffer<float>& src, juce::AbstractFifo& a, juce::Image& dest)
{
    if (!popHop (src, a)) return false;

    // FFT frames
    stft (frameL.getData(), tdBuf.getData(), fdL.getData());
    stft (frameR.getData(), tdBuf.getData(), fdR.getData());

    const int H = dest.getHeight();
    juce::Image::BitmapData pix (dest, juce::Image::BitmapData::writeOnly);

    for (int bi = 0; bi < (int) bands.size(); ++bi)
    {
        auto& b = bands[(size_t) bi];
        double Sxx = 0.0, Syy = 0.0, Sxy = 0.0;
        if (b.k0 >= 1)
            accumEdgeWeighted (Sxx, Syy, Sxy, fdL.getData(), fdR.getData(), b.k0, b.w0);
        for (int k = b.k0 + 1; k < b.k1; ++k)
            accumEdgeWeighted (Sxx, Syy, Sxy, fdL.getData(), fdR.getData(), k, 1.0);
        accumEdgeWeighted (Sxx, Syy, Sxy, fdL.getData(), fdR.getData(), b.k1, b.w1);

        const double E = 0.5 * (Sxx + Syy);
        const float  Edb = (float) juce::Decibels::gainToDecibels (std::sqrt (juce::jmax (1e-20, E)));
        const bool openNow = b.gateOpen ? (Edb > settings.energyGateDb - settings.gateHystDb)
                                        : (Edb > settings.energyGateDb + settings.gateHystDb);
        b.gateOpen = openNow;

        float r = 0.0f;
        float width01 = 0.0f;
        if (openNow && Sxx > 1e-30 && Syy > 1e-30)
        {
            const double denom = std::sqrt (Sxx * Syy) + 1e-30;
            r = (float) juce::jlimit (-1.0, 1.0, Sxy / denom);
            // Width proxy: |S| / (|M| + eps) using spectra magnitudes
            const double Mmag = std::sqrt (juce::jmax (1e-30, Sxx));
            const double Smag = std::sqrt (juce::jmax (1e-30, Syy));
            width01 = (float) juce::jlimit (0.0, 2.0, Smag / (Mmag + 1e-20));
        }

        // asym smoothing
        const float aUp = settings.aAtk, aDown = settings.aRel;
        float y = b.rSmooth;
        const float a = (std::abs (r) > std::abs (y)) ? aUp : aDown;
        y = juce::jlimit (-1.0f, 1.0f, y + a * (r - y));
        b.rSmooth = y;

        const int yPix = juce::jmap (bi, 0, (int) bands.size() - 1, H - 1, 0);
        pix.setPixelColour (writeX, yPix, mapColour (y, lutImg));
        widthPost[(size_t) bi] = width01;
    }

    writeX = (writeX + 1) % dest.getWidth();
    return true;
}

void StereoFieldEngine::process()
{
    if (frozen) return;
    // Try to produce up to 2 columns per call when audio is plentiful to reduce apparent lag
    int produced = 0;
    produced += computeRColumn (fifoPost, afifoPost, imgPost) ? 1 : 0;
    if (settings.enablePre) computeRColumn (fifoPre, afifoPre, imgPre);
    if (produced > 0)
    {
        // opportunistically add another column to catch up
        produced += computeRColumn (fifoPost, afifoPost, imgPost) ? 1 : 0;
        if (settings.enablePre) computeRColumn (fifoPre, afifoPre, imgPre);
    }
}


