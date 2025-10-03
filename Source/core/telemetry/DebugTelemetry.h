#pragma once
#include <juce_core/juce_core.h>

// One-line, throttled telemetry. No-ops in Release builds.
// Usage: FIELD_TELEM_THROTTLED("text " << value);

#if JUCE_DEBUG
	#define FIELD_TELEM_THROTTLED(msgStream)                                 \
	do {                                                                      \
		static int __field_telem_counter = 0;                                   \
		if ((++__field_telem_counter & 0x1FFF) == 0) /* ~every 8k calls */     \
		{                                                                      \
			juce::String __s; __s << msgStream;                                  \
			juce::Logger::outputDebugString (__s);                                \
		}                                                                      \
	} while (false)
#else
	#define FIELD_TELEM_THROTTLED(msgStream) do{}while(false)
#endif
