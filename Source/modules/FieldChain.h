#pragma once
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>
#include <cstdint>

#include "../core/signal/NullNode.h"
#include "../core/signal/FrameAccumulator.h"
#include "../core/signal/Sanitize.h"
#include "Mixing/Node_Gain.h"
#include "Mixing/Node_MSMatrix.h"
#include "Mixing/Node_Meter.h"

namespace field { namespace modules {

struct ProcessSpec
{
	double sampleRate = 44100.0;
	int maxBlockSize = 512;
	int numChannels  = 2;
};

struct INode
{
	virtual ~INode() = default;
	virtual void prepare (const ProcessSpec& spec) = 0;
	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample>& block) { processImpl (block); }
	virtual int latencySamples() const noexcept = 0;
private:
	virtual void processImpl (juce::dsp::AudioBlock<float>&)  = 0;
	virtual void processImpl (juce::dsp::AudioBlock<double>&) = 0;
};

struct Node_Null final : public INode
{
	void prepare (const ProcessSpec&) override {}
	int  latencySamples() const noexcept override { return 0; }
private:
	void processImpl (juce::dsp::AudioBlock<float>&)  override {}
	void processImpl (juce::dsp::AudioBlock<double>&) override {}
};

class FieldChain
{
public:
	struct Config
	{
		bool enableReverb = false;
		bool enableDynEq  = false;
		bool enableDelay  = false;
		bool enableMS     = false;
		bool enableGain   = false;
		bool enableMeter  = false;
		int  oversample   = 1;
	};

	void clear()
	{
		nodes.clear();
		totalLatency_ = 0;
	}

	void setBypassed (bool b) noexcept { bypassed_ = b; }
	bool isBypassed() const noexcept { return bypassed_; }

	void setConfig (const Config& cfg) noexcept { config_ = cfg; }
	const Config& getConfig() const noexcept { return config_; }

	void buildFromConfig()
	{
		// Reset optional stage flags
		msActive_ = config_.enableMS;
		gainActive_ = config_.enableGain;
		meterActive_ = config_.enableMeter;
		recalcLatency();
	}

	void prepare (double sampleRate, int maxBlock, int numCh)
	{
		spec_ = { sampleRate, maxBlock, numCh };
		for (auto& n : nodes) n->prepare (spec_);

		if (msActive_)    ms_.prepare<float>   (sampleRate, maxBlock, numCh);
		if (gainActive_)  gain_.prepare<float> (sampleRate, maxBlock, numCh);
		if (meterActive_) meter_.prepare<float>(sampleRate, maxBlock, numCh);
		warmed_ = false;
	}

	template <typename Sample>
	void process (juce::dsp::AudioBlock<Sample> block)
	{
		if (! warmed_) warmed_ = true;
		if (bypassed_) return;
		for (auto& n : nodes) n->process (block);
		if (meterActive_) meter_.process(block);
		if (msActive_)    ms_.process(block);
		if (gainActive_)  gain_.process(block);
	}

	int latencySamples() const noexcept { return totalLatency_; }

	void buildUnity()
	{
		clear();
		nodes.emplace_back (std::make_unique<Node_Null>());
		msActive_ = gainActive_ = meterActive_ = false;
		recalcLatency();
	}

private:
	void recalcLatency()
	{
		int sum = 0;
		for (auto& n : nodes) sum += n->latencySamples();
		// Optional stages currently report 0 latency
		totalLatency_ = sum;
	}

	ProcessSpec spec_{};
	std::vector<std::unique_ptr<INode>> nodes;
	int  totalLatency_ = 0;
	bool bypassed_     = false;
	bool warmed_       = false;

	Config config_{};
	bool msActive_ = false, gainActive_ = false, meterActive_ = false;
	mixing::Node_MSMatrix ms_{};
	mixing::Node_Gain     gain_{};
	mixing::Node_Meter<>  meter_{};
};

}} // namespace field::modules
