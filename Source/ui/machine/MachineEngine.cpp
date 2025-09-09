#include "MachineEngine.h"
#include "MachineHelpersJUCE.h"

MachineEngine::MachineEngine (MyPluginAudioProcessor& procRef, juce::AudioProcessorValueTreeState& stateRef)
: proc (procRef), apvts (stateRef)
{
}

void MachineEngine::startLearn (bool usePre, double seconds)
{
    stopTimer();
    usePreBus = usePre;
    tDuration = juce::jlimit (1.0, 300.0, seconds);
    tStart = juce::Time::getMillisecondCounterHiRes() * 0.001;
    state = Learning;
    setStatus ("Initializing analysis…");

    // Refresh targets from current context
    // (Context is set by UI before Learn via setContext)
    // Simple defaults; refined per-genre/venue below in finalize
    // Build targets now to inform safety rails during learn if needed
    {
        auto makeTargets = [this]() -> Targets
        {
            Targets t{};
            // Defaults (Pop + Streaming)
            t.corrLowMin = 0.60f; t.corrFullTarget = 0.85f;
            t.widthLoTarget = 0.95f; t.widthMidTarget = 1.15f; t.widthHiTarget = 1.20f; t.widthMax = 1.50f;
            t.slopeTarget = -3.0f; t.lfRumbleMaxDb = 6.0f; t.hfFizzMaxDb = 4.0f; t.sibilanceMax = 1.2f;
            t.depthLo = 0.15f; t.depthHi = 0.30f; t.spacePref0 = 0; t.spacePref1 = 1; t.spacePref2 = 2;
            t.duckDepthDefault = 0.5f; t.duckThrDb = -24.0f; t.duckRatio = 3.0f; t.duckAtkMs = 12.0f; t.duckRelMs = 220.0f;
            t.sideTiltDbOct = 1.5f; t.tiltPivotHz = 650.0f;

            // Genre adjustments
            switch (ctx.genre) {
                case 0: /* EDM/House */
                    t.widthMidTarget = 1.25f; t.widthHiTarget = 1.35f; t.widthMax = 1.60f; t.slopeTarget = -2.5f; t.depthLo = 0.12f; t.depthHi = 0.22f; t.duckRatio = 2.5f; break;
                case 1: /* Hip-Hop/Trap */
                    t.corrLowMin = 0.65f; t.widthMax = 1.40f; t.slopeTarget = -3.0f; t.depthLo = 0.10f; t.depthHi = 0.18f; t.duckDepthDefault = 0.6f; t.duckThrDb = -22.0f; break;
                case 2: /* Pop */
                    break;
                case 3: /* Rock/Indie */
                    t.widthMidTarget = 1.05f; t.widthHiTarget = 1.15f; t.widthMax = 1.40f; t.depthLo = 0.10f; t.depthHi = 0.22f; break;
                case 4: /* Acoustic/Jazz */
                    t.widthLoTarget = 0.90f; t.slopeTarget = -3.2f; t.depthLo = 0.18f; t.depthHi = 0.32f; t.spacePref0 = 2; t.spacePref1 = 1; t.spacePref2 = 0; break;
                case 5: /* Voice/Podcast */
                    t.corrLowMin = 0.80f; t.widthLoTarget = 1.00f; t.widthMidTarget = 1.00f; t.widthHiTarget = 1.00f; t.widthMax = 1.20f; t.depthLo = 0.06f; t.depthHi = 0.14f; t.spacePref0 = 0; t.spacePref1 = 1; t.spacePref2 = 2; t.duckDepthDefault = 0.7f; break;
                default: break;
            }
            // Venue adjustments
            switch (ctx.venue) {
                case 1: /* Club/PA */ t.duckRelMs = 160.0f; break;
                case 2: /* Theater/Cinema */ t.duckRelMs = 260.0f; t.widthMax = std::min (t.widthMax, 1.45f); break;
                case 3: /* Mobile */ t.widthMax = std::min (t.widthMax, 1.35f); break;
                default: break; // Streaming
            }
            return t;
        };
        tgt = makeTargets();
    }

    proposals.clear();
    A = Agg{};
    prepareOnce();
    startTimerHz (30);
}

