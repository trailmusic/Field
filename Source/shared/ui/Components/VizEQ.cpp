#include "VizEQ.h"

namespace VizEQ {

Biquad lowpassRBJ(double Fs, double f0, double Q)
{
    const double w0=2.0*kPI*f0/Fs, c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*Q);
    double b0=(1.0-c)*0.5, b1=1.0-c, b2=(1.0-c)*0.5;
    double a0=1.0+alpha, a1=-2.0*c, a2=1.0-alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b;
}

Biquad highpassRBJ(double Fs, double f0, double Q)
{
    const double w0=2.0*kPI*f0/Fs, c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*Q);
    double b0=(1.0+c)*0.5, b1=-(1.0+c), b2=(1.0+c)*0.5;
    double a0=1.0+alpha, a1=-2.0*c, a2=1.0-alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b;
}

Biquad lowshelfRBJ(double Fs,double f0,double GdB,double S)
{
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/2.0 * std::sqrt((A + 1.0/A)*(1.0/S - 1.0) + 2.0);
    const double twoRtA_alpha = 2.0*std::sqrt(A)*alpha;
    double b0=A*((A+1)-(A-1)*c + twoRtA_alpha);
    double b1=2*A*((A-1)-(A+1)*c);
    double b2=A*((A+1)-(A-1)*c - twoRtA_alpha);
    double a0=   (A+1)+(A-1)*c + twoRtA_alpha;
    double a1=-2*((A-1)+(A+1)*c);
    double a2=   (A+1)+(A-1)*c - twoRtA_alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b;
}

Biquad highshelfRBJ(double Fs,double f0,double GdB,double S)
{
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/2.0 * std::sqrt((A + 1.0/A)*(1.0/S - 1.0) + 2.0);
    const double twoRtA_alpha = 2.0*std::sqrt(A)*alpha;
    double b0=A*((A+1)+(A-1)*c + twoRtA_alpha);
    double b1=-2*A*((A-1)+(A+1)*c);
    double b2=A*((A+1)+(A-1)*c - twoRtA_alpha);
    double a0=   (A+1)-(A-1)*c + twoRtA_alpha;
    double a1= 2*((A-1)-(A+1)*c);
    double a2=   (A+1)-(A-1)*c - twoRtA_alpha;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b;
}

Biquad peakingRBJ_Q(double Fs,double f0,double GdB,double Q)
{
    const double A=std::pow(10.0,GdB/40.0); const double w0=2.0*kPI*f0/Fs; const double c=std::cos(w0), s=std::sin(w0);
    const double alpha = s/(2.0*std::max(1e-6,Q));
    double b0=1.0+alpha*A, b1=-2.0*c, b2=1.0-alpha*A;
    double a0=1.0+alpha/A, a1=-2.0*c, a2=1.0-alpha/A;
    Biquad b{b0,b1,b2,a1,a2}; b.normalize(a0); return b;
}

double softPix(double dB, double knee, double dBmax)
{
    const double s = (1.0 - std::exp(-std::abs(dB)/knee)) * dBmax;
    return dB>=0.0 ? s : -s;
}

} // namespace VizEQ
