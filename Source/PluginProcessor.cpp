#include "PluginProcessor.h"
#include "PluginEditor.h"

// =========================
// Parameter IDs
// =========================
namespace IDs {
    static constexpr const char* gain   = "gain_db";
    static constexpr const char* pan    = "pan";
    static constexpr const char* panL   = "pan_l";   // Split L
    static constexpr const char* panR   = "pan_r";   // Split R
    static constexpr const char* depth  = "depth";
    static constexpr const char* width  = "width";
    static constexpr const char* tilt   = "tilt";
    static constexpr const char* scoop  = "scoop";
    static constexpr const char* monoHz = "mono_hz";
    static constexpr const char* hpHz   = "hp_hz";
    static constexpr const char* lpHz   = "lp_hz";
    static constexpr const char* satDriveDb = "sat_drive_db";
    static constexpr const char* satMix     = "sat_mix";
    static constexpr const char* bypass     = "bypass";
    static constexpr const char* spaceAlgo  = "space_algo";   // 0=Room 1=Plate 2=Hall
    static constexpr const char* airDb      = "air_db";
    static constexpr const char* bassDb     = "bass_db";
    static constexpr const char* ducking    = "ducking";
    static constexpr const char* duckThrDb  = "duck_threshold_db";
    static constexpr const char* duckKneeDb = "duck_knee_db";
    static constexpr const char* duckRatio  = "duck_ratio";
    static constexpr const char* duckAtkMs  = "duck_attack_ms";
    static constexpr const char* duckRelMs  = "duck_release_ms";
    static constexpr const char* duckLAms   = "duck_lookahead_ms";
    static constexpr const char* duckRmsMs  = "duck_rms_ms";
    static constexpr const char* duckTarget = "duck_target"; // 0 WetOnly, 1 Global
    static constexpr const char* osMode     = "os_mode";      // 0 Off, 1=2x, 2=4x
    static constexpr const char* splitMode  = "split_mode";   // 0 normal, 1 split
    // EQ start freqs
    static constexpr const char* tiltFreq   = "tilt_freq";
    static constexpr const char* scoopFreq  = "scoop_freq";
    static constexpr const char* bassFreq   = "bass_freq";
    static constexpr const char* airFreq    = "air_freq";
    // Imaging
    static constexpr const char* xoverLoHz  = "xover_lo_hz";
    static constexpr const char* xoverHiHz  = "xover_hi_hz";
    static constexpr const char* widthLo    = "width_lo";
    static constexpr const char* widthMid   = "width_mid";
    static constexpr const char* widthHi    = "width_hi";
    static constexpr const char* rotationDeg= "rotation_deg";
    static constexpr const char* asymmetry  = "asymmetry";
    static constexpr const char* shufLoPct  = "shuffler_lo_pct";
    static constexpr const char* shufHiPct  = "shuffler_hi_pct";
    static constexpr const char* shufXHz    = "shuffler_xover_hz";
    static constexpr const char* monoSlope  = "mono_slope_db_oct";
    static constexpr const char* monoAud    = "mono_audition";
    
    // EQ shaping/Q link additions
    static constexpr const char* eqShelfShape = "eq_shelf_shape";  // S: 0.25..1.50
    static constexpr const char* eqFilterQ    = "eq_filter_q";     // global Q: 0.50..1.20
    static constexpr const char* tiltLinkS    = "tilt_link_s";     // link Tilt shelves to S
    static constexpr const char* eqQLink      = "eq_q_link";       // link HP/LP Q to global
    static constexpr const char* hpQ          = "hp_q";            // per-filter Q
    static constexpr const char* lpQ          = "lp_q";            // per-filter Q
    
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
    static constexpr const char* delayDuckDepth = "delay_duck_depth";
    static constexpr const char* delayDuckAttackMs = "delay_duck_attack_ms";
    static constexpr const char* delayDuckReleaseMs = "delay_duck_release_ms";
    static constexpr const char* delayDuckThresholdDb = "delay_duck_threshold_db";
    static constexpr const char* delayDuckRatio = "delay_duck_ratio";
    static constexpr const char* delayDuckLookaheadMs = "delay_duck_lookahead_ms";
    static constexpr const char* delayDuckLinkGlobal = "delay_duck_link_global";
    // Width Designer additions
    static constexpr const char* widthMode          = "width_mode";            // 0=Classic, 1=Designer
    static constexpr const char* widthSideTiltDbOct = "width_side_tilt_db_oct";
    static constexpr const char* widthTiltPivotHz   = "width_tilt_pivot_hz";
    static constexpr const char* widthAutoDepth     = "width_auto_depth";
    static constexpr const char* widthAutoThrDb     = "width_auto_thr_db";
    static constexpr const char* widthAutoAtkMs     = "width_auto_atk_ms";
    static constexpr const char* widthAutoRelMs     = "width_auto_rel_ms";
    static constexpr const char* widthMax           = "width_max";
    // Phase Modes
    static constexpr const char* phaseMode  = "phase_mode"; // 0 Zero, 1 Natural, 2 Hybrid Linear
}

// ================================================================
// Helper: safe access to APVTS raw values
// ================================================================
static inline float getParam (juce::AudioProcessorValueTreeState& apvts, const char* id)
{
    if (auto* p = apvts.getRawParameterValue (id)) return p->load();
    jassertfalse; return 0.0f;
}

// ------------------------------------------------
// New voicing helper for Room/Plate/Hall macro map
// ------------------------------------------------
static inline float lerpFloat (float a, float b, float t)
{
    t = juce::jlimit (0.0f, 1.0f, t);
    return a + (b - a) * t;
}

// Roughly map damping cutoff in Hz to JUCE Reverb damping [0..1]
// (not perceptual; placeholder until full engine is integrated)
static inline float mapDampHzToParam01 (float hz)
{
    const float lo = 2000.0f, hi = 12000.0f;
    return juce::jlimit (0.0f, 1.0f, (hz - lo) / (hi - lo));
}

static inline void computeReverbVoicing (int spaceAlgoIndex, float depth01, juce::dsp::Reverb::Parameters& out)
{
    const int t = juce::jlimit (0, 2, spaceAlgoIndex); // 0..2
    const float d = juce::jlimit (0.0f, 1.0f, depth01);

    // Ensure wet-only mixing occurs at our mix site
    out.dryLevel   = 0.0f;
    out.freezeMode = false;

    // Defaults
    float mix01   = 0.0f;
    float size01  = out.roomSize;
    float dampHz  = 6000.0f;
    float width01 = out.width;

    switch (t)
    {
        // Room
        case 0:
        {
            mix01   = lerpFloat (0.00f, 0.14f, d);
            size01  = lerpFloat (0.28f, 0.45f, d);
            dampHz  = lerpFloat (4000.f, 6000.f, d);
            width01 = lerpFloat (0.80f, 0.90f, d);
        } break;

        // Plate
        case 1:
        {
            mix01   = lerpFloat (0.00f, 0.28f, d);
            size01  = lerpFloat (0.45f, 0.60f, d);
            dampHz  = lerpFloat (6000.f, 8000.f, d);
            width01 = 0.90f;
        } break;

        // Hall
        case 2:
        default:
        {
            mix01   = lerpFloat (0.00f, 0.35f, d);
            size01  = lerpFloat (0.65f, 0.90f, d);
            dampHz  = lerpFloat (6500.f, 8000.f, d);
            width01 = 1.00f;
        } break;
    }

    out.wetLevel = juce::jlimit (0.0f, 1.0f, mix01);
    out.roomSize = juce::jlimit (0.0f, 1.0f, size01);
    out.damping  = juce::jlimit (0.0f, 1.0f, mapDampHzToParam01 (dampHz));
    out.width    = juce::jlimit (0.0f, 1.0f, width01);
}

// HostParams is declared in PluginProcessor.h

// ================================================================
// Float-only reverb adapter for double chains (parallel, wet return)
// ================================================================
struct FloatReverbAdapter
{
    juce::dsp::Reverb                      reverbF;
    juce::dsp::Reverb::Parameters          params;
    juce::AudioBuffer<float>               scratch;

    void prepare (double sr, int maxBlock, int chans)
    {
        scratch.setSize (chans, maxBlock, false, false, true);
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) maxBlock, (juce::uint32) chans };
        reverbF.prepare (spec);
    }

    void setParameters (const juce::dsp::Reverb::Parameters& p) { params = p; reverbF.setParameters (params); }

    // Renders a wet-only float buffer from a double AudioBlock and mixes back with wetLevel
    void processParallelMix (juce::dsp::AudioBlock<double> block, float wetLevel)
    {
        if (wetLevel <= 0.0001f) return;
        const int ch = (int) block.getNumChannels();
        const int n  = (int) block.getNumSamples();
        scratch.setSize (ch, n, false, false, true);

        // Copy double -> float (manual conversion)
        for (int c = 0; c < ch; ++c)
        {
            auto* dst = scratch.getWritePointer (c);
            auto* src = block.getChannelPointer (c);
            for (int i = 0; i < n; ++i)
                dst[i] = static_cast<float> (src[i]);
        }

        juce::dsp::AudioBlock<float> fblk (scratch);
        juce::dsp::ProcessContextReplacing<float> fctx (fblk);
        reverbF.process (fctx);

        // Mix wet back into double
        for (int c = 0; c < ch; ++c)
        {
            auto* d = block.getChannelPointer (c);
            auto* s = scratch.getReadPointer (c);
            for (int i = 0; i < n; ++i) d[i] += (double) s[i] * (double) wetLevel;
        }
    }
};

// ================================================================
// MyPluginAudioProcessor
// ================================================================
MyPluginAudioProcessor::MyPluginAudioProcessor()
: AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
, apvts (*this, &undo, "PARAMS", createParameterLayout())
{
    // Templated chain instances
    chainF = std::make_unique<FieldChain<float>>();
    chainD = std::make_unique<FieldChain<double>>();

    // Keep existing smoothers if declared in the header (no harm if unused here)
    for (auto* s : { &panSmoothed, &panLSmoothed, &panRSmoothed, &depthSmoothed, &widthSmoothed, &gainSmoothed, &tiltSmoothed,
                     &hpHzSmoothed, &lpHzSmoothed, &monoHzSmoothed,
                     &satDriveLin, &satMixSmoothed, &airSmoothed, &bassSmoothed, &duckingSmoothed })
        s->reset (currentSR, 0.005); // 5 ms smoothing

    apvts.addParameterListener (IDs::pan,  this);
    apvts.addParameterListener (IDs::gain, this);
}