void MachineEngine::stopLearn (bool finalize)
{
    if (finalize && state == Learning)
        finalizeAndBuildProposals();
    stopTimer();
    state = finalize ? Ready : Idle;
}

double MachineEngine::getRemainingSeconds() const
{
    if (state != Learning) return 0.0;
    const double tNow = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const double elapsed = juce::jmax (0.0, tNow - tStart);
    return juce::jlimit (0.0, tDuration, tDuration - elapsed);
}

std::vector<Proposal> MachineEngine::getProposals() const
{
    return proposals;
}

void MachineEngine::applyProposal (const Proposal& p, bool previewOnly)
{
    if (baselineParams.empty())
        captureBaseline();

    for (const auto& d : p.params)
    {
        const float clamped = juce::jlimit (d.lo, d.hi, d.target);
        setParam (d.id, clamped, /*notifyHost=*/true);
    }
    if (!previewOnly)
    {
        captureBaseline();
    }
}

void MachineEngine::applyProposal (const Proposal& p, float amount01, bool previewOnly)
{
    amount01 = juce::jlimit (0.0f, 1.0f, amount01);
    if (baselineParams.empty())
        captureBaseline();

    for (const auto& d : p.params)
    {
        const float clampedTarget = juce::jlimit (d.lo, d.hi, d.target);
        float current = d.current;
        // If current not provided, read from host
        if (auto* rp = apvts.getRawParameterValue (d.id))
            current = rp->load();
        const float blended = current + (clampedTarget - current) * amount01;
        setParam (d.id, blended, /*notifyHost=*/true);
    }
    if (!previewOnly)
        captureBaseline();
}

void MachineEngine::applyComposite (const std::vector<Proposal>& active, float amount01, bool previewOnly)
{
    amount01 = juce::jlimit (0.0f, 1.0f, amount01);
    if (baselineParams.empty())
        captureBaseline();

    // Merge deltas by id (last one wins if duplicates)
    juce::HashMap<juce::String, ParamDelta> merged;
    for (const auto& p : active)
    {
        for (const auto& d : p.params)
            merged.set (d.id, d);
    }
    for (auto it = merged.begin(); it != merged.end(); ++it)
    {
        const ParamDelta& d = it.getValue();
        const float clampedTarget = juce::jlimit (d.lo, d.hi, d.target);
        float current = d.current;
        if (auto* rp = apvts.getRawParameterValue (d.id))
            current = rp->load();
        const float blended = current + (clampedTarget - current) * amount01;
        setParam (d.id, blended, /*notifyHost=*/true);
    }
    if (!previewOnly)
        captureBaseline();
}

void MachineEngine::revertPreview()
{
    for (auto& kv : baselineParams)
        setParam (kv.first, kv.second, /*notifyHost=*/true);
    baselineParams.clear();
}

void MachineEngine::timerCallback()
{
    if (state != Learning) { stopTimer(); return; }
    pullAudioAndStep();
    // Update status with light progress info
    const double remain = getRemainingSeconds();
    if (remain > 0.0)
    {
        setStatus (juce::String ("Listening ") + (usePreBus ? "(Pre)" : "(Post)") +
                   ", " + juce::String ((int) std::ceil (remain)) + "s left");
    }
    if (getRemainingSeconds() <= 0.0)
    {
        finalizeAndBuildProposals();
        stopTimer();
        state = Ready;
        setStatus ("Ready — proposals generated");
    }
}

