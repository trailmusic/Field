#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>

namespace fieldml {

struct Band
{
    double fLow{}, fHigh{}, fCenter{}; // Hz
    int    kStart{}, kEnd{};           // FFT bin indices [inclusive]
};

inline std::vector<Band> makeThirdOctaveBands (double fMin,
                                               double fMax,
                                               double sampleRate,
                                               int    fftSize)
{
    std::vector<Band> out;
    if (sampleRate <= 0.0 || fftSize <= 0) return out;

    const int kMax = fftSize / 2; // real-FFT usable half
    const double halfStep = std::pow (2.0, 1.0 / 6.0); // ±1/6 octave from center
    const double fRef = 1000.0;

    auto kFromHz = [=](double f){ return std::log2 (f / fRef) * 3.0; };
    auto hzFromK = [=](double k){ return fRef * std::pow (2.0, k / 3.0); };

    const double kLo = std::floor (kFromHz (fMin * halfStep));
    const double kHi = std::ceil  (kFromHz (fMax / halfStep));

    for (int k = (int) kLo; k <= (int) kHi; ++k)
    {
        const double fC = hzFromK ((double) k);
        const double fL = fC / halfStep;
        const double fH = fC * halfStep;
        if (fH < fMin || fL > fMax) continue;

        int binL = (int) std::floor ((fL / sampleRate) * fftSize);
        int binH = (int) std::ceil  ((fH / sampleRate) * fftSize);
        binL = std::max (1, std::min (binL, kMax)); // skip DC for stability
        binH = std::max (1, std::min (binH, kMax));
        if (binH < binL) binH = binL;

        out.push_back (Band{ fL, fH, fC, binL, binH });
    }
    return out;
}

inline juce::Array<Band> makeThirdOctaveBandsJ (double fMin,
                                                double fMax,
                                                double sampleRate,
                                                int    fftSize)
{
    juce::Array<Band> arr;
    auto v = makeThirdOctaveBands (fMin, fMax, sampleRate, fftSize);
    arr.ensureStorageAllocated ((int) v.size());
    for (auto& b : v) arr.add (b);
    return arr;
}

inline std::vector<double> accumulateBandPower (const std::vector<double>& mag2Half,
                                                const std::vector<Band>& bands)
{
    std::vector<double> bandPow;
    bandPow.reserve (bands.size());
    const int K = (int) mag2Half.size();
    for (const auto& b : bands)
    {
        const int a = std::max (0, std::min (b.kStart, K - 1));
        const int z = std::max (0, std::min (b.kEnd,   K - 1));
        double s = 0.0;
        for (int k = a; k <= z; ++k) s += mag2Half[(size_t) k];
        bandPow.push_back (s);
    }
    return bandPow;
}

inline juce::Array<double> accumulateBandPowerJ (const juce::Array<double>& mag2Half,
                                                 const juce::Array<Band>&   bands)
{
    juce::Array<double> out; out.ensureStorageAllocated (bands.size());
    const int K = mag2Half.size();
    for (const auto& b : bands)
    {
        const int a = std::max (0, std::min (b.kStart, K - 1));
        const int z = std::max (0, std::min (b.kEnd,   K - 1));
        double s = 0.0;
        for (int k = a; k <= z; ++k) s += mag2Half[k];
        out.add (s);
    }
    return out;
}

template <typename Float>
inline juce::Array<double> accumulateBandPowerFromBuffer (const juce::AudioBuffer<Float>& halfPower,
                                                          int ch,
                                                          const juce::Array<Band>& bands)
{
    jassert (ch >= 0 && ch < halfPower.getNumChannels());
    const Float* p = halfPower.getReadPointer (ch);
    const int K = halfPower.getNumSamples();
    juce::Array<double> out; out.ensureStorageAllocated (bands.size());
    for (const auto& b : bands)
    {
        const int a = std::max (0, std::min (b.kStart, K - 1));
        const int z = std::max (0, std::min (b.kEnd,   K - 1));
        double s = 0.0;
        for (int k = a; k <= z; ++k) s += (double) p[k];
        out.add (s);
    }
    return out;
}

inline double estimateSlopeDbPerOctJ (const juce::Array<double>& fHz,
                                      const juce::Array<double>& dB,
                                      double pivotHz = 1000.0,
                                      const juce::Array<double>* weights = nullptr)
{
    const int N = std::min (fHz.size(), dB.size());
    if (N < 2) return 0.0;

    auto wAt = [&](int i)->double
    {
        if (weights != nullptr && weights->size() == N)
            return std::max (0.0, (*weights)[i]);
        return 1.0;
    };

    double W=0, xw=0, yw=0, xx=0, xy=0;
    for (int i = 0; i < N; ++i)
    {
        const double f = std::max (1e-3, fHz[i]);
        const double x = std::log2 (f / std::max (1e-3, pivotHz));
        const double y = dB[i];
        const double w = wAt (i);
        W  += w;
        xw += w * x;
        yw += w * y;
    }
    if (W <= 0.0) return 0.0;
    const double xbar = xw / W;
    const double ybar = yw / W;

    for (int i = 0; i < N; ++i)
    {
        const double f = std::max (1e-3, fHz[i]);
        const double x = std::log2 (f / std::max (1e-3, pivotHz));
        const double y = dB[i];
        const double w = wAt (i);
        const double dx = x - xbar;
        xx += w * dx * dx;
        xy += w * dx * (y - ybar);
    }
    if (xx <= 1e-12) return 0.0;
    return xy / xx;
}

inline double crestDbFromPeakRms (double peakAbs, double rms)
{
    if (rms <= 1e-12 || peakAbs <= 1e-12) return 0.0;
    return 20.0 * std::log10 (peakAbs / rms);
}

template <typename Float>
inline void peakAndRms (const juce::AudioBuffer<Float>& buf,
                        double& peakAbsOut,
                        double& rmsOut)
{
    const int ch = buf.getNumChannels();
    const int n  = buf.getNumSamples();
    long double acc = 0.0L;
    double peak = 0.0;
    for (int c = 0; c < ch; ++c)
    {
        const Float* p = buf.getReadPointer (c);
        for (int i = 0; i < n; ++i)
        {
            const double v = (double) p[i];
            acc += (long double) (v * v);
            const double a = std::abs (v);
            if (a > peak) peak = a;
        }
    }
    const long double denom = (long double) std::max (1, n);
    peakAbsOut = peak;
    rmsOut = std::sqrt ((double) (acc / denom));
}

template <typename Float>
inline double spectralFluxNormalizedJ (const juce::AudioBuffer<Float>& prevMag,
                                       const juce::AudioBuffer<Float>& currMag,
                                       int ch = 0)
{
    jassert (prevMag.getNumChannels() > ch && currMag.getNumChannels() > ch);
    const int N = std::min (prevMag.getNumSamples(), currMag.getNumSamples());
    if (N <= 0) return 0.0;
    const Float* a = prevMag.getReadPointer (ch);
    const Float* b = currMag.getReadPointer (ch);
    double num = 0.0, den = 0.0;
    for (int i = 0; i < N; ++i)
    {
        const double d = std::max (0.0, (double) b[i] - (double) a[i]);
        num += d;
        den += std::max (0.0, (double) b[i]);
    }
    if (den <= 1e-12) return 0.0;
    return std::clamp (num / den, 0.0, 1.0);
}

inline double drynessIndex (double crestDb,
                            double fluxNorm,
                            double crestRef = 14.0,
                            double fluxRef  = 0.15,
                            double aCrest   = 0.35,
                            double bFlux    = 2.00)
{
    const auto logistic = [] (double x){ return 1.0 / (1.0 + std::exp (-x)); };
    const double x = aCrest * (crestDb - crestRef) - bFlux * (fluxNorm - fluxRef);
    return std::clamp (logistic (x), 0.0, 1.0);
}

inline void bandMSFromLR (double PL, double PR, double r, double& Em, double& Es)
{
    PL = std::max (0.0, PL); PR = std::max (0.0, PR); r = std::clamp (r, -1.0, 1.0);
    const double cross = 2.0 * r * std::sqrt (PL * PR);
    Em = 0.5 * (PL + PR + cross);
    Es = 0.5 * (PL + PR - cross);
    if (Em < 0.0) Em = 0.0;
    if (Es < 0.0) Es = 0.0;
}

struct LmhSummary
{
    double corrLo{}, corrMid{}, corrHi{}, corrFull{};
    double Wlo{}, Wmid{}, Whi{}; // side-ratio Es / (Em+Es)
    double ELo{}, EMid{}, EHi{}, EAll{};
};

inline LmhSummary summarizeCorrelationWidthJ (const juce::Array<Band>&   bands,
                                              const juce::Array<double>& bandPowL,
                                              const juce::Array<double>& bandPowR,
                                              const juce::Array<double>& bandCorr,
                                              double xoverLoHz,
                                              double xoverHiHz)
{
    LmhSummary S{};
    const int N = std::min ({ bands.size(), bandPowL.size(), bandPowR.size(), bandCorr.size() });
    if (N <= 0) return S;

    double wLo=0, wMid=0, wHi=0, wAll=0;
    double cLo=0, cMid=0, cHi=0, cAll=0;
    double EmLo=0, EsLo=0, EmMid=0, EsMid=0, EmHi=0, EsHi=0, EmA=0, EsA=0;

    for (int i = 0; i < N; ++i)
    {
        const auto& b = bands[i];
        const double PL = std::max (0.0, bandPowL[i]);
        const double PR = std::max (0.0, bandPowR[i]);
        const double Psum = PL + PR;
        if (Psum <= 0.0) continue;

        const double w = Psum;
        const double r = std::clamp (bandCorr[i], -1.0, 1.0);
        double Em=0, Es=0; bandMSFromLR (PL, PR, r, Em, Es);

        auto accum = [&] (double& wAcc, double& cAcc, double& EmAcc, double& EsAcc)
        {
            wAcc += w; cAcc += w * r; EmAcc += Em; EsAcc += Es;
        };

        if      (b.fCenter <  xoverLoHz) accum (wLo,  cLo,  EmLo,  EsLo);
        else if (b.fCenter >= xoverHiHz) accum (wHi,  cHi,  EmHi,  EsHi);
        else                              accum (wMid, cMid, EmMid, EsMid);

        wAll += w; cAll += w * r; EmA += Em; EsA += Es;
    }

    auto mean  = [] (double s, double w){ return (w > 1e-12 ? s / w : 0.0); };
    auto ratio = [] (double num, double den){ return (den > 1e-12 ? std::clamp (num / den, 0.0, 1.0) : 0.0); };

    S.corrLo   = mean (cLo,  wLo);
    S.corrMid  = mean (cMid, wMid);
    S.corrHi   = mean (cHi,  wHi);
    S.corrFull = mean (cAll, wAll);

    const double ELo  = EmLo  + EsLo;
    const double EMid = EmMid + EsMid;
    const double EHi  = EmHi  + EsHi;
    const double EAll = EmA   + EsA;

    S.Wlo  = ratio (EsLo,  ELo);
    S.Wmid = ratio (EsMid, EMid);
    S.Whi  = ratio (EsHi,  EHi);

    S.ELo  = ELo;  S.EMid = EMid;  S.EHi = EHi;  S.EAll = EAll;
    return S;
}

template <typename Float>
inline LmhSummary summarizeFromHalfSpectra (const juce::AudioBuffer<Float>& halfPowerLR,
                                            const juce::Array<Band>& bands,
                                            const juce::Array<double>& bandCorr,
                                            double xoverLoHz,
                                            double xoverHiHz)
{
    jassert (halfPowerLR.getNumChannels() >= 2);
    auto PL = accumulateBandPowerFromBuffer (halfPowerLR, 0, bands);
    auto PR = accumulateBandPowerFromBuffer (halfPowerLR, 1, bands);
    return summarizeCorrelationWidthJ (bands, PL, PR, bandCorr, xoverLoHz, xoverHiHz);
}

} // namespace fieldml


