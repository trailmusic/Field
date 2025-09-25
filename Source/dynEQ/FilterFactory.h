#pragma once
#include <cmath>
#include <algorithm>

struct Biquad {
    double b0{1}, b1{0}, b2{0}, a0{1}, a1{0}, a2{0};
};

static inline void normalize(Biquad& c) {
    const double inv = 1.0 / c.a0;
    c.b0*=inv; c.b1*=inv; c.b2*=inv; c.a1*=inv; c.a2*=inv; c.a0 = 1.0;
}

static inline void guardParams(double fs, double& fc, double& Q) {
    fc = std::clamp(fc, 20.0, 0.49 * fs);
    Q  = std::max(Q, 1e-4);
}

// RBJ "cookbook" filters (correct, production-safe)

// Peaking (Bell)
static inline Biquad makePeaking(double fs, double fc, double Q, double gainDb)
{
    guardParams(fs, fc, Q);
    const double A  = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * M_PI * fc / fs;
    const double cw = std::cos(w0), sw = std::sin(w0);
    const double alpha = sw / (2.0 * Q);

    Biquad c;
    c.b0 = 1.0 + alpha * A;
    c.b1 = -2.0 * cw;
    c.b2 = 1.0 - alpha * A;
    c.a0 = 1.0 + alpha / A;      // ← important
    c.a1 = -2.0 * cw;
    c.a2 = 1.0 - alpha / A;
    normalize(c);
    return c;
}

// Low/High Shelves (with slope S, not Q)
static inline Biquad makeLowShelf(double fs, double fc, double gainDb, double S /* ~0.2..1.2 */)
{
    fc = std::clamp(fc, 20.0, 0.49*fs);
    S  = std::clamp(S,  0.2,  1.2);
    const double A  = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * M_PI * fc / fs;
    const double cw = std::cos(w0), sw = std::sin(w0);
    const double alpha = (sw/2.0) * std::sqrt((A + 1.0/A)*(1.0/S - 1.0) + 2.0);
    const double twoSqrtAalpha = 2.0 * std::sqrt(A) * alpha;

    Biquad c;
    c.b0 =    A*((A+1) - (A-1)*cw + twoSqrtAalpha);
    c.b1 =  2*A*((A-1) - (A+1)*cw);
    c.b2 =    A*((A+1) - (A-1)*cw - twoSqrtAalpha);
    c.a0 =       (A+1) + (A-1)*cw + twoSqrtAalpha;
    c.a1 =   -2*((A-1) + (A+1)*cw);
    c.a2 =       (A+1) + (A-1)*cw - twoSqrtAalpha;
    normalize(c);
    return c;
}

static inline Biquad makeHighShelf(double fs, double fc, double gainDb, double S /* ~0.2..1.2 */)
{
    fc = std::clamp(fc, 20.0, 0.49*fs);
    S  = std::clamp(S,  0.2,  1.2);
    const double A  = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * M_PI * fc / fs;
    const double cw = std::cos(w0), sw = std::sin(w0);
    const double alpha = (sw/2.0) * std::sqrt((A + 1.0/A)*(1.0/S - 1.0) + 2.0);
    const double twoSqrtAalpha = 2.0 * std::sqrt(A) * alpha;

    Biquad c;
    c.b0 =    A*((A+1) + (A-1)*cw + twoSqrtAalpha);
    c.b1 = -2*A*((A-1) + (A+1)*cw);
    c.b2 =    A*((A+1) + (A-1)*cw - twoSqrtAalpha);
    c.a0 =       (A+1) - (A-1)*cw + twoSqrtAalpha;
    c.a1 =    2*((A-1) - (A+1)*cw);
    c.a2 =       (A+1) - (A-1)*cw - twoSqrtAalpha;
    normalize(c);
    return c;
}

// Low-pass / High-pass
static inline Biquad makeLowpass(double fs, double fc, double Q)
{
    guardParams(fs, fc, Q);
    const double w0=2.0*M_PI*fc/fs, cw=std::cos(w0), sw=std::sin(w0);
    const double alpha = sw / (2.0 * Q);
    Biquad c;
    c.b0 = (1 - cw) * 0.5; c.b1 = 1 - cw; c.b2 = (1 - cw) * 0.5;
    c.a0 = 1 + alpha;      c.a1 = -2 * cw; c.a2 = 1 - alpha;
    normalize(c); return c;
}

static inline Biquad makeHighpass(double fs, double fc, double Q)
{
    guardParams(fs, fc, Q);
    const double w0=2.0*M_PI*fc/fs, cw=std::cos(w0), sw=std::sin(w0);
    const double alpha = sw / (2.0 * Q);
    Biquad c;
    c.b0 = (1 + cw) * 0.5; c.b1 = -(1 + cw); c.b2 = (1 + cw) * 0.5; 
    c.a0 = 1 + alpha;      c.a1 = -2 * cw;   c.a2 = 1 - alpha;
    normalize(c); return c;
}

