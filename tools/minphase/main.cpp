#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <complex>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "fft_real.h" // provides rfft/irfft for real sequences (you can swap with JUCE FFT if preferred)

static void usage() {
    std::cout <<
R"(MinPhase FIR Generator
Usage:
  minphase --in linear.csv --out-base MyHB63 --normalize unity

Options:
  --in         CSV file of linear-phase FIR taps (1 col), or multiple cols -> first col used
  --out-base   Base name for outputs (produces MyHB63_min.csv and MyHB63_min.h)
  --normalize  (none|unity|dc)  none=leave as-is, unity=normalize L2 to 1, dc=normalize DC gain to 1
  --fft-pad    Optional power-of-two FFT size (>= 2*N). Default: nextpow2(2*N)
Notes:
  * Input FIR should be odd-length linear-phase for best results (halfband OK).
  * Output has same length, minimum-phase, similar magnitude response.)" << std::endl;
}

static std::vector<double> readCsvColumn(const std::string& path) {
    std::ifstream in(path);
    if (!in.good()) { std::cerr << "Cannot open: " << path << "\n"; std::exit(1); }
    std::vector<double> taps;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cell;
        if (!std::getline(ss, cell, ',')) continue;
        taps.push_back(std::stod(cell));
    }
    if (taps.size() < 3) { std::cerr << "Need >= 3 taps\n"; std::exit(1); }
    return taps;
}

static size_t nextPow2(size_t v) { size_t p=1; while (p < v) p<<=1; return p; }

enum class NormMode { None, Unity, DC };

static void writeCsv(const std::string& path, const std::vector<double>& v) {
    std::ofstream out(path, std::ios::trunc);
    for (auto x : v) out << std::setprecision(17) << x << "\n";
}

static void writeHeader(const std::string& path, const std::string& sym, const std::vector<double>& v) {
    std::ofstream out(path, std::ios::trunc);
    out << "#pragma once\n#include <array>\n\n";
    out << "namespace MinPhaseTaps {\n";
    out << "constexpr std::array<double, " << v.size() << "> " << sym << " = {";
    for (size_t i=0;i<v.size();++i) {
        if (i%6==0) out << "\n  ";
        out << std::setprecision(17) << v[i];
        if (i+1<v.size()) out << ", ";
    }
    out << "\n};\n}\n";
}

int main(int argc, char** argv) {
    std::string inCSV, outBase="MinHB";
    NormMode nm = NormMode::None;
    size_t userFFT = 0;

    for (int i=1;i<argc;++i) {
        std::string a = argv[i];
        if (a=="--in" && i+1<argc) inCSV = argv[++i];
        else if (a=="--out-base" && i+1<argc) outBase = argv[++i];
        else if (a=="--normalize" && i+1<argc) {
            std::string m = argv[++i];
            if (m=="unity") nm = NormMode::Unity;
            else if (m=="dc") nm = NormMode::DC;
            else nm = NormMode::None;
        } else if (a=="--fft-pad" && i+1<argc) {
            userFFT = (size_t) std::stoul(argv[++i]);
        } else if (a=="--help" || a=="-h") { usage(); return 0; }
    }
    if (inCSV.empty()) { usage(); return 1; }

    auto h = readCsvColumn(inCSV);
    const int N = (int)h.size();

    // FFT length: nextpow2(2*N) unless provided
    size_t FFTN = userFFT ? userFFT : nextPow2((size_t)2*N);
    if ((FFTN & (FFTN - 1)) != 0) {
        std::cerr << "FFTN must be power-of-two\n"; return 1;
    }

    // 1) FFT of linear-phase taps
    std::vector<std::complex<double>> H;
    {
        std::vector<double> x(FFTN, 0.0);
        for (int n=0;n<N;++n) x[n] = h[n];
        H.resize(FFTN);
        rfft(x.data(), (int)FFTN, H.data()); // real->complex
    }

    // 2) log magnitude
    const double eps = 1e-20;
    std::vector<std::complex<double>> logH(FFTN);
    for (size_t k=0;k<FFTN;++k) {
        double mag = std::abs(H[k]);
        if (!(mag>eps)) mag = eps; // avoid log(0)
        logH[k] = std::log(std::complex<double>(mag, 0.0)); // ln|H|
    }

    // 3) IFFT -> real cepstrum
    std::vector<double> cep(FFTN*2, 0.0); // re/im interleaved for convenience
    {
        // Pack logH into real array as complex
        std::vector<std::complex<double>> tmp = logH;
        irfft(tmp.data(), (int)FFTN, cep.data()); // complex->real, result length FFTN
    }

    // 4) Minimum-phase cepstrum liftering:
    // c[0] unchanged; c[n>0]*=2; c[n<0]=0 (already zero in real IFFT packing).
    for (size_t n=1;n<FFTN/2;++n) cep[n] *= 2.0;
    for (size_t n=FFTN/2;n<FFTN;++n) cep[n] = 0.0;

    // 5) FFT of modified cepstrum -> complex spectrum; exponentiate
    std::vector<std::complex<double>> C(FFTN), G(FFTN);
    {
        // forward real->complex
        rfft(cep.data(), (int)FFTN, C.data());
        for (size_t k=0;k<FFTN;++k) G[k] = std::exp(C[k]);
    }

    // 6) IFFT -> time domain minimum-phase impulse response
    std::vector<double> g(FFTN*2, 0.0);
    irfft(G.data(), (int)FFTN, g.data());

    // 7) take first N taps (causal min-phase)
    std::vector<double> gN(N);
    const double scale = 1.0 / (double)FFTN;
    for (int n=0;n<N;++n) gN[n] = g[n] * scale;

    // 8) optional normalization
    if (nm == NormMode::Unity) {
        double sumsq=0.0; for (double v: gN) sumsq += v*v;
        const double norm = 1.0 / std::sqrt(std::max(1e-30, sumsq));
        for (auto& v: gN) v *= norm;
    } else if (nm == NormMode::DC) {
        // DC gain = sum of taps
        double dc = 0.0; for (double v: gN) dc += v;
        if (std::abs(dc) < 1e-12) dc = 1.0;
        const double s = 1.0 / dc;
        for (auto& v: gN) v *= s;
    }

    // 9) outputs
    const std::string csvOut = outBase + "_min.csv";
    const std::string hOut   = outBase + "_min.h";
    writeCsv(csvOut, gN);

    // Symbol: sanitize base
    std::string sym = outBase;
    for (auto& ch : sym) if (!std::isalnum((unsigned char)ch)) ch = '_';
    sym += "_min";

    writeHeader(hOut, sym, gN);

    std::cout << "Generated min-phase taps:\n  CSV: " << csvOut << "\n  HDR: " << hOut << "\n";
    std::cout << "Length: " << N << " taps; FFTN: " << FFTN << "\n";
    return 0;
}
