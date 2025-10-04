#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>
#include "engines/motion/MotionPath.h"
#include "engines/motion/MotionParams.h"

namespace motion::core {

struct HostInfo {
	double bpm = 120.0;
	bool   playing = false;
	double ppqPosition = 0.0;
	double ppqBarStart = 0.0;
	double samplesPerBeat = 0.0;
};

class EnvelopeFollower {
public:
	void set(float atkMs, float relMs, double sr) {
		aAtk = std::exp(-1.0f / (0.001f * atkMs * (float) sr));
		aRel = std::exp(-1.0f / (0.001f * relMs * (float) sr));
	}
	float tick(float x) {
		float rect = std::abs(x);
		float peak = juce::jmax(rect, peakPrev * aAtk);
		float rms2 = rect * rect * 0.1f + rmsPrev * 0.9f;
		float env = 0.6f * peak + 0.4f * std::sqrt(rms2);
		float coeff = (env > val ? aAtk : aRel);
		val = coeff * val + (1.0f - coeff) * env;
		peakPrev = peak; rmsPrev = rms2; return val;
	}
	void reset() { val = 0; peakPrev = 0; rmsPrev = 0; }
	float getValue() const { return val; }
private:
	float val = 0, peakPrev = 0, rmsPrev = 0;
	float aAtk = 0.99f, aRel = 0.999f;
};

class BiquadFilter {
public:
	void setHighShelf(double sr, float freq, float gainDb, float Q = 0.707f) {
		float w = 2.0f * juce::MathConstants<float>::pi * freq / (float) sr;
		float A = std::pow(10.0f, gainDb / 40.0f);
		float S = 1.0f;
		float alpha = std::sin(w) / 2.0f * std::sqrt((A + 1.0f/A) * (1.0f/S - 1.0f) + 2.0f);
		float b0 = A * ((A + 1.0f) + (A - 1.0f) * std::cos(w) + 2.0f * std::sqrt(A) * alpha);
		float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * std::cos(w));
		float b2 = A * ((A + 1.0f) + (A - 1.0f) * std::cos(w) - 2.0f * std::sqrt(A) * alpha);
		float a0 = (A + 1.0f) - (A - 1.0f) * std::cos(w) + 2.0f * std::sqrt(A) * alpha;
		float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * std::cos(w));
		float a2 = (A + 1.0f) - (A - 1.0f) * std::cos(w) - 2.0f * std::sqrt(A) * alpha;
		b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
		coeffs[0] = b0; coeffs[1] = b1; coeffs[2] = b2; coeffs[3] = a1; coeffs[4] = a2;
	}
	void setHPF(double sr, float freq, float Q = 0.707f) {
		float w = 2.0f * juce::MathConstants<float>::pi * freq / (float) sr;
		float alpha = std::sin(w) / (2.0f * Q);
		float b0 = (1.0f + std::cos(w)) / 2.0f;
		float b1 = -(1.0f + std::cos(w));
		float b2 = (1.0f + std::cos(w)) / 2.0f;
		float a0 = 1.0f + alpha;
		float a1 = -2.0f * std::cos(w);
		float a2 = 1.0f - alpha;
		b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
		coeffs[0] = b0; coeffs[1] = b1; coeffs[2] = b2; coeffs[3] = a1; coeffs[4] = a2;
	}
	void setLPF(double sr, float freq, float Q = 0.707f) {
		float w = 2.0f * juce::MathConstants<float>::pi * freq / (float) sr;
		float alpha = std::sin(w) / (2.0f * Q);
		float b0 = (1.0f - std::cos(w)) / 2.0f;
		float b1 = 1.0f - std::cos(w);
		float b2 = (1.0f - std::cos(w)) / 2.0f;
		float a0 = 1.0f + alpha;
		float a1 = -2.0f * std::cos(w);
		float a2 = 1.0f - alpha;
		b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;
		coeffs[0] = b0; coeffs[1] = b1; coeffs[2] = b2; coeffs[3] = a1; coeffs[4] = a2;
	}
	float processSample(float x) {
		float y = coeffs[0] * x + coeffs[1] * x1 + coeffs[2] * x2 - coeffs[3] * y1 - coeffs[4] * y2;
		x2 = x1; x1 = x; y2 = y1; y1 = y; return y;
	}
	void reset() { x1 = x2 = y1 = y2 = 0; }
private:
	float coeffs[5] = {1, 0, 0, 0, 0};
	float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};

template<typename T>
class SmoothedValue {
public:
	void setTarget(T target) { targetValue = target; }
	void setCurrentAndTargetValue(T value) { currentValue = targetValue = value; }
	T getNextValue() { currentValue += (targetValue - currentValue) * smoothingCoeff; return currentValue; }
	void setSmoothingCoeff(T coeff) { smoothingCoeff = coeff; }
	T getCurrentValue() const { return currentValue; }
private:
	T currentValue = 0, targetValue = 0, smoothingCoeff = (T) 0.1;
};

class FractionalDelay {
public:
	void prepare (double sr, int maxMs = 20) {
		sampleRate = sr; int maxSamps = (int) (sr * maxMs / 1000.0) + 8;
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

} // namespace motion::core
