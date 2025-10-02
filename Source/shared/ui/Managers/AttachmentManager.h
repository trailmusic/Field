#pragma once

#include <JuceHeader.h>

class MyPluginAudioProcessorEditor;

class AttachmentManager
{
public:
    AttachmentManager(MyPluginAudioProcessorEditor& editor);
    ~AttachmentManager() = default;
    
    // Parameter attachment methods
    void attachSliderParameter(const juce::String& parameterID, juce::Slider& slider);
    void attachButtonParameter(const juce::String& parameterID, juce::Button& button);
    void attachComboBoxParameter(const juce::String& parameterID, juce::ComboBox& comboBox);
    
    // Bulk attachment methods
    void attachAllParameters();
    void attachImagingParameters();
    void attachMainControlsParameters();
    void attachDelayParameters();
    void attachEQParameters();
    void attachBypassParameters();
    void attachMotionParameters();
    
    // Parameter detachment
    void detachAllParameters();
    void detachParameter(const juce::String& parameterID);
    
    // Safety checks
    bool isParameterValid(const juce::String& parameterID);
    void attachParameterSafely(const juce::String& parameterID, std::function<void()> attachmentFunction);
    void attachSliderParameterSafely(const juce::String& parameterID, juce::Slider& slider);
    void attachButtonParameterSafely(const juce::String& parameterID, juce::Button& button);
    void attachComboBoxParameterSafely(const juce::String& parameterID, juce::ComboBox& comboBox);
    
    // Utility methods
    bool isParameterAttached(const juce::String& parameterID) const;
    int getAttachmentCount() const;
    void logAttachmentStatus() const;
    
private:
    MyPluginAudioProcessorEditor& editor;
    
    // Attachment storage with parameter ID tracking
    struct SliderAttachmentInfo {
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        juce::String parameterID;
    };
    
    struct ButtonAttachmentInfo {
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
        juce::String parameterID;
    };
    
    struct ComboBoxAttachmentInfo {
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
        juce::String parameterID;
    };
    
    std::vector<SliderAttachmentInfo> sliderAttachments;
    std::vector<ButtonAttachmentInfo> buttonAttachments;
    std::vector<ComboBoxAttachmentInfo> comboBoxAttachments;
    
    // Helper methods
    void createSliderAttachment(const juce::String& parameterID, juce::Slider& slider);
    void createButtonAttachment(const juce::String& parameterID, juce::Button& button);
    void createComboBoxAttachment(const juce::String& parameterID, juce::ComboBox& comboBox);
    
    // Parameter ID constants
    struct ParameterIDs {
        // Main controls
        static constexpr const char* gain = "gain_db";
        static constexpr const char* inputGain = "input_gain_db";
        static constexpr const char* outputGain = "output_gain_db";
        static constexpr const char* mix = "mix";
        static constexpr const char* width = "width";
        static constexpr const char* tilt = "tilt";
        static constexpr const char* monoHz = "mono_hz";
        static constexpr const char* hpHz = "hp_hz";
        static constexpr const char* lpHz = "lp_hz";
        static constexpr const char* satDriveDb = "sat_drive_db";
        static constexpr const char* satMix = "sat_mix";
        static constexpr const char* airDb = "air_db";
        static constexpr const char* bassDb = "bass_db";
        static constexpr const char* scoop = "scoop";
        
        // Imaging
        static constexpr const char* widthLo = "width_lo";
        static constexpr const char* widthMid = "width_mid";
        static constexpr const char* widthHi = "width_hi";
        static constexpr const char* xoverLoHz = "xover_lo_hz";
        static constexpr const char* xoverHiHz = "xover_hi_hz";
        static constexpr const char* rotationDeg = "rotation_deg";
        static constexpr const char* asymmetry = "asymmetry";
        
        // EQ
        static constexpr const char* tiltFreq = "tilt_freq";
        static constexpr const char* scoopFreq = "scoop_freq";
        static constexpr const char* bassFreq = "bass_freq";
        static constexpr const char* airFreq = "air_freq";
        static constexpr const char* eqShelfShape = "eq_shelf_shape";
        static constexpr const char* eqFilterQ = "eq_filter_q";
        static constexpr const char* tiltLinkS = "tilt_link_s";
        static constexpr const char* eqQLink = "eq_q_link";
        static constexpr const char* hpQ = "hp_q";
        static constexpr const char* lpQ = "lp_q";
        
        // Panning
        static constexpr const char* pan = "pan";
        static constexpr const char* panL = "pan_l";
        static constexpr const char* panR = "pan_r";
        static constexpr const char* depth = "depth";
        
        // Mono maker
        static constexpr const char* monoSlopeDbOct = "mono_slope_db_oct";
        static constexpr const char* monoAudition = "mono_audition";
        
        // Bypass
        static constexpr const char* bypass = "bypass";
        static constexpr const char* osMode = "os_mode";
        
        // Delay parameters
        static constexpr const char* delayEnabled = "delay_enabled";
        static constexpr const char* delayMode = "delay_mode";
        static constexpr const char* delaySync = "delay_sync";
        static constexpr const char* delayGridFlavor = "delay_grid_flavor";
        static constexpr const char* delayTimeMs = "delay_time_ms";
        static constexpr const char* delayTimeDiv = "delay_time_div";
        static constexpr const char* delayFeedbackPct = "delay_feedback_pct";
        static constexpr const char* delayWet = "delay_wet";
        static constexpr const char* delayKillDry = "delay_kill_dry";
        static constexpr const char* delayFreeze = "delay_freeze";
        static constexpr const char* delayPingpong = "delay_pingpong";
        static constexpr const char* delayCrossfeedPct = "delay_crossfeed_pct";
        static constexpr const char* delayStereoSpreadPct = "delay_stereo_spread_pct";
        static constexpr const char* delayWidth = "delay_width";
        static constexpr const char* delayModRateHz = "delay_mod_rate_hz";
        static constexpr const char* delayModDepthMs = "delay_mod_depth_ms";
        static constexpr const char* delayWowflutter = "delay_wowflutter";
        static constexpr const char* delayJitterPct = "delay_jitter_pct";
        static constexpr const char* delayHpHz = "delay_hp_hz";
        static constexpr const char* delayLpHz = "delay_lp_hz";
        static constexpr const char* delayTiltDb = "delay_tilt_db";
        static constexpr const char* delaySat = "delay_sat";
        static constexpr const char* delayDiffusion = "delay_diffusion";
        static constexpr const char* delayDiffuseSizeMs = "delay_diffuse_size_ms";
        static constexpr const char* delayDuckSource = "delay_duck_source";
        static constexpr const char* delayDuckPost = "delay_duck_post";
        static constexpr const char* delayFilterType = "delay_filter_type";
        
        // Reverb parameters
        static constexpr const char* duckDepthDb = "duck_depth_db";
        static constexpr const char* duckAtkMs = "duck_atk_ms";
        static constexpr const char* duckRelMs = "duck_rel_ms";
        static constexpr const char* duckThrDb = "duck_thr_db";
        static constexpr const char* duckRatio = "duck_ratio";
    };
};
