#include "DelayPresetLibrary.h"
#include <juce_data_structures/juce_data_structures.h>
#include <juce_core/juce_core.h>

namespace
{
	static juce::File externalJson()
	{
		return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
			   .getChildFile ("Field/Presets/delay_presets.json");
	}

	static juce::File externalJsonAlt()
	{
		return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
			   .getChildFile ("Field/Field/Presets/delay_presets.json");
	}

	static juce::File localJson()
	{
		// Dev convenience: allows reading JSON placed next to this source file
		return juce::File (__FILE__).getParentDirectory().getChildFile ("delay_presets.json");
	}

	static juce::File externalJsonFactory()
	{
		return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
			   .getChildFile ("Field/Presets/delay_factory_presets.json");
	}

	static juce::File envJson()
	{
		const char* env = std::getenv("FIELD_PRESET_JSON");
		return (env != nullptr) ? juce::File (juce::String (env)) : juce::File();
	}

	static juce::Array<juce::File> bundleCandidates()
	{
		juce::Array<juce::File> out;
		auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
		// Typical macOS bundle: .../Contents/MacOS/Field → .../Contents/Resources/delay_factory_presets.json
		auto contents = exe.getParentDirectory().getParentDirectory();
		out.addIfNotAlreadyThere (contents.getChildFile ("Resources/delay_factory_presets.json"));
		// Next to binary (dev runs)
		out.addIfNotAlreadyThere (exe.getSiblingFile ("delay_factory_presets.json"));
		// One level up Resources (some packagers)
		out.addIfNotAlreadyThere (exe.getParentDirectory().getSiblingFile ("Resources/delay_factory_presets.json"));
		return out;
	}

