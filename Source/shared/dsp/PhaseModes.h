#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

static inline void makeKaiserWindow (std::vector<double>& w, double beta)
{
    auto I0 = [] (double x)
    {
        double s = 1.0, ds = 1.0, y = x*x/4.0;
        for (int k=1; k<32; ++k) { ds *= y / (k*k); s += ds; if (ds < 1e-12) break; }
        return s;
    };
    const double denom = I0 (beta);
    const size_t N = w.size();
    for (size_t n=0; n<N; ++n)
    {
        double t = (2.0*(double)n)/(double)(N-1) - 1.0;
        w[n] = I0 (beta * std::sqrt (1.0 - t*t)) / denom;
    }
}

static inline double sincN (double x)
{
    if (std::abs (x) < 1e-12) return 1.0;
    return std::sin (juce::MathConstants<double>::pi * x) / (juce::MathConstants<double>::pi * x);
}

static inline void designLowpass (std::vector<double>& h, double fcNorm, double gain = 1.0, double kaiserBeta = 8.6)
{
    const size_t N = h.size();
    jassert (N % 2 == 1);
    std::vector<double> w (N); w.assign (N, 0.0); makeKaiserWindow (w, kaiserBeta);
    const double M = (double) (N - 1);
    for (size_t n=0; n<N; ++n)
    {
        double m = (double)n - M/2.0;
        double x = 2.0 * fcNorm * sincN (2.0 * fcNorm * m);
        h[n] = gain * x * w[n];
    }
}

static inline void designHighpassFromLowpass (std::vector<double>& hHp, const std::vector<double>& hLp)
{
    jassert (hHp.size() == hLp.size());
    const size_t N = hHp.size();
    for (size_t n=0; n<N; ++n) hHp[n] = -hLp[n];
    hHp[N/2] += 1.0;
}

static inline void convolveSameLength (std::vector<double>& out, const std::vector<double>& a, const std::vector<double>& b)
{
    jassert (a.size() == b.size());
    const size_t N = a.size();
    std::vector<double> tmp (2*N - 1, 0.0);
    for (size_t i=0;i<N;++i)
        for (size_t j=0;j<N;++j)
            tmp[i+j] += a[i]*b[j];
    const size_t off = (tmp.size() - N)/2;
    out.resize (N);
    for (size_t n=0;n<N;++n) out[n] = tmp[off + n];
}

static inline void designLinearPhaseBandpassKernel (std::vector<float>& kernel, double fs, double hpHz, double lpHz, int N, double beta = 8.6)
{
    const double nyq = fs * 0.5;
    hpHz = juce::jlimit (20.0, nyq - 10.0, hpHz);
    lpHz = juce::jlimit (hpHz + 10.0, nyq - 1.0, lpHz);
    if ((N & 1) == 0) ++N;
    std::vector<double> lp (N), hp (N), band (N), tmp (N);
    designLowpass (lp, lpHz / nyq, 1.0, beta);
    designLowpass (tmp, hpHz / nyq, 1.0, beta);
    designHighpassFromLowpass (hp, tmp);
    convolveSameLength (band, lp, hp);
    double sum = 0.0; for (auto v : band) sum += v;
    const double norm = (std::abs (sum) > 1e-12 ? sum : 1.0);
    kernel.resize (N);
    for (int i=0;i<N;++i) kernel[(size_t)i] = (float) (band[(size_t)i] / norm);
}

// Build a composite linear-phase kernel from a target magnitude response (0..Nyquist).
// mags: size K/2+1 magnitudes (linear). K must be power of two >= N.
static inline void designLinearPhaseFromMagnitude (std::vector<float>& kernel,
                                                   const std::vector<double>& mags,
                                                   int    K,
                                                   int    N,
                                                   double beta = 8.6,
                                                   double refHz = 1000.0,
                                                   double fs = 48000.0)
{
    jassert ((int) mags.size() == K/2 + 1);
    if ((K & (K-1)) != 0) K = juce::nextPowerOfTwo (K);
    if ((N & 1) == 0) ++N;
    // Build real, even spectrum (packed real FFT format pairs)
    std::vector<std::complex<double>> Hc (K);
    for (int k = 0; k <= K/2; ++k)
    {
        double m = juce::jlimit (1e-6, 1000.0, mags[(size_t) k]);
        Hc[(size_t) k] = std::complex<double> (m, 0.0);
    }
    for (int k = 1; k < K/2; ++k)
        Hc[(size_t) (K - k)] = std::conj (Hc[(size_t) k]);

    // IFFT to time domain
    std::vector<std::complex<double>> hc (K);
    // naive DFT inverse (K up to ~32768 acceptable for occasional rebuilds)
    const double twopi = 2.0 * juce::MathConstants<double>::pi;
    for (int n = 0; n < K; ++n)
    {
        std::complex<double> acc (0.0, 0.0);
        for (int k = 0; k < K; ++k)
        {
            double ang = twopi * (double) n * (double) k / (double) K;
            std::complex<double> w (std::cos (ang), std::sin (ang));
            acc += Hc[(size_t) k] * w;
        }
        hc[(size_t) n] = acc / (double) K;
    }

    // Centered, windowed truncation to odd N
    std::vector<double> win (N); makeKaiserWindow (win, beta);
    std::vector<float> out ((size_t) N, 0.0f);
    const int mid = K / 2;
    const int half = N / 2;
    for (int i = 0; i < N; ++i)
    {
        int idx = mid - half + i;
        idx = juce::jlimit (0, K-1, idx);
        out[(size_t) i] = (float) (hc[(size_t) idx].real() * win[(size_t) i]);
    }
    // Normalize around ref Hz bin
    int refBin = juce::jlimit (1, K/2, (int) std::round (refHz / (fs * 0.5) * (K/2)));
    double refMag = juce::jlimit (1e-6, 1e6, mags[(size_t) refBin]);
    if (refMag > 1e-6)
    {
        for (auto& v : out) v = (float) (v / refMag);
    }
    kernel.swap (out);
}