void MachineEngine::prepareOnce()
{
    fft = juce::dsp::FFT (fftOrder);
    fftSize = 1 << fftOrder;
    hop = fftSize / 2;
    if (!win) win = std::make_unique<juce::dsp::WindowingFunction<float>> ((size_t) fftSize, juce::dsp::WindowingFunction<float>::hann, true);
    td.allocate ((size_t) fftSize, true);
    fd.allocate ((size_t) (2 * fftSize), true);
    tdL.allocate ((size_t) fftSize, true);
    tdR.allocate ((size_t) fftSize, true);
    fdL.allocate ((size_t) (2 * fftSize), true);
    fdR.allocate ((size_t) (2 * fftSize), true);
    prevMag.assign ((size_t) (fftSize / 2), 0.0f);

    bandBinRanges.clear();
    // Use 1/3-oct bands via helper for consistent UI metrics
    auto bands = fieldml::makeThirdOctaveBands (20.0, 20000.0, sampleRate, fftSize);
    for (const auto& b : bands) bandBinRanges.push_back ({ b.kStart, b.kEnd });
    A.bands = (int) bandBinRanges.size();
    A.bandE_L.assign ((size_t) A.bands, 0.0);
    A.bandE_R.assign ((size_t) A.bands, 0.0);
    A.bandSLL.assign ((size_t) A.bands, 0.0);
    A.bandSRR.assign ((size_t) A.bands, 0.0);
    A.bandSLR.assign ((size_t) A.bands, 0.0);
    A.bandCount.assign ((size_t) A.bands, 0.0);
}

void MachineEngine::pullAudioAndStep()
{
    static thread_local juce::AudioBuffer<float> tmp;
    const int maxPull = 4096;
    int n = (usePreBus ? proc.visPre.pull (tmp, maxPull)
                       : proc.visPost.pull (tmp, maxPull));
    if (n <= 0) return;
    setStatus ("Analyzing spectrum & stereo field…");
    const float* L = tmp.getReadPointer (0);
    const float* R = tmp.getNumChannels()>1 ? tmp.getReadPointer (1) : nullptr;
    stftAndAccumulate (L, R, n);

    for (int i = 0; i < n; ++i)
    {
        const float l = L ? L[i] : 0.0f;
        const float r = R ? R[i] : l;
        A.sumPowL += (double) l * l;
        A.sumPowR += (double) r * r;
        A.sumPow  += 0.5 * ((double) l * l + (double) r * r);
        A.sumAbsL += std::abs ((double) l);
        A.sumAbsR += std::abs ((double) r);
        A.peakL = juce::jmax (A.peakL, std::abs (l));
        A.peakR = juce::jmax (A.peakR, std::abs (r));
        A.sLL += (long double) l * (long double) l;
        A.sRR += (long double) r * (long double) r;
        A.sLR += (long double) l * (long double) r;
    }
    A.samples += n;
}

