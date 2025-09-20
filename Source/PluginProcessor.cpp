#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DevNotes.h"
#include "reverb/ReverbParameters.h"

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
    // Quality / Precision controls
    static constexpr const char* quality    = "quality";      // 0 Eco, 1 Standard, 2 High
    static constexpr const char* precision  = "precision";    // 0 Auto(Host), 1 Force32, 2 Force64
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

// ===== [UTIL] APVTS raw access helper =====
// Why: one-liner get with assert on missing IDs
DEV_BANNER("UTIL", "APVTS raw access", "assert on missing IDs")
static inline float getParam (juce::AudioProcessorValueTreeState& apvts, const char* id)
{
    if (auto* p = apvts.getRawParameterValue (id)) return p->load();
    jassertfalse; return 0.0f;
}

// ===== [REVERB] Macro voicing helper =====
// Why: map Room/Plate/Hall + Depth to JUCE Reverb params
DEV_BANNER("REVERB", "Macro voicing helper", "compute parameters from macro")
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
            dampHz  = lerpFloat (6000.f, 8000.0f, d);
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
, apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    // Templated chain instances
    chainF = std::make_unique<FieldChain<float>>();
    chainD = std::make_unique<FieldChain<double>>();
    // eco flag removed

    // Keep existing smoothers if declared in the header (no harm if unused here)
    for (auto* s : { &panSmoothed, &panLSmoothed, &panRSmoothed, &depthSmoothed, &widthSmoothed, &gainSmoothed, &tiltSmoothed,
                     &hpHzSmoothed, &lpHzSmoothed, &monoHzSmoothed,
                     &satDriveLin, &satMixSmoothed, &airSmoothed, &bassSmoothed, &duckingSmoothed })
        s->reset (currentSR, 0.005); // 5 ms smoothing

    apvts.addParameterListener (IDs::pan,  this);
    apvts.addParameterListener (IDs::gain, this);
    // Listen for quality/precision changes
    apvts.addParameterListener (IDs::quality,   this);
    apvts.addParameterListener (IDs::precision, this);
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
    // Prepare alias guards for OS-off nonlinear protection
    chainF->prepareAliasGuards (sampleRate);
    chainD->prepareAliasGuards (sampleRate);
    // Resize scratch for potential 32f->64f internal hop
    const int chans = juce::jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());
    scratch64.resize ((size_t) chans * (size_t) samplesPerBlock);
    // Apply initial quality/precision profile
    applyQualityFromParams();
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

    // Prepare new Reverb engine
    reverbEngine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    // Prepare delay UI bridge
    delayUiBridge.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    // Lock latency after initial compute
    updateLatencyForPhaseMode();
    latencyLocked = true;
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
    // Read quality/precision (if UI wants to branch inside chain later)
    // int quality = (int) apvts.getParameterAsValue (IDs::quality).getValue();
    // int precision = (int) apvts.getParameterAsValue (IDs::precision).getValue();
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
    // Reverb params ingress (APVTS -> HostParams)
    p.rvEnabled       = apvts.getRawParameterValue (ReverbIDs::enabled)->load() > 0.5f;
    p.rvKillDry       = apvts.getRawParameterValue (ReverbIDs::killDry)->load() > 0.5f;
    p.rvAlgo          = (int) apvts.getParameterAsValue (ReverbIDs::algo).getValue();
    p.rvPreDelayMs    = apvts.getRawParameterValue (ReverbIDs::preDelayMs)->load();
    p.rvDecaySec      = apvts.getRawParameterValue (ReverbIDs::decaySec)->load();
    p.rvDensityPct    = apvts.getRawParameterValue (ReverbIDs::densityPct)->load();
    p.rvDiffusionPct  = apvts.getRawParameterValue (ReverbIDs::diffusionPct)->load();
    p.rvModDepthCents = apvts.getRawParameterValue (ReverbIDs::modDepthCents)->load();
    p.rvModRateHz     = apvts.getRawParameterValue (ReverbIDs::modRateHz)->load();
    p.rvErLevelDb     = apvts.getRawParameterValue (ReverbIDs::erLevelDb)->load();
    p.rvErTimeMs      = apvts.getRawParameterValue (ReverbIDs::erTimeMs)->load();
    p.rvErDensityPct  = apvts.getRawParameterValue (ReverbIDs::erDensityPct)->load();
    p.rvErWidthPct    = apvts.getRawParameterValue (ReverbIDs::erWidthPct)->load();
    p.rvErToTailPct   = apvts.getRawParameterValue (ReverbIDs::erToTailPct)->load();
    p.rvHpfHz         = apvts.getRawParameterValue (ReverbIDs::hpfHz)->load();
    p.rvLpfHz         = apvts.getRawParameterValue (ReverbIDs::lpfHz)->load();
    p.rvTiltDb        = apvts.getRawParameterValue (ReverbIDs::tiltDb)->load();
    p.rvDreqLowX      = apvts.getRawParameterValue (ReverbIDs::dreqLowX)->load();
    p.rvDreqMidX      = apvts.getRawParameterValue (ReverbIDs::dreqMidX)->load();
    p.rvDreqHighX     = apvts.getRawParameterValue (ReverbIDs::dreqHighX)->load();
    p.rvWidthPct      = apvts.getRawParameterValue (ReverbIDs::widthPct)->load();
    p.rvWet01         = apvts.getRawParameterValue (ReverbIDs::wetMix01)->load();
    p.rvOutTrimDb     = apvts.getRawParameterValue (ReverbIDs::outTrimDb)->load();
    
    return p;
}

// Float path
void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals _;
    isDoublePrecEnabled = false;

    if (buffer.getNumSamples() <= 0) return;

    // Emergency safety: hard passthrough to confirm architecture vs. processing
    if (getSafePassthrough()) return;

    // Optional internal 64f hop on 32f hosts based on Precision parameter
    const int pMode = precisionMode.load(); // 0 Auto, 1 Force32, 2 Force64
    const bool want64Internal = (pMode == 2);
    if (want64Internal)
    {
        const int C = buffer.getNumChannels();
        const int N = buffer.getNumSamples();
        if ((int) scratch64.size() >= C * N)
        {
            double* d = scratch64.data();
            for (int ch = 0; ch < C; ++ch)
            {
                const float* src = buffer.getReadPointer (ch);
                double* dst = d + (size_t) ch * (size_t) N;
                for (int i = 0; i < N; ++i) dst[i] = (double) src[i];
            }

            std::vector<double*> chPtrs ((size_t) C);
            for (int ch = 0; ch < C; ++ch)
                chPtrs[(size_t) ch] = d + (size_t) ch * (size_t) N;
            juce::AudioBuffer<double> tmp (chPtrs.data(), C, N);
            processBlock (tmp, midi);

            for (int ch = 0; ch < C; ++ch)
            {
                float* dst = buffer.getWritePointer (ch);
                const double* src = d + (size_t) ch * (size_t) N;
                for (int i = 0; i < N; ++i) dst[i] = (float) src[i];
            }
            return;
        }
    }

    // Pre-DSP visualization feed (lock-free bus)
    if (buffer.getNumSamples() > 0 && buffer.getNumChannels() > 0)
    {
        const int chL = 0;
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
    // Update Motion host sync so Sync mode rates are correct
    motion::HostInfo hinfo; hinfo.bpm = hp.tempoBpm; hinfo.playing = transportIsPlaying.load();
    if (auto* ph = getPlayHead()) { if (auto pos = ph->getPosition()) { if (auto ppq = pos->getPpqPosition()) hinfo.ppqPosition = *ppq; if (auto bar = pos->getPpqPositionOfLastBarStart()) hinfo.ppqBarStart = *bar; }}
    hinfo.samplesPerBeat = (currentSR > 0.0 ? currentSR * 60.0 / juce::jmax (1e-6, hp.tempoBpm) : 0.0);
    motionEngine.setHostSync(hinfo);

    // Capture input RMS before processing
    {
        const int n = buffer.getNumSamples();
        if (n > 0 && buffer.getNumChannels() > 0)
        {
            long double s=0.0L; int cN = buffer.getNumChannels();
            for (int c=0;c<cN;++c){ auto* d = buffer.getReadPointer(c); for (int i=0;i<n;++i) { long double v=d[i]; s += v*v; } }
            const float rmsIn = (float) std::sqrt ((double) (s / juce::jmax (1, cN * n)));
            float o = meterInRms.load(); meterInRms.store (o + 0.2f * (rmsIn - o));
        }
    }

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
        const int chL = 0;
        const int chR = buffer.getNumChannels() > 1 ? 1 : 0;
        visPost.push (buffer.getReadPointer (chL),
                      buffer.getNumChannels() > 1 ? buffer.getReadPointer (chR) : nullptr,
                      buffer.getNumSamples());
    }

    // Feed Delay UI metrics (float path)
    {
        DelayMetricsFrame f;
        // Map HostParams to metrics
        f.tempoBpm = hp.tempoBpm; f.sync = hp.delaySync; f.timeDiv = hp.delayTimeDiv; f.gridFlavor = hp.delayGridFlavor; f.timeMs = hp.delayTimeMs;
        f.stereoSpreadPct = hp.delayStereoSpreadPct; f.pingpong = hp.delayPingpong; f.crossfeedPct = hp.delayCrossfeedPct; f.width = hp.delayWidth;
        f.feedbackPct = hp.delayFeedbackPct; f.wet01 = hp.delayWet; f.killDry = hp.delayKillDry; f.freeze = hp.delayFreeze;
        f.mode = hp.delayMode; f.modRateHz = hp.delayModRateHz; f.modDepthMs = hp.delayModDepthMs; f.jitterPct = hp.delayJitterPct;
        f.hpHz = hp.delayHpHz; f.lpHz = hp.delayLpHz; f.tiltDb = hp.delayTiltDb; f.sat = hp.delaySat; f.diffusion = hp.delayDiffusion; f.diffuseSizeMs = hp.delayDiffuseSizeMs;
        f.duckSource = hp.delayDuckSource; f.duckPost = hp.delayDuckPost; f.duckThrDb = hp.delayDuckThresholdDb; f.duckRatio = hp.delayDuckRatio; f.duckDepth = hp.delayDuckDepth; f.duckAtkMs = hp.delayDuckAttackMs; f.duckRelMs = hp.delayDuckReleaseMs; f.duckLookMs = hp.delayDuckLookaheadMs;
        f.duckGrDb = getCurrentDuckGrDb();
        // Simple block RMS estimates
        auto rmsOf = [] (const float* d, int n) -> float { long double s=0.0; for (int i=0;i<n;++i){ float v=d[i]; s+= (long double) v*v; } return std::sqrt ((double)(s / juce::jmax (1, n))); };
        if (buffer.getNumChannels() >= 2)
        {
            f.postRmsL = rmsOf (buffer.getReadPointer(0), buffer.getNumSamples());
            f.postRmsR = rmsOf (buffer.getReadPointer(1), buffer.getNumSamples());
            // Pre-RMS fallback to post
            f.preRmsL = f.postRmsL;
            f.preRmsR = f.postRmsR;
        }
        // If available, include delay wet RMS and last effective delay samples
        if (chainF)
        {
            f.wetRmsL = chainF->getDelayWetRmsL();
            f.wetRmsR = chainF->getDelayWetRmsR();
            f.timeSamplesL = chainF->getDelayLastSamplesL();
            f.timeSamplesR = chainF->getDelayLastSamplesR();
        }
        delayUiBridge.pushMetrics (f);
    }

    // Output RMS after processing
    {
        const int n = buffer.getNumSamples();
        if (n > 0 && buffer.getNumChannels() > 0)
        {
            long double s=0.0L; int cN = buffer.getNumChannels();
            for (int c=0;c<cN;++c){ auto* d = buffer.getReadPointer(c); for (int i=0;i<n;++i) { long double v=d[i]; s += v*v; } }
            const float rmsOut = (float) std::sqrt ((double) (s / juce::jmax (1, cN * n)));
            float o = meterOutRms.load(); meterOutRms.store (o + 0.2f * (rmsOut - o));
        }
    }

    // (XY oscilloscope will also read from visPost on the UI thread)
}

