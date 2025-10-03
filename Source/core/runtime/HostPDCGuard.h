#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

namespace field { namespace core { namespace runtime {

struct HostPDCGuard
{
	void reset() noexcept { desired_.store(0); applied_.store(0); playing_.store(false); }

	void setDesired (int samples) noexcept { desired_.store(samples, std::memory_order_release); }
	int  getDesired() const noexcept { return desired_.load(std::memory_order_acquire); }
	int  getApplied() const noexcept { return applied_.load(std::memory_order_acquire); }

	void applyIfChanged (juce::AudioProcessor& proc, bool transportStopped)
	{
		playing_.store(!transportStopped, std::memory_order_release);
		const int want = getDesired();
		const int have = getApplied();
		if (want != have)
		{
			if (!playing_.load(std::memory_order_acquire))
			{
				proc.setLatencySamples(want);
				applied_.store(want, std::memory_order_release);
				DBG("[PDC] applied host latency = " << want);
			}
			else
			{
				DBG("[PDC] defer latency change while playing (have=" << have << " want=" << want << ")");
			}
		}
	}
private:
	std::atomic<int>  desired_{0};
	std::atomic<int>  applied_{0};
	std::atomic<bool> playing_{false};
};

}}} // namespace field::core::runtime