void MachineEngine::stftAndAccumulate (const float* L, const float* R, int n)
{
    if (n <= 0) return;
    int idx = 0;
    while (idx + fftSize <= n)
    {
        for (int i = 0; i < fftSize; ++i)
        {
            const float l = L ? L[idx + i] : 0.0f;
            const float r = R ? R[idx + i] : l;
            td[i]  = 0.5f * (l + r);
            tdL[i] = l;
            tdR[i] = r;
        }
        if (win) { win->multiplyWithWindowingTable (td.get(), fftSize); win->multiplyWithWindowingTable (tdL.get(), fftSize); win->multiplyWithWindowingTable (tdR.get(), fftSize); }

        for (int i = 0; i < fftSize; ++i) { fd[i] = td[i]; fdL[i] = tdL[i]; fdR[i] = tdR[i]; }
        std::fill (fd.get()  + fftSize, fd.get()  + 2*fftSize, 0.0f);
        std::fill (fdL.get() + fftSize, fdL.get() + 2*fftSize, 0.0f);
        std::fill (fdR.get() + fftSize, fdR.get() + 2*fftSize, 0.0f);
        fft.performRealOnlyForwardTransform (fd.get());
        fft.performRealOnlyForwardTransform (fdL.get());
        fft.performRealOnlyForwardTransform (fdR.get());

        double fluxNum = 0.0, fluxDen = 1e-9;
        setStatus ("Measuring transients & tone…");
        for (int k = 0; k < fftSize/2; ++k)
        {
            const float re = fd[2*k];
            const float im = fd[2*k+1];
            const float mag = std::sqrt (re*re + im*im);
            const float prev = prevMag[(size_t) k];
            const float d = juce::jmax (mag - prev, 0.0f);
            prevMag[(size_t) k] = mag;
            fluxNum += (double) d;
            fluxDen += (double) (mag + 1e-9f);
        }
        A.fluxSum += (fluxDen > 0.0 ? (fluxNum / fluxDen) : 0.0);
        A.fluxFrames++;

        const int samplePts = 18;
        for (int s = 0; s < samplePts; ++s)
        {
            const int k = juce::jlimit (1, fftSize/2 - 1, (int) std::round ((double) s / (double) samplePts * (fftSize/2 - 1)));
            const double f = (double) k * (sampleRate / (double) fftSize);
            const double x = std::log2 (juce::jlimit (1.0, 20000.0, f));
            const float re = fd[2*k], im = fd[2*k+1];
            const double y = 20.0 * std::log10 (juce::jmax (1e-12f, std::sqrt (re*re + im*im)));
            A.regN    += 1.0;
            A.regSumX += x;
            A.regSumY += y;
            A.regSumXX+= x*x;
            A.regSumXY+= x*y;
        }

        for (int b = 0; b < A.bands; ++b)
        {
            const auto range = bandBinRanges[(size_t) b];
            const int k0 = range.first, k1 = range.second;
            double eM = 0.0;
            setStatus ("Correlating bands…");
            for (int k = k0; k < k1; ++k)
            {
                const float reM = fd[2*k];
                const float imM = fd[2*k+1];
                const double m2 = (double) reM * reM + (double) imM * imM;
                eM += m2;
                const float reL = fdL[2*k], imL = fdL[2*k+1];
                const float reR = fdR[2*k], imR = fdR[2*k+1];
                const double pL = (double) reL * reL + (double) imL * imL;
                const double pR = (double) reR * reR + (double) imR * imR;
                const double cross = (double) reL * (double) reR + (double) imL * (double) imR;
                A.bandSLL[(size_t) b] += pL;
                A.bandSRR[(size_t) b] += pR;
                A.bandSLR[(size_t) b] += cross;
            }
            A.bandE_L[(size_t) b] += eM * 0.5;
            A.bandE_R[(size_t) b] += eM * 0.5;
            A.bandCount[(size_t) b] += 1.0;
        }

        idx += hop;
    }
}

