// fft_real.h
#pragma once
// Wrapper around KissFFT's real FFT (kiss_fftr).
// Build with -DKISS_FFT_DOUBLE to use double precision.
// Requires kissfft headers in your include path.

#include <vector>
#include <complex>
#include <cstdlib>
#include <cstring>

extern "C" {
  #include <kiss_fftr.h>   // from KissFFT
}

#if defined(KISS_FFT_DOUBLE)
  using kiss_scalar = double;
#else
  using kiss_scalar = float;
#endif

// RAII plan for reusing forward/inverse configs
struct RealFFTPlan {
    int nfft = 0;
    kiss_fftr_cfg fwd = nullptr;
    kiss_fftr_cfg inv = nullptr;

    RealFFTPlan() = default;
    explicit RealFFTPlan(int N) { prepare(N); }
    ~RealFFTPlan() { destroy(); }

    void prepare(int N) {
        if (N == nfft && fwd && inv) return;
        destroy();
        nfft = N;
        // Last arg 'lenmem' = 0 lets kiss allocate
        fwd = kiss_fftr_alloc(nfft, /*inverse=*/0, nullptr, nullptr);
        inv = kiss_fftr_alloc(nfft, /*inverse=*/1, nullptr, nullptr);
        if (!fwd || !inv) { std::abort(); }
    }
    void destroy() {
        if (fwd) { free(fwd); fwd = nullptr; }
        if (inv) { free(inv); inv = nullptr; }
        nfft = 0;
    }
};

// Forward real→complex FFT
// time: N real samples
// freq: N complex bins (we fill the first N bins as {re,im}, with the KissFFT RFFT layout expanded)
// Note: kiss_fftr actually outputs N/2+1 bins; we mirror to N bins so caller sees a full complex array.
template <typename Real>
inline void rfft(const Real* time, int N, std::complex<Real>* freq)
{
    static_assert(std::is_same<Real, float>::value || std::is_same<Real, double>::value,
                  "Real must be float or double");

    static thread_local RealFFTPlan plan;
    plan.prepare(N);

    // KissFFT real FFT gives N/2+1 complex outputs
    const int nh = N/2 + 1;
    std::vector<kiss_fft_cpx> tmp(nh);
    // Cast input to kiss_scalar
    std::vector<kiss_scalar> x(N);
    for (int i=0; i<N; ++i) x[i] = (kiss_scalar) time[i];

    kiss_fftr(plan.fwd, x.data(), tmp.data());

    // Expand to full N complex spectrum in freq[]:
    // k = 0..nh-1 as provided; the rest are conjugate mirror.
    for (int k=0; k<nh; ++k)
        freq[k] = std::complex<Real>((Real)tmp[k].r, (Real)tmp[k].i);

    for (int k=nh; k<N; ++k) {
        // mirror of bin N-k
        const int m = N - k;
        const auto r = (Real) tmp[m].r;
        const auto i = (Real) tmp[m].i;
        freq[k] = std::complex<Real>(r, (Real)(-i));
    }
}

// Inverse complex→real FFT
// freq: N complex bins (Hermitian for real time-domain)
// time: N real samples (NOTE: KissFFT returns unnormalized; divide by N if you want unitary—your tool already does)
template <typename Real>
inline void irfft(const std::complex<Real>* freq, int N, Real* time)
{
    static_assert(std::is_same<Real, float>::value || std::is_same<Real, double>::value,
                  "Real must be float or double");

    static thread_local RealFFTPlan plan;
    plan.prepare(N);

    const int nh = N/2 + 1;
    std::vector<kiss_fft_cpx> tmp(nh);

    // Pack only the first N/2+1 bins expected by kiss_fftri.
    // freq[0..nh-1] should be the positive-frequency bins.
    for (int k=0; k<nh; ++k) {
        tmp[k].r = (kiss_scalar) freq[k].real();
        tmp[k].i = (kiss_scalar) freq[k].imag();
    }

    std::vector<kiss_scalar> x(N, (kiss_scalar)0);
    kiss_fftri(plan.inv, tmp.data(), x.data());

    for (int i=0; i<N; ++i)
        time[i] = (Real) x[i]; // unnormalized; scale by 1/N outside if desired
}
