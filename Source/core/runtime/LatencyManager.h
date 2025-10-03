#pragma once
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>

// Single-source-of-truth latency reporting. Only apply on the message thread.
struct LatencyManager
{
	void reset() noexcept { desired.store(0, std::memory_order_release); applied.store(0, std::memory_order_release); }

	void setDesired (int samples) noexcept { desired.store (samples, std::memory_order_release); }
	int  getDesired() const noexcept       { return desired.load  (std::memory_order_acquire); }
	int  getApplied() const noexcept       { return applied.load  (std::memory_order_acquire); }

	// Call from prepareToPlay()/graph (message thread). Applies only when changed.
	void applyIfChanged (juce::AudioProcessor& proc)
	{
		const int want = getDesired();
		if (want != getApplied())
		{
			proc.setLatencySamples (want);
			applied.store (want, std::memory_order_release);
		}
	}

private:
	std::atomic<int> desired { 0 };
	std::atomic<int> applied { 0 };
};