void MachineEngine::finalizeAndBuildProposals()
{
    proposals.clear();
    if (A.samples <= 0) return;
    setStatus ("Building recommendations…");

    const long double denom = std::sqrt (A.sLL * A.sRR) + 1e-18L;
    const double fullCorr = juce::jlimit (-1.0, 1.0, (double) (A.sLR / denom));

    auto bandAvg = [&] (double f0, double f1) {
        double num=0.0, denLL=0.0, denRR=0.0; int cnt=0;
        for (int b = 0; b < A.bands; ++b)
        {
            const int k0 = bandBinRanges[(size_t) b].first;
            const int k1 = bandBinRanges[(size_t) b].second;
            const double fc = 0.5 * ((double) k0 + (double) k1) * (sampleRate / (double) fftSize);
            if (fc < f0 || fc >= f1) continue;
            num   += A.bandSLR[(size_t) b];
            denLL += A.bandSLL[(size_t) b];
            denRR += A.bandSRR[(size_t) b];
            cnt++;
        }
        const double d = std::sqrt (denLL * denRR) + 1e-18;
        return cnt > 0 ? (num / d) : 1.0;
    };
    const double corrLow = bandAvg (20.0, 100.0);
    const double corrMid = bandAvg (100.0, 2000.0);
    const double corrHi  = bandAvg (2000.0, 20000.0);

    // LMH width ratios via helper using accumulated band powers/cross
    juce::Array<fieldml::Band> bandsJ;
    {
        auto bands = fieldml::makeThirdOctaveBands (20.0, 20000.0, sampleRate, fftSize);
        for (auto& b : bands) bandsJ.add ({ b.fLow, b.fHigh, b.fCenter, b.kStart, b.kEnd });
    }
    juce::Array<double> PL, PR, RB;
    for (int b = 0; b < A.bands; ++b) { PL.add (A.bandE_L[(size_t)b]); PR.add (A.bandE_R[(size_t)b]);
        const double den = std::sqrt (A.bandSLL[(size_t)b] * A.bandSRR[(size_t)b]) + 1e-18;
        RB.add (den > 0.0 ? A.bandSLR[(size_t)b] / den : 1.0);
    }
    auto readParam = [&](const char* pid, double def)->double { if (auto* p = apvts.getRawParameterValue (pid)) return (double) p->load(); return def; };
    auto lmh = fieldml::summarizeCorrelationWidthJ (bandsJ, PL, PR, RB, readParam ("xover_lo_hz", 150.0), readParam ("xover_hi_hz", 2000.0));

    auto get = [&] (const char* id) -> float { if (auto* p = apvts.getRawParameterValue (id)) return p->load(); return 0.0f; };
    struct HostSnap { double widthLo{}, widthMid{}, widthHi{}, xLo{}, xHi{}, rot{}, asym{}, monoHz{}, tiltDb{}, tiltFreq{}, hpHz{}, lpHz{}, bassDb{}, airDb{}, depth{}, duck{}, duckAtk{}, duckRel{}, duckThr{}, duckRatio{}; int algo{}; };
    HostSnap hp{};
    hp.widthLo = get ("width_lo"); hp.widthMid = get ("width_mid"); hp.widthHi = get ("width_hi");
    hp.xLo = get ("xover_lo_hz"); hp.xHi = get ("xover_hi_hz");
    hp.rot = get ("rotation_deg"); hp.asym = get ("asymmetry");
    hp.monoHz = get ("mono_hz");
    hp.tiltDb = get ("tilt"); hp.tiltFreq = get ("tilt_freq");
    hp.hpHz = get ("hp_hz"); hp.lpHz = get ("lp_hz");
    hp.bassDb = get ("bass_db"); hp.airDb = get ("air_db");
    hp.depth = get ("depth"); hp.algo = (int) apvts.getParameterAsValue ("space_algo").getValue();
    hp.duck = get ("ducking"); hp.duckAtk = get ("duck_attack_ms"); hp.duckRel = get ("duck_release_ms"); hp.duckThr = get ("duck_threshold_db"); hp.duckRatio = get ("duck_ratio");

    {
        Proposal p; p.id = "imaging"; p.title = "Stereo Field & Clarity"; p.summary = "Safer lows, balanced field, musical width.";
        float fMono = 0.0f;
        if (corrLow < 0.6)
        {
            fMono = (float) (corrLow < 0.3 ? 120.0 : 80.0);
            p.params.push_back ({ "mono_hz", (float) hp.monoHz, fMono, 0.0f, 300.0f, 1.0f, "Low-band correlation suggests mono-ing lows." });
        }
        auto tgtWmid = juce::jlimit (0.5f, 1.6f, (float) hp.widthMid + (fullCorr > 0.9 ? 0.20f : 0.00f));
        auto tgtWhi  = juce::jlimit (0.5f, 1.6f, (float) hp.widthHi  + (fullCorr > 0.9 ? 0.30f : 0.10f));
        auto tgtWlo  = juce::jlimit (0.5f, 1.6f, (float) hp.widthLo  + (corrLow < 0.5 ? -0.10f : 0.00f));
        p.params.push_back ({ "width_lo",  (float) hp.widthLo,  tgtWlo,  0.5f, 1.6f, 0.7f,  "Adjust low-band width toward safe image." });
        p.params.push_back ({ "width_mid", (float) hp.widthMid, tgtWmid, 0.5f, 1.6f, 1.0f,  "Open mid-band for presence in stereo." });
        p.params.push_back ({ "width_hi",  (float) hp.widthHi,  tgtWhi,  0.5f, 1.6f, 1.0f,  "Enhance sparkle width carefully." });
        const float RL = (float) (10.0 * std::log10 ((A.sumPowL + 1e-9) / (A.sumPowR + 1e-9)));
        const float rot = juce::jlimit (-10.0f, 10.0f, -0.6f * RL);
        if (std::abs (rot) > 0.4f)
            p.params.push_back ({ "rotation_deg", (float) hp.rot, rot, -45.0f, 45.0f, 0.6f, "Center left/right energy." });
        // Crossovers & shuffler nudges based on summaries
        if (corrLow < 0.55 && hp.xLo < 200.0)
            p.params.push_back ({ "xover_lo_hz", (float) hp.xLo, (float) juce::jlimit (40.0, 400.0, hp.xLo * 1.2), 40.0f, 400.0f, 0.4f, "Raise LF crossover for stability." });
        if (corrHi > 0.9 && hp.xHi < 4000.0)
            p.params.push_back ({ "xover_hi_hz", (float) hp.xHi, (float) juce::jlimit (800.0, 6000.0, hp.xHi * 1.1), 800.0f, 6000.0f, 0.3f, "Nudge HF crossover for clarity." });
        if (lmh.Wlo < 0.4 && lmh.Whi > 0.6)
        {
            p.params.push_back ({ "shuffler_lo_pct",  (float) get ("shuffler_lo_pct"),  130.0f, 0.0f, 200.0f, 0.3f, "Enhance LF impact (psycho)." });
            p.params.push_back ({ "shuffler_hi_pct",  (float) get ("shuffler_hi_pct"),  110.0f, 0.0f, 200.0f, 0.2f, "Subtle HF presence." });
            p.params.push_back ({ "shuffler_xover_hz",(float) get ("shuffler_xover_hz"), (float) juce::jlimit (150.0, 2000.0, hp.xLo * 4.0), 150.0f, 2000.0f, 0.2f, "Shuffler crossover tune." });
        }
        p.metrics.set ("corr_low", corrLow);
        p.metrics.set ("corr_mid", corrMid);
        p.metrics.set ("corr_hi",  corrHi);
        p.metrics.set ("full_corr", fullCorr);
        p.metrics.set ("Wlo",  (float) lmh.Wlo);
        p.metrics.set ("Wmid", (float) lmh.Wmid);
        p.metrics.set ("Whi",  (float) lmh.Whi);
        proposals.push_back (std::move (p));
    }

    {
        Proposal p; p.id = "tone"; p.title = "Tone & Balance"; p.summary = "Gentle spectral tilt to target, cleanup edges.";
        double slope = 0.0; const double denomR = (A.regN * A.regSumXX - A.regSumX * A.regSumX);
        if (std::abs (denomR) > 1e-9) slope = (A.regN * A.regSumXY - A.regSumX * A.regSumY) / denomR;
        const float targetSlope = -3.0f;
        float dTilt = juce::jlimit (-4.0f, 4.0f, (targetSlope - (float) slope) * 1.2f);
        if (std::abs (dTilt) > 0.3f)
            p.params.push_back ({ "tilt", (float) hp.tiltDb, (float) hp.tiltDb + dTilt, -12.0f, 12.0f, 1.0f, "Bring spectrum toward pink target." });
        // Shelves and scoop from band imbalances
        if (lmh.Wlo < 0.45)
            p.params.push_back ({ "bass_db", (float) hp.bassDb, (float) juce::jlimit (-6.0, 6.0, hp.bassDb + 1.5), -6.0f, 6.0f, 0.5f, "Support lows." });
        if (lmh.Whi < 0.55)
            p.params.push_back ({ "air_db", (float) hp.airDb, (float) juce::jlimit (0.0, 6.0, hp.airDb + 1.0), 0.0f, 6.0f, 0.4f, "Add presence/air." });
        if (lmh.Wmid > 0.65)
            p.params.push_back ({ "scoop", (float) hp.tiltDb /* using tiltDb var is fine */, (float) juce::jlimit (-12.0, 12.0, hp.tiltDb - 2.0), -12.0f, 12.0f, 0.5f, "Tame mid build-up." });
        p.metrics.set ("slope_db_per_oct", (float) slope);
        // Extra mini-meters: sibilance, rumble, fizz
        auto sumPow = [&](double f0, double f1) -> double
        {
            double s = 0.0;
            for (int i = 0; i < bandsJ.size(); ++i)
            {
                const auto& b = bandsJ.getReference(i);
                if (b.fCenter >= f0 && b.fCenter < f1)
                    s += PL[i] + PR[i];
            }
            return s;
        };
        const double p20_40   = sumPow (20.0,   40.0);
        const double p80_160  = sumPow (80.0,  160.0);
        const double p4k_8k   = sumPow (4000.0, 8000.0);
        const double p6k_10k  = sumPow (6000.0,10000.0);
        const double p16k_20k = sumPow (16000.0,20000.0);
        const double p1k_4k   = sumPow (1000.0, 4000.0);
        auto norm01 = [](double a, double b){ double d = a + b; return d > 1e-18 ? juce::jlimit (0.0, 1.0, a / d) : 0.0; };
        const float rumble01   = (float) norm01 (p20_40, p80_160);
        const float fizz01     = (float) norm01 (p16k_20k, p4k_8k);
        const float sibilance01= (float) norm01 (p6k_10k, p1k_4k);
        p.metrics.set ("lf_rumble",   rumble01);
        p.metrics.set ("hf_fizz",     fizz01);
        p.metrics.set ("sibilance",   sibilance01);
        // Gate HP/LP using measured rumble/fizz/sibilance and context thresholds
        const float rumbleGate  = tgt.lfRumbleMaxDb > 0.0f ? juce::jlimit (0.0f, 1.0f, tgt.lfRumbleMaxDb / 12.0f) : 0.5f;
        const float fizzGate    = tgt.hfFizzMaxDb  > 0.0f ? juce::jlimit (0.0f, 1.0f, tgt.hfFizzMaxDb  / 12.0f) : 0.4f;
        if (corrLow < juce::jmax (0.45, (double) (tgt.corrLowMin - 0.1)) || rumble01 > rumbleGate)
            p.params.push_back ({ "hp_hz", (float) hp.hpHz, (float) juce::jlimit (20.0, 1000.0, hp.hpHz > 20.0 ? hp.hpHz : 45.0), 20.0f, 1000.0f, 0.8f, "Reduce rumble in low end." });
        if (corrHi > 0.95 || fizz01 > fizzGate || sibilance01 > 0.6f)
            p.params.push_back ({ "lp_hz", (float) hp.lpHz, (float) juce::jlimit (2000.0, 20000.0, hp.lpHz < 20000.0 ? hp.lpHz : 17000.0), 2000.0f, 20000.0f, 0.6f, "Tame ultrasonic fizz/sibilance." });
        proposals.push_back (std::move (p));
    }

    {
        // Skip space/delay on Mix Bus/Master by default
        const bool allowFX = !(ctx.trackType == 6 /*Mix Bus*/ || ctx.trackType == 7 /*Master*/);
        Proposal p; p.id = "reverb"; p.title = "Reverb, Delay, Motion"; p.summary = allowFX ? "Depth and intelligent ducking." : "No reverb/delay on bus/master.";
        const double rms = std::sqrt (A.sumPow / (double) juce::jmax (1, A.samples));
        const double pk  = 0.5 * ((double) A.peakL + (double) A.peakR);
        const double crest = (rms > 1e-9 ? pk / rms : 1.0);
        const double avgFlux = (A.fluxFrames > 0 ? (A.fluxSum / (double) A.fluxFrames) : 0.0);
        // Dryness via helper mapping crest dB and normalized flux
        const double crestDb = (crest > 1e-9 ? 20.0 * std::log10 (crest) : 0.0);
        const double dryness = fieldml::drynessIndex (crestDb, juce::jlimit (0.0, 1.0, avgFlux));
        float depth = juce::jmap ((float) dryness, 0.0f, 1.0f, tgt.depthLo > 0.0f ? tgt.depthLo : 0.10f, tgt.depthHi > 0.0f ? tgt.depthHi : 0.30f);
        if (allowFX)
            p.params.push_back ({ "depth", (float) hp.depth, depth, 0.0f, 1.0f, 1.0f, "Add a touch of space." });
        double slope2 = 0.0; const double denom2 = (A.regN * A.regSumXX - A.regSumX * A.regSumX);
        if (std::abs (denom2) > 1e-9) slope2 = (A.regN * A.regSumXY - A.regSumX * A.regSumY) / denom2;
        int algo = (slope2 > -3.0 ? 1 : 2);
        if (allowFX)
            p.params.push_back ({ "space_algo", (float) hp.algo, (float) algo, 0.0f, 2.0f, 0.4f, "Algorithm based on brightness." });
        if (fullCorr > 0.80 && allowFX)
        {
            p.params.push_back ({ "ducking", (float) hp.duck, 0.6f, 0.0f, 1.0f, 1.0f, "Keep source forward during hits." });
            p.params.push_back ({ "duck_attack_ms", (float) hp.duckAtk, 12.0f, 1.0f, 80.0f, 0.6f, "" });
            p.params.push_back ({ "duck_release_ms", (float) hp.duckRel, 180.0f, 20.0f, 800.0f, 0.6f, "" });
            p.params.push_back ({ "duck_threshold_db", (float) hp.duckThr, -22.0f, -60.0f, 0.0f, 0.6f, "" });
            p.params.push_back ({ "duck_ratio", (float) hp.duckRatio, 3.0f, 1.0f, 20.0f, 0.6f, "" });
            p.params.push_back ({ "duck_target", 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, "Wet-only ducking." });
        }
        p.metrics.set ("crest", (float) crest);
        p.metrics.set ("avg_flux", (float) avgFlux);
        p.metrics.set ("dryness_index", (float) dryness);
        proposals.push_back (std::move (p));
    }
}

