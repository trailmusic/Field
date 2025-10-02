#pragma once

#include <JuceHeader.h>

// Debug-only safety sentinels for tracking component lifecycle
#if JUCE_DEBUG

// Timer watchdog - detects any timer still ticking after cleanup
struct TimerSentinel 
{ 
    static std::atomic<int> live; 
    TimerSentinel() { ++live; } 
    ~TimerSentinel() { --live; } 
    static int getLiveCount() { return live.load(); }
};
std::atomic<int> TimerSentinel::live{0};

// Listener watchdog - tracks active listeners
struct ListenerSentinel 
{ 
    static std::atomic<int> live; 
    ListenerSentinel() { ++live; } 
    ~ListenerSentinel() { --live; } 
    static int getLiveCount() { return live.load(); }
};
std::atomic<int> ListenerSentinel::live{0};

// RAII helper for listener management
template <typename Broadcaster, typename Listener>
struct ListenerGroup 
{
    std::vector<std::pair<Broadcaster*, Listener*>> pairs;
    
    ~ListenerGroup() 
    { 
        for (auto& p : pairs) 
        {
            if (p.first && p.second) 
            {
                p.first->removeChangeListener(p.second);
            }
        }
    }
    
    void add(Broadcaster& b, Listener& l) 
    { 
        b.addChangeListener(&l); 
        pairs.emplace_back(&b, &l); 
    }
    
    void removeAll()
    {
        for (auto& p : pairs) 
        {
            if (p.first && p.second) 
            {
                p.first->removeChangeListener(p.second);
            }
        }
        pairs.clear();
    }
};

#else

// Release builds - no overhead
struct TimerSentinel { static int getLiveCount() { return 0; } };
struct ListenerSentinel { static int getLiveCount() { return 0; } };

template <typename Broadcaster, typename Listener>
struct ListenerGroup 
{
    void add(Broadcaster&, Listener&) {}
    void removeAll() {}
};

#endif
