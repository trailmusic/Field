#pragma once

#include <JuceHeader.h>

namespace VizEQ {
struct Biquad {
    double b0=1, b1=0, b2=0, a1=0, a2=0;
    inline void normalize(double a0){ b0/=a0; b1/=a0; b2/=a0; a1/=a0; a2/=a0; }
    inline double magDB(double w) const {
        const double c1=std::cos(w), s1=std::sin(w);
        const double c2=std::cos(2*w), s2=std::sin(2*w);
        const double NR=b0 + b1*c1 + b2*c2;
        const double NI=     b1*s1 + b2*s2;
        const double DR=1.0 + a1*c1 + a2*c2;
        const double DI=     a1*s1 + a2*s2;
        const double m2=(NR*NR+NI*NI)/(DR*DR+DI*DI);
        return 20.0*std::log10(std::max(1e-12, std::sqrt(m2)));
    }
};

constexpr double kPI = juce::MathConstants<double>::pi;
constexpr double kSqrt2Inv = 0.7071067811865476;

Biquad lowpassRBJ(double Fs, double f0, double Q=kSqrt2Inv);
Biquad highpassRBJ(double Fs, double f0, double Q=kSqrt2Inv);
Biquad lowshelfRBJ(double Fs,double f0,double GdB,double S=1.0);
Biquad highshelfRBJ(double Fs,double f0,double GdB,double S=1.0);
Biquad peakingRBJ_Q(double Fs,double f0,double GdB,double Q);
double softPix(double dB, double knee=6.0, double dBmax=18.0);
} // namespace VizEQ