	static const char* kEmbeddedDelayPresetsJson = R"JSON(
[]
)JSON";

	static std::vector<DelayPresetMeta> hardcodedBuiltins()
	{
		std::vector<DelayPresetMeta> out;

		// #1 Lead Vox Safety Echo
		{
			DelayPresetMeta m;
			m.name = "Lead Vox Safety Echo";
			m.desc = "Always-on vocal support—clean 1/8 that adds depth without blur.";
			m.hint = "Set first; it should feel like part of the vocal.";
			m.tags.add("vocal"); m.tags.add("support"); m.tags.add("clean"); m.tags.add("1/8"); m.tags.add("ducked"); m.tags.add("digital");
			m.params.enabled = true; m.params.mode = 0; m.params.sync = true;
			m.params.timeMs = 250.0; m.params.timeDiv = 8; m.params.gridFlavor = 0; m.params.tempoBpm = 120.0;
			m.params.feedbackPct = 22.0; m.params.wet = 0.16; m.params.killDry = false; m.params.freeze = false;
			m.params.pingpong = false; m.params.crossfeedPct = 20.0; m.params.stereoSpreadPct = 20.0; m.params.width = 0.90;
			m.params.modRateHz = 0.35; m.params.modDepthMs = 2.5; m.params.wowflutter = 0.0; m.params.jitterPct = 1.5;
			m.params.hpHz = 140.0; m.params.lpHz = 9500.0; m.params.tiltDb = 0.0; m.params.sat = 0.05;
			m.params.diffusion = 0.0; m.params.diffuseSizeMs = 18.0;
			m.params.duckSource = 0; m.params.duckPost = true; m.params.duckDepth = 0.55;
			m.params.duckAttackMs = 10.0; m.params.duckReleaseMs = 180.0; m.params.duckThresholdDb = -24.0;
			m.params.duckRatio = 2.0; m.params.duckLookaheadMs = 5.0; m.params.duckLinkGlobal = true;
			out.push_back(std::move(m));
		}

		// #2 Analog Ping-Pong Dotted 1/8
		{
			DelayPresetMeta m;
			m.name = "Analog Ping-Pong Dotted 1/8";
			m.desc = "BBD-style dotted-eighth ping-pong; musical width without fizz.";
			m.hint = "Automate FEEDBACK for builds.";
			m.tags.add("analog"); m.tags.add("guitar"); m.tags.add("synth"); m.tags.add("ping-pong"); m.tags.add("dotted"); m.tags.add("1/8");
			m.params.enabled = true; m.params.mode = 1; m.params.sync = true;
			m.params.timeMs = 350.0; m.params.timeDiv = 8; m.params.gridFlavor = 1; m.params.tempoBpm = 120.0;
			m.params.feedbackPct = 48.0; m.params.wet = 0.24; m.params.killDry = false; m.params.freeze = false;
			m.params.pingpong = true; m.params.crossfeedPct = 30.0; m.params.stereoSpreadPct = 25.0; m.params.width = 1.0;
			m.params.modRateHz = 0.35; m.params.modDepthMs = 3.0; m.params.wowflutter = 0.0; m.params.jitterPct = 3.0;
			m.params.hpHz = 120.0; m.params.lpHz = 7500.0; m.params.tiltDb = 0.0; m.params.sat = 0.35;
			m.params.diffusion = 0.10; m.params.diffuseSizeMs = 14.0;
			m.params.duckSource = 2; m.params.duckPost = true; m.params.duckDepth = 0.50;
			m.params.duckAttackMs = 12.0; m.params.duckReleaseMs = 200.0; m.params.duckThresholdDb = -24.0;
			m.params.duckRatio = 2.0; m.params.duckLookaheadMs = 5.0; m.params.duckLinkGlobal = true;
			out.push_back(std::move(m));
		}

		return out;
	}

	static int straightIndexForDenom (int denom)
	{
		switch (denom)
		{
			case 64: return 1;  // 1/64
			case 32: return 4;  // 1/32
			case 16: return 7;  // 1/16
			case 8:  return 10; // 1/8
			case 4:  return 13; // 1/4
			case 2:  return 16; // 1/2
			case 1:  return 19; // 1/1
			default: return 10; // fallback 1/8
		}
	}

	static void fillParamsFromJson (DelayParams& p, const juce::var& v)
	{
		const auto* obj = v.getDynamicObject();
		if (obj == nullptr) return;

		auto get = [&] (const char* k) -> juce::var {
			const juce::Identifier id (k);
			return obj->hasProperty (id) ? obj->getProperty (id) : juce::var();
		};

		p.enabled          = (bool)   get ("enabled");
		p.mode             = juce::jlimit (0, 2, (int) get ("mode"));
		p.sync             = (bool)   get ("sync");
		p.timeMs           = (double) get ("timeMs");
		{
			const int denom = (int) get ("timeDiv");
			p.timeDiv       = juce::jlimit (0, 26, straightIndexForDenom (denom));
		}
		p.gridFlavor       = juce::jlimit (0, 2, (int) get ("gridFlavor"));
		p.tempoBpm         = (double) get ("tempoBpm");
		p.feedbackPct      = (double) get ("feedbackPct");
		p.wet              = (double) get ("wet");
		p.killDry          = (bool)   get ("killDry");
		p.freeze           = (bool)   get ("freeze");
		p.pingpong         = (bool)   get ("pingpong");
		p.crossfeedPct     = (double) get ("crossfeedPct");
		p.stereoSpreadPct  = (double) get ("stereoSpreadPct");
		p.width            = (double) get ("width");
		p.modRateHz        = (double) get ("modRateHz");
		p.modDepthMs       = (double) get ("modDepthMs");
		p.wowflutter       = (double) get ("wowflutter");
		p.jitterPct        = (double) get ("jitterPct");
		p.hpHz             = (double) get ("hpHz");
		p.lpHz             = (double) get ("lpHz");
		p.tiltDb           = (double) get ("tiltDb");
		p.sat              = (double) get ("sat");
		p.diffusion        = (double) get ("diffusion");
		p.diffuseSizeMs    = (double) get ("diffuseSizeMs");
		{
			int ds = (int) get ("duckSource");
			p.duckSource = juce::jlimit (0, 2, ds); // clamp 3->2 for "Both"
		}
		p.duckPost         = (bool)   get ("duckPost");
		p.duckDepth        = (double) get ("duckDepth");
		p.duckAttackMs     = (double) get ("duckAttackMs");
		p.duckReleaseMs    = (double) get ("duckReleaseMs");
		p.duckThresholdDb  = (double) get ("duckThresholdDb");
		p.duckRatio        = (double) get ("duckRatio");
		p.duckLookaheadMs  = (double) get ("duckLookaheadMs");
		p.duckLinkGlobal   = (bool)   get ("duckLinkGlobal");
	}

	static std::vector<DelayPresetMeta> buildLibrary()
	{
		std::vector<DelayPresetMeta> out;

		auto parseArray = [&] (const juce::var& arr)
		{
			if (! arr.isArray()) return;
			for (const auto& it : *arr.getArray())
			{
				if (! it.isObject()) continue;
				auto* obj = it.getDynamicObject(); if (obj == nullptr) continue;

				DelayPresetMeta meta;
				meta.name = obj->hasProperty ("name") ? obj->getProperty ("name").toString() : juce::String();
				meta.desc = obj->hasProperty ("desc") ? obj->getProperty ("desc").toString() : juce::String();
				meta.hint = obj->hasProperty ("hint") ? obj->getProperty ("hint").toString() : juce::String();

				if (auto tagsVar = obj->getProperty ("tags"); tagsVar.isArray())
					for (const auto& t : *tagsVar.getArray()) meta.tags.add (t.toString());

				if (auto paramsVar = obj->getProperty ("params"); paramsVar.isObject())
					fillParamsFromJson (meta.params, paramsVar);

				if (meta.name.isNotEmpty()) out.push_back (std::move (meta));
			}
		};

		auto parseTop = [&] (const juce::var& v)
		{
			if (v.isArray()) { parseArray (v); return; }
			if (auto* d = v.getDynamicObject())
			{
				const char* keys[] = { "presets","items","data","list","delay_presets" };
				bool any = false;
				for (auto* k : keys) { auto a = d->getProperty (k); if (a.isArray()) { parseArray (a); any = true; } }
				if (! any)
				{
					for (auto& kv : d->getProperties())
						if (kv.value.isArray()) parseArray (kv.value);
						else if (kv.value.isObject()) { juce::Array<juce::var> one; one.add (kv.value); parseArray (juce::var (one)); }
				}
			}
		};

		auto tryFile = [&] (const juce::File& f) -> bool
		{
			if (! f.existsAsFile()) return false;
			auto s = f.loadFileAsString();
			auto v = juce::JSON::parse (s);
			if (v.isVoid()) { DBG("[DelayPresets] JSON parse failed: " << f.getFullPathName()); return false; }
			parseTop (v);
			DBG("[DelayPresets] " << f.getFileName() << " -> parsed " << (int) out.size() << " presets");
			return ! out.empty();
		};

		// ---------- PRIORITY ORDER ----------
		if (auto f = envJson(); f.existsAsFile()) { DBG("[DelayPresets] ENV " << f.getFullPathName()); if (tryFile (f)) return out; }
		if (auto f = externalJsonFactory(); tryFile (f)) return out;
		if (auto f = externalJson();        tryFile (f)) return out;
		if (auto f = externalJsonAlt();     tryFile (f)) return out;
		for (auto f : bundleCandidates())   if (tryFile (f)) return out;
		if (auto f = localJson();           tryFile (f)) return out;

		// Embedded JSON (string below)
		{
			auto v = juce::JSON::parse (juce::String (kEmbeddedDelayPresetsJson));
			if (! v.isVoid()) { parseTop (v); DBG("[DelayPresets] embedded string -> " << (int) out.size()); if (! out.empty()) return out; }
		}

		// Last resort: compiled built-ins so UI is never empty
		DBG("[DelayPresets] No preset sources found; using hardcoded built-ins.");
		return hardcodedBuiltins();
	}

	static std::vector<DelayPresetMeta>& presetCache()
	{
		static std::vector<DelayPresetMeta> c;
		return c;
	}
}

void DelayPresets::resetCache()
{
    presetCache().clear();
}

const std::vector<DelayPresetMeta>& DelayPresets::all()
{
	auto& cache = presetCache();
	if (! cache.empty()) return cache;

	cache = buildLibrary();
	DBG("[DelayPresets] all() cache size=" << (int) cache.size());
	return cache;
}

const DelayPresetMeta* DelayPresets::findByName (const juce::String& name)
{
	const auto& lib = all();
	auto it = std::find_if (lib.begin(), lib.end(), [&] (const DelayPresetMeta& p){ return p.name == name; });
	return it != lib.end() ? &(*it) : nullptr;
}

juce::StringArray DelayPresets::names()
{
	juce::StringArray out;
	for (const auto& p : all()) out.add (p.name);
	out.sort (true);
	return out;
}


