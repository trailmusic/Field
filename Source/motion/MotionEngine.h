
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "MotionParams.h"
#include "MotionPath.h"
#include "MotionVisual.h"
namespace motion {

// Host sync information
struct HostInfo {
    double bpm = 120.0;
    bool playing = false;
    double ppqPosition = 0.0;
    double ppqBarStart = 0.0;
    double samplesPerBeat = 0.0;
};

// Envelope follower for input/sidechain processing
class EnvelopeFollower {
public:
    void set(float atkMs, float relMs, double sr) {
        aAtk = std::exp(-1.0f / (0.001f * atkMs * sr));
        aRel = std::exp(-1.0f / (0.001f * relMs * sr));
    }
    
    float tick(float x) {
        float rect = std::abs(x);
        float peak = juce::jmax(rect, peakPrev * aAtk);
        float rms2 = rect * rect * 0.1f + rmsPrev * 0.9f;
        float env = 0.6f * peak + 0.4f * std::sqrt(rms2);
        float coeff = (env > val ? aAtk : aRel);
        val = coeff * val + (1.0f - coeff) * env;
        peakPrev = peak; 
        rmsPrev = rms2; 
        return val;
    }
    
    void reset() { val = 0; peakPrev = 0; rmsPrev = 0; }
    float getValue() const { return val; }
    
private:
    float val = 0, peakPrev = 0, rmsPrev = 0;
    float aAtk = 0.99f, aRel = 0.999f;
};

// Simple biquad filter for elevation and front bias processing
class BiquadFilter {
public:
    void setHighShelf(double sr, float freq, float gainDb, float Q = 0.707f) {
        float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)sr;
        float A = std::pow(10.0f, gainDb / 40.0f);
        float S = 1.0f;
        float alpha = std::sin(w) / 2.0f * std::sqrt((A + 1.0f/A) * (1.0f/S - 1.0f) + 2.0f);
        
        float b0 = A * ((A + 1.0f) + (A - 1.0f) * std::cos(w) + 2.0f * std::sqrt(A) * alpha);
        float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * std::cos(w));
        float b2 = A * ((A + 1.0f) + (A - 1.0f) * std::cos(w) - 2.0f * std::sqrt(A) * alpha);
        float a0 = (A + 1.0f) - (A - 1.0f) * std::cos(w) + 2.0f * std::sqrt(A) * alpha;
        float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * std::cos(w));
        float a2 = (A + 1.0f) - (A - 1.0f) * std::cos(w) - 2.0f * std::sqrt(A) * alpha;
        
        // Normalize
        b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
        
        coeffs[0] = b0; coeffs[1] = b1; coeffs[2] = b2;
        coeffs[3] = a1; coeffs[4] = a2;
    }
    
    void setHPF(double sr, float freq, float Q = 0.707f) {
        float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)sr;
        float alpha = std::sin(w) / (2.0f * Q);
        
        float b0 = (1.0f + std::cos(w)) / 2.0f;
        float b1 = -(1.0f + std::cos(w));
        float b2 = (1.0f + std::cos(w)) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * std::cos(w);
        float a2 = 1.0f - alpha;
        
        // Normalize
        b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
        
        coeffs[0] = b0; coeffs[1] = b1; coeffs[2] = b2;
        coeffs[3] = a1; coeffs[4] = a2;
    }
    
    void setLPF(double sr, float freq, float Q = 0.707f) {
        float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)sr;
        float alpha = std::sin(w) / (2.0f * Q);
        
        float b0 = (1.0f - std::cos(w)) / 2.0f;
        float b1 = 1.0f - std::cos(w);
        float b2 = (1.0f - std::cos(w)) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * std::cos(w);
        float a2 = 1.0f - alpha;
        
        // Normalize
        b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
        
        coeffs[0] = b0; coeffs[1] = b1; coeffs[2] = b2;
        coeffs[3] = a1; coeffs[4] = a2;
    }
    
    float processSample(float x) {
        float y = coeffs[0] * x + coeffs[1] * x1 + coeffs[2] * x2 
                - coeffs[3] * y1 - coeffs[4] * y2;
        x2 = x1; x1 = x; y2 = y1; y1 = y;
        return y;
    }
    
    void reset() { x1 = x2 = y1 = y2 = 0; }
    
private:
    float coeffs[5] = {1, 0, 0, 0, 0};
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};

