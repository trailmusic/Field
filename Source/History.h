#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "IconSystem.h"

namespace field::history {

static inline juce::String nowHHMMSS()
{
    auto t = juce::Time::getCurrentTime();
    return t.formatted("%H:%M:%S");
}

struct HistoryEntry { juce::String label; juce::String time; int index = 0; };
struct Checkpoint   { juce::String name;  juce::String time; juce::ValueTree state; };

class HistoryManager : private juce::ValueTree::Listener, private juce::Timer
{
public:
    HistoryManager (juce::AudioProcessorValueTreeState& s, juce::UndoManager& u,
                    int maxSteps = 100, bool includeAutomationInHistory = false)
        : apvts (s), undo (u), maxHistory (maxSteps), includeAutomation (includeAutomationInHistory)
    {
        apvts.state.addListener (this);
        checkpointsRoot = apvts.state.getOrCreateChildWithName (IDs::checkpoints, nullptr);
        refreshCheckpoints();
    }

    ~HistoryManager() override { apvts.state.removeListener (this); }

    void begin (juce::String label) { undo.beginNewTransaction (label); pendingLabel = std::move(label); started = true; }
    void rename (juce::String label) { undo.setCurrentTransactionName (label); pendingLabel = std::move(label); }
    void end() { if (! started) return; addEntry (pendingLabel.isNotEmpty() ? pendingLabel : juce::String ("Edit")); pendingLabel.clear(); started = false; }

    void bindCustomGestureStart (const juce::String& label) { begin (label); }
    void bindCustomGestureEnd   (const juce::String& label) { rename (label); end(); }

    void bindToSlider (juce::Slider& slider, juce::String label)
    {
        // Chain existing handlers if set
        auto prevStart = slider.onDragStart;
        auto prevEnd   = slider.onDragEnd;
        auto name = std::make_shared<juce::String> (std::move (label));
        auto oldTxt = std::make_shared<juce::String>();

        // Only notify listeners/attachments on release to avoid mid-drag spam
        slider.setChangeNotificationOnlyOnRelease (true);

        slider.onDragStart = [this, &slider, prevStart, name, oldTxt]
        {
            if (prevStart) prevStart();
            *oldTxt = juce::String (slider.getValue(), 2);
            begin (*name);
        };

        slider.onDragEnd = [this, &slider, prevEnd, name, oldTxt]
        {
            if (prevEnd) prevEnd();
            rename (*name + juce::String (" ") + *oldTxt + juce::String (" → ") + juce::String (slider.getValue(), 2));
            end();
        };
    }

    void bindToButton (juce::Button& button, juce::String label)
    {
        button.onClick = [this, &button, label = std::move(label)]() mutable
        {
            juce::ignoreUnused (button);
            begin (label);
            rename (label + " ✅");
            end();
        };
    }

    bool canUndo() const { return undo.canUndo(); }
    bool canRedo() const { return undo.canRedo(); }
    bool undoStep()      { auto ok = undo.undo(); refreshFromUndo(); return ok; }
    bool redoStep()      { auto ok = undo.redo(); refreshFromUndo(); return ok; }

    const juce::Array<HistoryEntry>& getEntries() const { return entries; }
    int  getCursor() const { return cursor; }

    void clear() { undo.clearUndoHistory(); entries.clear(); cursor = -1; if (onChanged) onChanged(); }

    int saveCheckpoint (juce::String name)
    {
        auto snap = apvts.copyState();
        juce::ValueTree cp (IDs::checkpoint);
        cp.setProperty (IDs::name, name, nullptr);
        cp.setProperty (IDs::time, nowHHMMSS(), nullptr);
        cp.addChild (snap.createCopy(), -1, nullptr);
        checkpointsRoot.addChild (cp, -1, nullptr);
        refreshCheckpoints();
        return checkpoints.size() - 1;
    }

    bool recallCheckpoint (int index)
    {
        if (! juce::isPositiveAndBelow (index, checkpoints.size())) return false;
        begin ("Recall Checkpoint: " + checkpoints.getReference(index).name);
        auto cp = checkpointsRoot.getChild (index);
        auto state = cp.getChild (0);
        apvts.replaceState (state.createCopy());
        rename ("Recall Checkpoint: " + checkpoints.getReference(index).name + " @" + checkpoints.getReference(index).time);
        end();
        return true;
    }

    void deleteCheckpoint (int index)
    {
        if (! juce::isPositiveAndBelow (index, checkpoints.size())) return;
        checkpointsRoot.removeChild (index, nullptr);
        refreshCheckpoints();
    }