bool MyPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto in  = layouts.getMainInputChannelSet();
    auto out = layouts.getMainOutputChannelSet();
    if (in != out || out.isDisabled()) return false;
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void MyPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::FloatVectorOperations::disableDenormalisedNumberSupport();
    currentSR = sampleRate;

    for (auto* s : { &panSmoothed, &panLSmoothed, &panRSmoothed, &depthSmoothed, &widthSmoothed, &gainSmoothed, &tiltSmoothed,
                     &hpHzSmoothed, &lpHzSmoothed, &monoHzSmoothed,
                     &satDriveLin, &satMixSmoothed, &airSmoothed, &bassSmoothed, &duckingSmoothed })
        s->reset (sampleRate, 0.005);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock,
                                  (juce::uint32) getTotalNumOutputChannels() };

    chainF->prepare (spec);
    chainD->prepare (spec);
    updateLatencyForPhaseMode();

    // Initialize Motion Engine
    motionEngine.prepare(sampleRate, samplesPerBlock);
    
    // Set up Motion parameter pointers - Dual Panner System
    #include "motion/MotionIDs.h"
    using namespace motion;
    
    // Global parameters
    motionParams.enable = apvts.getRawParameterValue(id::enable);
    motionParams.pannerSelect = apvts.getRawParameterValue(id::panner_select);
    motionParams.headphoneSafe = apvts.getRawParameterValue(id::headphone_safe);
    motionParams.bassFloorHz = apvts.getRawParameterValue(id::bass_floor_hz);
    motionParams.occlusion = apvts.getRawParameterValue(id::occlusion);
    
    // P1 parameters
    motionParams.p1.path = apvts.getRawParameterValue(id::p1_path);
    motionParams.p1.rateHz = apvts.getRawParameterValue(id::p1_rate_hz);
    motionParams.p1.depth = apvts.getRawParameterValue(id::p1_depth_pct);
    motionParams.p1.phaseDeg = apvts.getRawParameterValue(id::p1_phase_deg);
    motionParams.p1.spread = apvts.getRawParameterValue(id::p1_spread_pct);
    motionParams.p1.elevBias = apvts.getRawParameterValue(id::p1_elev_bias);
    motionParams.p1.bounce = apvts.getRawParameterValue(id::p1_shape_bounce);
    motionParams.p1.jitter = apvts.getRawParameterValue(id::p1_jitter_amt);
    motionParams.p1.quantizeDiv = apvts.getRawParameterValue(id::p1_quantize_div);
    motionParams.p1.swing = apvts.getRawParameterValue(id::p1_swing_pct);
    motionParams.p1.mode = apvts.getRawParameterValue(id::p1_mode);
    motionParams.p1.retrig = apvts.getRawParameterValue(id::p1_retrig);
    motionParams.p1.holdMs = apvts.getRawParameterValue(id::p1_hold_ms);
    motionParams.p1.sens = apvts.getRawParameterValue(id::p1_sens);
    motionParams.p1.inertia = apvts.getRawParameterValue(id::p1_inertia_ms);
    motionParams.p1.frontBias = apvts.getRawParameterValue(id::p1_front_bias);
    motionParams.p1.doppler = apvts.getRawParameterValue(id::p1_doppler_amt);
    motionParams.p1.motionSend = apvts.getRawParameterValue(id::p1_motion_send);
    motionParams.p1.anchor = apvts.getRawParameterValue(id::p1_anchor_enable);
    
    // P2 parameters
    motionParams.p2.path = apvts.getRawParameterValue(id::p2_path);
    motionParams.p2.rateHz = apvts.getRawParameterValue(id::p2_rate_hz);
    motionParams.p2.depth = apvts.getRawParameterValue(id::p2_depth_pct);
    motionParams.p2.phaseDeg = apvts.getRawParameterValue(id::p2_phase_deg);
    motionParams.p2.spread = apvts.getRawParameterValue(id::p2_spread_pct);
    motionParams.p2.elevBias = apvts.getRawParameterValue(id::p2_elev_bias);
    motionParams.p2.bounce = apvts.getRawParameterValue(id::p2_shape_bounce);
    motionParams.p2.jitter = apvts.getRawParameterValue(id::p2_jitter_amt);
    motionParams.p2.quantizeDiv = apvts.getRawParameterValue(id::p2_quantize_div);
    motionParams.p2.swing = apvts.getRawParameterValue(id::p2_swing_pct);
    motionParams.p2.mode = apvts.getRawParameterValue(id::p2_mode);
    motionParams.p2.retrig = apvts.getRawParameterValue(id::p2_retrig);
    motionParams.p2.holdMs = apvts.getRawParameterValue(id::p2_hold_ms);
    motionParams.p2.sens = apvts.getRawParameterValue(id::p2_sens);
    motionParams.p2.inertia = apvts.getRawParameterValue(id::p2_inertia_ms);
    motionParams.p2.frontBias = apvts.getRawParameterValue(id::p2_front_bias);
    motionParams.p2.doppler = apvts.getRawParameterValue(id::p2_doppler_amt);
    motionParams.p2.motionSend = apvts.getRawParameterValue(id::p2_motion_send);
    motionParams.p2.anchor = apvts.getRawParameterValue(id::p2_anchor_enable);
    
    // Legacy parameter pointers (point to P1 for backward compatibility)
    motionParams.path = motionParams.p1.path;
    motionParams.rateHz = motionParams.p1.rateHz;
    motionParams.depth = motionParams.p1.depth;
    motionParams.phaseDeg = motionParams.p1.phaseDeg;
    motionParams.spread = motionParams.p1.spread;
    motionParams.elevBias = motionParams.p1.elevBias;
    motionParams.bounce = motionParams.p1.bounce;
    motionParams.jitter = motionParams.p1.jitter;
    motionParams.quantizeDiv = motionParams.p1.quantizeDiv;
    motionParams.swing = motionParams.p1.swing;
    motionParams.mode = motionParams.p1.mode;
    motionParams.retrig = motionParams.p1.retrig;
    motionParams.holdMs = motionParams.p1.holdMs;
    motionParams.sens = motionParams.p1.sens;
    motionParams.offsetDeg = motionParams.p1.phaseDeg; // Legacy mapping
    motionParams.inertia = motionParams.p1.inertia;
    motionParams.frontBias = motionParams.p1.frontBias;
    motionParams.doppler = motionParams.p1.doppler;
    motionParams.motionSend = motionParams.p1.motionSend;
    motionParams.anchor = motionParams.p1.anchor;
    
    motionEngine.setParams(&motionParams);
}

// Utility: build a HostParams snapshot each block
static HostParams makeHostParams (juce::AudioProcessorValueTreeState& apvts)
{
    HostParams p{};
    p.gainDb   = getParam(apvts, IDs::gain);
    p.pan      = getParam(apvts, IDs::pan);
    p.panL     = getParam(apvts, IDs::panL);
    p.panR     = getParam(apvts, IDs::panR);
    p.depth    = getParam(apvts, IDs::depth);
    p.width    = getParam(apvts, IDs::width);
    p.tiltDb   = getParam(apvts, IDs::tilt);
    p.scoopDb  = getParam(apvts, IDs::scoop);
    p.monoHz   = getParam(apvts, IDs::monoHz);
    p.hpHz     = getParam(apvts, IDs::hpHz);
    p.lpHz     = getParam(apvts, IDs::lpHz);
    p.satDriveDb = getParam(apvts, IDs::satDriveDb);
    p.satMix     = getParam(apvts, IDs::satMix);
    p.bypass     = (getParam(apvts, IDs::bypass) >= 0.5f);
    p.spaceAlgo  = (int) apvts.getParameterAsValue (IDs::spaceAlgo).getValue();
    p.airDb    = getParam(apvts, IDs::airDb);
    p.bassDb   = getParam(apvts, IDs::bassDb);
    p.ducking  = getParam(apvts, IDs::ducking);
    p.duckThresholdDb = getParam(apvts, IDs::duckThrDb);
    p.duckKneeDb      = getParam(apvts, IDs::duckKneeDb);
    p.duckRatio       = getParam(apvts, IDs::duckRatio);
    p.duckAttackMs    = getParam(apvts, IDs::duckAtkMs);
    p.duckReleaseMs   = getParam(apvts, IDs::duckRelMs);
    p.duckLookaheadMs = getParam(apvts, IDs::duckLAms);
    p.duckRmsMs       = getParam(apvts, IDs::duckRmsMs);
    p.duckTarget      = (int) apvts.getParameterAsValue (IDs::duckTarget).getValue();
    p.osMode   = (int) apvts.getParameterAsValue (IDs::osMode).getValue();
    p.splitMode= (bool) apvts.getParameterAsValue (IDs::splitMode).getValue();
    p.tiltFreq = getParam(apvts, IDs::tiltFreq);
    p.scoopFreq= getParam(apvts, IDs::scoopFreq);
    p.bassFreq = getParam(apvts, IDs::bassFreq);
    p.airFreq  = getParam(apvts, IDs::airFreq);
    // Imaging
    p.xoverLoHz      = getParam(apvts, IDs::xoverLoHz);
    p.xoverHiHz      = getParam(apvts, IDs::xoverHiHz);
    p.widthLo        = getParam(apvts, IDs::widthLo);
    p.widthMid       = getParam(apvts, IDs::widthMid);
    p.widthHi        = getParam(apvts, IDs::widthHi);
    p.rotationDeg    = getParam(apvts, IDs::rotationDeg);
    p.asymmetry      = getParam(apvts, IDs::asymmetry);
    p.shufflerLoPct  = getParam(apvts, IDs::shufLoPct);
    p.shufflerHiPct  = getParam(apvts, IDs::shufHiPct);
    p.shufflerXoverHz= getParam(apvts, IDs::shufXHz);
    p.monoSlopeDbOct = (int) juce::roundToInt (getParam(apvts, IDs::monoSlope));
    p.monoAudition   = (getParam(apvts, IDs::monoAud) >= 0.5f);
    // Width Designer
    p.widthMode          = (int) apvts.getParameterAsValue (IDs::widthMode).getValue();
    p.widthSideTiltDbOct = getParam(apvts, IDs::widthSideTiltDbOct);
    p.widthTiltPivotHz   = getParam(apvts, IDs::widthTiltPivotHz);
    p.widthAutoDepth     = getParam(apvts, IDs::widthAutoDepth);
    p.widthAutoThrDb     = getParam(apvts, IDs::widthAutoThrDb);
    p.widthAutoAtkMs     = getParam(apvts, IDs::widthAutoAtkMs);
    p.widthAutoRelMs     = getParam(apvts, IDs::widthAutoRelMs);
    p.widthMax           = getParam(apvts, IDs::widthMax);
    // New EQ/link params
    p.eqShelfShapeS  = getParam(apvts, IDs::eqShelfShape);
    p.eqFilterQ      = getParam(apvts, IDs::eqFilterQ);
    p.tiltLinkS      = (getParam(apvts, IDs::tiltLinkS) >= 0.5f);
    p.eqQLink        = (getParam(apvts, IDs::eqQLink)   >= 0.5f);
    p.hpQ            = getParam(apvts, IDs::hpQ);
    p.lpQ            = getParam(apvts, IDs::lpQ);
    
    // Delay parameters
    p.delayEnabled = (getParam(apvts, IDs::delayEnabled) >= 0.5f);
    p.delayMode = (int) apvts.getParameterAsValue(IDs::delayMode).getValue();
    p.delaySync = (getParam(apvts, IDs::delaySync) >= 0.5f);
    p.delayTimeMs = getParam(apvts, IDs::delayTimeMs);
    p.delayTimeDiv = (int) apvts.getParameterAsValue(IDs::delayTimeDiv).getValue();
    p.delayFeedbackPct = getParam(apvts, IDs::delayFeedbackPct);
    p.delayWet = getParam(apvts, IDs::delayWet);
    p.delayKillDry = (getParam(apvts, IDs::delayKillDry) >= 0.5f);
    p.delayFreeze = (getParam(apvts, IDs::delayFreeze) >= 0.5f);
    p.delayPingpong = (getParam(apvts, IDs::delayPingpong) >= 0.5f);
    p.delayCrossfeedPct = getParam(apvts, IDs::delayCrossfeedPct);
    p.delayStereoSpreadPct = getParam(apvts, IDs::delayStereoSpreadPct);
    p.delayWidth = getParam(apvts, IDs::delayWidth);
    p.delayModRateHz = getParam(apvts, IDs::delayModRateHz);
    p.delayModDepthMs = getParam(apvts, IDs::delayModDepthMs);
    p.delayWowflutter = getParam(apvts, IDs::delayWowflutter);
    p.delayJitterPct = getParam(apvts, IDs::delayJitterPct);
    p.delayHpHz = getParam(apvts, IDs::delayHpHz);
    p.delayLpHz = getParam(apvts, IDs::delayLpHz);
    p.delayTiltDb = getParam(apvts, IDs::delayTiltDb);
    p.delaySat = getParam(apvts, IDs::delaySat);
    p.delayDiffusion = getParam(apvts, IDs::delayDiffusion);
    p.delayDiffuseSizeMs = getParam(apvts, IDs::delayDiffuseSizeMs);
    p.delayDuckSource = (int) apvts.getParameterAsValue(IDs::delayDuckSource).getValue();
    p.delayDuckPost = (getParam(apvts, IDs::delayDuckPost) >= 0.5f);
    p.delayDuckDepth = getParam(apvts, IDs::delayDuckDepth);
    p.delayDuckAttackMs = getParam(apvts, IDs::delayDuckAttackMs);
    p.delayDuckReleaseMs = getParam(apvts, IDs::delayDuckReleaseMs);
    p.delayDuckThresholdDb = getParam(apvts, IDs::delayDuckThresholdDb);
    p.delayDuckRatio = getParam(apvts, IDs::delayDuckRatio);
    p.delayDuckLookaheadMs = getParam(apvts, IDs::delayDuckLookaheadMs);
    p.delayDuckLinkGlobal = (getParam(apvts, IDs::delayDuckLinkGlobal) >= 0.5f);
    p.phaseMode = (int) apvts.getParameterAsValue (IDs::phaseMode).getValue();
    
    return p;
}

