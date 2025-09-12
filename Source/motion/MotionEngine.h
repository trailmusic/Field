
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "MotionParams.h"
#include "MotionPath.h"
namespace motion {
class FractionalDelay {
public:
    void prepare (double sr, int maxMs = 20) {
        sampleRate = sr; int maxSamps = int(sr * maxMs / 1000.0) + 8;
        buffer.setSize(2, maxSamps); buffer.clear(); writePos = 0;
    }
    void reset() { buffer.clear(); writePos = 0; }
    inline void process (float* L, float* R, int n, float delaySamples) {
        const int len = buffer.getNumSamples();
        for (int i=0;i<n;++i) {
            buffer.setSample(0, writePos, L[i]);
            buffer.setSample(1, writePos, R[i]);
            float d = juce::jlimit(-float(len-4), float(len-4), delaySamples);
            float read = float(writePos) - d; while (read < 0) read += len; while (read >= len) read -= len;
            int i0 = int(read); float frac = read - i0;
            auto lag = [this](int ch, int idx, float f)->float{
                auto rd = [&](int pos){ return buffer.getSample(ch, pos); };
                int len = buffer.getNumSamples(); int x0=(idx-1+len)%len, x1=idx, x2=(idx+1)%len, x3=(idx+2)%len;
                float a=rd(x0), b=rd(x1), c=rd(x2), d=rd(x3);
                float c0=(-1.f/6.f)*a + 0.5f*b - 0.5f*c + (1.f/6.f)*d;
                float c1=0.5f*a - b + 0.5f*c;
                float c2=(-1.f/3.f)*a + 0.5f*b + (-1.f/6.f)*d;
                float c3=b; return ((c0*f + c1)*f + c2)*f + c3;
            };
            L[i] = lag(0, i0, frac); R[i] = lag(1, i0, frac);
            if (++writePos >= len) writePos = 0;
        }
    }
private:
    juce::AudioBuffer<float> buffer; double sampleRate=48000.0; int writePos=0;
};
struct PannerState { double sr=48000.0; PathGen path; float phase=0.0f; };
class MotionEngine {
public:
    void prepare (double sampleRate, int samplesPerBlock) {
        sr = sampleRate; blockSize = samplesPerBlock; p1.sr = p2.sr = sr; p1.path.prepare(sr); p2.path.prepare(sr); fd.prepare(sr);
    }
    void reset() { p1.phase = p2.phase = 0.0f; fd.reset(); }
    void setParams (const Params* p) { params = p; }
    void processBlock (float* L, float* R, int n) {
        if (!params) return; auto s = take(*params);
        const int k = 32;
        for (int i=0; i<n; i += k) {
            int m = juce::jmin(k, n - i);
            auto pose1 = nextPose(p1, s, m);
            auto pose2 = nextPose(p2, s, m, true);
            for (int j=0; j<m; ++j) {
                float az = (float)juce::jlimit(-1.0f, 1.0f, (s.pannerSelect == 2 ? 0.5f*(pose1[j].azimuth + pose2[j].azimuth)
                                                                                : (s.pannerSelect == 0 ? pose1[j].azimuth : pose2[j].azimuth)));
                float rad = (float)juce::jlimit(0.0f, 1.0f, (s.pannerSelect == 2 ? 0.5f*(pose1[j].radius + pose2[j].radius)
                                                                                : (s.pannerSelect == 0 ? pose1[j].radius : pose2[j].radius)));
                float elv = (float)juce::jlimit(-1.0f, 1.0f, (s.pannerSelect == 2 ? 0.5f*(pose1[j].elevation + pose2[j].elevation)
                                                                                   : (s.pannerSelect == 0 ? pose1[j].elevation : pose2[j].elevation)));
                float th = az * (juce::MathConstants<float>::halfPi);
                float gL = std::cos(th), gR = std::sin(th);
                float center = (1.0f - rad);
                float l = center + rad * gL;
                float r = center + rad * gR;
                L[i+j] *= l; R[i+j] *= r;
                (void)elv; // TODO: depth cues
            }
        }
        if (params && params->doppler && params->doppler->load() > 0.001f) {
            float dSamps = juce::jlimit(-8.0f, 8.0f, params->doppler->load() * 4.0f);
            fd.process(L, R, n, dSamps);
        }
    }
private:
    const Params* params = nullptr; double sr=48000.0; int blockSize=512; PannerState p1, p2; FractionalDelay fd;
    juce::Array<Pose> nextPose (PannerState& ps, const Snapshot& s, int m, bool isP2=false) {
        juce::Array<Pose> out; out.ensureStorageAllocated(m);
        float rate = s.rateHz;
        float phase = juce::degreesToRadians(s.phaseDeg + (isP2 ? s.offsetDeg : 0.0f));
        float depth = s.depth;
        float bounce = s.bounce;
        float jitter = s.jitter;
        float elev = s.elevBias;
        ps.path.set(static_cast<PathType>(s.path), rate, depth, phase, bounce, jitter, elev);
        float dt = 1.0f / 250.0f;
        for (int i=0;i<m;++i) out.add(ps.path.tick(dt));
        return out;
    }
};
}