    const juce::Array<Checkpoint>& getCheckpoints() const { return checkpoints; }

    bool exportCheckpoints (const juce::File& file)
    {
        juce::var v = checkpointsRoot.toXmlString();
        return file.replaceWithText (v.toString());
    }

    bool importCheckpoints (const juce::File& file, bool merge = true)
    {
        auto text = file.loadFileAsString(); if (text.isEmpty()) return false;
        auto xml = juce::parseXML (text); if (! xml) return false;
        auto vt = juce::ValueTree::fromXml (*xml); if (! vt.isValid()) return false;
        if (! merge) checkpointsRoot.removeAllChildren (nullptr);
        for (int i=0; i<vt.getNumChildren(); ++i) checkpointsRoot.addChild (vt.getChild(i).createCopy(), -1, nullptr);
        refreshCheckpoints();
        return true;
    }

    void setMaxHistory (int n) { maxHistory = juce::jmax (1, n); trimToCapacity(); }
    void setIncludeAutomationInHistory (bool b) { includeAutomation = b; }

    std::function<void()> onChanged;

private:
    struct IDs { static constexpr const char* checkpoints = "history.checkpoints"; static constexpr const char* checkpoint = "checkpoint"; static constexpr const char* name = "name"; static constexpr const char* time = "time"; };

    juce::AudioProcessorValueTreeState& apvts;
    juce::UndoManager& undo;
    int maxHistory = 100;
    bool includeAutomation = false;

    juce::Array<HistoryEntry> entries; int cursor = -1;
    juce::Array<Checkpoint>   checkpoints;
    juce::ValueTree checkpointsRoot { IDs::checkpoints };

    juce::String pendingLabel; bool started = false;

    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& pid) override
    {
        // Only respond to parameter value changes
        if (pid != juce::Identifier ("value")) return;
        // Coalesce UI drags: only record non-gesture changes here; gestures add entry on end()
        if (started) return;
        // Only log known parameters; coalesce with short debounce per param
        const juce::String idStr = tree.getType().toString();
        auto* p = apvts.getParameter (idStr);
        if (p == nullptr) return;
        const juce::String name = p->getName (64);
        const auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p);
        const juce::String newText = rp ? rp->getCurrentValueAsText() : p->getName (64);

        // Track first (old) and most recent (new) within debounce window
        if (! pendingOldTextById.contains (idStr))
            pendingOldTextById.set (idStr, lastKnownTextById.contains (idStr) ? lastKnownTextById[idStr] : newText);
        pendingNewTextById.set (idStr, newText);

        const juce::uint32 nowMs = juce::Time::getMillisecondCounter();
        if (! pendingLabelById.contains (idStr)) pendingIds.add (idStr);
        pendingLabelById.set (idStr, name);
        pendingTimeById.set (idStr, nowMs);
        if (! coalescing) { coalescing = true; startTimer (50); }
    }

    void timerCallback() override
    {
        const juce::uint32 now = juce::Time::getMillisecondCounter();
        juce::Array<int> toRemove;
        for (int i = 0; i < pendingIds.size(); ++i)
        {
            const juce::String id = pendingIds.getReference (i);
            const juce::uint32 last = pendingTimeById.contains (id) ? pendingTimeById[id] : 0;
            if (last > 0 && now - last >= 150)
            {
                const juce::String base = pendingLabelById.contains (id) ? pendingLabelById[id] : juce::String();
                const juce::String oldT = pendingOldTextById.contains (id) ? pendingOldTextById[id] : juce::String();
                const juce::String newT = pendingNewTextById.contains (id) ? pendingNewTextById[id] : juce::String();
                const juce::String lab = base.isNotEmpty() ? (base + " " + oldT + " \u2192 " + newT) : juce::String();
                if (lab.isNotEmpty()) addEntry (lab);
                pendingTimeById.remove (id);
                pendingLabelById.remove (id);
                pendingOldTextById.remove (id);
                pendingNewTextById.remove (id);
                if (base.isNotEmpty()) lastKnownTextById.set (id, newT);
                toRemove.add (i);
            }
        }
        for (int j = toRemove.size() - 1; j >= 0; --j) pendingIds.remove (toRemove[j]);
        if (pendingIds.isEmpty()) { stopTimer(); coalescing = false; }
    }

    void addEntry (juce::String label)
    {
        HistoryEntry e { std::move(label), nowHHMMSS(), entries.size() };
        if (cursor < entries.size() - 1) entries.removeRange (cursor + 1, entries.size() - (cursor + 1));
        entries.add (e); cursor = entries.size() - 1; trimToCapacity(); if (onChanged) onChanged();
    }
    void trimToCapacity()
    {
        while (entries.size() > maxHistory) { entries.remove (0); --cursor; }
        if (cursor < -1) cursor = -1;
    }
    void refreshFromUndo() { cursor = juce::jlimit (-1, entries.size() - 1, cursor + (undo.canRedo() ? -1 : 0)); }
    void refreshCheckpoints()
    {
        checkpoints.clear();
        for (int i=0; i<checkpointsRoot.getNumChildren(); ++i)
        {
            auto cp = checkpointsRoot.getChild (i);
            checkpoints.add ({ cp[IDs::name].toString(), cp[IDs::time].toString(), cp.getChild (0) });
        }
    }

    juce::OwnedArray<juce::MouseListener, juce::CriticalSection> mouseListeners;
    juce::uint32 lastAddMs { 0 }; juce::String lastLabel;
    // Coalescing state for non-gesture APVTS updates (per param)
    bool coalescing { false };
    juce::Array<juce::String> pendingIds;
    juce::HashMap<juce::String, juce::String> pendingLabelById;
    juce::HashMap<juce::String, juce::uint32>       pendingTimeById;
    juce::HashMap<juce::String, juce::String> pendingOldTextById;
    juce::HashMap<juce::String, juce::String> pendingNewTextById;
    juce::HashMap<juce::String, juce::String> lastKnownTextById;
};