// Double path
void MyPluginAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals _;
    isDoublePrecEnabled = true;

    if (buffer.getNumSamples() <= 0) return;

    // Emergency safety: hard passthrough to confirm architecture vs. processing
    if (getSafePassthrough()) return;

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
    // Update Motion host sync for double path as well
    motion::HostInfo hinfoD; hinfoD.bpm = hp.tempoBpm; hinfoD.playing = transportIsPlaying.load();
    if (auto* ph2 = getPlayHead()) { if (auto pos = ph2->getPosition()) { if (auto ppq = pos->getPpqPosition()) hinfoD.ppqPosition = *ppq; if (auto bar = pos->getPpqPositionOfLastBarStart()) hinfoD.ppqBarStart = *bar; }}
    hinfoD.samplesPerBeat = (currentSR > 0.0 ? currentSR * 60.0 / juce::jmax (1e-6, hp.tempoBpm) : 0.0);
    motionEngine.setHostSync(hinfoD);

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

    // Feed Delay UI metrics (double path)
    {
        DelayMetricsFrame f;
        f.tempoBpm = hp.tempoBpm; f.sync = hp.delaySync; f.timeDiv = hp.delayTimeDiv; f.gridFlavor = hp.delayGridFlavor; f.timeMs = hp.delayTimeMs;
        f.stereoSpreadPct = hp.delayStereoSpreadPct; f.pingpong = hp.delayPingpong; f.crossfeedPct = hp.delayCrossfeedPct; f.width = hp.delayWidth;
        f.feedbackPct = hp.delayFeedbackPct; f.wet01 = hp.delayWet; f.killDry = hp.delayKillDry; f.freeze = hp.delayFreeze;
        f.mode = hp.delayMode; f.modRateHz = hp.delayModRateHz; f.modDepthMs = hp.delayModDepthMs; f.jitterPct = hp.delayJitterPct;
        f.hpHz = hp.delayHpHz; f.lpHz = hp.delayLpHz; f.tiltDb = hp.delayTiltDb; f.sat = hp.delaySat; f.diffusion = hp.delayDiffusion; f.diffuseSizeMs = hp.delayDiffuseSizeMs;
        f.duckSource = hp.delayDuckSource; f.duckPost = hp.delayDuckPost; f.duckThrDb = hp.delayDuckThresholdDb; f.duckRatio = hp.delayDuckRatio; f.duckDepth = hp.delayDuckDepth; f.duckAtkMs = hp.delayDuckAttackMs; f.duckRelMs = hp.delayDuckReleaseMs; f.duckLookMs = hp.delayDuckLookaheadMs;
        f.duckGrDb = getCurrentDuckGrDb();
        auto rmsOfD = [] (const double* d, int n) -> float { long double s=0.0; for (int i=0;i<n;++i){ double v=d[i]; s+= v*v; } return (float) std::sqrt ((double)(s / juce::jmax (1, n))); };
        if (buffer.getNumChannels() >= 2)
        {
            f.postRmsL = rmsOfD (buffer.getReadPointer(0), buffer.getNumSamples());
            f.postRmsR = rmsOfD (buffer.getReadPointer(1), buffer.getNumSamples());
            f.preRmsL = f.postRmsL; // fallback
            f.preRmsR = f.postRmsR; // fallback
        }
        if (chainD)
        {
            f.wetRmsL = chainD->getDelayWetRmsL();
            f.wetRmsR = chainD->getDelayWetRmsR();
            f.timeSamplesL = chainD->getDelayLastSamplesL();
            f.timeSamplesR = chainD->getDelayLastSamplesR();
        }
        delayUiBridge.pushMetrics (f);
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
        if (mode == 3)
        {
            // Full Linear: use fullLinearConvolver latency
            if (isDoublePrecEnabled && chainD)
                latency = chainD->getFullLinearLatencySamples();
            else if (chainF)
                latency = chainF->getFullLinearLatencySamples();
        }
        else
        {
            // Hybrid Linear: use linConvolver latency
            if (isDoublePrecEnabled && chainD)
                latency = chainD->getLinearPhaseLatencySamples();
            else if (chainF)
                latency = chainF->getLinearPhaseLatencySamples();
        }
    }
    if (!latencyLocked)
        setLatencySamples (latency);
}

void MyPluginAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused (newValue);
    if (parameterID == IDs::phaseMode || parameterID == IDs::hpHz || parameterID == IDs::lpHz)
        updateLatencyForPhaseMode();
    if (parameterID == IDs::quality || parameterID == IDs::precision)
        applyQualityFromParams();
    // Detect explicit user overrides on os_mode and phase_mode so quality stops forcing them
    if (parameterID == IDs::osMode && !qualityApplyingGuard.load())
    {
        userOsOverride.store (true);
        osFollowQuality.store (false);
    }
    if (parameterID == IDs::phaseMode && !qualityApplyingGuard.load())
    {
        userPhaseOverride.store (true);
        phaseFollowQuality.store (false);
    }
    
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