// Float path
void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    isDoublePrecEnabled = false;

    // Pre-DSP visualization feed (lock-free bus)
    if (buffer.getNumSamples() > 0 && buffer.getNumChannels() > 0)
    {
        const int chL = buffer.getNumChannels() > 0 ? 0 : 0;
        const int chR = buffer.getNumChannels() > 1 ? 1 : 0;
        visPre.push (buffer.getReadPointer (chL),
                     buffer.getNumChannels() > 1 ? buffer.getReadPointer (chR) : nullptr,
                     buffer.getNumSamples());
    }

    auto hp = makeHostParams (apvts);
    // Sync helpers (UI → chain)
    hp.delayGridFlavor = (int) apvts.getParameterAsValue(IDs::delayGridFlavor).getValue();
    {
        double bpm = 120.0;
        if (auto* ph = getPlayHead())
        {
            if (auto pos = ph->getPosition())
            {
                if (auto bpmOpt = pos->getBpm())
                    bpm = *bpmOpt > 0.0 ? *bpmOpt : bpm;

                const bool isPlaying = pos->getIsPlaying();
                if (auto tOpt = pos->getTimeInSeconds())
                    transportTimeSeconds.store (*tOpt);
                transportIsPlaying.store (isPlaying);
            }
        }
        hp.tempoBpm = bpm;
    }
    chainF->setParameters (hp);     // cast/copy inside chain

    juce::dsp::AudioBlock<float> block (buffer);
    chainF->setParameters (hp);
    chainF->process (block);

    // Motion processing
    if (buffer.getNumChannels() >= 2)
    {
        motionEngine.processBlock(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
    }

    // Correlation + RMS/Peak meters (simple block estimate)
    if (buffer.getNumChannels() >= 2)
    {
        const int n = buffer.getNumSamples();
        const float* L = buffer.getReadPointer(0);
        const float* R = buffer.getReadPointer(1);
        double sLL=0.0, sRR=0.0, sLR=0.0;
        float peakL=0.0f, peakR=0.0f;
        for (int i=0;i<n;++i){ const float l=L[i], r=R[i]; sLL+= (double) l*l; sRR+= (double) r*r; sLR+= (double) l*r; peakL = juce::jmax (peakL, std::abs(l)); peakR = juce::jmax (peakR, std::abs(r)); }
        const double denom = std::sqrt (sLL * sRR) + 1e-12;
        const float corr = (float) juce::jlimit (-1.0, 1.0, sLR / denom);
        const float old = meterCorrelation.load();
        meterCorrelation.store (old + 0.1f * (corr - old));

        // RMS (per block), convert to dBFS-ish 0..1 scaling for UI
        const float rmsL = std::sqrt ((float) (sLL / (double) juce::jmax (1, n)));
        const float rmsR = std::sqrt ((float) (sRR / (double) juce::jmax (1, n)));
        // Smooth a bit for UI
        auto smooth = [] (std::atomic<float>& a, float v){ float o = a.load(); a.store (o + 0.2f * (v - o)); };
        smooth (meterRmsL,  rmsL);
        smooth (meterRmsR,  rmsR);
        smooth (meterPeakL, peakL);
        smooth (meterPeakR, peakR);
    }

    // Post-DSP visualization feed (lock-free bus)
    if (buffer.getNumSamples() > 0 && buffer.getNumChannels() > 0)
    {
        const int chL = buffer.getNumChannels() > 0 ? 0 : 0;
        const int chR = buffer.getNumChannels() > 1 ? 1 : 0;
        visPost.push (buffer.getReadPointer (chL),
                      buffer.getNumChannels() > 1 ? buffer.getReadPointer (chR) : nullptr,
                      buffer.getNumSamples());
    }

    // (XY oscilloscope will also read from visPost on the UI thread)
}

// Double path
void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    isDoublePrecEnabled = true;

    auto hp = makeHostParams (apvts);
    hp.delayGridFlavor = (int) apvts.getParameterAsValue(IDs::delayGridFlavor).getValue();
    {
        double bpm = 120.0;
        if (auto* ph = getPlayHead())
        {
            if (auto pos = ph->getPosition())
            {
                if (auto bpmOpt = pos->getBpm())
                    bpm = *bpmOpt > 0.0 ? *bpmOpt : bpm;

                const bool isPlaying = pos->getIsPlaying();
                if (auto tOpt = pos->getTimeInSeconds())
                    transportTimeSeconds.store (*tOpt);
                transportIsPlaying.store (isPlaying);
            }
        }
        hp.tempoBpm = bpm;
    }
    chainD->setParameters (hp);

    juce::dsp::AudioBlock<double> block (buffer);
    // PRE visualization (double path): copy input before processing
    if (buffer.getNumSamples() > 0 && buffer.getNumChannels() > 0)
    {
        static thread_local juce::AudioBuffer<float> tmpPreF;
        const int n = buffer.getNumSamples();
        const int chL = buffer.getNumChannels() > 0 ? 0 : 0;
        const int chR = buffer.getNumChannels() > 1 ? 1 : 0;
        tmpPreF.setSize (2, n, false, false, true);
        auto* dL = tmpPreF.getWritePointer (0);
        auto* dR = tmpPreF.getWritePointer (1);
        auto* sL = buffer.getReadPointer (chL);
        auto* sR = buffer.getReadPointer (chR);
        for (int i = 0; i < n; ++i) { dL[i] = (float) sL[i]; dR[i] = (float) sR[i]; }
        visPre.push (dL, dR, n);
    }
    chainD->setParameters (hp);
    chainD->process (block);

    // Correlation + RMS/Peak meters
    if (buffer.getNumChannels() >= 2)
    {
        const int n = buffer.getNumSamples();
        const double* L = buffer.getReadPointer(0);
        const double* R = buffer.getReadPointer(1);
        long double sLL=0.0, sRR=0.0, sLR=0.0;
        float peakL=0.0f, peakR=0.0f;
        for (int i=0;i<n;++i){ const double l=L[i], r=R[i]; sLL+=l*l; sRR+=r*r; sLR+=l*r; peakL = juce::jmax (peakL, (float) std::abs(l)); peakR = juce::jmax (peakR, (float) std::abs(r)); }
        const long double denom = std::sqrt (sLL * sRR) + 1e-18L;
        const float corr = (float) juce::jlimit (-1.0, 1.0, (double)(sLR / denom));
        const float old = meterCorrelation.load();
        meterCorrelation.store (old + 0.1f * (corr - old));

        const float rmsL = std::sqrt ((float) (sLL / (long double) juce::jmax (1, n)));
        const float rmsR = std::sqrt ((float) (sRR / (long double) juce::jmax (1, n)));
        auto smooth = [] (std::atomic<float>& a, float v){ float o = a.load(); a.store (o + 0.2f * (v - o)); };
        smooth (meterRmsL,  rmsL);
        smooth (meterRmsR,  rmsR);
        smooth (meterPeakL, peakL);
        smooth (meterPeakR, peakR);
    }

    // POST visualization (double path): convert to float and push
    if (buffer.getNumSamples() > 0 && buffer.getNumChannels() > 0)
    {
        // Pre-DSP feed isn't available here; double path uses post only to avoid extra copies.
        // If needed, maintain a pre-copy before processing.
        static thread_local juce::AudioBuffer<float> tmpVis;
        const int n = buffer.getNumSamples();
        const int chL = buffer.getNumChannels() > 0 ? 0 : 0;
        const int chR = buffer.getNumChannels() > 1 ? 1 : 0;
        tmpVis.setSize (2, n, false, false, true);
        auto* dstL = tmpVis.getWritePointer (0);
        auto* dstR = tmpVis.getWritePointer (1);
        auto* srcL = buffer.getReadPointer (chL);
        auto* srcR = buffer.getReadPointer (chR);
        for (int i = 0; i < n; ++i) { dstL[i] = (float) srcL[i]; dstR[i] = (float) srcR[i]; }
        visPost.push (dstL, dstR, n);
    }

    // Feed XYPad waveform/spectral visuals
    if (onAudioSample && buffer.getNumSamples() > 0)
    {
        const int chL = buffer.getNumChannels() > 0 ? 0 : 0;
        const int chR = buffer.getNumChannels() > 1 ? 1 : 0;
        auto* L = buffer.getReadPointer (chL);
        auto* R = buffer.getReadPointer (chR);
        const int stride = 64;
        for (int i = 0; i < buffer.getNumSamples(); i += stride)
            onAudioSample (L[i], R[i]);
    }
}

// =========================
// State
// =========================
juce::AudioProcessorEditor* MyPluginAudioProcessor::createEditor() { return (juce::AudioProcessorEditor*) new MyPluginAudioProcessorEditor (*this); }

void MyPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (state.isValid()) if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void MyPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes)) {
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
        
        // Notify editor (if open) to rebind on message thread
        if (auto* ed = dynamic_cast<MyPluginAudioProcessorEditor*>(getActiveEditor())) {
            juce::MessageManager::callAsync([ed] {
                ed->updateMotionParameterAttachmentsOnMessageThread();
            });
        }
    }
}

void MyPluginAudioProcessor::updateLatencyForPhaseMode()
{
    int mode = (int) apvts.getParameterAsValue (IDs::phaseMode).getValue();
    int latency = 0;
    if (mode == 2 || mode == 3) // Hybrid Linear or Full Linear
    {
        if (isDoublePrecEnabled && chainD)
            latency = chainD->getLinearPhaseLatencySamples();
        else if (chainF)
            latency = chainF->getLinearPhaseLatencySamples();
    }
    setLatencySamples (latency);
}

void MyPluginAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused (newValue);
    if (parameterID == IDs::phaseMode || parameterID == IDs::hpHz || parameterID == IDs::lpHz)
        updateLatencyForPhaseMode();
    
    // No auto-seeding of P2 from P1 – both panners share identical factory defaults by layout

    // Link policy A: write-through mirroring for motion parameters
    if (isLinkOn() && !mirrorGuard.load()) {
        // Map P1->P2 parameter twins
        static const std::pair<juce::String, juce::String> twins[] = {
            { motion::id::p1_path, motion::id::p2_path },
            { motion::id::p1_rate_hz, motion::id::p2_rate_hz },
            { motion::id::p1_depth_pct, motion::id::p2_depth_pct },
            { motion::id::p1_phase_deg, motion::id::p2_phase_deg },
            { motion::id::p1_spread_pct, motion::id::p2_spread_pct },
            { motion::id::p1_elev_bias, motion::id::p2_elev_bias },
            { motion::id::p1_shape_bounce, motion::id::p2_shape_bounce },
            { motion::id::p1_jitter_amt, motion::id::p2_jitter_amt },
            { motion::id::p1_swing_pct, motion::id::p2_swing_pct },
            { motion::id::p1_quantize_div, motion::id::p2_quantize_div },
            { motion::id::p1_mode, motion::id::p2_mode },
            { motion::id::p1_retrig, motion::id::p2_retrig },
            { motion::id::p1_hold_ms, motion::id::p2_hold_ms },
            { motion::id::p1_sens, motion::id::p2_sens },
            { motion::id::p1_inertia_ms, motion::id::p2_inertia_ms },
            { motion::id::p1_front_bias, motion::id::p2_front_bias },
            { motion::id::p1_doppler_amt, motion::id::p2_doppler_amt },
            { motion::id::p1_motion_send, motion::id::p2_motion_send },
            { motion::id::p1_anchor_enable, motion::id::p2_anchor_enable }
        };
        
        for (const auto& twin : twins) {
            if (parameterID == twin.first) {
                mirrorGuard.store(true);
                if (auto* p1 = apvts.getParameter(twin.first)) {
                    if (auto* p2 = apvts.getParameter(twin.second)) {
                        juce::MessageManager::callAsync([p2, v = p1->getValue()] {
                            p2->setValueNotifyingHost(v);
                        });
                    }
                }
                mirrorGuard.store(false);
                break;
            }
        }
    }
}