juce::String MachineEngine::getStatus() const
{
    return status;
}

juce::String MachineEngine::getAnalysisInfo() const
{
    juce::String info;
    info << "FFT=" << fftSize << ", Hop=" << hop
         << ", Bands=" << A.bands << ", Fs=" << (int) std::round (sampleRate) << " Hz";
    return info;
}

void MachineEngine::setStatus (const juce::String& s)
{
    status = s;
    // Log status transitions and unique phase messages (avoid log spam per frame)
    const juce::String key = s;
    const bool isPhaseMsg = (key.containsIgnoreCase ("Initializing") ||
                             key.containsIgnoreCase ("Listening")    ||
                             key.containsIgnoreCase ("Analyzing")    ||
                             key.containsIgnoreCase ("Measuring")    ||
                             key.containsIgnoreCase ("Correlating")  ||
                             key.containsIgnoreCase ("Building")     ||
                             key.containsIgnoreCase ("Ready"));
    if (! isPhaseMsg) return;
    const juce::ScopedLock sl (msgLock);
    if (key != lastLoggedStatus)
    {
        lastLoggedStatus = key;
        messageLog.add (key);
        // Keep log short
        while (messageLog.size() > 8)
            messageLog.remove (0);
    }
}

juce::StringArray MachineEngine::getMessages() const
{
    const juce::ScopedLock sl (msgLock);
    return messageLog;
}