class HistoryPanel : public juce::Component, private juce::Button::Listener, private juce::ListBoxModel
{
public:
    HistoryPanel()
    {
        addButtons();
        // Defer model assignment until after members are constructed to avoid any
        // potential re-entrancy or undefined behaviour during construction.
        list.setName ("history");
        pins.setName ("checkpoints");
        list.setModel (this);
        pins.setModel (&pinsModel);
        addAndMakeVisible (list);
        addAndMakeVisible (pins);
        setOpaque (false);
        addAndMakeVisible (topBar);
        closeBtn.onClick = [this]{ if (onClose) onClose(); };
    }
    void setModel (HistoryManager& m) { model = &m; if (model) model->onChanged = [this]{ refresh(); }; refresh(); }
    void refresh()
    {
        if (! model) return; list.updateContent(); list.repaint(); pinsModel.sync (*model); pins.updateContent(); pins.repaint(); undoBtn.setEnabled (model->canUndo()); redoBtn.setEnabled (model->canRedo());
    }
    void resized() override
    {
        auto r = getLocalBounds();
        // Background matching FieldLNF gradient/border style
        backgroundBounds = r.toFloat().reduced (6.0f);
        auto inner = r.reduced (16);
        // Header bar container
        auto headerArea = inner.removeFromTop (40);
        topBar.setBounds (headerArea);
        // Close button inside header
        if (closeBtn.getParentComponent() != this)
        {
            addAndMakeVisible (closeBtn);
            closeBtn.setTooltip ("Close");
            closeBtn.setAlpha (0.9f);
        }
        auto closeArea = headerArea.removeFromRight (40).reduced (4);
        closeBtn.setBounds (closeArea);
        auto top = inner.removeFromTop (40);
        undoBtn.setBounds (top.removeFromLeft (120));
        redoBtn.setBounds (top.removeFromLeft (120));
        savePin.setBounds (top.removeFromLeft (200));
        importPins.setBounds (top.removeFromRight (120));
        exportPins.setBounds (top.removeFromRight (120));
        auto cols = inner; auto left = cols.removeFromLeft (cols.getWidth() / 2).reduced (8);
        histLabel.setBounds (left.removeFromTop (20)); list.setBounds (left);
        auto right = cols.reduced (8); pinsLabel.setBounds (right.removeFromTop (20)); pins.setBounds (right);
    }
    void paint (juce::Graphics& g) override
    {
        // Try to derive colours from parent LNF if available
        juce::Colour panel = juce::Colour (0xFF3A3D45);
        juce::Colour hl    = juce::Colour (0xFF4A4D55);
        juce::ignoreUnused (getLookAndFeel());
        auto r = backgroundBounds;
        juce::Colour top = panel.brighter (0.10f), bot = panel.darker (0.10f);
        juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (hl.withAlpha (0.65f));
        g.drawRoundedRectangle (r, 8.0f, 1.2f);
        // soft outer elevation
        g.setColour (juce::Colours::black.withAlpha (0.30f));
        g.drawRoundedRectangle (r.expanded (1.5f), 9.5f, 1.2f);

        // Draw header background inside topBar area to give a clear container
        auto hb = topBar.getBounds().toFloat();
        if (! hb.isEmpty())
        {
            auto ht = panel.brighter (0.18f);
            auto hbCol = panel.brighter (0.06f);
            g.setGradientFill (juce::ColourGradient (ht, hb.getX(), hb.getY(), hbCol, hb.getX(), hb.getBottom(), false));
            g.fillRoundedRectangle (hb, 6.0f);
            g.setColour (hl.withAlpha (0.55f));
            g.drawRoundedRectangle (hb, 6.0f, 1.0f);
        }
    }
private:
    // Button with IconSystem icon + text label, matching our panel style
    class IconTextButton : public juce::Button
    {
    public:
        IconTextButton (IconSystem::IconType it, juce::String text, bool iconRight = false)
        : juce::Button (text), icon (it), placeIconRight (iconRight) {}
        void paintButton (juce::Graphics& g, bool over, bool down) override
        {
            auto r = getLocalBounds().toFloat().reduced (2.0f);
            juce::Colour panel = juce::Colour (0xFF3A3D45);
            juce::Colour sh    = juce::Colour (0xFF2A2C30);
            juce::Colour hl    = juce::Colour (0xFF4A4D55);
            juce::Colour top = panel.brighter (0.10f), bot = panel.darker (0.10f);
            g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false));
            g.fillRoundedRectangle (r, 6.0f);
            g.setColour (down ? sh : (over ? hl : sh));
            g.drawRoundedRectangle (r, 6.0f, 1.0f);
            // icon + text
            auto left = r.reduced (8.0f);
            auto iconArea = left.removeFromLeft (left.getHeight());
            auto textArea = left;
            if (placeIconRight)
            {
                // swap areas to draw text first and icon at right
                auto textFirst = r.reduced (8.0f);
                auto textOnly = textFirst.removeFromLeft (textFirst.getWidth() - textFirst.getHeight());
                iconArea = textFirst; textArea = textOnly;
            }
            auto iconCol = juce::Colours::white.withAlpha (0.92f);
            IconSystem::drawIcon (g, icon, iconArea, iconCol);
            g.setColour (juce::Colours::white.withAlpha (0.95f));
            g.setFont (juce::Font (juce::FontOptions (14.0f).withStyle ("Bold")));
            g.drawFittedText (getButtonText(), textArea.toNearestInt(), juce::Justification::centredLeft, 1);
        }
        void clicked() override { if (onClick) onClick(); }
        std::function<void()> onClick;
    private:
        IconSystem::IconType icon; bool placeIconRight { false };
    };
    HistoryManager* model = nullptr;
    juce::Component topBar;
    IconTextButton undoBtn { IconSystem::LeftArrow, "Undo", false }, redoBtn { IconSystem::RightArrow, "Redo", true };
    // Close button (red X) in top-right
    class CloseXButton : public juce::Button
    {
    public:
        CloseXButton() : juce::Button ("close") {}
        void paintButton (juce::Graphics& g, bool over, bool down) override
        {
            auto r = getLocalBounds().toFloat();
            auto col = juce::Colour (0xFFE53935); // red
            if (down) col = col.darker (0.2f); else if (over) col = col.brighter (0.1f);
            IconSystem::drawIcon (g, IconSystem::X, r.reduced (2.0f), col);
        }
        void clicked() override { if (onClick) onClick(); }
        std::function<void()> onClick;
    } closeBtn;
