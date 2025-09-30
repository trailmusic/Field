#include "PresetManager.h"

using namespace juce;

ParamMap::ParamMap()
{
	map.set("enabled",              "delay_enabled");
	map.set("mode",                 "delay_mode");
	map.set("sync",                 "delay_sync");
	map.set("timeMs",               "delay_time_ms");
	map.set("timeDiv",              "delay_time_div");
	map.set("gridFlavor",           "delay_grid_flavor");
	map.set("tempoBpm",             "tempo_bpm");
	map.set("feedbackPct",          "delay_feedback_pct");
	map.set("wet",                  "delay_wet");
	map.set("killDry",              "delay_kill_dry");
	map.set("freeze",               "delay_freeze");
	map.set("pingpong",             "delay_pingpong");
	map.set("crossfeedPct",         "delay_crossfeed_pct");
	map.set("stereoSpreadPct",      "delay_stereo_spread_pct");
	map.set("width",                "delay_width");
	map.set("modRateHz",            "delay_mod_rate_hz");
	map.set("modDepthMs",           "delay_mod_depth_ms");
	map.set("wowflutter",           "delay_wowflutter");
	map.set("jitterPct",            "delay_jitter_pct");
	map.set("hpHz",                 "delay_hp_hz");
	map.set("lpHz",                 "delay_lp_hz");
	map.set("tiltDb",               "delay_tilt_db");
	map.set("sat",                  "delay_sat");
	map.set("diffusion",            "delay_diffusion");
	map.set("diffuseSizeMs",        "delay_diffuse_size_ms");
	map.set("duckSource",           "delay_duck_source");
	map.set("duckPost",             "delay_duck_post");
	map.set("duckDepth",            "delay_duck_depth");
	map.set("duckAttackMs",         "delay_duck_attack_ms");
	map.set("duckReleaseMs",        "delay_duck_release_ms");
	map.set("duckThresholdDb",      "delay_duck_threshold_db");
	map.set("duckRatio",            "delay_duck_ratio");
	map.set("duckLookaheadMs",      "delay_duck_lookahead_ms");
	map.set("duckLinkGlobal",       "delay_duck_link_global");
}

NewPresetManager::NewPresetManager (AudioProcessorValueTreeState& s, UndoManager* um)
: apvts(s), undo(um) {}

void NewPresetManager::setParam (RangedAudioParameter* p, const var& v)
{
	if (p == nullptr) return;
	float target = p->getDefaultValue();
	if (v.isBool())   target = (v ? 1.0f : 0.0f);
	else if (v.isInt())    target = (float) (int) v;
	else if (v.isDouble()) target = (float) (double) v;
	
	p->beginChangeGesture();
	p->setValueNotifyingHost (p->convertTo0to1 (target));
	p->endChangeGesture();
}

void NewPresetManager::applyPresetAtomic (const LibraryPreset& pr)
{
	if (undo) undo->beginNewTransaction ("Load Preset: " + pr.meta.name);
	for (int i = 0; i < pr.params.size(); ++i)
	{
		const String logical = pr.params.getName(i).toString();
		const String id = paramMap.map.contains (logical) ? paramMap.map[logical] : logical;
		if (auto* p = apvts.getParameter (id))
		{
			const var val = pr.params.getValueAt (i);
			setParam (p, val);
		}
	}
}

void NewPresetManager::auditionStart (const LibraryPreset& p)
{
	snapshotBeforeAudition = apvts.copyState();
	applyPresetAtomic (p);
}

void NewPresetManager::auditionCancel ()
{
	if (snapshotBeforeAudition.isValid()) apvts.replaceState (snapshotBeforeAudition);
	snapshotBeforeAudition = {};
}

void NewPresetManager::auditionCommitToSlot (bool toA)
{
	if (toA) slotAState = apvts.copyState(); else slotBState = apvts.copyState();
	snapshotBeforeAudition = {};
}

void NewPresetManager::loadToSlot (const LibraryPreset& p, bool toA)
{
	applyPresetAtomic (p);
	auditionCommitToSlot (toA);
}

LibraryPreset NewPresetManager::currentAsPreset (String name, String category, StringArray tags, String description, String hint, String author)
{
	LibraryPreset pr;
	pr.meta.id = Uuid();
	pr.meta.name = name; pr.meta.category = category; pr.meta.author = author;
	pr.meta.description = description; pr.meta.hint = hint;
	pr.meta.tags = tags; pr.meta.isFactory = false;
	pr.meta.createdAt = Time::getCurrentTime().toMilliseconds();

	for (auto it = paramMap.map.begin(); it != paramMap.map.end(); ++it)
	{
		const auto logical = it.getKey();
		const auto id = it.getValue();
		if (auto* p = apvts.getParameter(id))
		{
			auto norm = p->getValue();
			auto denorm = p->convertFrom0to1 (norm);
			pr.params.set (logical, denorm);
		}
	}
	return pr;
}


