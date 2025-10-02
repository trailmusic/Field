// batch_minphase.cpp
// Batch min-phase FIR generator: multiple inputs → one MinPhaseBank.h (plus optional CSVs)

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
#include <cctype>

#include "fft_real.h" // KissFFT wrapper (rfft/irfft) from previous message

static void usage() {
    std::cout <<
R"(Batch Min-Phase FIR Generator
Usage:
  batch_minphase --out-header MinPhaseBank.h --prefix HB --normalize (none|unity|dc) [--emit-csv]
                     --in HB63_linear.csv HB95_linear.csv HB127_linear.csv [...]

Notes:
  * Each input CSV should contain one FIR tap per line (linear-phase).
  * Order is inferred from filename by scanning digits (e.g., '63' in HB63_linear.csv).
  * Output header defines:
        namespace MinPhaseBank {
            struct TapSet { const double* data; int length; int order; };
            extern const TapSet registry[];
            extern const int registryCount;
        }
)" << std::endl;
}

enum class NormMode { None, Unity, DC };

static std::vector<double> readCsvColumn(const std::string& path) {
    std::ifstream in(path);
    if (!in.good()) { std::cerr << "Cannot open: " << path << "\n"; std::exit(1); }
    std::vector<double> taps;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        // use first column
        std::stringstream ss(line);
        std::string cell;
        if (!std::getline(ss, cell, ',')) continue;
        try { taps.push_back(std::stod(cell)); }
        catch (...) { /* ignore parse errors */ }
    }
    if (taps.size() < 3) { std::cerr << "Need >= 3 taps in " << path << "\n"; std::exit(1); }
    return taps;
}

static size_t nextPow2(size_t v) { size_t p=1; while (p < v) p<<=1; return p; }

struct Item {
    std::string inPath;
    std::string baseName; // e.g., "HB63"
    int order = 0;        // parsed from filename digits
    std::vector<double> linear;   // input FIR
    std::vector<double> minphase; // output FIR
};

static int parseOrderFromFilename(const std::string& path) {
    int acc = 0, found = 0;
    for (char ch : path) {
        if (std::isdigit((unsigned char)ch)) { acc = acc*10 + (ch - '0'); found = 1; }
        else if (found) break;
    }
    return found ? acc : 0;
}

static void rfft_vec(const std::vector<double>& x, int N, std::vector<std::complex<double>>& X) {
    X.resize(N);
    rfft(x.data(), N, X.data());
}
static void irfft_vec(const std::vector<std::complex<double>>& X, int N, std::vector<double>& x) {
    x.resize(N);
    irfft(X.data(), N, x.data());
}

// Linear-phase → Minimum-phase via real-cepstrum liftering
static std::vector<double> linearToMinPhase(const std::vector<double>& h, int fftN)
{
    const int N = (int)h.size();
    const double eps = 1e-20;

    // 1) FFT of h
    std::vector<double> x(fftN, 0.0);
    for (int n=0; n<N; ++n) x[n] = h[n];
    std::vector<std::complex<double>> H;
    rfft_vec(x, fftN, H);

    // 2) log |H|
    std::vector<std::complex<double>> logH(fftN);
    for (int k=0; k<fftN; ++k) {
        double mag = std::abs(H[k]);
        if (!(mag>eps)) mag = eps;
        logH[k] = std::log(std::complex<double>(mag, 0.0));
    }

    // 3) IFFT -> real cepstrum
    std::vector<double> cep;
    irfft_vec(logH, fftN, cep); // returns unnormalized (kiss), will scale later

    // 4) lifter -> min-phase: c[0] unchanged; c[n>0]*=2; c[n<0]=0
    // After above irfft, cep is length N; negative-quefrencies implicit.
    // Double the *positive* indices (1..FFT/2-1). Zero the upper half.
    // We'll operate directly on 'cep' real seq representing 0..N-1.
    // Double 1..(FFT/2 - 1)
    const int half = fftN/2;
    for (int n=1; n<half; ++n) cep[n] *= 2.0;
    for (int n=half; n<fftN; ++n) cep[n] = 0.0;

    // 5) FFT(cep) and exponentiate
    std::vector<std::complex<double>> C;
    rfft_vec(cep, fftN, C);
    std::vector<std::complex<double>> G(fftN);
    for (int k=0; k<fftN; ++k) G[k] = std::exp(C[k]);

    // 6) IFFT -> g, take first N, scale by 1/FFT
    std::vector<double> g;
    irfft_vec(G, fftN, g);
    const double scale = 1.0 / (double)fftN;
    std::vector<double> gN(N);
    for (int n=0; n<N; ++n) gN[n] = g[n] * scale;
    return gN;
}