void MachineEngine::captureBaseline()
{
    baselineParams.clear();
    static const char* ids[] = {
        "mono_hz", /* omit mono_slope_db_oct for now */ "width_lo", "width_mid", "width_hi", "rotation_deg",
        "tilt", "hp_hz", "lp_hz", "depth", "space_algo", "ducking", "duck_attack_ms", "duck_release_ms", "duck_threshold_db", "duck_ratio"
    };
    for (auto* id : ids)
    {
        if (auto* p = apvts.getRawParameterValue (id))
            baselineParams.emplace_back (id, p->load());
    }
}

void MachineEngine::setParam (const char* id, float v, bool notifyHost)
{
    if (auto* ap = apvts.getParameter (id))
    {
        // Safety rails (basic)
        if (std::strcmp(id, "width_hi") == 0) {
            v = juce::jlimit (0.0f, tgt.widthMax > 0.0f ? tgt.widthMax : 2.0f, v);
        }
        if (std::strcmp(id, "hp_hz") == 0) {
            float monoHz = v;
            if (auto* rp = apvts.getRawParameterValue ("mono_hz")) monoHz = juce::jmax (v, rp->load());
            v = juce::jmin (v, monoHz + 20.0f);
        }
        if (notifyHost) ap->beginChangeGesture();
        auto range = apvts.getParameterRange (id);
        ap->setValueNotifyingHost (range.convertTo0to1 (v));
        if (notifyHost) ap->endChangeGesture();
    }
}
