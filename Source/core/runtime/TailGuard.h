#pragma once
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>

namespace field { namespace core { namespace runtime {

struct TailGuard
{
	void reset() noexcept { desiredSec_.store(0.0); appliedSec_.store(0.0); playing_.store(false); }

	void setDesiredSeconds (double s) noexcept { desiredSec_.store(s); }
	double getDesiredSeconds() const noexcept { return desiredSec_.load(); }
	double getAppliedSeconds() const noexcept { return appliedSec_.load(); }

	void applyIfChanged (juce::AudioProcessor& proc, bool transportStopped, int reportedLatencySamples)
	{
		playing_.store(!transportStopped);
		const double want = getDesiredSeconds();
		const double have = getAppliedSeconds();
#if JUCE_DEBUG
		if (reportedLatencySamples > 0 && want <= 0.0)
			DBG("[TailGuard] WARN: latency>0 but tailSeconds==0 (verify linear-phase/look-ahead).");
#endif
		if (want != have)
		{
			if (!playing_.load())
			{
				appliedSec_.store(want);
				DBG("[TailGuard] applied tailSeconds = " << want);
			}
			else
			{
				DBG("[TailGuard] defer tailSeconds change while playing (have=" << have << " want=" << want << ")");
			}
		}
	}
private:
	std::atomic<double> desiredSec_{0.0};
	std::atomic<double> appliedSec_{0.0};
	std::atomic<bool>   playing_{false};
};

}}} // namespace field::core::runtime