// =========================
// Parameter Layout
// =========================
juce::AudioProcessorValueTreeState::ParameterLayout MyPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::gain, 1 }, "Gain +", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::pan, 1 }, "Pan", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::panL, 1 }, "Pan L", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::panR, 1 }, "Pan R", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::depth, 1 }, "Depth", juce::NormalisableRange<float> (0.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::width, 1 }, "Width", juce::NormalisableRange<float> (0.5f, 4.0f, 0.00001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::tilt, 1 }, "Tone (Tilt)", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::scoop, 1 }, "Scoop", juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::monoHz, 1 }, "Mono Maker (Hz)", juce::NormalisableRange<float> (0.0f, 300.0f, 0.01f, 0.5f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::hpHz, 1 }, "HP (Hz)", juce::NormalisableRange<float> (20.0f, 1000.0f, 0.01f, 0.5f), 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::lpHz, 1 }, "LP (Hz)", juce::NormalisableRange<float> (2000.0f, 20000.0f, 0.01f, 0.5f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::satDriveDb, 1 }, "Saturation Drive (dB)", juce::NormalisableRange<float> (0.0f, 36.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::satMix, 1 }, "Saturation Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::bypass, 1 }, "Bypass", juce::NormalisableRange<float> (0.0f, 1.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::spaceAlgo, 1 }, "Reverb Algorithm", juce::StringArray { "Room", "Plate", "Hall" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::airDb, 1 }, "Air", juce::NormalisableRange<float> (0.0f, 6.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::bassDb, 1 }, "Bass", juce::NormalisableRange<float> (-6.0f, 6.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::ducking, 1 }, "Ducking", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckThrDb, 1 },  "Duck Threshold (dB)", juce::NormalisableRange<float> (-60.0f, 0.0f, 0.01f), -18.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckKneeDb, 1 }, "Duck Knee (dB)",      juce::NormalisableRange<float> (0.0f, 18.0f, 0.01f), 6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckRatio, 1 },  "Duck Ratio",          juce::NormalisableRange<float> (1.0f, 20.0f, 0.01f), 6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckAtkMs, 1 },  "Duck Attack (ms)",    juce::NormalisableRange<float> (1.0f, 80.0f, 0.01f), 12.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckRelMs, 1 },  "Duck Release (ms)",   juce::NormalisableRange<float> (20.0f, 800.0f, 0.1f), 180.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckLAms, 1 },   "Duck Lookahead (ms)", juce::NormalisableRange<float> (0.0f, 20.0f, 0.01f), 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckRmsMs, 1 },  "Duck RMS (ms)",       juce::NormalisableRange<float> (2.0f, 50.0f, 0.01f), 15.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::duckTarget, 1 }, "Duck Target", juce::StringArray { "WetOnly", "Global" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::osMode, 1 }, "Oversampling", juce::StringArray { "Off", "2x", "4x", "8x", "16x" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::splitMode, 1 }, "Split Mode", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::tiltFreq, 1 },  "Tilt Frequency", juce::NormalisableRange<float> (100.0f, 1000.0f, 1.0f, 0.5f), 500.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::scoopFreq, 1 }, "Scoop Frequency", juce::NormalisableRange<float> (200.0f, 2000.0f, 1.0f, 0.5f), 800.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::bassFreq, 1 },  "Bass Frequency", juce::NormalisableRange<float> (50.0f, 500.0f, 1.0f, 0.5f), 150.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::airFreq, 1 },   "Air Frequency",  juce::NormalisableRange<float> (2000.0f, 20000.0f, 10.0f, 0.5f), 8000.0f));

    // Imaging params (P0)
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::xoverLoHz, 1 },   "Xover Lo (Hz)", juce::NormalisableRange<float> (40.0f, 400.0f, 0.01f, 0.5f), 150.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::xoverHiHz, 1 },   "Xover Hi (Hz)", juce::NormalisableRange<float> (800.0f, 6000.0f, 0.01f, 0.5f), 2000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthLo,   1 },   "Width Low",     juce::NormalisableRange<float> (0.0f, 2.0f, 0.0001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthMid,  1 },   "Width Mid",     juce::NormalisableRange<float> (0.0f, 2.0f, 0.0001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthHi,   1 },   "Width High",    juce::NormalisableRange<float> (0.0f, 2.0f, 0.0001f), 1.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::rotationDeg,1 },  "Rotation (deg)",juce::NormalisableRange<float> (-45.0f, 45.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::asymmetry, 1 },   "Asymmetry",     juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::shufLoPct, 1 },   "Shuffler Low %",juce::NormalisableRange<float> (0.0f, 200.0f, 0.01f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::shufHiPct, 1 },   "Shuffler High %",juce::NormalisableRange<float> (0.0f, 200.0f, 0.01f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::shufXHz,   1 },   "Shuffler Xover (Hz)",juce::NormalisableRange<float> (150.0f, 2000.0f, 0.01f, 0.5f), 700.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::monoSlope,1 },   "Mono Slope (dB/oct)", juce::StringArray { "6", "12", "24" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::monoAud, 1 },      "Mono Audition", false));

    // Width Designer
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::widthMode, 1 }, "Width Mode", juce::StringArray { "Classic", "Designer" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthSideTiltDbOct, 1 }, "Side Tilt (dB/oct)", juce::NormalisableRange<float> (-6.0f, 6.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthTiltPivotHz, 1 }, "Tilt Pivot (Hz)", juce::NormalisableRange<float> (150.0f, 2000.0f, 1.0f, 0.5f), 650.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthAutoDepth, 1 }, "Auto-Width Depth", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthAutoThrDb, 1 }, "Auto-Width Threshold (S/M dB)", juce::NormalisableRange<float> (-24.0f, 12.0f, 0.01f), -3.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthAutoAtkMs, 1 }, "Auto-Width Attack (ms)", juce::NormalisableRange<float> (1.0f, 200.0f, 0.1f), 25.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthAutoRelMs, 1 }, "Auto-Width Release (ms)", juce::NormalisableRange<float> (20.0f, 1200.0f, 0.1f), 250.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::widthMax, 1 }, "Max Width", juce::NormalisableRange<float> (0.5f, 2.5f, 0.001f), 2.0f));

    // Phase Mode
    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ IDs::phaseMode, 1 }, "Phase Mode",
        juce::StringArray{ "Zero", "Natural", "Hybrid Linear", "Full Linear" }, 0));

    // EQ shape/Q additions
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::eqShelfShape, 1 }, "Shelf Shape (S)", juce::NormalisableRange<float> (0.25f, 1.50f, 0.001f), 0.90f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::eqFilterQ,    1 }, "Filter Q",        juce::NormalisableRange<float> (0.50f, 1.20f, 0.001f), 0.7071f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ IDs::tiltLinkS,    1 }, "Tilt Uses Shelf S", true));
    params.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID{ IDs::eqQLink,      1 }, "Link HP/LP Q",      true));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::hpQ,          1 }, "HP Q",             juce::NormalisableRange<float> (0.50f, 1.20f, 0.001f), 0.7071f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::lpQ,          1 }, "LP Q",             juce::NormalisableRange<float> (0.50f, 1.20f, 0.001f), 0.7071f));

    // Delay parameters
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::delayEnabled, 1 }, "Delay Enabled", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::delayMode, 1 }, "Delay Mode", juce::StringArray { "Digital", "Analog", "Tape" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::delaySync, 1 }, "Delay Sync", true));
    // Grid flavor: Straight / Dotted / Triplet
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::delayGridFlavor, 1 }, "Delay Grid Flavor", juce::StringArray { "Straight", "Dotted", "Triplet" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayTimeMs, 1 }, "Delay Time (ms)", juce::NormalisableRange<float>(1.0f, 4000.0f, 0.1f, 0.5f), 350.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::delayTimeDiv, 1 }, "Delay Division", juce::StringArray { "1/64T", "1/64", "1/64D", "1/32T", "1/32", "1/32D", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4T", "1/4", "1/4D", "1/2T", "1/2", "1/2D", "1/1T", "1/1", "1/1D", "2/1T", "2/1", "2/1D" }, 12));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayFeedbackPct, 1 }, "Delay Feedback (%)", juce::NormalisableRange<float>(0.0f, 98.0f, 0.1f), 36.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayWet, 1 }, "Delay Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::delayKillDry, 1 }, "Delay Kill Dry", false));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::delayFreeze, 1 }, "Delay Freeze", false));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::delayPingpong, 1 }, "Delay Ping-Pong", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayCrossfeedPct, 1 }, "Delay Crossfeed (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 35.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayStereoSpreadPct, 1 }, "Delay Stereo Spread (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 25.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayWidth, 1 }, "Delay Width", juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayModRateHz, 1 }, "Delay Mod Rate (Hz)", juce::NormalisableRange<float>(0.01f, 12.0f, 0.01f, 0.5f), 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayModDepthMs, 1 }, "Delay Mod Depth (ms)", juce::NormalisableRange<float>(0.0f, 12.0f, 0.01f), 3.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayWowflutter, 1 }, "Delay Wow/Flutter", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayJitterPct, 1 }, "Delay Jitter (%)", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayHpHz, 1 }, "Delay HP (Hz)", juce::NormalisableRange<float>(20.0f, 2000.0f, 0.1f, 0.5f), 120.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayLpHz, 1 }, "Delay LP (Hz)", juce::NormalisableRange<float>(2000.0f, 20000.0f, 1.0f, 0.5f), 12000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayTiltDb, 1 }, "Delay Tilt (dB)", juce::NormalisableRange<float>(-6.0f, 6.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delaySat, 1 }, "Delay Saturation", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayDiffusion, 1 }, "Delay Diffusion", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayDiffuseSizeMs, 1 }, "Delay Diffuse Size (ms)", juce::NormalisableRange<float>(5.0f, 50.0f, 0.1f), 18.0f));
    // Duck source: tap location or external sidechain
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::delayDuckSource, 1 }, "Delay Duck Source", juce::StringArray { "Pre", "Post", "External" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::delayDuckPost, 1 }, "Delay Duck Post", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayDuckDepth, 1 }, "Delay Duck Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.6f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayDuckAttackMs, 1 }, "Delay Duck Attack (ms)", juce::NormalisableRange<float>(1.0f, 200.0f, 0.1f), 12.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayDuckReleaseMs, 1 }, "Delay Duck Release (ms)", juce::NormalisableRange<float>(20.0f, 1000.0f, 0.1f), 220.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayDuckThresholdDb, 1 }, "Delay Duck Threshold (dB)", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -26.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayDuckRatio, 1 }, "Delay Duck Ratio", juce::NormalisableRange<float>(1.0f, 8.0f, 0.01f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::delayDuckLookaheadMs, 1 }, "Delay Duck Lookahead (ms)", juce::NormalisableRange<float>(0.0f, 15.0f, 0.01f), 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ IDs::delayDuckLinkGlobal, 1 }, "Delay Duck Link Global", true));

    // Motion parameters - Dual Panner System
    #include "motion/MotionIDs.h"
    using namespace motion;
    
    // Global parameters (shared)
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id::enable, 1 }, "Motion Enable", true));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id::panner_select, 1 }, "Motion Panner", choiceListPanner(), (int)PannerSelect::P1));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id::headphone_safe, 1 }, "Headphone Safe", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::bass_floor_hz, 1 }, "Bass Floor (Hz)", juce::NormalisableRange<float>(20.0f, 250.0f, 0.0f, 0.35f), 120.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::occlusion, 1 }, "Occlusion", juce::NormalisableRange<float>(0.0f, 1.0f), 0.4f));
    
    // P1-specific parameters
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id::p1_path, 1 }, "P1 Path", choiceListPath(), (int)PathType::Circle));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_rate_hz, 1 }, "P1 Rate (Hz)", juce::NormalisableRange<float>(0.05f, 16.0f, 0.0f, 0.33f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_depth_pct, 1 }, "P1 Depth", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_phase_deg, 1 }, "P1 Phase", juce::NormalisableRange<float>(0.0f, 360.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_spread_pct, 1 }, "P1 Spread", juce::NormalisableRange<float>(0.0f, 2.0f), 1.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_elev_bias, 1 }, "P1 Elevation Bias", juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_shape_bounce, 1 }, "P1 Bounce/Tension", juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_jitter_amt, 1 }, "P1 Jitter", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id::p1_quantize_div, 1 }, "P1 Quantize", choiceListQuant(), (int)QuantizeDiv::Off));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_swing_pct, 1 }, "P1 Swing", juce::NormalisableRange<float>(0.0f, 0.75f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id::p1_mode, 1 }, "P1 Motion Mode", choiceListMode(), (int)MotionMode::Sync));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id::p1_retrig, 1 }, "P1 Retrigger", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_hold_ms, 1 }, "P1 Hold (ms)", juce::NormalisableRange<float>(0.0f, 500.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_sens, 1 }, "P1 Sensitivity", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_inertia_ms, 1 }, "P1 Inertia (ms)", juce::NormalisableRange<float>(0.0f, 500.0f, 0.0f, 0.4f), 120.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_front_bias, 1 }, "P1 Front Bias", juce::NormalisableRange<float>(-1.0f, 1.0f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_doppler_amt, 1 }, "P1 Doppler", juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p1_motion_send, 1 }, "P1 Motion Send", juce::NormalisableRange<float>(0.0f, 1.0f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id::p1_anchor_enable, 1 }, "P1 Anchor Center", true));
    
    // P2-specific parameters
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id::p2_path, 1 }, "P2 Path", choiceListPath(), (int)PathType::Circle));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_rate_hz, 1 }, "P2 Rate (Hz)", juce::NormalisableRange<float>(0.05f, 16.0f, 0.0f, 0.33f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_depth_pct, 1 }, "P2 Depth", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_phase_deg, 1 }, "P2 Phase", juce::NormalisableRange<float>(0.0f, 360.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_spread_pct, 1 }, "P2 Spread", juce::NormalisableRange<float>(0.0f, 2.0f), 1.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_elev_bias, 1 }, "P2 Elevation Bias", juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_shape_bounce, 1 }, "P2 Bounce/Tension", juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_jitter_amt, 1 }, "P2 Jitter", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id::p2_quantize_div, 1 }, "P2 Quantize", choiceListQuant(), (int)QuantizeDiv::Off));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_swing_pct, 1 }, "P2 Swing", juce::NormalisableRange<float>(0.0f, 0.75f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id::p2_mode, 1 }, "P2 Motion Mode", choiceListMode(), (int)MotionMode::Sync));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id::p2_retrig, 1 }, "P2 Retrigger", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_hold_ms, 1 }, "P2 Hold (ms)", juce::NormalisableRange<float>(0.0f, 500.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_sens, 1 }, "P2 Sensitivity", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_inertia_ms, 1 }, "P2 Inertia (ms)", juce::NormalisableRange<float>(0.0f, 500.0f, 0.0f, 0.4f), 120.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_front_bias, 1 }, "P2 Front Bias", juce::NormalisableRange<float>(-1.0f, 1.0f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_doppler_amt, 1 }, "P2 Doppler", juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ id::p2_motion_send, 1 }, "P2 Motion Send", juce::NormalisableRange<float>(0.0f, 1.0f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id::p2_anchor_enable, 1 }, "P2 Anchor Center", true));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MyPluginAudioProcessor(); }

// =====================================================================
// ==================  Templated FieldChain Implementation  =============
// =====================================================================

template <typename Sample>
static inline Sample softClipT (Sample x)
{
    // atan clip scaled for ~unity at +/-1
    return std::atan (x) * (Sample) (1.0 / 1.2533141373155);
}

// --------- prepare/reset ---------