// Notch / Band-pass (CSG) / All-pass
static inline Biquad makeNotch(double fs, double fc, double Q)
{
    guardParams(fs, fc, Q);
    const double w0=2.0*M_PI*fc/fs, cw=std::cos(w0), sw=std::sin(w0);
    const double alpha = sw / (2.0 * Q);
    Biquad c;
    c.b0 = 1; c.b1 = -2*cw; c.b2 = 1;
    c.a0 = 1 + alpha; c.a1 = -2*cw; c.a2 = 1 - alpha;
    normalize(c); return c;
}

static inline Biquad makeBandpassCSG(double fs, double fc, double Q)
{
    guardParams(fs, fc, Q);
    const double w0=2.0*M_PI*fc/fs, cw=std::cos(w0), sw=std::sin(w0);
    const double alpha = sw / (2.0 * Q);
    Biquad c;
    c.b0 =  alpha; c.b1 = 0.0; c.b2 = -alpha;   // constant-skirt-gain
    c.a0 = 1+alpha; c.a1 = -2*cw; c.a2 = 1-alpha;
    normalize(c); return c;
}

static inline Biquad makeAllpass(double fs, double fc, double Q)
{
    guardParams(fs, fc, Q);
    const double w0=2.0*M_PI*fc/fs, cw=std::cos(w0), sw=std::sin(w0);
    const double alpha = sw / (2.0 * Q);
    Biquad c;
    c.b0 = 1 - alpha; c.b1 = -2*cw; c.b2 = 1 + alpha;
    c.a0 = 1 + alpha; c.a1 = -2*cw; c.a2 = 1 - alpha;
    normalize(c); return c;
}

// Bandwidth & Slope helpers

// Bell: bandwidth (octaves) → alpha form
static inline Biquad makePeakingBW(double fs, double fc, double BW_oct, double gainDb)
{
    fc = std::clamp(fc, 20.0, 0.49*fs);
    const double A  = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * M_PI * fc / fs;
    const double sw = std::sin(w0);
    const double alpha = sw * std::sinh( (std::log(2.0)/2.0) * BW_oct * w0 / std::max(sw, 1e-9) );
    const double cw = std::cos(w0);

    Biquad c;
    c.b0 = 1.0 + alpha * A;
    c.b1 = -2.0 * cw;
    c.b2 = 1.0 - alpha * A;
    c.a0 = 1.0 + alpha / A;
    c.a1 = -2.0 * cw;
    c.a2 = 1.0 - alpha / A;
    normalize(c);
    return c;
}

// Shelf: map "dB per octave" → RBJ S (musical mapping)
// 12 dB/oct ≈ S=1, clamp to sensible musical range
static inline double mapDbPerOctToS(double dbPerOct)
{
    dbPerOct = std::clamp(dbPerOct, 3.0, 18.0);
    const double t = (dbPerOct - 12.0) / 9.0;   // ~[-1..+1]
    return std::clamp(1.0 + 0.6 * t, 0.2, 1.2);
}

// Dynamic-safe path: TPT/SVF kernel (clean modulation)
struct TPTSVF {
    double g=0, R=0;   // g = tan(pi*fc/fs), R = 1/(2Q)
    double s1=0, s2=0;

    void setParams(double fs, double fc, double Q) {
        fc = std::clamp(fc, 20.0, 0.49*fs);
        Q  = std::max(Q, 1e-4);
        g = std::tan(M_PI * fc / fs);
        R = 1.0 / (2.0 * Q);
    }
    void reset() { s1 = s2 = 0; }

    // One-sample; returns lp/bp/hp simultaneously
    inline void process(double x, double& lp, double& bp, double& hp) {
        const double h = 1.0 / (1.0 + 2.0*R*g + g*g);
        const double v1 = (x - s1 - 2.0*R*s2) * h;
        hp = v1;
        bp = g * v1 + s2;
        lp = g * bp + s1;
        s1 = lp + s1;
        s2 = bp + s2;
    }
};

// Filter types enum
enum class FilterType {
    Bell,
    LowShelf,
    HighShelf,
    Lowpass,
    Highpass,
    Bandpass,
    Notch,
    Allpass
};

// Factory function to create filters by type
static inline Biquad createFilter(FilterType type, double fs, double fc, double Q, double gainDb = 0.0, double S = 1.0) {
    switch (type) {
        case FilterType::Bell:
            return makePeaking(fs, fc, Q, gainDb);
        case FilterType::LowShelf:
            return makeLowShelf(fs, fc, gainDb, S);
        case FilterType::HighShelf:
            return makeHighShelf(fs, fc, gainDb, S);
        case FilterType::Lowpass:
            return makeLowpass(fs, fc, Q);
        case FilterType::Highpass:
            return makeHighpass(fs, fc, Q);
        case FilterType::Bandpass:
            return makeBandpassCSG(fs, fc, Q);
        case FilterType::Notch:
            return makeNotch(fs, fc, Q);
        case FilterType::Allpass:
            return makeAllpass(fs, fc, Q);
        default:
            return Biquad{}; // Identity filter
    }
}
