#pragma once
#include <atomic>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include "core/params/ParamIDs.h"

namespace field { namespace core { namespace runtime {

class ParamChangeBus : private juce::AudioProcessorValueTreeState::Listener
{
public:
	using APVTS = juce::AudioProcessorValueTreeState;

	explicit ParamChangeBus (APVTS& apvts) : apvts_(apvts)
	{
		topologyIDs_ = {
			field::params::kChainDelayEnable,
			field::params::kChainDynEqEnable,
			field::params::kChainReverbEnable
		};
		latencyIDs_ = {
			field::params::kQualityOSFactor,
			field::params::kReverbLinearPhase,
			field::params::kReverbFIRHalfLen,
			field::params::kDynEqLookAheadMs,
			field::params::kDelayLookAheadMs
		};
		for (auto* list : { &topologyIDs_, &latencyIDs_ })
			for (auto& id : *list)
				apvts_.addParameterListener (id, this);
	}

	~ParamChangeBus() override
	{
		for (auto* list : { &topologyIDs_, &latencyIDs_ })
			for (auto& id : *list)
				apvts_.removeParameterListener (id, this);
	}

	bool consumeTopologyChanged()  noexcept { return topologyChanged_.exchange(false); }
	bool consumeLatencyChanged()   noexcept { return latencyChanged_.exchange(false);  }

	bool peekTopologyChanged() const noexcept { return topologyChanged_.load(); }
	bool peekLatencyChanged()  const noexcept { return latencyChanged_.load();  }

private:
	void parameterChanged (const juce::String& paramID, float) override
	{
		for (auto& id : topologyIDs_) if (paramID == id) { topologyChanged_.store(true); return; }
		for (auto& id : latencyIDs_)  if (paramID == id) { latencyChanged_.store(true);  return; }
	}

	APVTS& apvts_;
	std::vector<juce::String> topologyIDs_, latencyIDs_;
	std::atomic<bool> topologyChanged_{false};
	std::atomic<bool> latencyChanged_{false};
};

}}} // namespace field::core::runtime