template <typename Sample>
void FieldChain<Sample>::prepare (const juce::dsp::ProcessSpec& spec)
{
    sr = spec.sampleRate;

    hpFilter.prepare (spec);
    lpFilter.prepare (spec);
    monoLP.prepare (spec.sampleRate);
    bandLowLP_L.prepare (spec);
    bandLowLP_R.prepare (spec);
    bandHighHP_L.prepare (spec);
    bandHighHP_R.prepare (spec);
    shuffLP_L.prepare (spec);
    shuffLP_R.prepare (spec);
    depthLPF.prepare (spec);
    lowShelf.prepare (spec);
    highShelf.prepare (spec);
    airFilter.prepare (spec);
    bassFilter.prepare (spec);
    scoopFilter.prepare (spec);

    hpFilter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    lpFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    // monoLP type handled internally (Butterworth sections)
    bandLowLP_L.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    bandLowLP_R.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    bandHighHP_L.setType (juce::dsp::LinkwitzRileyFilterType::highpass);
    bandHighHP_R.setType (juce::dsp::LinkwitzRileyFilterType::highpass);
    shuffLP_L.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    shuffLP_R.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    depthLPF.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    {
        const Sample nyq = (Sample) (sr * 0.49);
        depthLPF.setCutoffFrequency (juce::jlimit ((Sample) 20, nyq, (Sample) 20000));
    }
    depthLPF.setResonance ((Sample) 0.707);

    if constexpr (std::is_same_v<Sample, double>)
    {
        reverbD = std::make_unique<FloatReverbAdapter>();
        reverbD->prepare (spec.sampleRate, (int) spec.maximumBlockSize, (int) spec.numChannels);
    }
    else
    {
        juce::dsp::ProcessSpec rspec { spec.sampleRate, spec.maximumBlockSize, spec.numChannels };
        reverbF = std::make_unique<juce::dsp::Reverb>();
        reverbF->prepare (rspec);
    }

    // Default reverb params
    rvParams.roomSize   = 0.45f;
    rvParams.width      = 1.0f;
    rvParams.damping    = 0.35f;
    rvParams.dryLevel   = 1.0f;
    rvParams.wetLevel   = 0.0f;
    rvParams.freezeMode = 0.0f;

    // No OS by default
    oversampling.reset();
    lastOsMode = -1;

    // Prepare ducker
    ducker.prepare (sr, (int) spec.maximumBlockSize, 24);
    
    // Prepare delay engine
    delayEngine.prepare (sr, (int) spec.numChannels);

    // Reverb: preallocate buses and init smoothed wet
    dryBusBuf.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    wetBusBuf.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    const double smoothMs = 0.02;
    wetMixSmoothed.reset (sr, smoothMs);
    wetMixSmoothed.setCurrentAndTargetValue (rvParams.wetLevel);
    roomSizeSmoothed.reset (sr, smoothMs);
    roomSizeSmoothed.setCurrentAndTargetValue (rvParams.roomSize);
    dampingSmoothed.reset (sr, smoothMs);
    dampingSmoothed.setCurrentAndTargetValue (rvParams.damping);
    widthSmoothed.reset (sr, smoothMs);
    widthSmoothed.setCurrentAndTargetValue (rvParams.width);

    // Prepare linear-phase convolver (allocated lazily on first use)
    if (! linConvolver)
        linConvolver = std::make_unique<OverlapSaveConvolver<Sample>>();
    linConvolver->prepare (spec.sampleRate, (int) spec.maximumBlockSize, linKernelLen, (int) spec.numChannels);
}

template <typename Sample>
void FieldChain<Sample>::reset()
{
    hpFilter.reset(); lpFilter.reset(); monoLP.reset(); depthLPF.reset();
    lowShelf.reset(); highShelf.reset(); airFilter.reset(); bassFilter.reset(); scoopFilter.reset();
    if (oversampling) oversampling->reset();
    if constexpr (std::is_same_v<Sample, double>) { if (reverbD) reverbD->reverbF.reset(); }
    else                                           { if (reverbF) reverbF->reset(); }
}

// --------- parameter ingress ---------

template <typename Sample>
void FieldChain<Sample>::setParameters (const HostParams& hp)
{
    // Cast/copy once per block into Sample domain
    params.gainLin   = juce::Decibels::decibelsToGain ((Sample) hp.gainDb);
    params.pan       = (Sample) hp.pan;
    params.panL      = (Sample) hp.panL;
    params.panR      = (Sample) hp.panR;
    params.depth     = (Sample) juce::jlimit (0.0, 1.0, hp.depth);
    params.width     = (Sample) juce::jlimit (0.5, 4.0, hp.width);
    params.tiltDb    = (Sample) hp.tiltDb;
    params.scoopDb   = (Sample) hp.scoopDb;
    params.monoHz    = (Sample) hp.monoHz;
    params.hpHz      = (Sample) hp.hpHz;
    params.lpHz      = (Sample) hp.lpHz;
    // New EQ shape/Q
    params.shelfShapeS = (Sample) juce::jlimit (0.25, 1.50, hp.eqShelfShapeS);
    params.filterQ     = (Sample) juce::jlimit (0.50, 1.20, hp.eqFilterQ);
    params.hpQ         = (Sample) juce::jlimit (0.50, 1.20, hp.hpQ);
    params.lpQ         = (Sample) juce::jlimit (0.50, 1.20, hp.lpQ);
    params.tiltLinkS   = hp.tiltLinkS;
    params.eqQLink     = hp.eqQLink;
    params.satDriveLin = (Sample) juce::Decibels::decibelsToGain (hp.satDriveDb);
    params.satMix    = (Sample) juce::jlimit (0.0, 1.0, hp.satMix);
    params.bypass    = hp.bypass;
    params.spaceAlgo = hp.spaceAlgo;
    params.airDb     = (Sample) hp.airDb;
    params.bassDb    = (Sample) hp.bassDb;
    params.ducking   = (Sample) juce::jlimit (0.0, 1.0, hp.ducking);
    params.osMode    = hp.osMode;
    params.splitMode = hp.splitMode;
    params.tiltFreq  = (Sample) hp.tiltFreq;
    params.scoopFreq = (Sample) hp.scoopFreq;
    params.bassFreq  = (Sample) hp.bassFreq;
    params.airFreq   = (Sample) hp.airFreq;
    // Ducking advanced
    params.duckThresholdDb = (Sample) hp.duckThresholdDb;
    params.duckKneeDb      = (Sample) hp.duckKneeDb;
    params.duckRatio       = (Sample) hp.duckRatio;
    params.duckAttackMs    = (Sample) hp.duckAttackMs;
    params.duckReleaseMs   = (Sample) hp.duckReleaseMs;
    params.duckLookaheadMs = (Sample) hp.duckLookaheadMs;
    params.duckRmsMs       = (Sample) hp.duckRmsMs;
    params.duckTarget      = hp.duckTarget;
    // Imaging
    params.xoverLoHz = (Sample) juce::jlimit (40.0, 400.0, hp.xoverLoHz);
    params.xoverHiHz = (Sample) juce::jlimit (800.0, 6000.0, hp.xoverHiHz);
    params.widthLo   = (Sample) juce::jlimit (0.0, 2.0, hp.widthLo);
    params.widthMid  = (Sample) juce::jlimit (0.0, 2.0, hp.widthMid);
    params.widthHi   = (Sample) juce::jlimit (0.0, 2.0, hp.widthHi);
    params.rotationRad = (Sample) (hp.rotationDeg * juce::MathConstants<double>::pi / 180.0);
    params.asymmetry = (Sample) juce::jlimit (-1.0, 1.0, hp.asymmetry);
    params.shufflerLo = (Sample) juce::jlimit (0.0, 2.0, hp.shufflerLoPct * 0.01);
    params.shufflerHi = (Sample) juce::jlimit (0.0, 2.0, hp.shufflerHiPct * 0.01);
    params.shufflerXoverHz = (Sample) juce::jlimit (150.0, 2000.0, hp.shufflerXoverHz);
    params.monoSlopeDbOct = hp.monoSlopeDbOct;
    params.monoAudition   = hp.monoAudition;
    // Width Designer
    params.widthMode          = hp.widthMode;
    params.widthSideTiltDbOct = (Sample) hp.widthSideTiltDbOct;
    params.widthTiltPivotHz   = (Sample) hp.widthTiltPivotHz;
    params.widthAutoDepth     = (Sample) juce::jlimit (0.0, 1.0, hp.widthAutoDepth);
    params.widthAutoThrDb     = (Sample) hp.widthAutoThrDb;
    params.widthAutoAtkMs     = (Sample) hp.widthAutoAtkMs;
    params.widthAutoRelMs     = (Sample) hp.widthAutoRelMs;
    params.widthMax           = (Sample) juce::jlimit (0.5, 2.5, hp.widthMax);
    // Precompute AW alphas
    auto msToAlpha = [this](Sample ms)
    {
        const Sample T = juce::jmax ((Sample)1e-3, ms * (Sample)0.001);
        const Sample a = (Sample) (1.0 - std::exp (-1.0 / (T * (Sample) sr)));
        return juce::jlimit ((Sample)1e-5, (Sample)0.9999, a);
    };
    aw_alphaAtk = msToAlpha (params.widthAutoAtkMs);
    aw_alphaRel = msToAlpha (params.widthAutoRelMs);
    
    // Delay parameters
    params.delayEnabled = hp.delayEnabled;
    params.delayMode = hp.delayMode;
    params.delaySync = hp.delaySync;
    params.delayTimeMs = (Sample)hp.delayTimeMs;
    params.delayTimeDiv = hp.delayTimeDiv;
    params.delayFeedbackPct = (Sample)hp.delayFeedbackPct;
    params.delayWet = (Sample)hp.delayWet;
    params.delayKillDry = hp.delayKillDry;
    params.delayFreeze = hp.delayFreeze;
    params.delayPingpong = hp.delayPingpong;
    params.delayCrossfeedPct = (Sample)hp.delayCrossfeedPct;
    params.delayStereoSpreadPct = (Sample)hp.delayStereoSpreadPct;
    params.delayWidth = (Sample)hp.delayWidth;
    params.delayModRateHz = (Sample)hp.delayModRateHz;
    params.delayModDepthMs = (Sample)hp.delayModDepthMs;
    params.delayWowflutter = (Sample)hp.delayWowflutter;
    params.delayJitterPct = (Sample)hp.delayJitterPct;
    params.delayHpHz = (Sample)hp.delayHpHz;
    params.delayLpHz = (Sample)hp.delayLpHz;
    params.delayTiltDb = (Sample)hp.delayTiltDb;
    params.delaySat = (Sample)hp.delaySat;
    params.delayDiffusion = (Sample)hp.delayDiffusion;
    params.delayDiffuseSizeMs = (Sample)hp.delayDiffuseSizeMs;
    params.delayDuckSource = hp.delayDuckSource;
    params.delayDuckPost = hp.delayDuckPost;
    params.delayDuckDepth = (Sample)hp.delayDuckDepth;
    params.delayDuckAttackMs = (Sample)hp.delayDuckAttackMs;
    params.delayDuckReleaseMs = (Sample)hp.delayDuckReleaseMs;
    params.delayDuckThresholdDb = (Sample)hp.delayDuckThresholdDb;
    params.delayDuckRatio = (Sample)hp.delayDuckRatio;
    params.delayDuckLookaheadMs = (Sample)hp.delayDuckLookaheadMs;
    params.delayDuckLinkGlobal = hp.delayDuckLinkGlobal;
    // Sync helpers
    params.delayGridFlavor = hp.delayGridFlavor;
    params.tempoBpm        = hp.tempoBpm;
}

// --------- processing utilities ---------

template <typename Sample>
void FieldChain<Sample>::ensureOversampling (int osModeIndex)
{
    if (lastOsMode == osModeIndex && oversampling) return;
    lastOsMode = osModeIndex;

    if (osModeIndex <= 0) { oversampling.reset(); return; }

    int factor = 1;
    switch (osModeIndex)
    {
        case 1: factor = 2;  break; // 2x
        case 2: factor = 4;  break; // 4x
        case 3: factor = 8;  break; // 8x
        case 4: factor = 16; break; // 16x
        default: factor = 1; break; // Off (should have returned earlier)
    }
    const int stages = juce::roundToInt (std::log2 (factor));

    oversampling = std::make_unique<juce::dsp::Oversampling<Sample>> (
        (int) 2, stages, juce::dsp::Oversampling<Sample>::filterHalfBandPolyphaseIIR);
    oversampling->reset();
}

// --------- per-module DSP (Sample domain) ---------

