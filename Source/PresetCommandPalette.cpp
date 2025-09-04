#include "PresetCommandPalette.h"
using namespace juce;

PresetCommandPalette::PresetCommandPalette (PresetRegistry& r) : reg(r)
{
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    addAndMakeVisible(search);
    search.setTextToShowWhenEmpty("Search (tag:vox is:fav)", Colours::grey);
    search.addListener(this);

    addAndMakeVisible(andMode); andMode.setToggleState(true, dontSendNotification);

    addAndMakeVisible(list);
    list.setModel(this);
    list.setRowHeight(56);
    list.setMultipleSelectionEnabled(false);
    list.setClickingTogglesRowSelection(false);

    for (auto* b : { &apply,&loadA,&loadB,&star,&saveAs }) addAndMakeVisible(b);
    // Label hover to indicate actions
    apply.setTooltip("Apply preset (Enter)");
    loadA.setTooltip("Load to slot A (A)");
    loadB.setTooltip("Load to slot B (B)");
    star.setTooltip ("Toggle favorite (Space)");
    saveAs.setTooltip("Save a new preset");
    for (auto* l : { &desc,&hint,&meta }) { addAndMakeVisible(l); l->setColour(Label::textColourId, Colours::lightgrey); }

    apply.onClick = [this]{ if (auto* p = sel()) if (onApply) onApply(*p); };
    loadA.onClick = [this]{ if (auto* p = sel()) if (onLoadToSlot) onLoadToSlot(*p, true); };
    loadB.onClick = [this]{ if (auto* p = sel()) if (onLoadToSlot) onLoadToSlot(*p, false); };
    star.onClick  = [this]{ if (auto* p = sel()) if (onStar) onStar(*p, !p->isFavorite); refresh(); };
    saveAs.onClick= [this]{ if (onSaveAs) onSaveAs ("New Preset", {}, ""); };

    refresh();
    grabKeyboardFocus();
}

void PresetCommandPalette::show (PresetRegistry& reg, Component& anchor,
                                 std::function<void(const PresetEntry&)> onApply,
                                 std::function<void(const PresetEntry&, bool)> onLoadToSlot,
                                 std::function<void(const PresetEntry&, bool)> onStar,
                                 std::function<void(const juce::String&, const juce::StringArray&, const juce::String&)> onSaveAs,
                                 const juce::String& initialQuery)
{
    auto menu = std::make_unique<PresetCommandPalette>(reg);
    menu->onApply = std::move(onApply);
    menu->onLoadToSlot = std::move(onLoadToSlot);
    menu->onStar = std::move(onStar);
    menu->onSaveAs = std::move(onSaveAs);
    if (initialQuery.isNotEmpty()) menu->search.setText (initialQuery, dontSendNotification);
    menu->setSize (jlimit(560, 880, anchor.getParentMonitorArea().getWidth()-80), 420);
    CallOutBox::launchAsynchronously (std::move(menu), anchor.getScreenBounds(), nullptr);
}

void PresetCommandPalette::resized()
{
    auto r = getLocalBounds().reduced(10);
    auto top = r.removeFromTop(28);
    // Compact top bar: search grows, then AND toggle, then action buttons
    auto topLeft = top;
    search.setBounds (topLeft.removeFromLeft (juce::jmax(260, topLeft.getWidth() - 360)));
    andMode.setBounds (topLeft.removeFromLeft (100));
    apply.setBounds (topLeft.removeFromLeft (64));
    loadA.setBounds (topLeft.removeFromLeft (86));
    loadB.setBounds (topLeft.removeFromLeft (86));
    star.setBounds  (topLeft.removeFromLeft (36));
    saveAs.setBounds(topLeft.removeFromLeft (86));

    // Narrow right info pane
    auto right = r.removeFromRight(220);
    desc.setBounds(right.removeFromTop(58));
    hint.setBounds(right.removeFromTop(40));
    meta.setBounds(right);

    // Give remaining width to list
    list.setBounds(r);
}

void PresetCommandPalette::paint(Graphics& g)
{
    g.fillAll(Colours::black.withAlpha(0.92f));
    g.setColour(Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 8.0f, 2.0f);
}

int PresetCommandPalette::getNumRows() { return results.size(); }

void PresetCommandPalette::paintListBoxItem(int row, Graphics& g, int w, int h, bool sel)
{
    if ((unsigned)row >= results.size()) return;
    auto& p = *results.getReference(row);

    g.setColour(sel ? Colours::white.withAlpha(0.10f) : Colours::white.withAlpha(0.04f));
    g.fillRect(0, 0, w, h-1);

    g.setColour(Colours::white); g.setFont(15.0f);
    g.drawText(p.name, 12, 6, w-24, 18, Justification::left);

    // Category chip (e.g., Delay)
    if (p.category.isNotEmpty())
    {
        auto cat = "  " + p.category + "  ";
        g.setFont(11.0f);
        int cw = g.getCurrentFont().getStringWidth(cat) + 10;
        g.setColour(Colours::cornflowerblue.withAlpha(0.18f));
        g.fillRoundedRectangle((float)(w - cw - 14), 6.0f, (float)cw, 16.0f, 8.0f);
        g.setColour(Colours::lightsteelblue);
        g.drawText(cat, w - cw - 14, 5, cw, 18, Justification::centred);
    }

    int x=12, y=26; g.setFont(12.0f);
    // Always show a leading "Delay" chip to distinguish type
    {
        auto chip = juce::String(" Delay "); int bw = g.getCurrentFont().getStringWidth(chip) + 12;
        g.setColour(Colours::mediumseagreen.withAlpha(0.18f));
        g.fillRoundedRectangle((float)x,(float)y,(float)bw,16.f,8.f);
        g.setColour(Colours::aquamarine.darker(0.2f));
        g.drawText(chip, x, y-1, bw, 16, Justification::centred);
        x += bw + 6;
    }
    for (auto& t : p.tags)
    {
        auto chip = " " + t + " "; int bw = g.getCurrentFont().getStringWidth(chip) + 12;
        g.setColour(Colours::white.withAlpha(0.10f)); g.fillRoundedRectangle((float)x,(float)y,(float)bw,16.f,8.f);
        g.setColour(Colours::lightgrey); g.drawText(chip, x, y-1, bw, 16, Justification::centred);
        x += bw + 6; if (x > w - 80) { x=12; y += 18; }
    }

    g.setColour(p.isFavorite ? Colours::yellow : Colours::darkgrey);
    g.fillEllipse((float)(w-22), 8.f, 12.f, 12.f);
}