template <typename Sample>
struct OverlapSaveConvolver
{
    void enableCrossfade (int ms) { xfadeMs = juce::jmax (0, ms); }
    void prepare (double sampleRate, int maxBlock, int kernelLen, int numChannels)
    {
        fs = sampleRate; N = kernelLen; ch = juce::jmax (1, numChannels);
        const int L = juce::nextPowerOfTwo (maxBlock + N - 1);
        fftOrder = (int) std::log2 ((double) L); fftSize = 1 << fftOrder;
        fft = std::make_unique<juce::dsp::FFT> (fftOrder);

        H.assign (2 * fftSize, 0.0f);
        X.assign (2 * fftSize, 0.0f);
        Y.assign (2 * fftSize, 0.0f);

        overlap.setSize (ch, N-1); overlap.clear();
        blockBuf.setSize (ch, fftSize); blockBuf.clear();
    }

    void setKernel (const std::vector<float>& kernelIn)
    {
        jassert ((int) kernelIn.size() == N);
        // compute to temp then swap; xfade will ramp in process
        std::vector<float> Hnew (2 * (size_t) fftSize, 0.0f);
        std::vector<float> hPadded ((size_t)fftSize, 0.0f);
        for (int i=0;i<N;++i) hPadded[(size_t)i] = kernelIn[(size_t)i];
        std::memcpy (Hnew.data(), hPadded.data(), sizeof(float)* (size_t) fftSize);
        fft->performRealOnlyForwardTransform (Hnew.data());
        H.swap (Hnew);
        kernelSet = true;
        latencySamples = (N - 1) / 2;
        if (xfadeMs > 0) { xfadeSamples = (int) (xfadeMs * 0.001 * fs); xfadePos = 0; }
    }

    int getLatencySamples() const { return latencySamples; }
    bool isReady() const { return kernelSet; }

    void process (juce::dsp::AudioBlock<Sample> block)
    {
        if (! kernelSet) return;
        const int numSamples = (int) block.getNumSamples();
        const int C = juce::jmin ((int) block.getNumChannels(), ch);

        int pos = 0;
        while (pos < numSamples)
        {
            const int chunk = juce::jmin (numSamples - pos, fftSize - (N - 1));
            for (int c=0;c<C;++c)
            {
                auto* dst = blockBuf.getWritePointer (c);
                const int P = N - 1;
                std::memcpy (dst, overlap.getReadPointer (c), sizeof(float) * (size_t) P);
                auto* src = block.getChannelPointer (c) + pos;
                for (int i=0;i<chunk;++i) dst[P + i] = (float) src[i];
                for (int i=P + chunk; i<fftSize; ++i) dst[i] = 0.0f;
            }

            for (int c=0;c<C;++c)
            {
                std::memcpy (X.data(), blockBuf.getReadPointer (c), sizeof(float)* (size_t) fftSize);
                fft->performRealOnlyForwardTransform (X.data());

                Y[0] = X[0] * H[0];
                Y[1] = X[1] * H[1];
                for (int k=2;k<2*fftSize; k+=2)
                {
                    const float xr = X[k], xi = X[k+1];
                    const float hr = H[k], hi = H[k+1];
                    Y[k]   = xr*hr - xi*hi;
                    Y[k+1] = xr*hi + xi*hr;
                }

                fft->performRealOnlyInverseTransform (Y.data());

                auto* out = block.getChannelPointer (c) + pos;
                const int P = N - 1;
                for (int i=0;i<chunk;++i)
                {
                    float y = Y[P + i];
                    if (xfadeSamples > 0 && xfadePos < xfadeSamples)
                    {
                        const float t = (float) xfadePos / (float) juce::jmax (1, xfadeSamples);
                        y *= t;
                        ++xfadePos;
                    }
                    out[i] = (Sample) y;
                }

                std::memcpy (overlap.getWritePointer (c),
                             blockBuf.getReadPointer (c) + chunk, sizeof(float) * (size_t) P);
            }
            pos += chunk;
        }
    }

    double fs = 48000.0; int N = 0, ch = 2; int fftOrder = 0, fftSize = 0; int latencySamples = 0; bool kernelSet = false;
    std::unique_ptr<juce::dsp::FFT> fft; std::vector<float> H, X, Y; juce::AudioBuffer<float> overlap, blockBuf;
    int xfadeMs { 0 }, xfadeSamples { 0 }, xfadePos { 0 };
};


