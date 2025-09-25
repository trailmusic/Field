#pragma once
#include <JuceHeader.h>

// Zoom state management with smooth animation
class ZoomState
{
public:
    enum class Mode { Auto, Manual };
    
    ZoomState()
    {
        currentHalfRangeDb = 18.0f;
        targetHalfRangeDb = 18.0f;
        mode = Mode::Manual;
        minHalfRangeDb = 6.0f;
        maxHalfRangeDb = 36.0f;
        animationSpeed = 0.15f; // 0.1 = slow, 0.3 = fast
    }
    
    void prepare (double sampleRate)
    {
        // Prepare for animation updates
        juce::ignoreUnused (sampleRate);
    }
    
    void update()
    {
        // Smooth animation towards target
        const float current = currentHalfRangeDb.load();
        const float target = targetHalfRangeDb.load();
        const float diff = target - current;
        if (std::abs (diff) > 0.1f)
        {
            currentHalfRangeDb.store(current + diff * animationSpeed);
        }
        else
        {
            currentHalfRangeDb.store(target);
        }
    }
    
    // Getters
    float getCurrent() const { return currentHalfRangeDb.load(); }
    float getTarget() const { return targetHalfRangeDb.load(); }
    Mode getMode() const { return mode.load(); }
    float getMin() const { return minHalfRangeDb; }
    float getMax() const { return maxHalfRangeDb; }
    
    // Setters
    void setTarget (float halfRangeDb)
    {
        targetHalfRangeDb.store(juce::jlimit (minHalfRangeDb, maxHalfRangeDb, halfRangeDb));
        if (mode.load() == Mode::Auto)
        {
            currentHalfRangeDb.store(targetHalfRangeDb.load()); // Instant for auto mode
        }
    }
    
    void setMode (Mode newMode)
    {
        mode.store(newMode);
        if (newMode == Mode::Auto)
        {
            // Auto mode - let the system control the zoom
        }
        else
        {
            // Manual mode - user controls the zoom
        }
    }
    
    void setHalfRangeDb (float halfRangeDb)
    {
        setTarget (halfRangeDb);
        setMode (Mode::Manual);
    }
    
    void setAuto (bool on)
    {
        setMode (on ? Mode::Auto : Mode::Manual);
    }
    
    void setAnimationSpeed (float speed)
    {
        animationSpeed = juce::jlimit (0.05f, 0.5f, speed);
    }
    
    // Preset values
    static constexpr float PRESET_6 = 6.0f;
    static constexpr float PRESET_12 = 12.0f;
    static constexpr float PRESET_18 = 18.0f;
    static constexpr float PRESET_24 = 24.0f;
    static constexpr float PRESET_36 = 36.0f;
    
    // Get closest preset
    float getClosestPreset (float value) const
    {
        const float presets[] = { PRESET_6, PRESET_12, PRESET_18, PRESET_24, PRESET_36 };
        float closest = presets[0];
        float minDiff = std::abs (value - presets[0]);
        
        for (int i = 1; i < 5; ++i)
        {
            const float diff = std::abs (value - presets[i]);
            if (diff < minDiff)
            {
                minDiff = diff;
                closest = presets[i];
            }
        }
        
        return closest;
    }
    
    // Check if value is close to a preset
    bool isNearPreset (float value, float tolerance = 1.0f) const
    {
        const float presets[] = { PRESET_6, PRESET_12, PRESET_18, PRESET_24, PRESET_36 };
        for (int i = 0; i < 5; ++i)
        {
            if (std::abs (value - presets[i]) <= tolerance)
                return true;
        }
        return false;
    }
    
private:
    std::atomic<float> currentHalfRangeDb { 18.0f };
    std::atomic<float> targetHalfRangeDb { 18.0f };
    std::atomic<Mode> mode { Mode::Manual };
    float minHalfRangeDb = 6.0f;
    float maxHalfRangeDb = 36.0f;
    float animationSpeed = 0.15f;
};