static void writeCsv(const std::string& path, const std::vector<double>& v) {
    std::ofstream out(path, std::ios::trunc);
    for (auto x : v) out << std::setprecision(17) << x << "\n";
}

static void writeHeader(const std::string& path, const std::vector<Item>& items, const std::string& prefix) {
    std::ofstream out(path, std::ios::trunc);
    out << "#pragma once\n#include <array>\n#include <cstddef>\n\n";
    out << "namespace MinPhaseBank {\n";
    out << "struct TapSet { const double* data; int length; int order; };\n\n";

    // Emit constexpr arrays
    for (const auto& item : items) {
        out << "constexpr std::array<double, " << item.minphase.size() << "> " << item.baseName << "_min = {";
        for (size_t i=0; i<item.minphase.size(); ++i) {
            if (i%6==0) out << "\n  ";
            out << std::setprecision(17) << item.minphase[i];
            if (i+1<item.minphase.size()) out << ", ";
        }
        out << "\n};\n\n";
    }

    // Registry
    out << "constexpr TapSet registry[] = {\n";
    for (const auto& item : items) {
        out << "  { " << item.baseName << "_min.data(), (int)" << item.baseName << "_min.size(), " << item.order << " },\n";
    }
    out << "};\n";
    out << "constexpr int registryCount = (int)(sizeof(registry)/sizeof(registry[0]));\n";
    out << "}\n";
}

int main(int argc, char** argv) {
    std::string outHeader = "MinPhaseBank.h";
    std::string prefix = "HB";
    NormMode nm = NormMode::None;
    bool emitCsv = false;
    std::vector<std::string> inputs;

    for (int i=1; i<argc; ++i) {
        std::string a = argv[i];
        if (a=="--out-header" && i+1<argc) outHeader = argv[++i];
        else if (a=="--prefix" && i+1<argc) prefix = argv[++i];
        else if (a=="--normalize" && i+1<argc) {
            std::string m = argv[++i];
            if (m=="unity") nm = NormMode::Unity;
            else if (m=="dc") nm = NormMode::DC;
            else nm = NormMode::None;
        } else if (a=="--emit-csv") emitCsv = true;
        else if (a=="--in") {
            // collect all remaining args as input files
            for (int j=i+1; j<argc; ++j) inputs.push_back(argv[j]);
            break;
        } else if (a=="--help" || a=="-h") { usage(); return 0; }
    }

    if (inputs.empty()) { usage(); return 1; }

    std::vector<Item> items;
    for (const auto& path : inputs) {
        Item item;
        item.inPath = path;
        item.order = parseOrderFromFilename(path);
        if (item.order == 0) {
            std::cerr << "Could not parse order from: " << path << "\n"; continue;
        }

        // Extract base name (e.g., "HB63" from "HB63_linear.csv")
        std::string base = path;
        size_t dot = base.find_last_of('.');
        if (dot != std::string::npos) base = base.substr(0, dot);
        size_t under = base.find_last_of('_');
        if (under != std::string::npos) base = base.substr(0, under);
        item.baseName = prefix + std::to_string(item.order);

        item.linear = readCsvColumn(path);
        const int fftN = (int)nextPow2((size_t)2*item.linear.size());
        item.minphase = linearToMinPhase(item.linear, fftN);

        // Normalize
        if (nm == NormMode::Unity) {
            double sumsq=0.0; for (double v: item.minphase) sumsq += v*v;
            const double norm = 1.0 / std::sqrt(std::max(1e-30, sumsq));
            for (auto& v: item.minphase) v *= norm;
        } else if (nm == NormMode::DC) {
            double dc = 0.0; for (double v: item.minphase) dc += v;
            if (std::abs(dc) < 1e-12) dc = 1.0;
            const double s = 1.0 / dc;
            for (auto& v: item.minphase) v *= s;
        }

        if (emitCsv) {
            const std::string csvOut = item.baseName + "_min.csv";
            writeCsv(csvOut, item.minphase);
            std::cout << "Wrote: " << csvOut << "\n";
        }

        items.push_back(item);
    }

    if (items.empty()) { std::cerr << "No valid inputs processed\n"; return 1; }

    // Sort by order
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return a.order < b.order; });

    writeHeader(outHeader, items, prefix);
    std::cout << "Generated: " << outHeader << "\n";
    std::cout << "Processed " << items.size() << " filters\n";
    return 0;
}