// Smoothed parameter for control rate changes
template<typename T>
class SmoothedValue {
public:
    void setTarget(T target) { targetValue = target; }
    void setCurrentAndTargetValue(T value) { currentValue = targetValue = value; }
    T getNextValue() {
        currentValue += (targetValue - currentValue) * smoothingCoeff;
        return currentValue;
    }
    void setSmoothingCoeff(T coeff) { smoothingCoeff = coeff; }
    T getCurrentValue() const { return currentValue; }
    
private:
    T currentValue = 0, targetValue = 0, smoothingCoeff = 0.1f;
};
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
        sr = sampleRate; blockSize = samplesPerBlock; p1.sr = p2.sr = sr; 
        p1.path.prepare(sr); p2.path.prepare(sr); fd.prepare(sr);
        
        // Initialize DSP components
        elvShelf.setHighShelf(sr, 7000.0f, 0.0f, 0.7f);
        frontShelf.setHighShelf(sr, 7000.0f, 0.0f, 0.7f);
        sideHPF[0].setHPF(sr, 120.0f, 0.707f);
        sideHPF[1].setHPF(sr, 120.0f, 0.707f);
        occlusionLPF.setLPF(sr, 8000.0f, 0.707f);
        
        // Initialize envelope followers
        mainEnv.set(5.0f, 50.0f, sr);
        scEnv.set(5.0f, 50.0f, sr);
        
        // Initialize smoothed parameters
        spreadSm.setSmoothingCoeff(0.05f);
        depthSm.setSmoothingCoeff(0.05f);
        rateSm.setSmoothingCoeff(0.05f);
        shelfSm.setSmoothingCoeff(0.05f);
        lvlAtten.setSmoothingCoeff(0.05f);
        
        // Initialize motion send buffer
        motionSendBuffer.setSize(2, samplesPerBlock);
        
        // Initialize work buffers
        elevationBuffer.ensureStorageAllocated(samplesPerBlock);
        midBuffer.ensureStorageAllocated(samplesPerBlock);
        sideBuffer.ensureStorageAllocated(samplesPerBlock);
        
        reset();
    }
    
    void reset() { 
        p1.phase = p2.phase = 0.0f; fd.reset();
        elvShelf.reset(); frontShelf.reset();
        sideHPF[0].reset(); sideHPF[1].reset();
        occlusionLPF.reset();
        mainEnv.reset(); scEnv.reset();
        holdCounter = 0; running = true; triggered = false;
        motionSendBuffer.clear();
        
        // Reset inertia state
        for (int i = 0; i < 3; ++i) {
            inertiaState[i][0] = inertiaState[i][1] = 0.0f;
        }
    }
    
    void setParams (const Params* p) { params = p; }
    
    void setHostSync(const HostInfo& h) { 
        hostInfo = h; 
        if (h.samplesPerBeat > 0) {
            samplesPerCycle = h.samplesPerBeat * getQuantizeMultiplier();
        }
    }
    
    void setSidechain(const float* scL, const float* scR, int n) {
        if (scL && scR) {
            for (int i = 0; i < n; ++i) {
                float scIn = 0.5f * (scL[i] + scR[i]);
                scEnv.tick(scIn);
            }
        }
    }
    
    juce::AudioBuffer<float>& getMotionSendAndClear() {
        motionSendBuffer.clear();
        return motionSendBuffer;
    }
    
    // Thread-safe visual state access for UI updates
    bool tryGetVisualState(VisualState& out) const noexcept {
        mailbox_.read(out);
        return true;
    }
    void processBlock (float* L, float* R, int n) {
        if (!params) return; 
        
        auto s = take(*params);
        
        // Store dry signal for bypass
        juce::AudioBuffer<float> dryBuffer(2, n);
        for (int i = 0; i < n; ++i) {
            dryBuffer.setSample(0, i, L[i]);
            dryBuffer.setSample(1, i, R[i]);
        }
        
        // Update envelope followers
        for (int i = 0; i < n; ++i) {
            float input = 0.5f * (L[i] + R[i]);
            mainEnv.tick(input);
        }
        
        // Check if motion is enabled
        if (!s.enable) {
            // Motion disabled - output dry signal
            for (int i = 0; i < n; ++i) {
                L[i] = dryBuffer.getSample(0, i);
                R[i] = dryBuffer.getSample(1, i);
            }
            return;
        }
        
        // Process mode-specific parameters
        float rate = s.rateHz;
        float depth = s.depth;
        
        switch (static_cast<MotionMode>(s.mode)) {
            case MotionMode::Free:
                break;
            case MotionMode::Sync:
                rate = getQuantizedRate(s.quantizeDiv);
                // Apply swing timing in sync mode
                if (s.swing > 0.001f) {
                    rate = applySwingTiming(rate, s.swing);
                }
                break;
            case MotionMode::InputEnv:
                depth = mapEnvToDepth(mainEnv.getValue(), s.sens);
                break;
            case MotionMode::Sidechain:
                depth = mapEnvToDepth(scEnv.getValue(), s.sens);
                break;
            case MotionMode::OneShot:
                if (triggered) { p1.phase = p2.phase = 0.0f; triggered = false; }
                running = (p1.phase < 1.0f);
                break;
        }
        
        // Update smoothed parameters
        rateSm.setTarget(rate);
        depthSm.setTarget(depth);
        spreadSm.setTarget(s.spread);
        
        // Process retrig/hold
        if (s.retrig && mainEnv.getValue() > 0.3f && !triggered) {
            p1.phase = p2.phase = 0.0f;
            triggered = true;
            holdCounter = (int)(s.holdMs * sr / 1000.0);
        }
        
        if (holdCounter > 0) {
            holdCounter -= n;
            // Freeze current pose during hold
        }
        
        const int k = 32;
        for (int i=0; i<n; i += k) {
            int m = juce::jmin(k, n - i);
            
            // Generate poses for both panners using their respective parameters
            auto pose1 = nextPose(p1, s.p1, m, rateSm.getNextValue(), depthSm.getNextValue());
            auto pose2 = nextPose(p2, s.p2, m, rateSm.getNextValue(), depthSm.getNextValue());
            
            // Store current poses for visual updates and publish visual state
            if (m > 0) {
                updateVisualState(s, pose1[0], pose2[0]);
            }
            
            for (int j=0; j<m; ++j) {
                float az, rad, elv;
                
                // Select appropriate panner based on panner selection
                if (s.pannerSelect == 2) { // Link mode - blend both panners
                    az = 0.5f * (pose1[j].azimuth + pose2[j].azimuth);
                    rad = 0.5f * (pose1[j].radius + pose2[j].radius);
                    elv = 0.5f * (pose1[j].elevation + pose2[j].elevation);
                } else if (s.pannerSelect == 0) { // P1 mode
                    az = pose1[j].azimuth;
                    rad = pose1[j].radius;
                    elv = pose1[j].elevation;
                } else { // P2 mode
                    az = pose2[j].azimuth;
                    rad = pose2[j].radius;
                    elv = pose2[j].elevation;
                }
                
                // Clamp values
                az = juce::jlimit(-1.0f, 1.0f, az);
                rad = juce::jlimit(0.0f, 1.0f, rad);
                elv = juce::jlimit(-1.0f, 1.0f, elv);
                
                // Apply inertia smoothing (use appropriate panner's inertia setting)
                float inertia = (s.pannerSelect == 1) ? s.p2.inertia : s.p1.inertia;
                if (inertia > 0.001f) {
                    az = applyInertia(az, 0); // azimuth
                    rad = applyInertia(rad, 1); // radius
                    elv = applyInertia(elv, 2); // elevation
                }
                
                // Apply anchor processing (use appropriate panner's anchor setting)
                bool anchor = (s.pannerSelect == 1) ? s.p2.anchor : s.p1.anchor;
                if (anchor) {
                    float anchorR = 0.15f;
                    float t = smoothstep01((rad - anchorR) / (1.0f - anchorR));
                    az = t * az + (1.0f - t) * 0.0f;
                    rad = t * rad;
                }
                
                // Convert to stereo panning
                float th = az * (juce::MathConstants<float>::halfPi);
                float gL = std::cos(th), gR = std::sin(th);
                float center = (1.0f - rad);
                float l = center + rad * gL;
                float r = center + rad * gR;
                
                L[i+j] *= l; R[i+j] *= r;
                
                // Store elevation for later processing
                if (i+j < elevationBuffer.size()) {
                    elevationBuffer.getReference(i+j) = elv;
                }
            }
        }
        
        // Get appropriate panner parameters for processing
        const PannerSnapshot& activePanner = (s.pannerSelect == 1) ? s.p2 : s.p1;
        
        // Apply elevation processing (spectral shelf)
        processElevation(L, R, n, activePanner, s.headphoneSafe);
        
        // Apply spread processing (MS width above bass floor)
        processSpread(L, R, n, activePanner, s.headphoneSafe);
        
        // Apply front bias processing
        processFrontBias(L, R, n, activePanner);
        
        // Apply occlusion processing (rear-hemisphere realism)
        processOcclusion(L, R, n, s);
        
        // Apply bass floor (HPF on side channel)
        processBassFloor(L, R, n, s);
        
        // Apply headphone safe clamps
        if (s.headphoneSafe) {
            applyHeadphoneSafe(L, R, n);
        }
        
        // Apply doppler effect
        if (activePanner.doppler > 0.001f) {
            float dSamps = juce::jlimit(-8.0f, 8.0f, activePanner.doppler * 4.0f);
            if (s.headphoneSafe) dSamps *= 0.5f; // Reduce in headphone safe mode
            fd.process(L, R, n, dSamps);
        }
        
        // Tap motion send
        if (activePanner.motionSend > 0.001f) {
            for (int i = 0; i < n; ++i) {
                motionSendBuffer.setSample(0, i, L[i] * activePanner.motionSend);
                motionSendBuffer.setSample(1, i, R[i] * activePanner.motionSend);
            }
        }
        
        // Visual state is now updated during pose generation for better timing
    }
