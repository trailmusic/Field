
#pragma once
#include <juce_core/juce_core.h>
#include "MotionIDs.h"
namespace motion {
struct Pose { float azimuth=0.0f; float elevation=0.0f; float radius=0.0f; };
class PathGen {
public:
    void prepare (double sampleRate, int controlRateHz = 250) { sr = sampleRate; ctrlHz = (float) controlRateHz; reset(); }
    void reset() { t = 0.0f; rng.setSeedRandomly(); targetTheta = 0.0f; currentTheta = 0.0f; }
    void set (PathType type, float rateHz, float depth, float phase0, float bounce, float jitter, float elevBias) {
        path = type; rate = juce::jlimit (0.01f, 32.0f, rateHz);
        rad = juce::jlimit (0.0f, 1.0f, depth);
        phase = phase0; shape = juce::jlimit (0.0f, 1.0f, bounce);
        jit = juce::jlimit (0.0f, 1.0f, jitter);
        elev = juce::jlimit (-1.0f, 1.0f, elevBias);
    }
    Pose tick (float dt) {
        t += dt;
        float w = 2.0f * juce::MathConstants<float>::pi * rate;
        switch (path) {
            case PathType::Circle: {
                float th = phase + w * t; return { std::sin(th), elev, rad };
            }
            case PathType::Figure8: {
                float th = phase + w * t; return { std::sin(th), std::sin(2.0f*th)*0.6f + elev*0.4f, rad };
            }
            case PathType::Bounce: {
                float tri = 2.0f * std::abs(std::fmod((phase + t*rate), 1.0f) - 0.5f) - 1.0f;
                float s = smoothstep(tri, shape); return { s, elev, rad };
            }
            case PathType::Arc: {
                float tri = 2.0f * std::abs(std::fmod((phase + t*rate), 1.0f) - 0.5f) - 1.0f;
                float span = 0.6f; float s = smoothstep(tri, shape) * span; return { s, elev, rad };
            }
            case PathType::Spiral: {
                float th = phase + w * t; float rmod = rad * (0.6f + 0.4f * std::sin(0.2f * th)); return { std::sin(th), elev, rmod };
            }
            case PathType::Polygon: {
                const int N = 5; float u = std::fmod(phase + t*rate, 1.0f); int idx = int(u * N);
                float th = (idx / (float)N) * 2.0f * juce::MathConstants<float>::pi;
                float next = ((idx+1)%N) / (float)N * 2.0f * juce::MathConstants<float>::pi;
                float gl = 0.2f; float thg = juce::jlimit(th, next, th + gl * (next-th)); return { std::sin(thg), elev, rad };
            }
            case PathType::RandomWalk: {
                if (--hold <= 0) { targetTheta = rng.nextFloat()*2.0f*juce::MathConstants<float>::pi - juce::MathConstants<float>::pi;
                    hold = int(ctrlHz * juce::jmap(rate, 0.1f, 4.0f, 1.0f, 0.125f)); }
                currentTheta += 0.1f * (targetTheta - currentTheta);
                float jitterOff = (jit > 0.0f) ? (rng.nextFloat() - 0.5f) * 0.05f * jit : 0.0f;
                return { std::sin(currentTheta + jitterOff), elev, rad };
            }
            case PathType::UserShape: {
                float th = phase + w * t; return { std::sin(th), elev, rad };
            }
        }
        return {};
    }
private:
    double sr = 48000.0; float ctrlHz = 250.0f;
    PathType path = PathType::Circle; float rate=0.5f, rad=0.5f, phase=0.0f, shape=0.0f, jit=0.0f, elev=0.0f;
    float t = 0.0f; juce::Random rng; int hold = 0; float targetTheta=0.0f, currentTheta=0.0f;
    static float smoothstep(float x, float k) {
        float u = 0.5f * (x + 1.0f); float e = u*u*(3.0f - 2.0f*u);
        float out = juce::jmap(k, 0.0f, 1.0f, u, e); return out * 2.0f - 1.0f;
    }
};
}
