#include "PresetStore.h"

using namespace juce;

static File defaultUserDir()
{
#if JUCE_MAC
	return File::getSpecialLocation(File::userApplicationDataDirectory)
	        .getChildFile("Field").getChildFile("Field").getChildFile("Presets");
#else
	return File::getSpecialLocation(File::userApplicationDataDirectory)
	        .getChildFile("Field").getChildFile("Field").getChildFile("Presets");
#endif
}

PresetStore::PresetStore()
{
	setUserDir (defaultUserDir());
	setMetaDB  (userDir.getSiblingFile("Library").getChildFile("library_meta.json"));
	loadMetaDB();
}

void PresetStore::setUserDir (File dir) { userDir = dir; userDir.createDirectory(); }
void PresetStore::setMetaDB  (File f)   { metaDbPath = f; metaDbPath.getParentDirectory().createDirectory(); }

void PresetStore::addFactoryPreset (const LibraryPreset& p)
{
	auto q = p; q.meta.isFactory = true;
	if (q.meta.id.isNull() || byId.contains(q.meta.id)) q.meta.id = Uuid();
	byId.set (q.meta.id, q);
	order.add (q.meta.id);
}

void PresetStore::clearFactory()
{
	Array<Uuid> toRemove;
	for (auto& id : order) if (byId[id].meta.isFactory) toRemove.add (id);
	for (auto& id : toRemove) { byId.remove(id); order.removeFirstMatchingValue(id); }
}

static Optional<LibraryPreset> readPresetFile (const File& f)
{
	auto txt = f.loadFileAsString(); if (txt.isEmpty()) return {};
	auto v = JSON::parse (txt);
	return PresetStore::fromJson (v);
}

void PresetStore::scan()
{
	Array<Uuid> toRemove;
	for (auto& id : order) if (! byId[id].meta.isFactory) toRemove.add(id);
	for (auto& id : toRemove) { byId.remove(id); order.removeFirstMatchingValue(id); }

	if (userDir.exists())
	{
		Array<File> files; userDir.findChildFiles (files, File::findFiles, false, "*.json");
		for (auto& f : files)
			if (auto p = readPresetFile (f)) { byId.set (p->meta.id, *p); order.add (p->meta.id); }
	}

	loadMetaDB();
	if (metaDB != nullptr)
	{
		auto* favs = metaDB->getProperty("favorites").getDynamicObject();
		if (favs != nullptr)
			for (auto& id : order)
			{
				const auto key = id.toString();
				if (favs->hasProperty (key))
				{
					auto tmp = byId[id];
					tmp.meta.isFavorite = (bool) favs->getProperty (key);
					byId.set (id, tmp);
				}
			}

		auto* usage = metaDB->getProperty("usage").getDynamicObject();
		if (usage != nullptr)
			for (auto& id : order)
				if (auto* u = usage->getProperty(id.toString()).getDynamicObject())
				{
					auto tmp = byId[id];
					tmp.meta.timesUsed  = (int)  u->getProperty("times");
					tmp.meta.lastUsedAt = (int64_t)u->getProperty("lastUsedAt");
					byId.set (id, tmp);
				}
	}
}

bool PresetStore::saveUserPreset (const LibraryPreset& p)
{
	auto fn = userDir.getChildFile (p.meta.id.toString() + ".json");
	auto j = toJson (p);
	return fn.replaceWithText (JSON::toString (j, true));
}

bool PresetStore::overwriteUserPreset (const LibraryPreset& p)
{
	jassert (! p.meta.isFactory);
	return saveUserPreset (p);
}

bool PresetStore::setFavorite (const Uuid& id, bool fav)
{
	if (! byId.contains(id)) return false;
	auto tmpFav = byId[id];
	tmpFav.meta.isFavorite = fav;
	byId.set (id, tmpFav);

	if (metaDB == nullptr) metaDB = new DynamicObject();
	auto* favs = metaDB->getProperty("favorites").getDynamicObject();
	if (favs == nullptr) { favs = new DynamicObject(); metaDB->setProperty("favorites", var(favs)); }
	favs->setProperty (id.toString(), fav);
	saveMetaDB();
	return true;
}