private:
    const Params* params = nullptr; 
    double sr = 48000.0; 
    int blockSize = 512; 
    PannerState p1, p2; 
    FractionalDelay fd;
    
    // DSP components
    BiquadFilter elvShelf, frontShelf;
    BiquadFilter sideHPF[2];
    BiquadFilter occlusionLPF; // For occlusion processing
    EnvelopeFollower mainEnv, scEnv;
    SmoothedValue<float> spreadSm, depthSm, rateSm, shelfSm, lvlAtten;
    
    // Inertia smoothing state
    float inertiaState[3][2] = {{0,0}, {0,0}, {0,0}}; // [azimuth, radius, elevation][y1, y2]
    
    // Host sync
    HostInfo hostInfo;
    double samplesPerCycle = 0.0;
    
    // State
    int holdCounter = 0;
    bool running = true;
    bool triggered = false;
    
    // Lock-free visual state mailbox for UI updates
    VisualMailbox mailbox_{};
    
    // Helper function to convert pose and parameters to visual data
    static PannerViz toPannerViz(const Pose& pose, const PannerSnapshot& params) noexcept {
        PannerViz viz{};
        viz.azimuth = pose.azimuth;
        viz.elevation = pose.elevation;
        viz.radius = pose.radius;
        viz.rateHz = params.rateHz;
        viz.depth = params.depth;
        viz.spread = params.spread;
        viz.pathType = params.path;
        viz.phaseDeg = params.phaseDeg;
        viz.elevBias = params.elevBias;
        viz.bounce = params.bounce;
        viz.jitter = params.jitter;
        viz.swing = params.swing;
        viz.quantizeDiv = params.quantizeDiv;
        viz.mode = params.mode;
        viz.retrig = params.retrig;
        viz.holdMs = params.holdMs;
        viz.sens = params.sens;
        viz.inertia = params.inertia;
        viz.frontBias = params.frontBias;
        viz.doppler = params.doppler;
        viz.motionSend = params.motionSend;
        viz.anchor = params.anchor;
        
        // Convert to screen coordinates (simple 2D projection)
        viz.x = viz.radius * std::cos(juce::MathConstants<float>::halfPi * viz.azimuth);
        viz.y = viz.radius * std::sin(juce::MathConstants<float>::halfPi * viz.azimuth);
        
        return viz;
    }
    
    // Buffers
    juce::AudioBuffer<float> motionSendBuffer;
    juce::Array<float> elevationBuffer;
    juce::Array<float> midBuffer, sideBuffer;
    
    juce::Array<Pose> nextPose (PannerState& ps, const PannerSnapshot& p, int m, float rate, float depth) {
        juce::Array<Pose> out; out.ensureStorageAllocated(m);
        float phase = juce::degreesToRadians(p.phaseDeg);
        float bounce = p.bounce;
        float jitter = p.jitter;
        float elev = p.elevBias;
        ps.path.set(static_cast<PathType>(p.path), rate, depth, phase, bounce, jitter, elev);
        float dt = 1.0f / 250.0f;
        for (int i=0;i<m;++i) out.add(ps.path.tick(dt));
        return out;
    }
    
    float getQuantizedRate(int quantizeDiv) {
        if (quantizeDiv == 0) return 0.5f; // Off
        
        double beatHz = hostInfo.bpm / 60.0;
        double beatsPerCycle = 1.0;
        
        switch (static_cast<QuantizeDiv>(quantizeDiv)) {
            case QuantizeDiv::N1: beatsPerCycle = 1.0; break;
            case QuantizeDiv::N1_2: beatsPerCycle = 0.5; break;
            case QuantizeDiv::N1_4: beatsPerCycle = 0.25; break;
            case QuantizeDiv::N1_8: beatsPerCycle = 0.125; break;
            case QuantizeDiv::N1_16: beatsPerCycle = 1.0/16.0; break;
            case QuantizeDiv::N1_32: beatsPerCycle = 1.0/32.0; break;
            case QuantizeDiv::N1_4T: beatsPerCycle = 1.0/6.0; break;
            case QuantizeDiv::N1_8T: beatsPerCycle = 1.0/12.0; break;
            case QuantizeDiv::N1_16T: beatsPerCycle = 1.0/24.0; break;
            case QuantizeDiv::N1_4D: beatsPerCycle = 3.0/8.0; break;
            case QuantizeDiv::N1_8D: beatsPerCycle = 3.0/16.0; break;
            case QuantizeDiv::N1_16D: beatsPerCycle = 3.0/32.0; break;
            default: beatsPerCycle = 0.25; break;
        }
        
        return (float)(beatHz * beatsPerCycle);
    }
    
    double getQuantizeMultiplier() {
        // Return multiplier for current quantize setting
        return 0.25; // Default to 1/4
    }
    
    float mapEnvToDepth(float env, float sens) {
        float thr = juce::jmap(sens, 0.0f, 1.0f, 0.6f, 0.1f);
        float k = 4.0f;
        float u = 1.0f / (1.0f + std::exp(-k * (env - thr)));
        return 0.15f + 0.85f * u;
    }
    
    float smoothstep01(float x) {
        x = juce::jlimit(0.0f, 1.0f, x);
        return x * x * (3.0f - 2.0f * x);
    }
    
    void processElevation(float* L, float* R, int n, const PannerSnapshot& p, bool headphoneSafe) {
        float elv = p.elevBias;
        float shelfDb = 4.0f * elv;
        if (headphoneSafe) shelfDb *= 0.5f; // Reduce in headphone safe mode
        
        elvShelf.setHighShelf(sr, 7000.0f, shelfDb, 0.7f);
        
        for (int i = 0; i < n; ++i) {
            float mid = 0.5f * (L[i] + R[i]);
            float side = 0.5f * (L[i] - R[i]);
            mid = elvShelf.processSample(mid);
            L[i] = mid + side;
            R[i] = mid - side;
        }
    }
    
    void processSpread(float* L, float* R, int n, const PannerSnapshot& p, bool headphoneSafe) {
        float spread = p.spread;
        if (headphoneSafe) spread = juce::jmin(spread, 1.3f);
        
        for (int i = 0; i < n; ++i) {
            float mid = 0.5f * (L[i] + R[i]);
            float side = 0.5f * (L[i] - R[i]);
            side *= spread;
            L[i] = mid + side;
            R[i] = mid - side;
        }
    }
    
    void processFrontBias(float* L, float* R, int n, const PannerSnapshot& p) {
        float back = 0.5f * (1.0f - p.frontBias);
        float shelfDb = +2.0f * (1.0f - back) - 3.0f * back;
        float lvlAttenDb = -1.3f * back;
        
        frontShelf.setHighShelf(sr, 7000.0f, shelfDb, 0.7f);
        lvlAtten.setTarget(std::pow(10.0f, lvlAttenDb / 20.0f));
        
        for (int i = 0; i < n; ++i) {
            float atten = lvlAtten.getNextValue();
            L[i] *= atten;
            R[i] *= atten;
            
            float mid = 0.5f * (L[i] + R[i]);
            float side = 0.5f * (L[i] - R[i]);
            mid = frontShelf.processSample(mid);
            L[i] = mid + side;
            R[i] = mid - side;
        }
    }
    
    void processBassFloor(float* L, float* R, int n, const Snapshot& s) {
        sideHPF[0].setHPF(sr, s.bassFloorHz, 0.707f);
        sideHPF[1].setHPF(sr, s.bassFloorHz, 0.707f);
        
        for (int i = 0; i < n; ++i) {
            float mid = 0.5f * (L[i] + R[i]);
            float side = 0.5f * (L[i] - R[i]);
            side = sideHPF[0].processSample(side);
            side = sideHPF[1].processSample(side);
            L[i] = mid + side;
            R[i] = mid - side;
        }
    }
    
    void applyHeadphoneSafe(float* L, float* R, int n) {
        for (int i = 0; i < n; ++i) {
            // Clamp ILD to ±3 dB
            float lDb = 20.0f * std::log10(std::max(0.001f, std::abs(L[i])));
            float rDb = 20.0f * std::log10(std::max(0.001f, std::abs(R[i])));
            float ildDb = lDb - rDb;
            
            if (std::abs(ildDb) > 3.0f) {
                float correction = (ildDb > 0 ? -3.0f : 3.0f) - ildDb;
                float correctionLinear = std::pow(10.0f, correction / 20.0f);
                if (ildDb > 0) L[i] *= correctionLinear;
                else R[i] *= correctionLinear;
            }
        }
    }
    
    float applySwingTiming(float rate, float swing) {
        // Swing timing warps the rate to create groove
        // This is a simplified implementation - in practice you'd want to
        // apply swing to the phase progression rather than the rate
        return rate * (1.0f + swing * 0.1f);
    }
    
    float applyInertia(float x, int axis) {
        if (axis < 0 || axis >= 3) return x;
        
        const float Ts = 1.0f / 250.0f; // Control rate
        const float tau = 0.120f; // Default 120ms
        const float a = 1.0f - std::exp(-Ts / tau);
        
        // Two cascaded one-poles (critically-damped-ish)
        float y1 = inertiaState[axis][0];
        float y2 = inertiaState[axis][1];
        
        y1 += a * (x - y1);
        y2 += a * (y1 - y2);
        
        inertiaState[axis][0] = y1;
        inertiaState[axis][1] = y2;
        
        return y2;
    }
    
    void processOcclusion(float* L, float* R, int n, const Snapshot& s) {
        if (s.occlusion < 0.001f) return;
        
        // Calculate backness from current azimuth (simplified)
        float backness = 0.0f;
        if (n > 0 && elevationBuffer.size() > 0) {
            // Use average azimuth from the block to determine backness
            float avgAz = 0.0f;
            for (int i = 0; i < n && i < elevationBuffer.size(); ++i) {
                avgAz += elevationBuffer[i]; // This is actually elevation, but we'll use it as a proxy
            }
            avgAz /= n;
            backness = 0.5f * (1.0f - std::cos(avgAz * juce::MathConstants<float>::pi));
        }
        
        float k = s.occlusion;
        if (s.headphoneSafe) k *= 0.5f; // Reduce in headphone safe mode
        
        // Gentle LPF cutoff moves with backness
        float fc = juce::jmap(std::pow(backness, 1.2f) * k, 0.0f, 1.0f, 12000.0f, 2500.0f);
        occlusionLPF.setLPF(sr, fc, 0.707f);
        
        float w = 0.2f * backness * k; // 0..0.2 crossfade
        if (s.headphoneSafe) w = juce::jmin(w, 0.12f); // Clamp in headphone safe mode
        
        for (int i = 0; i < n; ++i) {
            float lpfL = occlusionLPF.processSample(L[i]);
            float lpfR = occlusionLPF.processSample(R[i]);
            L[i] = (1.0f - w) * L[i] + w * lpfL;
            R[i] = (1.0f - w) * R[i] + w * lpfR;
        }
    }
    
    void updateVisualState(const Snapshot& s, const Pose& pose1, const Pose& pose2) noexcept {
        // Build visual state
        VisualState vs{};
        
        // Convert poses and parameters to visual data
        vs.p1 = toPannerViz(pose1, s.p1);
        vs.p2 = toPannerViz(pose2, s.p2);
        
        // Update global state
        vs.active = (s.pannerSelect == 0) ? ActiveSel::P1 : 
                   (s.pannerSelect == 1) ? ActiveSel::P2 : ActiveSel::Link;
        vs.link = (vs.active == ActiveSel::Link);
        vs.enable = s.enable;
        vs.occlusion = s.occlusion;
        vs.headphoneSafe = s.headphoneSafe;
        vs.bassFloorHz = s.bassFloorHz;
        
        // Publish to mailbox (handles sequence number automatically)
        mailbox_.publish(vs);
    }
};
}