// Map APVTS quality/precision to internal state and any derived settings
void MyPluginAudioProcessor::applyQualityFromParams()
{
    int q = 1, p = 0;
    if (auto* qp = apvts.getParameter (IDs::quality))
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (qp)) q = c->getIndex();
    if (auto* pp = apvts.getParameter (IDs::precision))
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (pp)) p = c->getIndex();

    qualityMode.store (q);
    precisionMode.store (p);
    // Apply recommended os_mode and phase_mode only if following is enabled
    const auto setChoiceIndex = [this] (const juce::String& pid, int idx)
    {
        if (auto* p = apvts.getParameter (pid))
        {
            if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (p))
            {
                const int cur = c->getIndex();
                if (cur != idx)
                {
                    qualityApplyingGuard.store (true);
                    p->beginChangeGesture();
                    const int n = c->choices.size();
                    const float norm = n > 1 ? (float) idx / (float) (n - 1) : 0.0f;
                    p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, norm));
                    p->endChangeGesture();
                    qualityApplyingGuard.store (false);
                }
            }
        }
    };

    // Recommend values per quality
    int recOs = 0; // Off by default
    int recPhase = 3; // Prefer Full Linear by default
    switch (q)
    {
        case 0: /* Eco    */ recOs = 0; recPhase = 0; break;
        case 2: /* High   */ recOs = 2; recPhase = 3; break; // 4x OS, Full Linear
        default:/* Standard*/ recOs = 1; recPhase = 2; break; // 2x OS, Hybrid Linear
    }

    if (osFollowQuality.load())    setChoiceIndex (IDs::osMode,    recOs);
    if (phaseFollowQuality.load()) setChoiceIndex (IDs::phaseMode,  recPhase);
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
    // Set a non-zero default so Reverb is audible when enabled
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::depth, 1 }, "Depth", juce::NormalisableRange<float> (0.0f, 1.0f, 0.0001f), 0.35f));
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
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::airDb, 1 }, "Air", juce::NormalisableRange<float> (0.0f, 6.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::bassDb, 1 }, "Bass", juce::NormalisableRange<float> (-6.0f, 6.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::ducking, 1 }, "Ducking", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckThrDb, 1 },  "Duck Threshold (dB)", juce::NormalisableRange<float> (-60.0f, 0.0f, 0.01f), -18.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckKneeDb, 1 }, "Duck Knee (dB)",      juce::NormalisableRange<float> (0.0f, 18.0f, 0.01f), 6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckRatio, 1 },  "Duck Ratio",          juce::NormalisableRange<float> (1.0f, 20.0f, 0.01f), 6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckAtkMs, 1 },  "Duck Attack (ms)",    juce::NormalisableRange<float> (1.0f, 80.0f, 0.01f), 12.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckRelMs, 1 },  "Duck Release (ms)",   juce::NormalisableRange<float> (20.0f, 800.0f, 0.1f), 180.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckLAms, 1 },   "Duck Lookahead (ms)", juce::NormalisableRange<float> (0.0f, 20.0f, 0.01f), 5.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ IDs::duckRmsMs, 1 },  "Duck RMS (ms)",       juce::NormalisableRange<float> (2.0f, 50.0f, 0.01f), 15.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::duckTarget, 1 }, "Duck Target", juce::StringArray { "WetOnly", "Global" }, 0));
    // Quality / Precision user controls
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::quality, 1 },   "Quality",   juce::StringArray { "Eco", "Standard", "High" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::precision, 1 }, "Precision", juce::StringArray { "Auto (Host)", "Force 32-bit", "Force 64-bit" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ IDs::osMode, 1 }, "Oversampling", juce::StringArray { "Off", "2x", "4x", "8x", "16x" }, 0));
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
    // Default Motion to OFF
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id::enable, 1 }, "Motion Enable", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ id::panner_select, 1 }, "Motion Panner", choiceListPanner(), (int)PannerSelect::P1));
    params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ id::headphone_safe, 1 }, "Headphone Safe", false));
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

    // Append new Reverb parameters
    addReverbParameters (params);

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
    // DC blocker
    dcBlocker.prepare (spec);
    dcBlocker.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    dcBlocker.setCutoffFrequency ((Sample) 8.0);
    dcBlocker.setResonance ((Sample) 0.707);
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
    // Initialize HP/LP with safe fully-open defaults to avoid first-block artifacts
    hpFilter.setCutoffFrequency ((Sample) 20);
    lpFilter.setCutoffFrequency ((Sample) juce::jmin<Sample> ((Sample)20000, (Sample)(sr*0.49)));
    hpFilter.setResonance ((Sample) 0.707);
    lpFilter.setResonance ((Sample) 0.707);

    lrHpL.prepare (spec);
    lrHpR.prepare (spec);
    lrLpL.prepare (spec);
    lrLpR.prepare (spec);

    lrHpL.setType (juce::dsp::LinkwitzRileyFilterType::highpass);
    lrHpR.setType (juce::dsp::LinkwitzRileyFilterType::highpass);
    lrLpL.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    lrLpR.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);

    lrHpL.setCutoffFrequency ((Sample) 20);
    lrHpR.setCutoffFrequency ((Sample) 20);
    lrLpL.setCutoffFrequency ((Sample) juce::jmin<Sample> ((Sample)20000, (Sample)(sr*0.49)));
    lrLpR.setCutoffFrequency ((Sample) juce::jmin<Sample> ((Sample)20000, (Sample)(sr*0.49)));

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

    // Init tone smoothers (slightly slower for silkier feel)
    const double toneSmoothMs = 0.015;   // slightly slower smoothing to reduce zipper/crackle
    const double hpLpSmoothMs = 0.060;   // base smoothing
    for (auto* s : { &tiltDbSm, &tiltFreqSm, &bassDbSm, &bassFreqSm, &airDbSm, &airFreqSm, &scoopDbSm, &scoopFreqSm })
        s->reset (spec.sampleRate, toneSmoothMs);
    for (auto* s : { &hpHzSm, &lpHzSm })
        s->reset (spec.sampleRate, hpLpSmoothMs);
    // Init HP/LP logs so first exp() is valid
    hpHzSm.setCurrentAndTargetValue ((Sample) std::log (20.0));
    lpHzSm.setCurrentAndTargetValue ((Sample) std::log (juce::jmin (20000.0, spec.sampleRate * 0.49)));
    // Smooth engage mix
    hpLpEngage.reset (spec.sampleRate, 0.06);
    hpLpEngage.setCurrentAndTargetValue ((Sample) 0);
    tiltDbSm.setCurrentAndTargetValue ((Sample) 0);
    tiltFreqSm.setCurrentAndTargetValue ((Sample) 500);
    bassDbSm.setCurrentAndTargetValue ((Sample) 0);
    bassFreqSm.setCurrentAndTargetValue ((Sample) 120);
    airDbSm.setCurrentAndTargetValue ((Sample) 0);
    airFreqSm.setCurrentAndTargetValue ((Sample) 8000);
    scoopDbSm.setCurrentAndTargetValue ((Sample) 0);
    scoopFreqSm.setCurrentAndTargetValue ((Sample) 1000);
    hpHzSm.setCurrentAndTargetValue ((Sample) 20);
    lpHzSm.setCurrentAndTargetValue ((Sample) 20000);
}