template <typename Sample>
void FieldChain<Sample>::applyHP_LP (Block block, Sample hpHz, Sample lpHz)
{
    // If both are at defaults (fully open), skip entirely to avoid any sonic change
    if (hpHz <= (Sample)20 && lpHz >= (Sample)20000)
        return;

    const Sample nyq = (Sample) (sr * 0.49);
    hpHz = juce::jlimit ((Sample) 20,  juce::jmin ((Sample) 1000, nyq),  hpHz);
    lpHz = juce::jlimit ((Sample) 1000, juce::jmin ((Sample) 20000, nyq), lpHz);
    // Set cutoffs
    hpFilter.setCutoffFrequency (hpHz);
    lpFilter.setCutoffFrequency (lpHz);
    // Set resonance (Q): either global or per-filter
    const Sample Qhp = params.eqQLink ? params.filterQ : params.hpQ;
    const Sample Qlp = params.eqQLink ? params.filterQ : params.lpQ;
    hpFilter.setResonance (juce::jlimit ((Sample)0.5, (Sample)1.2, Qhp));
    lpFilter.setResonance (juce::jlimit ((Sample)0.5, (Sample)1.2, Qlp));
    CtxRep ctx (block);
    hpFilter.process (ctx);
    lpFilter.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::updateTiltEQ (Sample tiltDb, Sample pivotHz)
{
    const double fs = sr;
    // Map pivot to complementary shelves
    const double nyq = fs * 0.49;
    const double lowFc  = juce::jlimit (50.0,  1000.0, (double) pivotHz * 0.30);
    const double highFc = juce::jlimit (1500.0, juce::jmin (20000.0, nyq), (double) pivotHz * 12.0);
    const Sample lowGain  = (Sample) juce::Decibels::decibelsToGain ( juce::jlimit (-12.0, 12.0, (double) tiltDb));
    const Sample highGain = (Sample) juce::Decibels::decibelsToGain (-juce::jlimit (-12.0, 12.0, (double) tiltDb));

    const Sample S = params.tiltLinkS ? params.shelfShapeS : (Sample)0.90;
    auto lowCoef  = juce::dsp::IIR::Coefficients<Sample>::makeLowShelf  (fs, lowFc,  S, lowGain);
    auto highCoef = juce::dsp::IIR::Coefficients<Sample>::makeHighShelf (fs, highFc, S, highGain);
    lowShelf.coefficients  = lowCoef;
    highShelf.coefficients = highCoef;
}

template <typename Sample>
void FieldChain<Sample>::applyTiltEQ (Block block, Sample tiltDb, Sample pivotHz)
{
    updateTiltEQ (tiltDb, pivotHz);
    CtxRep ctx (block);
    lowShelf.process (ctx);
    highShelf.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::applyScoopEQ (Block block, Sample scoopDb, Sample scoopFreq)
{
    if (std::abs ((double) scoopDb) < 0.1) return;
    const Sample nyq = (Sample) (sr * 0.49);
    scoopFreq = juce::jlimit ((Sample) 20, nyq, scoopFreq);
    // Map shelf shape S (0.25..1.25) to a reasonable peaking Q range (wider→narrower)
    const Sample Sshape = params.shelfShapeS;
    const Sample qPeak  = juce::jlimit ((Sample)0.5, (Sample)2.0, (Sample) juce::jmap ((double) Sshape, 0.25, 1.25, 0.5, 2.0));
    auto coef = juce::dsp::IIR::Coefficients<Sample>::makePeakFilter (sr, scoopFreq, qPeak, (Sample) juce::Decibels::decibelsToGain ((double) scoopDb));
    scoopFilter.coefficients = coef;
    CtxRep ctx (block); scoopFilter.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::applyBassShelf (Block block, Sample bassDb, Sample bassFreq)
{
    if (std::abs ((double) bassDb) < 0.1) return;
    const Sample nyq = (Sample) (sr * 0.49);
    bassFreq = juce::jlimit ((Sample) 20, nyq, bassFreq);
    const Sample S = params.shelfShapeS;
    auto coef = juce::dsp::IIR::Coefficients<Sample>::makeLowShelf (sr, bassFreq, S, (Sample) juce::Decibels::decibelsToGain ((double) bassDb));
    bassFilter.coefficients = coef; CtxRep ctx (block); bassFilter.process (ctx);
}

template <typename Sample>
void FieldChain<Sample>::applyAirBand (Block block, Sample airDb, Sample airFreq)
{
    if (airDb <= (Sample)0.05) return; // positive-only air
    const Sample nyq = (Sample) (sr * 0.49);
    airFreq = juce::jlimit ((Sample) 1000, nyq, airFreq);
    const Sample S = params.shelfShapeS * (Sample)0.3333333; // keep air gentle; scale S
    auto coef = juce::dsp::IIR::Coefficients<Sample>::makeHighShelf (sr, airFreq, juce::jlimit ((Sample)0.2, (Sample)1.50, S), (Sample) juce::Decibels::decibelsToGain ((double) airDb));
    airFilter.coefficients = coef; CtxRep ctx (block); airFilter.process (ctx);
}

// Build composite linear-phase FIR for full macro tone and apply
template <typename Sample>
void FieldChain<Sample>::applyFullLinearFIR (Block block)
{
    if (! fullLinearConvolver)
        fullLinearConvolver = std::make_unique<OverlapSaveConvolver<Sample>>();
    fullLinearConvolver->prepare (sr, (int) block.getNumSamples(), fullKernelLen, (int) block.getNumChannels());

    // Build a simple target magnitude by sampling our current macro tone params
    // Grid size for design (power of two)
    const int K = 16384;
    std::vector<double> mags ((size_t) (K/2 + 1), 1.0);
    const double nyq = sr * 0.5;

    // Simple shelves/peaks approximations (magnitude only)
    auto dbToLin = [](double db){ return std::pow (10.0, db / 20.0); };
    const double tiltGainLow  = dbToLin ((double) params.tiltDb * 0.5);
    const double tiltGainHigh = dbToLin (-(double) params.tiltDb * 0.5);
    const double bassGain     = dbToLin ((double) params.bassDb);
    const double airGain      = dbToLin ((double) params.airDb);
    const double scoopGainDb  = (double) params.scoopDb;

    for (int k = 0; k <= K/2; ++k)
    {
        const double f = (double) k / (double) (K/2) * nyq;
        double m = 1.0;
        // HP/LP as smooth tapers (sigmoid) to avoid sharp corners in design magnitude
        const double hp = 1.0 / (1.0 + std::exp (-(f - (double) params.hpHz) / juce::jmax (1.0, (double) params.hpHz * 0.1)));
        const double lp = 1.0 / (1.0 + std::exp ( (f - (double) params.lpHz) / juce::jmax (1.0, (double) params.lpHz * 0.1)));
        m *= hp * lp;
        // Tilt: pivot at tiltFreq
        const double oct = std::log2 (juce::jlimit (20.0, nyq, f) / juce::jmax (1.0, (double) params.tiltFreq));
        const double tiltCurve = oct >= 0.0 ? tiltGainHigh : tiltGainLow;
        // Bass shelf: below bassFreq
        const double bassCurve = 1.0 + (bassGain - 1.0) * (1.0 / (1.0 + std::exp ((f - (double) params.bassFreq) / juce::jmax (1.0, (double) params.bassFreq * 0.25))));
        // Air shelf: above airFreq
        const double airCurve  = 1.0 + (airGain - 1.0)  * (1.0 / (1.0 + std::exp (-(f - (double) params.airFreq) / juce::jmax (1.0, (double) params.airFreq * 0.25))));
        // Scoop: gentle peaking dip around scoopFreq
        const double q = 0.8; const double bw = juce::jmax (10.0, (double) params.scoopFreq * 0.6);
        const double gS = dbToLin (scoopGainDb);
        const double gauss = std::exp (-(f - (double) params.scoopFreq) * (f - (double) params.scoopFreq) / (2.0 * bw * bw));
        const double scoopCurve = 1.0 + (gS - 1.0) * gauss;

        m *= tiltCurve * bassCurve * airCurve * scoopCurve;
        mags[(size_t) k] = juce::jlimit (1e-6, 1000.0, m);
    }

    // Design kernel from magnitude
    std::vector<float> kernel;
    designLinearPhaseFromMagnitude (kernel, mags, K, fullKernelLen, 8.6, 1000.0, sr);
    fullLinearConvolver->setKernel (kernel);
    fullLinearConvolver->process (block);
}

template <typename Sample>
void FieldChain<Sample>::applyWidthMS (Block block, Sample width)
{
    if (block.getNumChannels() < 2) return;
    width = juce::jlimit ((Sample)0.5, (Sample)4.0, width);
    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const int N = (int) block.getNumSamples();
    const Sample k = (Sample)0.7071067811865476;
    
    for (int i = 0; i < N; ++i)
    {
        const Sample l = L[i], r = R[i];
        Sample M = k * (l + r);
        Sample S = k * (l - r);
        if (width < (Sample)1)      S *= width;
        else if (width > (Sample)1) S *= (Sample) (1.0 + (double)(width - (Sample)1.0) * 0.6);
        L[i] = k * (M + S);
        R[i] = k * (M - S);
    }
}

// --- Imaging helpers ---
template <typename Sample>
void FieldChain<Sample>::applyThreeBandWidth (Block block,
                                              Sample loHz, Sample hiHz,
                                              Sample wLo, Sample wMid, Sample wHi)
{
    if (block.getNumChannels() < 2) return;
    const Sample nyq = (Sample) (sr * 0.49);
    loHz = juce::jlimit ((Sample)40,  (Sample)400,  loHz);
    hiHz = juce::jlimit ((Sample)800, juce::jmin ((Sample)6000, nyq), hiHz);
    if (hiHz <= loHz) hiHz = juce::jlimit ((Sample) (loHz + (Sample) 10), juce::jmin ((Sample)6000, nyq), (Sample) (loHz + (Sample) 10));
    wLo  = juce::jlimit ((Sample)0, (Sample)2, wLo);
    wMid = juce::jlimit ((Sample)0, (Sample)2, wMid);
    wHi  = juce::jlimit ((Sample)0, (Sample)2, wHi);

    bandLowLP_L.setCutoffFrequency (loHz);
    bandLowLP_R.setCutoffFrequency (loHz);
    bandHighHP_L.setCutoffFrequency (hiHz);
    bandHighHP_R.setCutoffFrequency (hiHz);

    const int n  = (int) block.getNumSamples();
    juce::AudioBuffer<Sample> low (2, n), high (2, n);
    for (int c = 0; c < 2; ++c)
    {
        low.copyFrom  (c, 0, block.getChannelPointer (c), n);
        high.copyFrom (c, 0, block.getChannelPointer (c), n);
    }

    // filter
    {
        juce::dsp::AudioBlock<Sample> lb (low), hb (high);
        auto lL = lb.getSingleChannelBlock (0); auto lR = lb.getSingleChannelBlock (1);
        auto hL = hb.getSingleChannelBlock (0); auto hR = hb.getSingleChannelBlock (1);
        juce::dsp::ProcessContextReplacing<Sample> ctxLL (lL), ctxLR (lR), ctxHL (hL), ctxHR (hR);
        bandLowLP_L.process (ctxLL); bandLowLP_R.process (ctxLR);
        bandHighHP_L.process (ctxHL); bandHighHP_R.process (ctxHR);
    }

    // derive mid = full - low - high
    juce::AudioBuffer<Sample> mid (2, n);
    for (int c = 0; c < 2; ++c)
    {
        auto* full = block.getChannelPointer (c);
        auto* lo   = low.getWritePointer (c);
        auto* hi   = high.getWritePointer (c);
        auto* md   = mid.getWritePointer (c);
        for (int i = 0; i < n; ++i) md[i] = full[i] - lo[i] - hi[i];
    }

    auto msWidth = [] (Sample& L, Sample& R, Sample w)
    {
        const Sample k = (Sample)0.7071067811865476;
        Sample M = k * (L + R);
        Sample S = k * (L - R);
        S *= w;
        L = k * (M + S);
        R = k * (M - S);
    };

    auto applyWidthToBuffer = [&] (juce::AudioBuffer<Sample>& buf, Sample w)
    {
        auto* L = buf.getWritePointer (0);
        auto* R = buf.getWritePointer (1);
        for (int i = 0; i < n; ++i) msWidth (L[i], R[i], w);
    };

    applyWidthToBuffer (low, wLo);
    applyWidthToBuffer (mid, wMid);
    applyWidthToBuffer (high, wHi);

    // sum back
    for (int c = 0; c < 2; ++c)
    {
        auto* dst = block.getChannelPointer (c);
        auto* lo  = low.getReadPointer (c);
        auto* md  = mid.getReadPointer (c);
        auto* hi  = high.getReadPointer (c);
        for (int i = 0; i < n; ++i) dst[i] = lo[i] + md[i] + hi[i];
    }
}

template <typename Sample>
void FieldChain<Sample>::applyShufflerWidth (Block block, Sample xoverHz, Sample wLow, Sample wHigh)
{
    if (block.getNumChannels() < 2) return;
    const Sample nyq = (Sample) (sr * 0.49);
    xoverHz = juce::jlimit ((Sample)150, juce::jmin ((Sample)2000, nyq), xoverHz);
    shuffLP_L.setCutoffFrequency (xoverHz);
    shuffLP_R.setCutoffFrequency (xoverHz);

    const int n = (int) block.getNumSamples();
    juce::AudioBuffer<Sample> low (2, n);
    for (int c = 0; c < 2; ++c) low.copyFrom (c, 0, block.getChannelPointer (c), n);
    {
        juce::dsp::AudioBlock<Sample> lb (low);
        auto lL = lb.getSingleChannelBlock (0), lR = lb.getSingleChannelBlock (1);
        juce::dsp::ProcessContextReplacing<Sample> ctxL (lL), ctxR (lR);
        shuffLP_L.process (ctxL); shuffLP_R.process (ctxR);
    }
    // high = full - low
    juce::AudioBuffer<Sample> high (2, n);
    for (int c = 0; c < 2; ++c)
    {
        auto* full = block.getChannelPointer (c);
        auto* lo   = low.getWritePointer (c);
        auto* hi   = high.getWritePointer (c);
        for (int i = 0; i < n; ++i) hi[i] = full[i] - lo[i];
    }

    auto widthBuf = [&] (juce::AudioBuffer<Sample>& buf, Sample w)
    {
        auto* L = buf.getWritePointer (0);
        auto* R = buf.getWritePointer (1);
        const Sample k = (Sample)0.7071067811865476;
        for (int i = 0; i < n; ++i)
        {
            Sample l=L[i], r=R[i];
            Sample M = k * (l + r);
            Sample S = k * (l - r) * w;
            L[i] = k*(M+S); R[i] = k*(M-S);
        }
    };
    widthBuf (low,  wLow);
    widthBuf (high, wHigh);

    for (int c = 0; c < 2; ++c)
    {
        auto* dst = block.getChannelPointer (c);
        auto* lo  = low.getReadPointer (c);
        auto* hi  = high.getReadPointer (c);
        for (int i = 0; i < n; ++i) dst[i] = lo[i] + hi[i];
    }
}

template <typename Sample>
void FieldChain<Sample>::applyRotationAsym (Block block, Sample rotationRad, Sample asym)
{
    if (block.getNumChannels() < 2) return;
    const Sample k = (Sample)0.7071067811865476;
    const Sample c = std::cos (rotationRad);
    const Sample s = std::sin (rotationRad);

    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const int n = (int) block.getNumSamples();
    for (int i = 0; i < n; ++i)
    {
        const Sample l = L[i], r = R[i];
        Sample M = k * (l + r);
        Sample S = k * (l - r);
        // rotation matrix in M/S
        const Sample Mr = c * M - s * S;
        const Sample Sr = s * M + c * S;
        // asymmetry: small crossfeed
        const Sample Mx = Mr + asym * Sr * (Sample)0.15;
        const Sample Sx = Sr - asym * Mr * (Sample)0.15;
        L[i] = k * (Mx + Sx);
        R[i] = k * (Mx - Sx);
    }
}

template <typename Sample>
void FieldChain<Sample>::applyMonoMaker (Block block, Sample monoHz)
{
    if (block.getNumChannels() < 2) return;
    if (monoHz <= (Sample)0) return;

    const Sample nyq = (Sample) (sr * 0.49);
    monoHz = juce::jlimit ((Sample)20, juce::jmin ((Sample)300, nyq), monoHz);

    // Slope from params (6/12/24 dB/oct)
    // Convert APVTS choice index (0/1/2) to dB/oct (6/12/24) if needed
    const int slopeDb = (params.monoSlopeDbOct <= 3 ? (params.monoSlopeDbOct == 0 ? 6 : params.monoSlopeDbOct == 1 ? 12 : 24)
                                                    : params.monoSlopeDbOct);
    monoLP.setSlopeDbPerOct (slopeDb > 0 ? slopeDb : 12);
    monoLP.setCutoff (monoHz);

    // Create temp low buffer (copy)
    juce::AudioBuffer<Sample> low (2, (int) block.getNumSamples());
    for (int c = 0; c < 2; ++c)
        low.copyFrom (c, 0, block.getChannelPointer (c), (int) block.getNumSamples());

    // Filter lows per channel with variable slope
    {
        juce::dsp::AudioBlock<Sample> lb (low);
        monoLP.processToLow (lb);
    }

    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const auto* LL = low.getReadPointer (0);
    const auto* RR = low.getReadPointer (1);
    const int N = (int) block.getNumSamples();

    for (int i = 0; i < N; ++i)
    {
        const Sample monoBass = (Sample)0.5 * (LL[i] + RR[i]);
        const Sample lHigh = L[i] - LL[i];
        const Sample rHigh = R[i] - RR[i];
        L[i] = lHigh + monoBass;
        R[i] = rHigh + monoBass;
    }
}

template <typename Sample>
void FieldChain<Sample>::applyPan (Block block, Sample pan)
{
    if (block.getNumChannels() < 2) return;
    pan = juce::jlimit ((Sample)-1, (Sample)1, pan);
    auto positionToGains = [] (Sample x, Sample& gL, Sample& gR)
    {
        const Sample theta = (juce::jlimit ((Sample)-1, (Sample)1, x) + (Sample)1) * (Sample) (juce::MathConstants<double>::pi * 0.25);
        gL = std::cos (theta); gR = std::sin (theta);
    };
    const Sample pL = juce::jlimit ((Sample)-1, (Sample)1, (Sample)-1 + pan);
    const Sample pR = juce::jlimit ((Sample)-1, (Sample)1, (Sample)+1 + pan);
    Sample gLL, gLR, gRL, gRR; positionToGains (pL, gLL, gRL); positionToGains (pR, gLR, gRR);

    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const int N = (int) block.getNumSamples();
    for (int i = 0; i < N; ++i)
    {
        const Sample l = L[i], r = R[i];
        L[i] = gLL * l + gLR * r;
        R[i] = gRL * l + gRR * r;
    }
}

template <typename Sample>
void FieldChain<Sample>::applySplitPan (Block block, Sample panL, Sample panR)
{
    if (block.getNumChannels() < 2) return;
    panL = juce::jlimit ((Sample)-1, (Sample)1, panL);
    panR = juce::jlimit ((Sample)-1, (Sample)1, panR);
    auto positionToGains = [] (Sample x, Sample& gL, Sample& gR)
    {
        const Sample theta = (juce::jlimit ((Sample)-1, (Sample)1, x) + (Sample)1) * (Sample) (juce::MathConstants<double>::pi * 0.25);
        gL = std::cos (theta); gR = std::sin (theta);
    };
    Sample gLL, gLR, gRL, gRR; positionToGains (panL, gLL, gRL); positionToGains (panR, gLR, gRR);

    auto* L = block.getChannelPointer (0);
    auto* R = block.getChannelPointer (1);
    const int N = (int) block.getNumSamples();
    for (int i = 0; i < N; ++i)
    {
        const Sample l = L[i], r = R[i];
        L[i] = gLL * l + gLR * r;
        R[i] = gRL * l + gRR * r;
    }
}

template <typename Sample>
void FieldChain<Sample>::applySaturationOnBlock (juce::dsp::AudioBlock<Sample> b, Sample driveLin)
{
    const int ch = (int) b.getNumChannels();
    const int n  = (int) b.getNumSamples();
    for (int c = 0; c < ch; ++c)
    {
        auto* d = b.getChannelPointer (c);
        for (int i = 0; i < n; ++i)
        {
            const Sample input = d[i] * driveLin;
            Sample y = softClipT (input);
            if ((double) driveLin > 2.0)
                y += (Sample) (std::sin ((double) input * 2.0) * 0.1 * ((double) driveLin - 2.0));
            d[i] = y;
        }
    }
}

template <typename Sample>
void FieldChain<Sample>::applySaturation (Block block, Sample driveLin, Sample mix01, int osModeIndex)
{
    if (mix01 <= (Sample)0.0001 || driveLin <= (Sample)1.0001) return;
    ensureOversampling (osModeIndex);

    juce::AudioBuffer<Sample> dry ((int) block.getNumChannels(), (int) block.getNumSamples());
    for (int c = 0; c < (int) block.getNumChannels(); ++c)
        dry.copyFrom (c, 0, block.getChannelPointer (c), (int) block.getNumSamples());

    if (oversampling)
    {
        auto up = oversampling->processSamplesUp (block);
        applySaturationOnBlock (up, driveLin);
        oversampling->processSamplesDown (block);
    }
    else
    {
        applySaturationOnBlock (block, driveLin);
    }

    // Mix dry back in: manual loop over AudioBlock
    const Sample dryGain = (Sample)1 - mix01;
    for (int c = 0; c < (int) block.getNumChannels(); ++c)
    {
        auto* dst = block.getChannelPointer (c);
        auto* src = dry.getReadPointer (c);
        for (int i = 0; i < (int) block.getNumSamples(); ++i)
            dst[i] += src[i] * dryGain;
    }
}

 
// Reverb algorithms (Room/Plate/Hall) macro mapping, parallel wet return

template <typename Sample>
void FieldChain<Sample>::applySpaceAlgorithm (Block block, Sample depth01, int algo)
{
    depth01 = juce::jlimit ((Sample)0, (Sample)1, depth01);

    // When depth is effectively zero, ensure no algorithm alters the dry path
    // and force wetLevel to zero. This prevents audible changes from the switch
    // while Reverb is off and saves CPU.
    if (depth01 <= (Sample) 0.0001)
    {
        rvParams.wetLevel = 0.0f;
        return;
    }

    // New macro mapping for all algorithms: 0=Room, 1=Plate, 2=Hall
    computeReverbVoicing ((int) algo, (float) depth01, rvParams);
    // Smooth macro params per-sample downstream
    roomSizeSmoothed.setTargetValue (rvParams.roomSize);
    dampingSmoothed .setTargetValue (rvParams.damping);
    widthSmoothed   .setTargetValue (rvParams.width);

    if constexpr (std::is_same_v<Sample, double>)
    {
        jassert (reverbD);
        reverbD->setParameters (rvParams);
        juce::dsp::AudioBlock<double> dblk (block);
        reverbD->processParallelMix (dblk, rvParams.wetLevel);
    }
    else
    {
        jassert (reverbF);
        // Update smoothed params then write to JUCE reverb
        auto cur = rvParams;
        cur.roomSize = roomSizeSmoothed.getNextValue();
        cur.damping  = dampingSmoothed.getNextValue();
        cur.width    = widthSmoothed.getNextValue();
        reverbF->setParameters (cur);
        // make wet copy
        juce::AudioBuffer<float> wetBuf ((int) block.getNumChannels(), (int) block.getNumSamples());
        for (int c = 0; c < (int) block.getNumChannels(); ++c)
            wetBuf.copyFrom (c, 0, block.getChannelPointer (c), (int) block.getNumSamples());
        juce::dsp::AudioBlock<float> fblk (wetBuf);
        juce::dsp::ProcessContextReplacing<float> fctx (fblk);
        reverbF->process (fctx);
        // Ensure dry is only applied once: JUCE Reverb dryLevel is zeroed in computeReverbVoicing
        // Mix wet back once using rvParams.wetLevel (dry already in dst)
        for (int c = 0; c < (int) block.getNumChannels(); ++c)
        {
            auto* dst = block.getChannelPointer (c);
            auto* src = wetBuf.getReadPointer (c);
            for (int i = 0; i < (int) block.getNumSamples(); ++i)
                dst[i] += src[i] * (Sample) rvParams.wetLevel;
        }
    }
}

// Render reverb into a provided wet buffer (same channels/samples as current block)
template <typename Sample>
void FieldChain<Sample>::renderSpaceWet (juce::AudioBuffer<Sample>& wet)
{
    const int ch = wet.getNumChannels();
    const int n  = wet.getNumSamples();
    if constexpr (std::is_same_v<Sample, double>)
    {
        juce::dsp::AudioBlock<double> b (wet);
        if (reverbD)
        {
            auto cur = rvParams; cur.dryLevel = 0.0f; cur.wetLevel = rvParams.wetLevel;
            reverbD->setParameters (cur);
            reverbD->processParallelMix (b, cur.wetLevel); // adds wet into buffer
        }
    }
    else
    {
        if (reverbF)
        {
            auto cur = rvParams;
            cur.roomSize = roomSizeSmoothed.getNextValue();
            cur.damping  = dampingSmoothed.getNextValue();
            cur.width    = widthSmoothed.getNextValue();
            reverbF->setParameters (cur);
            juce::dsp::AudioBlock<float> fblk (wet);
            juce::dsp::ProcessContextReplacing<float> fctx (fblk);
            // Clear first then render
            for (int c = 0; c < ch; ++c) wet.clear (c, 0, n);
            reverbF->process (fctx);
            // At this point wet contains 100% wet; scale by wetLevel to match reverb depth
            for (int c = 0; c < ch; ++c)
            {
                auto* d = wet.getWritePointer (c);
                for (int i = 0; i < n; ++i) d[i] *= (Sample) rvParams.wetLevel;
            }
        }
    }
}

// --------- main process (Sample) ---------

template <typename Sample>
void FieldChain<Sample>::process (Block block)
{
    if (params.bypass) return;

    // Input gain
    block.multiplyBy (params.gainLin);

    // Phase Mode: 2 = Hybrid Linear → FIR HP/LP; 3 = Full Linear → composite FIR tone+HP/LP
    if (params.phaseMode == 3)
    {
        applyFullLinearFIR (block);
    }
    else if (params.phaseMode == 2)
    {
        ensureLinearPhaseKernel (sr, params.hpHz, params.lpHz, (int) block.getNumSamples(), (int) block.getNumChannels());
        linConvolver->process (block);
    }
    else
    {
        // Clean filters
        applyHP_LP (block, params.hpHz, params.lpHz);
    }

    // Imaging & placement
    if (params.splitMode) applySplitPan (block, params.panL, params.panR);
    else                  applyPan     (block, params.pan);

    // Three-band width first
    applyThreeBandWidth (block, params.xoverLoHz, params.xoverHiHz,
                         params.widthLo, params.widthMid, params.widthHi);
    // Shuffler (2-band lightweight)
    applyShufflerWidth (block, params.shufflerXoverHz, params.shufflerLo, params.shufflerHi);
    // Width Designer: frequency-tilted S (before rotation)
    if (params.widthMode == 1)
    {
        // side-only tilt: low/high shelves around a pivot on S
        const double fs = sr;
        const double nyq = fs * 0.49;
        const double fLo = juce::jlimit (50.0,  2000.0, (double) params.widthTiltPivotHz * 0.6);
        const double fHi = juce::jlimit (800.0, juce::jmin (20000.0, nyq), (double) params.widthTiltPivotHz * 1.6);
        const double octSpan = 3.0;
        const double totalDb = (double) params.widthSideTiltDbOct * octSpan;
        const float gHi = (float) juce::Decibels::decibelsToGain ( juce::jlimit (-12.0, 12.0, totalDb * 0.5));
        const float gLo = (float) juce::Decibels::decibelsToGain (-juce::jlimit (-12.0, 12.0, totalDb * 0.5));
        const float Sshape = 0.90f;
        sTiltLow .coefficients = juce::dsp::IIR::Coefficients<Sample>::makeLowShelf  (fs, fLo, Sshape, gLo);
        sTiltHigh.coefficients = juce::dsp::IIR::Coefficients<Sample>::makeHighShelf (fs, fHi, Sshape, gHi);

        if (block.getNumChannels() >= 2 && (std::abs ((double) params.widthSideTiltDbOct) > 0.01))
        {
            const int N = (int) block.getNumSamples();
            juce::AudioBuffer<Sample> Sbuf (1, N);
            auto* L = block.getChannelPointer (0);
            auto* R = block.getChannelPointer (1);
            const Sample k = (Sample)0.7071067811865476;
            auto* S = Sbuf.getWritePointer (0);
            for (int i = 0; i < N; ++i) { S[i] = k * (L[i] - R[i]); }
            juce::dsp::AudioBlock<Sample> Sb (Sbuf);
            juce::dsp::ProcessContextReplacing<Sample> ctx (Sb);
            sTiltLow.process (ctx); sTiltHigh.process (ctx);
            for (int i = 0; i < N; ++i)
            {
                const Sample l=L[i], r=R[i];
                const Sample M = k*(l + r);
                const Sample Sf= S[i];
                L[i] = k*(M + Sf);
                R[i] = k*(M - Sf);
            }
        }
    }
    // Rotation + Asymmetry (global)
    applyRotationAsym (block, params.rotationRad, params.asymmetry);

    // Core tone
    applyTiltEQ  (block, params.tiltDb,  params.tiltFreq);
    applyScoopEQ (block, params.scoopDb, params.scoopFreq);
    applyBassShelf (block, params.bassDb, params.bassFreq);
    applyAirBand   (block, params.airDb,  params.airFreq);

    // Reverb: compute rvParams then render wet-only to buffer 'wet'
    applySpaceAlgorithm (block, params.depth, params.spaceAlgo);

    // Build dry (post tone/imaging) and wet buses (preallocated)
    const int ch = (int) block.getNumChannels();
    const int n  = (int) block.getNumSamples();
    jassert (n <= dryBusBuf.getNumSamples());
    jassert (n <= wetBusBuf.getNumSamples());
    dryBusBuf.setSize (juce::jmax (1, ch), dryBusBuf.getNumSamples(), false, false, true);
    wetBusBuf.setSize (juce::jmax (1, ch), wetBusBuf.getNumSamples(), false, false, true);
    for (int c = 0; c < ch; ++c)
    {
        auto* dst = dryBusBuf.getWritePointer (c);
        auto* src = block.getChannelPointer (c);
        std::memcpy (dst, src, sizeof (Sample) * (size_t) n);
        wetBusBuf.clear (c, 0, n);
    }
    // Render reverb into wet (100% wet)
    renderSpaceWet (wetBusBuf);

    // LF mono
    applyMonoMaker (block, params.monoHz);

    // Width Designer: dynamic clamp (after imaging/mono, before post FX)
    if (params.widthMode == 1 && params.widthAutoDepth > (Sample)0.0001 && block.getNumChannels() >= 2)
    {
        auto* L = block.getChannelPointer (0);
        auto* R = block.getChannelPointer (1);
        const int N = (int) block.getNumSamples();
        const Sample k = (Sample)0.7071067811865476;

        long double sumM2=0.0L, sumS2=0.0L;
        for (int i=0;i<N;++i)
        {
            const Sample l=L[i], r=R[i];
            const Sample M = k*(l + r);
            const Sample S = k*(l - r);
            sumM2 += (long double) (M*M);
            sumS2 += (long double) (S*S);
        }
        const double rmsM = std::sqrt ((double)sumM2 / juce::jmax (1, N));
        const double rmsS = std::sqrt ((double)sumS2 / juce::jmax (1, N));
        const double smDb = juce::Decibels::gainToDecibels ((float)(rmsS / (rmsM + 1e-20)));

        double over = smDb - (double) params.widthAutoThrDb;
        over = juce::jlimit (0.0, 24.0, over);
        const double red = (double) params.widthAutoDepth * (over / 12.0);
        const double gTarget = std::pow (10.0, -red / 20.0);

        Sample g = aw_env;
        const Sample gT = (Sample) gTarget;
        for (int i = 0; i < N; ++i)
        {
            const Sample alpha = (gT < g) ? aw_alphaAtk : aw_alphaRel;
            g = g + alpha * (gT - g);
            aw_env = g;

            const Sample l=L[i], r=R[i];
            Sample M = k*(l + r);
            Sample S = k*(l - r);
            const Sample ceiling = params.widthMax;
            const Sample sClamp  = juce::jmin (g, ceiling);
            S *= sClamp;
            L[i] = k*(M + S);
            R[i] = k*(M - S);
        }
    }

    // Nonlinear (apply saturation equally to dry and wet prior to sum) to preserve FX tone
    {
        juce::dsp::AudioBlock<Sample> dryBlock (dryBusBuf);
        juce::dsp::AudioBlock<Sample> wetBlock (wetBusBuf);
        applySaturation (dryBlock, params.satDriveLin, params.satMix, params.osMode);
        applySaturation (wetBlock, params.satDriveLin, params.satMix, params.osMode);
    }
    
    // Delay processing
    if (params.delayEnabled)
    {
        // Convert parameters to DelayParams structure
        DelayParams delayParams;
        delayParams.enabled = params.delayEnabled;
        delayParams.mode = params.delayMode;
        delayParams.sync = params.delaySync;
        delayParams.timeMs = params.delayTimeMs;
        delayParams.timeDiv = params.delayTimeDiv;
        // Pass grid flavor and tempo for sync mode from FieldParams snapshot
        delayParams.gridFlavor = params.delayGridFlavor;
        delayParams.tempoBpm   = params.tempoBpm;
        delayParams.feedbackPct = params.delayFeedbackPct;
        delayParams.wet = params.delayWet;
        delayParams.killDry = params.delayKillDry;
        delayParams.freeze = params.delayFreeze;
        delayParams.pingpong = params.delayPingpong;
        delayParams.crossfeedPct = params.delayCrossfeedPct;
        delayParams.stereoSpreadPct = params.delayStereoSpreadPct;
        delayParams.width = params.delayWidth;
        delayParams.modRateHz = params.delayModRateHz;
        delayParams.modDepthMs = params.delayModDepthMs;
        delayParams.wowflutter = params.delayWowflutter;
        delayParams.jitterPct = params.delayJitterPct;
        delayParams.hpHz = params.delayHpHz;
        delayParams.lpHz = params.delayLpHz;
        delayParams.tiltDb = params.delayTiltDb;
        delayParams.sat = params.delaySat;
        delayParams.diffusion = params.delayDiffusion;
        delayParams.diffuseSizeMs = params.delayDiffuseSizeMs;
        delayParams.duckSource = params.delayDuckSource;
        delayParams.duckPost = params.delayDuckPost;
        delayParams.duckDepth = params.delayDuckDepth;
        delayParams.duckAttackMs = params.delayDuckAttackMs;
        delayParams.duckReleaseMs = params.delayDuckReleaseMs;
        delayParams.duckThresholdDb = params.delayDuckThresholdDb;
        delayParams.duckRatio = params.delayDuckRatio;
        delayParams.duckLookaheadMs = params.delayDuckLookaheadMs;
        delayParams.duckLinkGlobal = params.delayDuckLinkGlobal;
        
        delayEngine.setParameters(delayParams);
        
        // Process delay on the main block
        float scL = 0.0f, scR = 0.0f;
        if (ch > 0) scL = (float)block.getSample(0, 0);
        if (ch > 1) scR = (float)block.getSample(1, 0);
        delayEngine.process(block, scL, scR);
        // After delayEngine updated the block in-place, reflect that into the dry bus
        for (int c = 0; c < ch; ++c)
        {
            auto* dst = dryBusBuf.getWritePointer (c);
            auto* src = block.getChannelPointer (c);
            std::memcpy (dst, src, sizeof (Sample) * (size_t) n);
        }
    }

    // Duck wet against dry (WetOnly), only when Reverb wet is active to save CPU
    const bool spaceWetActive = (rvParams.wetLevel > 0.0001f);
    if (params.ducking > (Sample) 0.001 && spaceWetActive)
    {
        fielddsp::DuckParams p;
        p.maxDepthDb  = (float) juce::jlimit ((double)0.0, (double)36.0, (double) params.ducking * 24.0);
        p.thresholdDb = (float) params.duckThresholdDb;
        p.kneeDb      = (float) params.duckKneeDb;
        p.ratio       = (float) params.duckRatio;
        p.attackMs    = (float) params.duckAttackMs;
        p.releaseMs   = (float) params.duckReleaseMs;
        p.lookaheadMs = (float) params.duckLookaheadMs;
        p.rmsMs       = (float) params.duckRmsMs;
        p.bypass      = (p.maxDepthDb <= 0.001f);
        ducker.setParams (p);

        ducker.processWet (wetBusBuf.getWritePointer (0), wetBusBuf.getWritePointer (juce::jmin (1, ch-1)),
                           dryBusBuf.getReadPointer (0), dryBusBuf.getReadPointer (juce::jmin (1, ch-1)), n);
    }
    else
    {
        // Skip ducking entirely when reverb wet is zero; UI will idle the GR meter
    }

    // Equal-power mix: apply smoothed wet to avoid clicks
    wetMixSmoothed.setTargetValue (rvParams.wetLevel);
    // Sum Dry + Wet back to output block (equal-power law):
    // mix=0 -> 100% dry, mix=1 -> 100% wet
    for (int c = 0; c < ch; ++c)
    {
        auto* out = block.getChannelPointer (c);
        const Sample* d = dryBusBuf.getReadPointer (c);
        const Sample* w = wetBusBuf.getReadPointer (c);
        for (int i = 0; i < n; ++i)
        {
            const float mix = wetMixSmoothed.getNextValue();
            const float theta = juce::jlimit (0.0f, 1.0f, mix) * juce::MathConstants<float>::halfPi;
            const float a = std::cos (theta);
            const float b = std::sin (theta);
            out[i] = (Sample) (a * (double) d[i] + b * (double) w[i]);
        }
    }
}

template <typename Sample>
void FieldChain<Sample>::ensureLinearPhaseKernel (double sampleRate, Sample hpHz, Sample lpHz, int maxBlock, int numChannels)
{
    if (! linConvolver)
        linConvolver = std::make_unique<OverlapSaveConvolver<Sample>>();
    linConvolver->prepare (sampleRate, maxBlock, linKernelLen, juce::jmax (1, numChannels));

    const double hp = juce::jlimit (20.0, 1000.0, (double) hpHz);
    const double lp = juce::jlimit (1000.0, 20000.0, (double) lpHz);
    if (! linConvolver->isReady() || (float) hp != lastHpHzLP || (float) lp != lastLpHzLP)
    {
        std::vector<float> kernel; designLinearPhaseBandpassKernel (kernel, sampleRate, hp, lp, linKernelLen, 8.6);
        linConvolver->setKernel (kernel);
        lastHpHzLP = (float) hp; lastLpHzLP = (float) lp;
    }
}

template <typename Sample>
float FieldChain<Sample>::getCurrentDuckGrDb() const
{
    // Return current gain reduction in dB (>=0). Safe cast to float for metering.
    return (float) ducker.getCurrentGainReductionDb();
}

// Explicit instantiation
template struct FieldChain<float>;
template struct FieldChain<double>;