void PresetStore::bumpUsage (const Uuid& id)
{
	if (! byId.contains(id)) return;
	auto item = byId[id];
	auto& m = item.meta;
	m.timesUsed++; m.lastUsedAt = Time::getCurrentTime().toMilliseconds();
	byId.set (id, item);

	if (metaDB == nullptr) metaDB = new DynamicObject();
	auto* usage = metaDB->getProperty("usage").getDynamicObject();
	if (usage == nullptr) { usage = new DynamicObject(); metaDB->setProperty("usage", var(usage)); }
	var uVar = usage->getProperty(id.toString());
	DynamicObject* u = uVar.isObject() ? uVar.getDynamicObject() : nullptr;
	if (u == nullptr) { u = new DynamicObject(); usage->setProperty(id.toString(), var(u)); }
	u->setProperty ("times", m.timesUsed);
	u->setProperty ("lastUsedAt", (int64_t) m.lastUsedAt);
	saveMetaDB();
}

Array<LibraryPreset> PresetStore::getAll() const
{
	Array<LibraryPreset> out; for (auto& id : order) out.add (byId[id]); return out;
}

Optional<LibraryPreset> PresetStore::getById (const Uuid& id) const
{
	if (! byId.contains(id)) return {}; return byId[id];
}

void PresetStore::loadMetaDB()
{
	if (! metaDbPath.existsAsFile()) { metaDB = new DynamicObject(); return; }
	auto s = metaDbPath.loadFileAsString(); auto v = JSON::parse (s);
	if (auto* d = v.getDynamicObject()) metaDB = d; else metaDB = new DynamicObject();
}

void PresetStore::saveMetaDB() const
{
	if (metaDB == nullptr) return;
	metaDbPath.replaceWithText (JSON::toString (var(metaDB.get()), true));
}

juce::var PresetStore::toJson (const LibraryPreset& p)
{
	DynamicObject* d = new DynamicObject();
	d->setProperty("id", p.meta.id.toString());
	d->setProperty("name", p.meta.name);
	d->setProperty("category", p.meta.category);
	d->setProperty("subcategory", p.meta.subcategory);
	d->setProperty("author", p.meta.author);
	d->setProperty("description", p.meta.description);
	d->setProperty("hint", p.meta.hint);
	d->setProperty("tags", var(p.meta.tags));
	d->setProperty("isFactory", p.meta.isFactory);
	d->setProperty("schemaVersion", p.meta.schemaVersion);
	d->setProperty("engineVersion", p.meta.engineVersion);

	DynamicObject* params = new DynamicObject();
	for (int i = 0; i < p.params.size(); ++i)
		params->setProperty (p.params.getName(i).toString(), p.params.getValueAt (i));
	d->setProperty ("params", var(params));
	return var(d);
}

juce::Optional<LibraryPreset> PresetStore::fromJson (const var& v)
{
	auto* d = v.getDynamicObject(); if (d == nullptr) return {};
	LibraryPreset p;
	p.meta.id = juce::Uuid (d->getProperty("id").toString()); if (p.meta.id.isNull()) p.meta.id = Uuid();
	p.meta.name        = d->getProperty("name").toString();
	p.meta.category    = d->getProperty("category").toString();
	p.meta.subcategory = d->getProperty("subcategory").toString();
	p.meta.author      = d->getProperty("author").toString();
	p.meta.description = d->getProperty("description").toString();
	p.meta.hint        = d->getProperty("hint").toString();
	if (auto* arr = d->getProperty("tags").getArray()) for (auto& t : *arr) p.meta.tags.add (t.toString());
	p.meta.isFactory     = (bool) d->getProperty("isFactory");
	p.meta.schemaVersion = (int)  d->getProperty("schemaVersion");
	p.meta.engineVersion = (int)  d->getProperty("engineVersion");

	if (auto* pv = d->getProperty("params").getDynamicObject())
		for (auto& k : pv->getProperties()) p.params.set (k.name.toString(), k.value);
	return p;
}