template <typename Sample>
void FieldChain<Sample>::reset()
{
    hpFilter.reset(); lpFilter.reset(); monoLP.reset(); depthLPF.reset();
    lowShelf.reset(); highShelf.reset(); airFilter.reset(); bassFilter.reset(); scoopFilter.reset();
    dcBlocker.reset();
    if (oversampling) oversampling->reset();
    if constexpr (std::is_same_v<Sample, double>) { if (reverbD) reverbD->reverbF.reset(); }
    else                                           { /* removed JUCE Reverb reset */ }
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
    // Detect macro tone edits to trigger temporary Full Linear mode
    const bool toneChanged = (
        params.tiltDb   != (Sample) hp.tiltDb   ||
        params.scoopDb  != (Sample) hp.scoopDb  ||
        params.hpHz     != (Sample) hp.hpHz     ||
        params.lpHz     != (Sample) hp.lpHz     ||
        params.airDb    != (Sample) hp.airDb    ||
        params.bassDb   != (Sample) hp.bassDb   ||
        params.tiltFreq != (Sample) hp.tiltFreq ||
        params.scoopFreq!= (Sample) hp.scoopFreq||
        params.bassFreq != (Sample) hp.bassFreq ||
        params.airFreq  != (Sample) hp.airFreq);

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
    // Push smoothed targets
    tiltDbSm.setTargetValue   (params.tiltDb);
    tiltFreqSm.setTargetValue (juce::jlimit ((Sample) 50,  (Sample) 5000, params.tiltFreq));
    bassDbSm.setTargetValue   (params.bassDb);
    bassFreqSm.setTargetValue (juce::jlimit ((Sample) 20,  (Sample) 2000, params.bassFreq));
    airDbSm.setTargetValue    (params.airDb);
    airFreqSm.setTargetValue  (juce::jlimit ((Sample) 2000, (Sample) 20000, params.airFreq));
    scoopDbSm.setTargetValue  (params.scoopDb);
    scoopFreqSm.setTargetValue(juce::jlimit ((Sample) 100, (Sample) 12000, params.scoopFreq));
    // Push HP/LP targets in log-domain for perceptual smoothing
    {
        const Sample nyq = (Sample) (sr * 0.49);
        const Sample hpT = juce::jlimit ((Sample) 20,   (Sample) 1000,  params.hpHz);
        const Sample lpCeil = juce::jmin ((Sample) 20000, nyq * (Sample) 0.45);
        const Sample lpT = juce::jlimit ((Sample) 1000, lpCeil, params.lpHz);
        hpHzSm.setTargetValue ((Sample) std::log ((double) hpT));
        lpHzSm.setTargetValue ((Sample) std::log ((double) lpT));
    }

    // If tone changed, hold auto-linear for a short window (converted to samples)
    if (paramsPrimed && toneChanged)
    {
        if (params.phaseMode >= 2)
        {
            const int add = (int) std::ceil (autoLinearHoldSec * sr);
            autoLinearSamplesLeft = juce::jmax (autoLinearSamplesLeft, add);
            fullLinearKernelDirty = true; // trigger FIR redesign once
        }
        else
        {
            autoLinearSamplesLeft = 0; // do not arm FIR in Zero/Natural
        }
    }
    paramsPrimed = true;
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
    // Reverb (cast to Sample)
    params.rvEnabled       = hp.rvEnabled;
    params.rvKillDry       = hp.rvKillDry;
    params.rvAlgo          = hp.rvAlgo;
    params.rvPreDelayMs    = (Sample) hp.rvPreDelayMs;
    params.rvDecaySec      = (Sample) hp.rvDecaySec;
    params.rvDensityPct    = (Sample) hp.rvDensityPct;
    params.rvDiffusionPct  = (Sample) hp.rvDiffusionPct;
    params.rvModDepthCents = (Sample) hp.rvModDepthCents;
    params.rvModRateHz     = (Sample) hp.rvModRateHz;
    params.rvErLevelDb     = (Sample) hp.rvErLevelDb;
    params.rvErTimeMs      = (Sample) hp.rvErTimeMs;
    params.rvErDensityPct  = (Sample) hp.rvErDensityPct;
    params.rvErWidthPct    = (Sample) hp.rvErWidthPct;
    params.rvErToTailPct   = (Sample) hp.rvErToTailPct;
    params.rvHpfHz         = (Sample) hp.rvHpfHz;
    params.rvLpfHz         = (Sample) hp.rvLpfHz;
    params.rvTiltDb        = (Sample) hp.rvTiltDb;
    params.rvDreqLowX      = (Sample) hp.rvDreqLowX;
    params.rvDreqMidX      = (Sample) hp.rvDreqMidX;
    params.rvDreqHighX     = (Sample) hp.rvDreqHighX;
    params.rvWidthPct      = (Sample) hp.rvWidthPct;
    params.rvWet01         = (Sample) hp.rvWet01;
    params.rvOutTrimDb     = (Sample) hp.rvOutTrimDb;
    params.phaseMode = hp.phaseMode;
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
    const Sample nyq = (Sample) (sr * 0.49);
    hpHz = juce::jlimit ((Sample) 20,  juce::jmin ((Sample) 1000, nyq),  hpHz);
    lpHz = juce::jlimit ((Sample) 1000, juce::jmin ((Sample) 20000, nyq * (Sample) 0.45), lpHz);
    if (lpHz <= hpHz) lpHz = juce::jlimit ((Sample) (hpHz + (Sample) 50), juce::jmin ((Sample) 20000, nyq * (Sample) 0.45), lpHz);
    // Safety: avoid razor-thin band-pass which sounds like a telephone
    if (hpHz >= lpHz - (Sample) 50)
    {
        if (hpHz > (Sample) 20)
        {
            hpFilter.setCutoffFrequency (hpHz);
            const Sample Qhp = params.eqQLink ? params.filterQ : params.hpQ;
            hpFilter.setResonance (juce::jlimit ((Sample)0.5, (Sample)1.2, Qhp));
            CtxRep ctx (block);
            hpFilter.process (ctx);
        }
        return; // skip LP
    }
    // Compute effective Qs with droop near extremes
    const Sample Qhp = params.eqQLink ? params.filterQ : params.hpQ;
    const Sample Qlp = params.eqQLink ? params.filterQ : params.lpQ;
    const double nyqD = (double) nyq;
    const double nhp = juce::jlimit (0.0, 1.0, (double) hpHz / juce::jmax (1e-9, nyqD));
    const double nlp = juce::jlimit (0.0, 1.0, (double) lpHz / juce::jmax (1e-9, nyqD * 0.45));
    const Sample qhpEff = (Sample) juce::jlimit (0.5, 0.75, (double) Qhp * (1.0 - 0.7 * nhp * nhp));
    const Sample qlpEff = (Sample) juce::jlimit (0.5, 0.75, (double) Qlp * (1.0 - 0.7 * nlp * nlp));

    // Prepare temp buffer once per call
    const int ch = (int) block.getNumChannels();
    const int N  = (int) block.getNumSamples();
    if (hpLpTempPreparedCh != ch || hpLpTempPreparedNs < N)
    {
        hpLpTemp.setSize (juce::jmax (1, ch), juce::jmax (1, N), false, false, true);
        hpLpTempPreparedCh = ch; hpLpTempPreparedNs = N;
    }

    // If target differs from last applied, (re)start a short bank crossfade
    if (hpHz != lastAppliedHpHz || (hpLpPinchState == 0 && lpHz != lastAppliedLpHz))
    {
        hpLpXfadeTotal = hpLpXfadeSamplesLeft = 128; // slightly longer than before
        lastAppliedHpHz = hpHz;
        if (hpLpPinchState == 0) lastAppliedLpHz = lpHz; else lastAppliedLpHz = (Sample) -1;
    }

    // Process in small slices and crossfade banks when retuning
    const int slice = 16;
    for (int i = 0; i < N; i += slice)
    {
        const int m = juce::jmin (slice, N - i);
        auto activeSlice = block.getSubBlock ((size_t) i, (size_t) m);
        CtxRep ctxActive (activeSlice);

        // Set target params on the inactive bank
        if (!hpLpUseBankB)
        {
            hpFilter.setResonance (qhpEff);
            hpFilter.setCutoffFrequency (hpHz);
            if (hpLpPinchState == 0) { lpFilter.setResonance (qlpEff); lpFilter.setCutoffFrequency (lpHz); }
        }
        else
        {
            hpFilter.setResonance (qhpEff);
            hpFilter.setCutoffFrequency (hpHz);
            if (hpLpPinchState == 0) { lpFilter.setResonance (qlpEff); lpFilter.setCutoffFrequency (lpHz); }
        }

        // Make a dry copy for the inactive bank to ensure both banks see identical input
        for (int c = 0; c < ch; ++c)
            juce::FloatVectorOperations::copy (hpLpTemp.getWritePointer (c, i), block.getChannelPointer (c) + i, m);
        juce::dsp::AudioBlock<Sample> tmpBlk (hpLpTemp.getArrayOfWritePointers(), (size_t) ch, (size_t) i, (size_t) m);
        CtxRep ctxTmp (tmpBlk);

        // Render both banks: active into place, inactive into temp (from dry copy)
        if (!hpLpUseBankB)
        {
            // A active
            hpFilter.process (ctxActive);
            if (hpLpPinchState == 0) lpFilter.process (ctxActive);
            // B inactive → process on dry copy
            hpFilterB.process (ctxTmp);
            if (hpLpPinchState == 0) lpFilterB.process (ctxTmp);
        }
        else
        {
            // B active
            hpFilterB.process (ctxActive);
            if (hpLpPinchState == 0) lpFilterB.process (ctxActive);
            // A inactive → process on dry copy
            hpFilter.process (ctxTmp);
            if (hpLpPinchState == 0) lpFilter.process (ctxTmp);
        }

        // If a retune occurred, crossfade over this slice
        if (hpLpXfadeSamplesLeft > 0)
        {
            for (int c = 0; c < ch; ++c)
            {
                auto* yAct = block.getChannelPointer (c) + i;
                auto* yNew = hpLpTemp.getReadPointer (c, i);
                for (int n = 0; n < m; ++n)
                {
                    const float xf = 1.0f - (float) hpLpXfadeSamplesLeft / (float) juce::jmax (1, hpLpXfadeTotal);
                    yAct[n] = (Sample) ((1.0f - xf) * (float) yAct[n] + xf * (float) yNew[n]);
                    --hpLpXfadeSamplesLeft;
                }
            }
            if (hpLpXfadeSamplesLeft <= 0) { hpLpUseBankB = !hpLpUseBankB; }
        }
    }
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
    // Use existing coeffs if small changes and cooldown active; updateTiltEQ handles gating
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
    // Only re-prepare when block size or channel count changes
    const int blk = (int) block.getNumSamples();
    const int ch  = (int) block.getNumChannels();
    if (blk != fullPreparedBlockLen || ch != fullPreparedChannels)
    {
        fullLinearConvolver->prepare (sr, blk, fullKernelLen, ch);
        fullPreparedBlockLen = blk;
        fullPreparedChannels = ch;
    }

    // Atomically adopt a pending kernel if available (built on background thread)
    if (pendingFullKernel)
    {
        fullLinearConvolver->setKernel (*pendingFullKernel);
        activeFullKernel = pendingFullKernel;
        pendingFullKernel.reset();
    }

    // If no active kernel yet (very first run), fall back to clean HP/LP to avoid stall
    if (!activeFullKernel)
    {
        ensureLinearPhaseKernel (sr, params.hpHz, params.lpHz, (int) block.getNumSamples(), (int) block.getNumChannels());
        linConvolver->process (block);
        return;
    }

    fullLinearConvolver->process (block);
}