void PresetCommandPalette::listBoxItemClicked(int row, const MouseEvent&)
{
    selectedRow = row; list.selectRow(row);
    list.repaintRow(row);
    if (auto* p = sel())
    {
        desc.setText(p->desc, dontSendNotification);
        hint.setText("Hint: " + p->hint, dontSendNotification);
        meta.setText("Author: " + p->author + "   Tags: " + p->tags.joinIntoString(", "), dontSendNotification);
    }
}

void PresetCommandPalette::listBoxItemDoubleClicked(int row, const MouseEvent&)
{
    if ((unsigned)row >= results.size()) return;
    if (onApply) onApply (*results.getReference(row));
}

const PresetEntry* PresetCommandPalette::sel() const
{
    if (selectedRow < 0 || selectedRow >= results.size()) return nullptr;
    return results[selectedRow];
}

PresetCommandPalette::Query PresetCommandPalette::parseQuery(String s) const
{
    Query q; q.andMode = andMode.getToggleState();
    auto toks = StringArray::fromTokens (s, " ", "\"");
    for (auto t : toks)
    {
        if (t.startsWithIgnoreCase("tag:")) q.tags.add (t.fromFirstOccurrenceOf(":", false, false));
        else if (t == "is:fav") q.favOnly = true;
        else q.text << t << " ";
    }
    q.text = q.text.trim(); return q;
}

bool PresetCommandPalette::matches(const PresetEntry& p, const Query& q) const
{
    if (q.favOnly && !p.isFavorite) return false;
    if (q.text.isNotEmpty())
    {
        auto hay = (p.name + " " + p.desc + " " + p.hint + " " + p.tags.joinIntoString(" ")).toLowerCase();
        if (!hay.contains (q.text.toLowerCase())) return false;
    }
    if (!q.tags.isEmpty())
    {
        int hits = 0; for (auto& t : q.tags) if (p.tags.contains (t, true)) ++hits;
        if (q.andMode ? (hits != q.tags.size()) : (hits == 0)) return false;
    }
    return true;
}

void PresetCommandPalette::refresh()
{
    results.clearQuick();
    auto q = parseQuery (search.getText().trim());
    auto& all = reg.all();
    DBG("[PresetCommandPalette] registry size=" << all.size());
    for (auto& p : all) if (matches(p, q)) results.add (&p);
    DBG("[PresetCommandPalette] results size=" << results.size());
    list.updateContent();
    if (!results.isEmpty())
    {
        selectedRow = 0; list.selectRow(0);
        desc.setText(results[0]->desc, dontSendNotification);
        hint.setText("Hint: " + results[0]->hint, dontSendNotification);
        meta.setText("Author: " + results[0]->author + "   Tags: " + results[0]->tags.joinIntoString(", "), dontSendNotification);
    }
    repaint();
}

bool PresetCommandPalette::keyPressed(const KeyPress& k, Component*)
{
    if (k == KeyPress::escapeKey) { getTopLevelComponent()->exitModalState(0); return true; }
    if (k == KeyPress::returnKey) { if (auto* p = sel()) if (onApply) onApply(*p); return true; }
    if (k == KeyPress::upKey)
    {
        if (selectedRow > 0) { list.selectRow(--selectedRow); if (auto* p = sel()) { desc.setText(p->desc, dontSendNotification); hint.setText("Hint: " + p->hint, dontSendNotification); meta.setText("Author: " + p->author + "   Tags: " + p->tags.joinIntoString(", "), dontSendNotification); } }
        return true;
    }
    if (k == KeyPress::downKey)
    {
        if (selectedRow < results.size()-1) { list.selectRow(++selectedRow); if (auto* p = sel()) { desc.setText(p->desc, dontSendNotification); hint.setText("Hint: " + p->hint, dontSendNotification); meta.setText("Author: " + p->author + "   Tags: " + p->tags.joinIntoString(", "), dontSendNotification); } }
        return true;
    }
    if (k.getTextCharacter()=='a'||k.getTextCharacter()=='A') { if (auto* p = sel()) if (onLoadToSlot) onLoadToSlot(*p, true); return true; }
    if (k.getTextCharacter()=='b'||k.getTextCharacter()=='B') { if (auto* p = sel()) if (onLoadToSlot) onLoadToSlot(*p, false); return true; }
    if (k.getTextCharacter()==' ')                            { if (auto* p = sel()) if (onStar) onStar(*p, !p->isFavorite); refresh(); return true; }
    return false;
}


