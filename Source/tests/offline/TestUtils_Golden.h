#pragma once
#include <cstdint>
#include <vector>
#include <cstring>
#include <random>

namespace field::tests {

inline uint64_t fnv1a64 (const void* data, size_t len) noexcept {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

template<typename Sample>
inline uint64_t hashAudio(const std::vector<Sample>& v) noexcept {
    return fnv1a64(v.data(), v.size() * sizeof(Sample));
}

template<typename Sample>
inline std::vector<Sample> makeDeterministicInput(size_t samples, uint32_t seed = 0xC0FFEEu) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-0.5, 0.5);
    std::vector<Sample> x(samples);
    for (size_t i=0;i<samples;++i) x[i] = static_cast<Sample>(dist(rng));
    return x;
}

template<typename Sample>
inline void monoToStereo(const std::vector<Sample>& mono, std::vector<Sample>& L, std::vector<Sample>& R) {
    L = mono;
    R = mono;
}

} // namespace field::tests