template <typename Sample>
void FieldChain<Sample>::applyWidthMS (Block block, Sample width)
{
    if (block.getNumChannels() < 2) return;
    // Unity guard: no-op at width = 1
    if (std::abs ((double) (width - (Sample) 1)) < 1e-6) return;
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
    // Unity guard: no-op when all bands are ~1.0
    if (std::abs((double)(wLo - (Sample)1))  < 1e-6 &&
        std::abs((double)(wMid - (Sample)1)) < 1e-6 &&
        std::abs((double)(wHi - (Sample)1))  < 1e-6)
        return;
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
    // Unity guard: no-op when both bands ~1.0
    if (std::abs((double)(wLow - (Sample)1)) < 1e-6 &&
        std::abs((double)(wHigh - (Sample)1)) < 1e-6)
        return;
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
    // Unity guard: no-op at zero rotation and asymmetry
    if (std::abs ((double) rotationRad) < 1e-9 && std::abs ((double) asym) < 1e-9) return;
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
    const int prevOs = lastOsMode;
    ensureOversampling (osModeIndex);
    if (lastOsMode != prevOs)
    {
        osXfadeTotal = osXfadeSamplesLeft = juce::jlimit (32, 256, (int) block.getNumSamples());
    }

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
        if (!aliasGuardsPrepared) prepareAliasGuards (sr);
        {
            juce::dsp::ProcessContextReplacing<Sample> ctx (block);
            aliasGuardHP.process (ctx);
        }
        applySaturationOnBlock (block, driveLin);
        {
            juce::dsp::ProcessContextReplacing<Sample> ctx (block);
            aliasGuardLP.process (ctx);
        }
    }

    // Mix dry back in with optional crossfade when OS factor changed
    const Sample dryGain = (Sample)1 - mix01;
    const int N = (int) block.getNumSamples();
    for (int c = 0; c < (int) block.getNumChannels(); ++c)
    {
        auto* y = block.getChannelPointer (c);
        auto* d = dry.getReadPointer (c);
        for (int i = 0; i < N; ++i)
        {
            Sample wet = y[i];
            if (osXfadeSamplesLeft > 0)
            {
                const float a = (float) (osXfadeTotal - osXfadeSamplesLeft) / (float) juce::jmax (1, osXfadeTotal);
                --osXfadeSamplesLeft;
                wet = (Sample) ((1.0f - a) * (double) d[i] + a * (double) wet);
            }
            y[i] = dryGain * d[i] + mix01 * wet;
        }
    }
}

 
// Reverb algorithms (Room/Plate/Hall) macro mapping, parallel wet return

template <typename Sample>
void FieldChain<Sample>::applySpaceAlgorithm (Block /*block*/, Sample depth01, int algo)
{
    depth01 = juce::jlimit ((Sample)0, (Sample)1, depth01);

    // Compute macro voicing only; actual rendering happens in renderSpaceWet()
    if (depth01 <= (Sample) 0.0001)
    {
        rvParams.wetLevel = 0.0f;
        return;
    }

    computeReverbVoicing ((int) algo, (float) depth01, rvParams);
    roomSizeSmoothed.setTargetValue (rvParams.roomSize);
    dampingSmoothed .setTargetValue (rvParams.damping);
    widthSmoothed   .setTargetValue (rvParams.width);
}

// Render reverb into a provided wet buffer (same channels/samples as current block)
template <typename Sample>
void FieldChain<Sample>::renderSpaceWet (juce::AudioBuffer<Sample>& wet)
{
    const int ch = wet.getNumChannels();
    const int n  = wet.getNumSamples();
    for (int c = 0; c < ch; ++c) wet.clear (c, 0, n);

    if (!params.rvEnabled)
    {
        rv_tailRms = 0.0f; rv_erRms = 0.0f; return;
    }

    // Early reflections: simple multi-tap per channel with width control
    const int   erTaps    = 8;
    const float erMs      = juce::jlimit (5.0f, 120.0f, (float) params.rvErTimeMs);
    const float erDensity = juce::jlimit (0.0f, 1.0f, (float) params.rvErDensityPct * 0.01f);
    const float erLevel   = juce::Decibels::decibelsToGain ((float) params.rvErLevelDb);
    const int   baseEr    = juce::jmax (1, (int) std::round (erMs * 0.001 * sr));
    const float width01   = juce::jlimit (0.0f, 1.2f, (float) (params.rvWidthPct * 0.01));

    // Tail: lightweight 8-line FDN with Householder feedback (dense, CPU-light)
    static constexpr int K = 8;
    static thread_local std::array<std::vector<Sample>, K> dl;
    static thread_local std::array<int, K> idx{};
    static thread_local std::array<int, K> len{};
    // Line lengths: prime-ish around ~40..120 ms * decay factor
    const float decaySec = juce::jlimit (0.2f, 20.0f, (float) params.rvDecaySec);
    const float baseMs   = juce::jlimit (30.0f, 180.0f, decaySec * 90.0f);
    for (int k = 0; k < K; ++k)
    {
        const float jitter = 0.85f + 0.3f * ((k * 13) % 17) / 17.0f;
        const int Lsamp = juce::jmax (1, (int) std::round ((baseMs * jitter) * 0.001 * sr));
        len[(size_t) k] = Lsamp;
        if ((int) dl[(size_t) k].size() != Lsamp) { dl[(size_t) k].assign ((size_t) Lsamp, (Sample) 0); idx[(size_t) k] = 0; }
    }
    // Frequency-independent loss scaled by DR-EQ sums (placeholder)
    const float dreq = juce::jlimit (0.3f, 2.0f, (float) params.rvDreqLowX * 0.33f
                                                   + (float) params.rvDreqMidX * 0.34f
                                                   + (float) params.rvDreqHighX * 0.33f);
    const float loopAtt = std::exp (-1.0f / juce::jmax (1.0f, (float) (decaySec * sr / 3.0f))) * (1.0f / dreq);
    const float fbGain  = juce::jlimit (0.0f, 0.995f, loopAtt);

    // Process block
    for (int i = 0; i < n; ++i)
    {
        // Householder feedback: y = (1/K) * sum(lines)
        Sample sumY = 0;
        for (int k = 0; k < K; ++k) sumY += dl[(size_t) k][(size_t) idx[(size_t) k]];
        const Sample yAvg = sumY / (Sample) K;

        // Write back with loss and tiny decorrelated jitter
        for (int k = 0; k < K; ++k)
        {
            auto& buf = dl[(size_t) k];
            int  & rp  = idx[(size_t) k];
            const int Ls = len[(size_t) k];
            const Sample xIn = yAvg; // feedback matrix is implicit
            buf[(size_t) rp] = (Sample) (xIn * fbGain);
            if (++rp >= Ls) rp = 0;
        }

        // ER contribution (stereo spread)
        const float erMix = erLevel * erDensity;
        float erL = 0.0f, erR = 0.0f;
        // Pattern select: 0=Modern FDN (cluster), 1=Chamber (irregular), 2=Platey (even), 3=Vintage (sparser)
        for (int t = 0; t < erTaps; ++t)
        {
            float mult = 1.0f;
            switch (params.rvAlgo)
            {
                case 1: mult = 0.7f + 0.5f * ((t * 11) % 17) / 17.0f; break; // chamber
                case 2: mult = 0.8f + 0.2f * (t / (float) juce::jmax (1, erTaps-1)); break; // platey
                case 3: mult = 0.6f + 0.8f * ((t * 7) % 13) / 13.0f; break; // vintage scatter
                default: mult = 0.6f + 0.6f * (t / (float) juce::jmax (1, erTaps-1)); break; // modern
            }
            const int off = juce::jmax (1, (int) std::round (baseEr * mult));
            const int Lread = (idx[0] - (off % len[0]) + len[0]) % len[0];
            const float v = (float) dl[0][(size_t) Lread] * erMix;
            const float pan = juce::jlimit (0.0f, 1.0f, (t / (float) (erTaps-1)));
            const float l = std::cos (juce::MathConstants<float>::halfPi * pan * width01);
            const float r = std::sin (juce::MathConstants<float>::halfPi * pan * width01);
            erL += v * l; erR += v * r;
        }

        if (ch > 0) wet.getWritePointer (0)[i] += (Sample) erL + yAvg;
        if (ch > 1) wet.getWritePointer (1)[i] += (Sample) erR + yAvg;
    }

    // Wet tone: simple HPF/LPF (one-pole) and tilt (broad shelves) on wet
    auto onePoleHP = [&](Sample& st, Sample x, float fc){ const float a = juce::jlimit (0.0f, 1.0f, (float)(2.0f * juce::MathConstants<double>::pi * fc / sr)); st += a * (x - st); return (Sample) (x - st); };
    auto onePoleLP = [&](Sample& st, Sample x, float fc){ const float a = juce::jlimit (0.0f, 1.0f, (float)(2.0f * juce::MathConstants<double>::pi * fc / sr)); st += a * (x - st); return st; };
    const float hp = juce::jlimit (20.0f, 500.0f, (float) params.rvHpfHz);
    const float lp = juce::jlimit (1000.0f, 20000.0f, (float) params.rvLpfHz);
    const float tiltDb = juce::jlimit (-6.0f, 6.0f, (float) params.rvTiltDb);
    const float tiltGHi = std::pow (10.0f, ( tiltDb * 0.5f) / 20.0f);
    const float tiltGLo = std::pow (10.0f, (-tiltDb * 0.5f) / 20.0f);
    for (int i = 0; i < n; ++i)
    {
        if (ch > 0)
        {
            auto* L = wet.getWritePointer (0);
            Sample x = L[i]; x = onePoleHP (rv_hpStateL, x, hp); x = onePoleLP (rv_lpStateL, x, lp);
            // crude tilt: split at ~1 kHz with a single state
            const float split = 1000.0f;
            auto lo = onePoleLP (rv_tiltLP_L, x, split);
            auto hi = (Sample) (x - lo);
            L[i] = (Sample) (lo * tiltGLo + hi * tiltGHi);
        }
        if (ch > 1)
        {
            auto* R = wet.getWritePointer (1);
            Sample x = R[i]; x = onePoleHP (rv_hpStateR, x, hp); x = onePoleLP (rv_lpStateR, x, lp);
            const float split = 1000.0f;
            auto lo = onePoleLP (rv_tiltLP_R, x, split);
            auto hi = (Sample) (x - lo);
            R[i] = (Sample) (lo * tiltGLo + hi * tiltGHi);
        }
    }

    // Simple wet scaling by mix and output trim
    const float wetGain = juce::jlimit (0.0f, 2.0f, (float) params.rvWet01 * std::pow (10.0f, (float) params.rvOutTrimDb / 20.0f));
    for (int c = 0; c < ch; ++c)
    {
        auto* d = wet.getWritePointer (c);
        for (int i = 0; i < n; ++i) d[i] *= (Sample) wetGain;
    }

    // Meters
    long double sumL = 0.0L, sumR = 0.0L;
    if (ch > 0){ auto* L = wet.getReadPointer (0); for (int i=0;i<n;++i) sumL += (long double) L[i]*L[i]; }
    if (ch > 1){ auto* R = wet.getReadPointer (1); for (int i=0;i<n;++i) sumR += (long double) R[i]*R[i]; }
    const float rms = (float) std::sqrt ((double) ((sumL + sumR) / juce::jmax (1, ch * n)));
    rv_tailRms = rms; rv_erRms = rms * 0.5f;
}