public:
    std::function<void()> onClose;
    juce::TextButton savePin { "Save Checkpoint" }, exportPins { "Export" }, importPins { "Import" };
    juce::Label histLabel { {}, "History" }, pinsLabel { {}, "Checkpoints" };
    juce::ListBox list; 
    juce::ListBox pins;

    int getNumRows() override { return model ? model->getEntries().size() : 0; }
    void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected) override
    {
        if (! model) return; g.fillAll (selected ? juce::Colours::white.withAlpha (0.07f) : juce::Colours::transparentBlack);
        const int total = model->getEntries().size();
        if (total <= 0) return;
        if (! juce::isPositiveAndBelow (row, total)) return;
        const int idx = total - 1 - row; // newest at top
        const auto& e = model->getEntries().getReference (idx);
        g.setColour (juce::Colours::white.withAlpha (0.85f)); g.drawText (e.time + "  •  " + e.label, 8, 0, w - 16, h, juce::Justification::centredLeft);
    }
    void listBoxItemClicked (int row, const juce::MouseEvent&) override
    {
        if (! model) return;
        const int total = model->getEntries().size();
        if (! juce::isPositiveAndBelow (row, total)) return;
        // Map clicked row to internal index
        const int idx = total - 1 - row;
        const int cur = model->getCursor();
        if (idx < cur) model->undoStep();
        else if (idx > cur) model->redoStep();
        refresh();
    }

    struct PinsModel : juce::ListBoxModel {
        juce::Array<Checkpoint> data; void sync (HistoryManager& hm) { data = hm.getCheckpoints(); }
        int getNumRows() override { return data.size(); }
        void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool sel) override
        {
            g.fillAll (sel ? juce::Colours::white.withAlpha (0.07f) : juce::Colours::transparentBlack);
            const int total = data.size();
            if (! juce::isPositiveAndBelow (row, total)) return; 
            const int idx = total - 1 - row; // newest at top
            auto t = data.getReference (idx);
            g.setColour (juce::Colours::white.withAlpha (0.85f)); g.drawText (t.time + "  •  " + t.name, 8, 0, w-16, h, juce::Justification::centredLeft);
        }
    } pinsModel;

    void addButtons()
    {
        addAndMakeVisible (undoBtn); undoBtn.addListener (this); undoBtn.setWantsKeyboardFocus (false); undoBtn.setButtonText ("Undo");
        addAndMakeVisible (redoBtn); redoBtn.addListener (this); redoBtn.setWantsKeyboardFocus (false); redoBtn.setButtonText ("Redo");
        addAndMakeVisible (savePin); savePin.addListener (this); savePin.setWantsKeyboardFocus (false); savePin.setButtonText ("Save Checkpoint");
        addAndMakeVisible (exportPins); exportPins.addListener (this); exportPins.setWantsKeyboardFocus (false); exportPins.setButtonText ("Export");
        addAndMakeVisible (importPins); importPins.addListener (this); importPins.setWantsKeyboardFocus (false); importPins.setButtonText ("Import");
        addAndMakeVisible (histLabel); histLabel.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (pinsLabel); pinsLabel.setJustificationType (juce::Justification::centredLeft);
    }
    void buttonClicked (juce::Button* b) override
    {
        if (! model) return;
        if (b == &undoBtn)      model->undoStep();
        else if (b == &redoBtn) model->redoStep();
        else if (b == &savePin)
        {
            auto aw = std::make_unique<juce::AlertWindow> ("Save Checkpoint", "Name:", juce::AlertWindow::NoIcon);
            aw->addTextEditor ("n", "Checkpoint"); aw->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey)); aw->addButton ("Cancel", 0);
            juce::Component::SafePointer<HistoryPanel> self (this);
            aw->enterModalState (true, juce::ModalCallbackFunction::create ([self, awPtr = aw.get()](int r)
            {
                if (self == nullptr) return;
                if (r == 1 && self->model) { self->model->saveCheckpoint (awPtr->getTextEditorContents ("n")); self->refresh(); }
            }), true);
            ownedAlerts.add (std::move (aw));
        }
        else if (b == &exportPins)
        {
            activeChooser = std::make_unique<juce::FileChooser> ("Export Checkpoints", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.xml");
            juce::Component::SafePointer<HistoryPanel> self (this);
            activeChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                                  [self] (const juce::FileChooser& fc)
                                  {
                                      if (self == nullptr) return;
                                      auto f = fc.getResult(); if (f.getFullPathName().isNotEmpty() && self->model) { self->model->exportCheckpoints (f); self->refresh(); }
                                      self->activeChooser.reset();
                                  });
        }
        else if (b == &importPins)
        {
            activeChooser = std::make_unique<juce::FileChooser> ("Import Checkpoints", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.xml");
            juce::Component::SafePointer<HistoryPanel> self (this);
            activeChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                  [self] (const juce::FileChooser& fc)
                                  {
                                      if (self == nullptr) return;
                                      auto f = fc.getResult(); if (f.existsAsFile() && self->model) { self->model->importCheckpoints (f, true); self->refresh(); }
                                      self->activeChooser.reset();
                                  });
        }
        else if (b == &closeBtn)
        {
            if (onClose) onClose();
        }
        refresh();
    }
    juce::Rectangle<float> backgroundBounds;
    juce::OwnedArray<juce::AlertWindow> ownedAlerts;
    std::unique_ptr<juce::FileChooser> activeChooser;
};

} // namespace field::history