// --------- main process (Sample) ---------

template <typename Sample>
void FieldChain<Sample>::process (Block block)
{
    if (params.bypass) return;

    // Flush denormals to avoid CPU spikes/crackle on quiet passages
    juce::ScopedNoDenormals noDenormals;

    // Input gain
    block.multiplyBy (params.gainLin);

    // eco path removed

    // [PHASE][core] Modes: 2=Hybrid FIR HP/LP, 3=Full FIR (tone+HP/LP). 0/1 use IIR (LR4).
    // Optional auto-linear during edits: force Full Linear while autoLinearSamplesLeft > 0
    const bool autoLinearActive = (autoLinearSamplesLeft > 0);
    const bool useFIR = (params.phaseMode >= 2) || autoLinearActive;
    const bool useIIR = (params.phaseMode <= 1) && !autoLinearActive;
    if (useFIR)
    {
        if (params.phaseMode == 3 || autoLinearActive)
        {
            // [FIR][neutral-bypass] Full Linear: bypass FIR when tone is neutral to avoid needless latency/pre‑ringing
            const bool toneNeutral = std::abs ((double) params.tiltDb)  < 1e-4 &&
                                     std::abs ((double) params.scoopDb) < 1e-4 &&
                                     std::abs ((double) params.bassDb)  < 1e-4 &&
                                     std::abs ((double) params.airDb)   < 1e-4 &&
                                     params.hpHz <= (Sample) 21 &&
                                     params.lpHz >= (Sample) 19900;
            if (!toneNeutral)
                applyFullLinearFIR (block);
        }
        else // params.phaseMode == 2
        {
            // [FIR][neutral-bypass] Hybrid: bypass FIR when HP/LP are neutral; FIR kernel is gain‑normalized
            if (! (params.hpHz <= (Sample) 21 && params.lpHz >= (Sample) 19900))
            {
                ensureLinearPhaseKernel (sr, params.hpHz, params.lpHz, (int) block.getNumSamples(), (int) block.getNumChannels());
                linConvolver->process (block);
            }
        }
    }
    // else useIIR -> handled later in sub-block loop; no-op here

    // Imaging & placement
    if (params.splitMode) applySplitPan (block, params.panL, params.panR);
    else                  applyPan     (block, params.pan);

    // Imaging & mono (Classic width only when widthMode == 0)
    if (params.widthMode == 0) {
        applyWidthMS (block, params.width);
    }
    // Mono maker before tone
    applyMonoMaker (block, params.monoHz);
    // HP/LP is applied in the smoothed sub-block pipeline below (to avoid double-filtering)

    // Core tone using smoothed, gated sub-block path (below)
    // (no early return; continue into sub-block pipeline)

    // (skipped while diagnosing)
    // Width Designer: frequency-tilted S (before rotation) when Designer mode only
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

    // Core tone: skip IIR tone when forcing Full Linear this block
    if (! (params.phaseMode == 3 || autoLinearActive))
    {
        // Process in small chunks and use smoothed values to avoid zipper noise
        const int total = (int) block.getNumSamples();
        const int channels = (int) block.getNumChannels();
        const int step = juce::jlimit (64, 128, total); // sub-block size: steadier param updates
        // Prepare tone crossfade buffers once per block if a rebuild will occur
        bool willRebuildTone = false;
        {
            const Sample tDb = tiltDbSm.getTargetValue();
            const Sample tHz = tiltFreqSm.getTargetValue();
            const Sample scDb= scoopDbSm.getTargetValue();
            const Sample scHz= scoopFreqSm.getTargetValue();
            const Sample bDb = bassDbSm.getTargetValue();
            const Sample bHz = bassFreqSm.getTargetValue();
            const Sample aDb = airDbSm.getTargetValue();
            const Sample aHz = airFreqSm.getTargetValue();
            auto epsChanged = [&](Sample a, Sample b, Sample eps){ return a < (Sample) 0 || std::abs ((double) (a - b)) > (double) eps; };
            const bool tiltNeeds  = epsChanged (lastTiltDb,  tDb,  (Sample)0.05) || epsChanged (lastTiltHz,  tHz,  (Sample)1.0);
            const bool scoopNeeds = epsChanged (lastScoopDb, scDb, (Sample)0.05) || epsChanged (lastScoopHz, scHz, (Sample)1.0);
            const bool bassNeeds  = epsChanged (lastBassDb,  bDb,  (Sample)0.05) || epsChanged (lastBassHz,  bHz,  (Sample)1.0);
            const bool airNeeds   = epsChanged (lastAirDb,   aDb,  (Sample)0.05) || epsChanged (lastAirHz,   aHz,  (Sample)1.0);
            willRebuildTone = (tiltNeeds || scoopNeeds || bassNeeds || airNeeds);
        }
        if (willRebuildTone && toneXfadeSamplesLeft <= 0)
        {
            toneDryBuf.setSize ((int) block.getNumChannels(), (int) block.getNumSamples());
            for (int c = 0; c < (int) block.getNumChannels(); ++c)
                std::memcpy (toneDryBuf.getWritePointer (c), block.getChannelPointer (c), sizeof (Sample) * (size_t) block.getNumSamples());
            toneXfadeTotal = toneXfadeSamplesLeft = juce::jlimit (64, 256, (int) block.getNumSamples());
        }

        for (int start = 0; start < total; start += step)
        {
            const int len = juce::jmin (step, total - start);
            juce::dsp::AudioBlock<Sample> sub = block.getSubBlock ((size_t) start, (size_t) len);

            // Advance smoothed ramps by sub-block length for stable per-block params
            tiltDbSm   .skip (len); const Sample tDb = tiltDbSm.getCurrentValue();
            tiltFreqSm .skip (len); const Sample tHz = tiltFreqSm.getCurrentValue();
            scoopDbSm  .skip (len); const Sample scDb= scoopDbSm.getCurrentValue();
            scoopFreqSm.skip (len); const Sample scHz= scoopFreqSm.getCurrentValue();
            bassDbSm   .skip (len); const Sample bDb = bassDbSm.getCurrentValue();
            bassFreqSm .skip (len); const Sample bHz = bassFreqSm.getCurrentValue();
            airDbSm    .skip (len); const Sample aDb = airDbSm.getCurrentValue();
            airFreqSm  .skip (len); const Sample aHz = airFreqSm.getCurrentValue();

            // Smooth HP/LP cutoffs: handled in slice loop below (log-domain smoothing)
            hpHzSm.skip (len); lpHzSm.skip (len);

            // Apply smoothed tone
            // Gate tone coefficient rebuilds with epsilon + cooldown to avoid glitches under rapid moves.
            // IMPORTANT: Never process an IIR filter without initialized coeffs.
            const Sample tDbEps = (Sample) 0.05, fHzEps = (Sample) 1.0;
            if (--toneCoeffCooldownSamples < 0) toneCoeffCooldownSamples = 0;
            auto epsChanged = [&](Sample a, Sample b, Sample eps){ return a < (Sample) 0 || std::abs ((double) (a - b)) > (double) eps; };
            const bool tiltNeeds  = epsChanged (lastTiltDb,  tDb,  tDbEps)  || epsChanged (lastTiltHz,  tHz,  fHzEps);
            const bool scoopNeeds = epsChanged (lastScoopDb, scDb, tDbEps)  || epsChanged (lastScoopHz, scHz, fHzEps);
            const bool bassNeeds  = epsChanged (lastBassDb,  bDb,  tDbEps)  || epsChanged (lastBassHz,  bHz,  fHzEps);
            const bool airNeeds   = epsChanged (lastAirDb,   aDb,  tDbEps)  || epsChanged (lastAirHz,   aHz,  fHzEps);

            const bool uninitTilt  = (lastTiltDb  > (Sample) 1.0e8) || (lastTiltHz  > (Sample) 1.0e8);
            const bool uninitScoop = (lastScoopDb > (Sample) 1.0e8) || (lastScoopHz > (Sample) 1.0e8);
            const bool uninitBass  = (lastBassDb  > (Sample) 1.0e8) || (lastBassHz  > (Sample) 1.0e8);
            const bool uninitAir   = (lastAirDb   > (Sample) 1.0e8) || (lastAirHz   > (Sample) 1.0e8);

            const bool allowToneRebuild = (toneCoeffCooldownSamples == 0);
            bool rebuiltAny = false;

            // Tilt
            if ((allowToneRebuild || uninitTilt) && tiltNeeds)
            {
                applyTiltEQ (sub, tDb, tHz); lastTiltDb = tDb; lastTiltHz = tHz; rebuiltAny = true;
            }
            else if (!uninitTilt)
            {
                CtxRep ctxTilt (sub);
                lowShelf.process (ctxTilt);
                highShelf.process (ctxTilt);
            }
            // Scoop
            if ((allowToneRebuild || uninitScoop) && scoopNeeds)
            {
                applyScoopEQ (sub, scDb, scHz); lastScoopDb = scDb; lastScoopHz = scHz; rebuiltAny = true;
            }
            else if (!uninitScoop)
            {
                CtxRep ctxSc (sub);
                scoopFilter.process (ctxSc);
            }
            // Bass
            if ((allowToneRebuild || uninitBass) && bassNeeds)
            {
                applyBassShelf (sub, bDb, bHz); lastBassDb = bDb; lastBassHz = bHz; rebuiltAny = true;
            }
            else if (!uninitBass)
            {
                CtxRep ctxB (sub);
                bassFilter.process (ctxB);
            }
            // Air
            if ((allowToneRebuild || uninitAir) && airNeeds)
            {
                applyAirBand (sub, aDb, aHz); lastAirDb = aDb; lastAirHz = aHz; rebuiltAny = true;
            }
            else if (!uninitAir)
            {
                CtxRep ctxA (sub);
                airFilter.process (ctxA);
            }

            if (rebuiltAny)
                toneCoeffCooldownSamples = 64;
            // [IIR][LR4] Zero/Natural: non‑resonant Linkwitz–Riley HP/LP per channel
            // [IIR][smoothing] ~90 ms smoothing; [IIR][epsilon] ≥3 Hz retune gate
            if (params.phaseMode == 0 || params.phaseMode == 1)
            {
                const Sample nyqLoc = (Sample) (sr * 0.49);
                // Slightly slower smoothing for IIR path
                hpHzSm.reset (sr, 0.090);
                lpHzSm.reset (sr, 0.090);
                const Sample hpNowHz = (Sample) std::exp (hpHzSm.getCurrentValue());
                const Sample lpNowHz = (Sample) std::exp (lpHzSm.getCurrentValue());
                const Sample hpC = juce::jlimit ((Sample) 20,   (Sample) 1000,  hpNowHz);
                const Sample lpC = juce::jlimit ((Sample) 1000, juce::jmin ((Sample) 20000, nyqLoc * (Sample) 0.45), lpNowHz);
                // Epsilon gate to avoid micro-retunes
                static Sample lastHpLR = (Sample) -1, lastLpLR = (Sample) -1;
                const Sample eps = (Sample) 3.0;
                if (lastHpLR < 0 || std::abs ((double)(hpC - lastHpLR)) >= (double) eps)
                { lrHpL.setCutoffFrequency (hpC); lrHpR.setCutoffFrequency (hpC); lastHpLR = hpC; }
                if (lastLpLR < 0 || std::abs ((double)(lpC - lastLpLR)) >= (double) eps)
                { lrLpL.setCutoffFrequency (lpC); lrLpR.setCutoffFrequency (lpC); lastLpLR = lpC; }
                // Process stereo with LR filters
                if (sub.getNumChannels() >= 1)
                {
                    auto ch0 = sub.getSingleChannelBlock ((size_t) 0);
                    CtxRep ctx0 (ch0);
                    lrHpL.process (ctx0);
                    lrLpL.process (ctx0);
                }
                if (sub.getNumChannels() >= 2)
                {
                    auto ch1 = sub.getSingleChannelBlock ((size_t) 1);
                    CtxRep ctx1 (ch1);
                    lrHpR.process (ctx1);
                    lrLpR.process (ctx1);
                }
                hpHzSm.skip (len);
                lpHzSm.skip (len);
            }
        }

        // Apply tone crossfade if active to mask coefficient swap bursts
        if (toneXfadeSamplesLeft > 0)
        {
            const int N = (int) block.getNumSamples();
            const int C = (int) block.getNumChannels();
            for (int i = 0; i < N; ++i)
            {
                const float a = 1.0f - (float) toneXfadeSamplesLeft / (float) juce::jmax (1, toneXfadeTotal);
                --toneXfadeSamplesLeft;
                for (int c = 0; c < C; ++c)
                {
                    auto* y = block.getChannelPointer (c);
                    const Sample d = toneDryBuf.getSample (c, i);
                    y[i] = (Sample) ((1.0f - a) * (double) d + a * (double) y[i]);
                }
                if (toneXfadeSamplesLeft <= 0) break;
            }
        }
    }

    // DC blocker after tone to eliminate DC creep (fixed cutoff in IIR modes)
    {
        const bool iirMode = (params.phaseMode == 0 || params.phaseMode == 1);
        const bool movingHpLp = (std::abs ((double) (hpHzSm.getTargetValue() - hpHzSm.getCurrentValue())) > 1e-6) ||
                                 (std::abs ((double) (lpHzSm.getTargetValue() - lpHzSm.getCurrentValue())) > 1e-6);
        const Sample dcCut = iirMode ? (Sample) 8.0 : (movingHpLp ? (Sample) 15.0 : (Sample) 8.0);
        dcBlocker.setCutoffFrequency (dcCut);
        CtxRep ctx (block);
        dcBlocker.process (ctx);
    }

    // Tiny dither to decorrelate state transitions (post-filter) only while HP/LP moving
    {
        const bool movingHpLp = (std::abs ((double) (hpHzSm.getTargetValue() - hpHzSm.getCurrentValue())) > 1e-6) ||
                                 (std::abs ((double) (lpHzSm.getTargetValue() - lpHzSm.getCurrentValue())) > 1e-6);
        if (movingHpLp)
        {
            const int ch = (int) block.getNumChannels();
            const int n  = (int) block.getNumSamples();
            static uint32_t rng = 0x1234567u;
            for (int c = 0; c < ch; ++c)
            {
                auto* y = block.getChannelPointer (c);
                for (int i = 0; i < n; ++i)
                {
                    rng = 1664525u * rng + 1013904223u;
                    const float noise = ((int)(rng >> 9) & 0x7FFF) * (1.0f / 32768.0f) - 0.5f; // ~[-0.5,0.5]
                    y[i] += (Sample) (noise * 0.001f); // ~ -60 dBFS
                }
            }
        }
    }

    // Reverb: compute rvParams then render wet-only to buffer 'wet'
    applySpaceAlgorithm (block, params.depth, params.spaceAlgo);

    // Build dry (post tone/imaging) and wet buses (preallocated)
    const int ch = (int) block.getNumChannels();
    const int n  = (int) block.getNumSamples();
    jassert (n <= dryBusBuf.getNumSamples());
    jassert (n <= wetBusBuf.getNumSamples());
    dryBusBuf.setSize (juce::jmax (1, ch), dryBusBuf.getNumSamples(), false, false, true);
    wetBusBuf.setSize (juce::jmax (1, ch), wetBusBuf.getNumSamples(), false, false, true);
    delayWetBuf.setSize (juce::jmax (1, ch), wetBusBuf.getNumSamples(), false, false, true);
    for (int c = 0; c < ch; ++c)
    {
        auto* dst = dryBusBuf.getWritePointer (c);
        auto* src = block.getChannelPointer (c);
        std::memcpy (dst, src, sizeof (Sample) * (size_t) n);
        wetBusBuf.clear (c, 0, n);
    }
    // Render reverb into wet (100% wet)
    renderSpaceWet (wetBusBuf);

    // (moved) LF mono is applied after final dry/wet mix

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
    
    // Delay processing (render to dedicated delayWetBuf; mixed later independently of reverb wet)
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
        // Render as wet-only for bus mixing regardless of UI Kill Dry
        delayParams.killDry = true;
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

        // Render delay wet-only by processing a copy of the dry bus
        for (int c = 0; c < ch; ++c)
            std::memcpy (delayWetBuf.getWritePointer (c), dryBusBuf.getReadPointer (c), sizeof (Sample) * (size_t) n);
        juce::dsp::AudioBlock<Sample> dblk (delayWetBuf);
        float scL = 0.0f, scR = 0.0f; // neutral SC here
        delayEngine.process (dblk, scL, scR);

        // Compute delay wet RMS for UI telemetry
        if (delayWetBuf.getNumChannels() >= 2)
        {
            const int n = (int) dblk.getNumSamples();
            auto rmsOf = [] (const float* d, int nSamples) -> float { long double s=0.0; for (int i=0;i<nSamples;++i){ float v=d[i]; s+= (long double) v*v; } return std::sqrt ((double)(s / juce::jmax (1, nSamples))); };
            if constexpr (std::is_same_v<Sample, float>)
            {
                delay_wetRmsL = rmsOf (delayWetBuf.getReadPointer(0), n);
                delay_wetRmsR = rmsOf (delayWetBuf.getReadPointer(1), n);
            }
            else
            {
                // For double chain, compute in double then cast
                auto rmsOfD = [] (const double* d, int nSamples) -> float { long double s=0.0; for (int i=0;i<nSamples;++i){ double v=d[i]; s+= v*v; } return (float) std::sqrt ((double)(s / juce::jmax (1, nSamples))); };
                delay_wetRmsL = rmsOfD (delayWetBuf.getReadPointer(0), n);
                delay_wetRmsR = rmsOfD (delayWetBuf.getReadPointer(1), n);
            }
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

    // Equal-power mix: Depth × Wet slider
    const float rvMix = juce::jlimit (0.0f, 1.0f, rvParams.wetLevel * (float) params.rvWet01);
    wetMixSmoothed.setTargetValue (rvMix);
    // Sum Dry + Wet back to output block (equal-power law) with one shared mix per sample:
    // mix=0 -> 100% dry, mix=1 -> 100% wet
    for (int i = 0; i < n; ++i)
    {
        const float mix = wetMixSmoothed.getNextValue();
        const float theta = juce::jlimit (0.0f, 1.0f, mix) * juce::MathConstants<float>::halfPi;
        const float a = std::cos (theta);
        const float b = std::sin (theta);
        for (int c = 0; c < ch; ++c)
        {
            auto* out = block.getChannelPointer (c);
            const Sample* d = dryBusBuf.getReadPointer (c);
            const Sample* w = wetBusBuf.getReadPointer (c);
            out[i] = (Sample) (a * (double) d[i] + b * (double) w[i]);
        }
    }
    // Mix in Delay wet after reverb mix using its own wet control (ungated by reverb)
    if (params.delayEnabled)
    {
        // DelayEngine already applied its internal Wet; since we forced KillDry, mix unscaled
        for (int c = 0; c < ch; ++c)
        {
            auto* out = block.getChannelPointer (c);
            const Sample* dw = delayWetBuf.getReadPointer (c);
            for (int i = 0; i < n; ++i)
                out[i] += dw[i];
        }
    }
    // LF mono (apply after dry/wet sum so lows stay centered across full output)
    applyMonoMaker (block, params.monoHz);
    // Decrement auto-linear countdown by processed samples
    if (autoLinearSamplesLeft > 0)
    {
        autoLinearSamplesLeft = juce::jmax (0, autoLinearSamplesLeft - (int) block.getNumSamples());
    }
}

template <typename Sample>
void FieldChain<Sample>::ensureLinearPhaseKernel (double sampleRate, Sample hpHz, Sample lpHz, int maxBlock, int numChannels)
{
    if (! linConvolver)
        linConvolver = std::make_unique<OverlapSaveConvolver<Sample>>();
    // Only re-prepare when block size or channel count changes
    const int ch = juce::jmax (1, numChannels);
    if (maxBlock != linPreparedBlockLen || ch != linPreparedChannels)
    {
        linConvolver->prepare (sampleRate, maxBlock, linKernelLen, ch);
        linPreparedBlockLen = maxBlock;
        linPreparedChannels = ch;
    }

    const double hp = juce::jlimit (20.0, 1000.0, (double) hpHz);
    const double lp = juce::jlimit (1000.0, 20000.0, (double) lpHz);
    // Debounce parameters; if editing, defer heavy rebuild to background
    const float dHp = std::abs ((float) hp - lastDesignedHpHzLP);
    const float dLp = std::abs ((float) lp - lastDesignedLpHzLP);
    const bool largeDelta = (dHp > 5.0f) || (dLp > 5.0f);
    if (linKernelCooldownSamples > 0) --linKernelCooldownSamples;
    if (! linConvolver->isReady())
    {
        std::vector<float> kernel; designLinearPhaseBandpassKernel (kernel, sampleRate, hp, lp, linKernelLen, 8.6);
        // Normalize kernel to ~unity gain at geometric mean frequency inside passband
        if (!kernel.empty())
        {
            const double fs = sampleRate;
            const double fc = juce::jlimit (hp * 1.5, lp * 0.5, std::sqrt (hp * lp));
            const double w  = 2.0 * juce::MathConstants<double>::pi * (fc / fs);
            double re = 0.0, im = 0.0; const int M = (int) kernel.size();
            for (int n = 0; n < M; ++n)
            {
                const double phase = -w * n;
                re += (double) kernel[n] * std::cos (phase);
                im += (double) kernel[n] * std::sin (phase);
            }
            const double mag = std::sqrt (re*re + im*im);
            const double s = (mag > 1e-9) ? (1.0 / mag) : 1.0;
            if (std::abs (s - 1.0) > 1e-6)
                for (auto& k : kernel) k = (float) (k * s);
        }
        linConvolver->setKernel (kernel);
        lastHpHzLP = (float) hp; lastLpHzLP = (float) lp;
        lastDesignedHpHzLP = (float) hp; lastDesignedLpHzLP = (float) lp;
        linKernelCooldownSamples = juce::jmax (maxBlock, 256);
    }
    else if (largeDelta)
    {
        // Defer redesign when editing to avoid audio-thread work
        if (std::is_same<Sample,double>::value)
        {
            // processor-level background pool requested via requestLinearPhaseRedesign from float path
        }
        else
        {
            // For float chain, just update last values; background swap handled by processor gate
            lastHpHzLP = (float) hp; lastLpHzLP = (float) lp;
        }
    }
}

template <typename Sample>
float FieldChain<Sample>::getCurrentDuckGrDb() const
{
    // Return current gain reduction in dB (>=0). Safe cast to float for metering.
    return (float) ducker.getCurrentGainReductionDb();
}

template <typename Sample>
float FieldChain<Sample>::getReverbErRms() const { return rv_erRms; }

template <typename Sample>
float FieldChain<Sample>::getReverbTailRms() const { return rv_tailRms; }

template <typename Sample>
float FieldChain<Sample>::getDelayWetRmsL() const { return delay_wetRmsL; }

template <typename Sample>
float FieldChain<Sample>::getDelayWetRmsR() const { return delay_wetRmsR; }

template <typename Sample>
double FieldChain<Sample>::getDelayLastSamplesL() const { return (double) delayEngine.getLastDelaySamplesL(); }

template <typename Sample>
double FieldChain<Sample>::getDelayLastSamplesR() const { return (double) delayEngine.getLastDelaySamplesR(); }

// Explicit instantiation
template struct FieldChain<float>;
template struct FieldChain<double>;
